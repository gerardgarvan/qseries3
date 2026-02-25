#ifndef REPL_H
#define REPL_H

#include "parser.h"
#include "series.h"
#include "frac.h"
#include "convert.h"
#include <map>
#include <vector>
#include <string>
#include <variant>
#include <optional>
#include <stdexcept>
#include <cstdint>
#include <iostream>
#include <deque>
#include <algorithm>
#include <sstream>

// EnvValue: variable can hold Series or Jacobi product
using EnvValue = std::variant<Series, std::vector<JacFactor>>;

struct Environment {
    std::map<std::string, EnvValue> env;
    int T = 50;

    Environment() {
        env["q"] = Series::q(T);
    }
};

// EvalResult: union of all possible evaluation outcomes
struct DisplayOnly {};  // tag for series/coeffs display-only built-ins

using EvalResult = std::variant<
    Series,
    std::map<int, Frac>,                              // prodmake
    std::vector<std::pair<int, Frac>>,                // etamake
    std::vector<JacFactor>,                           // jacprodmake
    std::vector<std::vector<Frac>>,                   // findhom/findnonhom
    std::optional<std::vector<Frac>>,                 // findhomcombo/findnonhomcombo
    QFactorResult,                                    // qfactor
    int64_t,                                          // legendre/sigma
    DisplayOnly,
    std::monostate                                    // set_trunc
>;

// Helper: get Series from EnvValue (for variable lookup in arithmetic)
inline Series getSeriesFromEnv(const EnvValue& v) {
    if (std::holds_alternative<Series>(v))
        return std::get<Series>(v);
    throw std::runtime_error("variable holds Jacobi product, not series");
}

// evalToInt: evaluate expression that must be integer-valued (sum bounds, exponents, etc.)
inline int64_t evalToInt(const Expr* e, Environment& env,
    const std::map<std::string, int64_t>& sumIndices) {
    if (!e) throw std::runtime_error("evalToInt: null expr");
    switch (e->tag) {
        case Expr::Tag::IntLit:
            return e->intVal;
        case Expr::Tag::Var: {
            auto it = sumIndices.find(e->varName);
            if (it != sumIndices.end())
                return it->second;
            throw std::runtime_error("expected integer (variable '" + e->varName + "' not in sum scope)");
        }
        case Expr::Tag::BinOp: {
            int64_t l = evalToInt(e->left.get(), env, sumIndices);
            int64_t r = evalToInt(e->right.get(), env, sumIndices);
            switch (e->binOp) {
                case BinOp::Add: return l + r;
                case BinOp::Sub: return l - r;
                case BinOp::Mul: return l * r;
                case BinOp::Div: return l / r;
                case BinOp::Pow: {
                    if (r < 0) throw std::runtime_error("negative exponent in integer power");
                    int64_t acc = 1;
                    for (int64_t i = 0; i < r; ++i) acc *= l;
                    return acc;
                }
            }
        }
        case Expr::Tag::UnOp:
            return -evalToInt(e->operand.get(), env, sumIndices);
        default:
            throw std::runtime_error("expected integer expression");
    }
}

// isQLike: true if expr evaluates to q (for q^exp special case)
inline bool isQLike(const Expr* e) {
    return e && e->tag == Expr::Tag::Q;
}

inline EvalResult eval(const Expr* e, Environment& env,
    const std::map<std::string, int64_t>& sumIndices);

inline EvalResult evalExpr(const Expr* e, Environment& env,
    const std::map<std::string, int64_t>& sumIndices) {
    if (!e) throw std::runtime_error("eval: null expr");
    switch (e->tag) {
        case Expr::Tag::IntLit:
            return Series::constant(Frac(e->intVal), env.T);
        case Expr::Tag::Q: {
            auto it = env.env.find("q");
            if (it != env.env.end())
                return getSeriesFromEnv(it->second);
            return Series::q(env.T);
        }
        case Expr::Tag::Var: {
            auto it = sumIndices.find(e->varName);
            if (it != sumIndices.end())
                return Series::constant(Frac(it->second), env.T);
            auto ev = env.env.find(e->varName);
            if (ev == env.env.end())
                throw std::runtime_error("undefined variable: " + e->varName);
            return getSeriesFromEnv(ev->second);
        }
        case Expr::Tag::BinOp: {
            if (e->binOp == BinOp::Pow && isQLike(e->left.get())) {
                int64_t expVal = evalToInt(e->right.get(), env, sumIndices);
                return Series::qpow(static_cast<int>(expVal), env.T);
            }
            Series l = std::get<Series>(eval(e->left.get(), env, sumIndices));
            Series r = std::get<Series>(eval(e->right.get(), env, sumIndices));
            switch (e->binOp) {
                case BinOp::Add: return l + r;
                case BinOp::Sub: return l - r;
                case BinOp::Mul: return l * r;
                case BinOp::Div: return l / r;
                case BinOp::Pow: {
                    int64_t expVal = evalToInt(e->right.get(), env, sumIndices);
                    return l.pow(static_cast<int>(expVal));
                }
            }
        }
        case Expr::Tag::UnOp: {
            Series s = std::get<Series>(eval(e->operand.get(), env, sumIndices));
            return -s;
        }
        case Expr::Tag::Call:
            // Stub: unknown built-in → DisplayOnly (built-ins in 10-02)
            return DisplayOnly{};
        case Expr::Tag::List:
            throw std::runtime_error("list evaluation not yet implemented");
        case Expr::Tag::Sum: {
            int64_t lo = evalToInt(e->lo.get(), env, sumIndices);
            int64_t hi = evalToInt(e->hi.get(), env, sumIndices);
            Series acc = Series::zero(env.T);
            auto idx = sumIndices;
            for (int64_t n = lo; n <= hi; ++n) {
                idx[e->sumVar] = n;
                Series term = std::get<Series>(eval(e->body.get(), env, idx));
                acc = (acc + term).truncTo(env.T);
            }
            return acc;
        }
    }
    throw std::runtime_error("eval: unhandled expression");
}

inline EvalResult eval(const Expr* e, Environment& env,
    const std::map<std::string, int64_t>& sumIndices) {
    return evalExpr(e, env, sumIndices);
}

// --- Display helpers (Garvan style) ---

inline std::string formatProdmake(const std::map<int, Frac>& a) {
    std::vector<std::string> num_parts, den_parts;
    for (const auto& [n, an] : a) {
        if (an.isZero()) continue;
        std::string part = "(1-q" + (n == 1 ? "" : Series::expToUnicode(n)) + ")";
        int ex = 0;
        if (an.den == BigInt(1) && an.num.d.size() == 1 && an.num.d[0] <= 1000)
            ex = an.num.neg ? -static_cast<int>(an.num.d[0]) : static_cast<int>(an.num.d[0]);
        if (ex == 0) continue;
        ex = (ex < 0) ? -ex : ex;  // use |ex| for exponent in (1-q^n)^{|ex|}
        if (ex > 1) part += Series::expToUnicode(ex);
        if (an > Frac(0))
            den_parts.push_back(part);
        else
            num_parts.push_back(part);
    }
    std::string num_str, den_str;
    for (const auto& s : num_parts) num_str += (num_str.empty() ? "" : " ") + s;
    for (const auto& s : den_parts) den_str += (den_str.empty() ? "" : " ") + s;
    if (num_str.empty() && den_str.empty()) return "1";
    if (den_str.empty()) return num_str;
    if (num_str.empty()) return "1 / ((" + den_str + "))";
    return "(" + num_str + ") / ((" + den_str + "))";
}

inline std::string formatEtamake(const std::vector<std::pair<int, Frac>>& eta) {
    std::vector<std::string> num_parts, den_parts;
    for (const auto& [k, e] : eta) {
        if (e.isZero()) continue;
        std::string part = "\xCE\xB7(" + (k == 1 ? "\xCF\x84" : std::to_string(k) + "\xCF\x84") + ")";
        if (k != 1) part = "\xCE\xB7(" + std::to_string(k) + "\xCF\x84)";
        int ex = 0;
        if (e.den == BigInt(1) && e.num.d.size() == 1 && e.num.d[0] <= 100)
            ex = e.num.neg ? -static_cast<int>(e.num.d[0]) : static_cast<int>(e.num.d[0]);
        if (ex == 0) continue;
        if (ex != 1) part += Series::expToUnicode(ex);
        if (e > Frac(0))
            num_parts.push_back(part);
        else
            den_parts.push_back(part);
    }
    std::string num_str, den_str;
    for (const auto& s : num_parts) num_str += (num_str.empty() ? "" : " ") + s;
    for (const auto& s : den_parts) den_str += (den_str.empty() ? "" : " ") + s;
    if (num_str.empty() && den_str.empty()) return "1";
    if (den_str.empty()) return num_str;
    if (num_str.empty()) return "1 / (" + den_str + ")";
    return num_str + " / (" + den_str + ")";
}

inline std::string formatQfactor(const QFactorResult& qf) {
    std::string out;
    if (qf.q_power != 0)
        out += "q" + Series::expToUnicode(qf.q_power);
    std::vector<std::string> num_parts, den_parts;
    for (const auto& [n, an] : qf.num_exponents) {
        if (an.isZero()) continue;
        std::string part = "(1-q" + Series::expToUnicode(n) + ")";
        int ex = 0;
        if (an.den == BigInt(1) && an.num.d.size() == 1 && an.num.d[0] <= 100)
            ex = an.num.neg ? -static_cast<int>(an.num.d[0]) : static_cast<int>(an.num.d[0]);
        if (ex > 1) part += Series::expToUnicode(ex);
        if (ex > 0) num_parts.push_back(part);
    }
    for (const auto& [n, an] : qf.den_exponents) {
        if (an.isZero()) continue;
        std::string part = "(1-q" + Series::expToUnicode(n) + ")";
        int ex = 0;
        if (an.den == BigInt(1) && an.num.d.size() == 1 && an.num.d[0] <= 100)
            ex = an.num.neg ? -static_cast<int>(an.num.d[0]) : static_cast<int>(an.num.d[0]);
        if (ex > 1) part += Series::expToUnicode(ex);
        if (ex > 0) den_parts.push_back(part);
    }
    std::string num_str, den_str;
    for (const auto& s : num_parts) num_str += s;
    for (const auto& s : den_parts) den_str += s;
    if (!num_str.empty() && !out.empty()) out += "\xC2\xB7";  // middle dot
    if (!num_str.empty()) out += num_str;
    if (!den_str.empty()) out += " / ((" + den_str + "))";
    return out.empty() ? "1" : out;
}

inline std::string formatRelation(const std::vector<Frac>& coeffs,
    const std::vector<std::vector<int>>& monomialExponents,
    std::vector<std::string> varNames = {}) {
    if (coeffs.empty()) return "0";
    if (monomialExponents.empty() || coeffs.size() != monomialExponents.size()) {
        std::string fallback;
        for (size_t i = 0; i < coeffs.size(); ++i) {
            if (i) fallback += " ";
            fallback += coeffs[i].str();
        }
        return fallback;
    }
    if (varNames.empty()) {
        int k = 0;
        for (const auto& e : monomialExponents) {
            if (static_cast<int>(e.size()) > k) k = static_cast<int>(e.size());
        }
        static const char* subs[] = {"\xe2\x82\x81","\xe2\x82\x82","\xe2\x82\x83","\xe2\x82\x84","\xe2\x82\x85"};
        for (int i = 0; i < k; ++i)
            varNames.push_back(std::string("X") + (i < 5 ? subs[i] : std::to_string(i+1)));
    }
    std::string out;
    bool first = true;
    for (size_t i = 0; i < coeffs.size(); ++i) {
        if (coeffs[i].isZero()) continue;
        if (!first && coeffs[i] > Frac(0)) out += "+";
        first = false;
        if (coeffs[i] == Frac(-1))
            out += "-";
        else if (!coeffs[i].isOne() && coeffs[i] != Frac(-1))
            out += coeffs[i].str();
        for (size_t j = 0; j < monomialExponents[i].size() && j < varNames.size(); ++j) {
            int exp = monomialExponents[i][j];
            if (exp == 0) continue;
            out += varNames[j];
            if (exp != 1) out += Series::expToUnicode(exp);
        }
    }
    return out.empty() ? "0" : out;
}

inline std::string trim(const std::string& s) {
    size_t i = 0, j = s.size();
    while (i < j && (s[i] == ' ' || s[i] == '\t')) ++i;
    while (j > i && (s[j-1] == ' ' || s[j-1] == '\t')) --j;
    return s.substr(i, j - i);
}

inline void display(const EvalResult& res, Environment& env, int /*T*/) {
    std::visit([&env](auto&& arg) {
        using T = std::decay_t<decltype(arg)>;
        if constexpr (std::is_same_v<T, Series>) {
            std::cout << arg.str(30) << std::endl;
        } else if constexpr (std::is_same_v<T, std::map<int, Frac>>) {
            std::cout << formatProdmake(arg) << std::endl;
        } else if constexpr (std::is_same_v<T, std::vector<std::pair<int, Frac>>>) {
            std::cout << formatEtamake(arg) << std::endl;
        } else if constexpr (std::is_same_v<T, std::vector<JacFactor>>) {
            std::cout << jac2prod(arg) << std::endl;
        } else if constexpr (std::is_same_v<T, std::vector<std::vector<Frac>>>) {
            for (const auto& r : arg)
                std::cout << formatRelation(r, {}, {}) << std::endl;
        } else if constexpr (std::is_same_v<T, std::optional<std::vector<Frac>>>) {
            if (arg) std::cout << formatRelation(*arg, {}, {}) << std::endl;
            else std::cout << "(no solution)" << std::endl;
        } else if constexpr (std::is_same_v<T, QFactorResult>) {
            std::cout << formatQfactor(arg) << std::endl;
        } else if constexpr (std::is_same_v<T, int64_t>) {
            std::cout << arg << std::endl;
        } else if constexpr (std::is_same_v<T, DisplayOnly>) {
            (void)arg;
        } else if constexpr (std::is_same_v<T, std::monostate>) {
            (void)arg;
        }
    }, res);
}

inline EvalResult evalStmt(const Stmt* s, Environment& env) {
    if (!s) throw std::runtime_error("evalStmt: null stmt");
    if (s->tag == Stmt::Tag::Assign) {
        EvalResult res = eval(s->assignRhs.get(), env, {});
        if (std::holds_alternative<Series>(res)) {
            env.env[s->assignName] = std::get<Series>(res);
            return res;
        }
        if (std::holds_alternative<std::vector<JacFactor>>(res)) {
            env.env[s->assignName] = std::get<std::vector<JacFactor>>(res);
            return res;
        }
        throw std::runtime_error("assignment requires Series or Jacobi product");
    }
    return eval(s->expr.get(), env, {});
}

inline void runRepl() {
    std::cout << R"(
  /\  /\
 ( o  o )
 (  __  )
  \____/
 q-series REPL (Maple-like), version 1.0
)" << std::endl;

    Environment env;
    std::deque<std::string> history;
    const size_t maxHistory = 100;

    for (;;) {
        std::cout << "qseries> " << std::flush;
        std::string line;
        if (!std::getline(std::cin, line)) break;
        std::string trimmed = trim(line);
        if (trimmed.empty()) continue;

        history.push_back(trimmed);
        if (history.size() > maxHistory) history.pop_front();

        try {
            StmtPtr stmt = parse(trimmed);
            EvalResult res = evalStmt(stmt.get(), env);
            if (std::holds_alternative<std::monostate>(res))
                continue;
            display(res, env, env.T);
        } catch (const std::exception& e) {
            std::cerr << "error: " << e.what() << std::endl;
        }
    }
}

#endif
