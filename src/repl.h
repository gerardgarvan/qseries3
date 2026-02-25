#ifndef REPL_H
#define REPL_H

#include "parser.h"
#include "series.h"
#include "frac.h"
#include "convert.h"
#include "qfuncs.h"
#include "relations.h"
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

struct RelationKernelResult {
    std::vector<std::vector<Frac>> basis;
    std::vector<std::vector<int>> monomialExponents;
};
struct RelationComboResult {
    std::optional<std::vector<Frac>> coeffs;
    std::vector<std::vector<int>> monomialExponents;
};

using EvalResult = std::variant<
    Series,
    std::map<int, Frac>,                              // prodmake
    std::vector<std::pair<int, Frac>>,                // etamake
    std::vector<JacFactor>,                           // jacprodmake
    RelationKernelResult,                             // findhom/findnonhom
    RelationComboResult,                              // findhomcombo/findnonhomcombo
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

// Get Series from EvalResult (throws if not Series)
inline Series evalAsSeries(const Expr* e, Environment& env,
    const std::map<std::string, int64_t>& sumIndices) {
    EvalResult r = eval(e, env, sumIndices);
    if (!std::holds_alternative<Series>(r))
        throw std::runtime_error("expected series");
    return std::get<Series>(r);
}

inline EvalResult dispatchBuiltin(const std::string& name,
    const std::vector<ExprPtr>& args, Environment& env,
    const std::map<std::string, int64_t>& sumIndices) {
    Series q = getSeriesFromEnv(env.env.at("q"));
    int T = env.T;

    auto ev = [&](size_t i) { return evalAsSeries(args[i].get(), env, sumIndices); };
    auto evi = [&](size_t i) { return evalToInt(args[i].get(), env, sumIndices); };

    if (name == "aqprod") {
        if (args.size() != 4)
            throw std::runtime_error("aqprod(a,q,n,T) expects 4 arguments");
        return aqprod(ev(0), ev(1), static_cast<int>(evi(2)), static_cast<int>(evi(3)));
    }
    if (name == "etaq") {
        if (args.size() == 2) {
            int k = static_cast<int>(evi(0));
            int Tr = static_cast<int>(evi(1));
            return etaq(q, k, Tr);
        }
        if (args.size() == 3)
            return etaq(ev(0), static_cast<int>(evi(1)), static_cast<int>(evi(2)));
        throw std::runtime_error("etaq(k,T) or etaq(q,k,T)");
    }
    if (name == "theta2" || name == "theta3" || name == "theta4") {
        if (args.size() == 1) {
            int Tr = static_cast<int>(evi(0));
            if (name == "theta2") return theta2(q, Tr);
            if (name == "theta3") return theta3(q, Tr);
            return theta4(q, Tr);
        }
        if (args.size() == 2) {
            int Tr = static_cast<int>(evi(1));
            if (name == "theta2") return theta2(ev(0), Tr);
            if (name == "theta3") return theta3(ev(0), Tr);
            return theta4(ev(0), Tr);
        }
        throw std::runtime_error("theta2/3/4(T) or theta2/3/4(q,T)");
    }
    if (name == "theta") {
        if (args.size() == 2)
            return theta(ev(0), q, static_cast<int>(evi(1)));
        if (args.size() == 3)
            return theta(ev(0), ev(1), static_cast<int>(evi(2)));
        throw std::runtime_error("theta(z,T) or theta(z,q,T)");
    }
    if (name == "qbin") {
        if (args.size() == 3)
            return qbin(q, static_cast<int>(evi(0)), static_cast<int>(evi(1)), static_cast<int>(evi(2)));
        if (args.size() == 4)
            return qbin(ev(0), static_cast<int>(evi(1)), static_cast<int>(evi(2)), static_cast<int>(evi(3)));
        throw std::runtime_error("qbin(m,n,T) or qbin(q,m,n,T)");
    }
    if (name == "tripleprod") {
        if (args.size() != 3)
            throw std::runtime_error("tripleprod(z,q,T) expects 3 arguments");
        return tripleprod(ev(0), ev(1), static_cast<int>(evi(2)));
    }
    if (name == "quinprod") {
        if (args.size() != 3)
            throw std::runtime_error("quinprod(z,q,T) expects 3 arguments");
        return quinprod(ev(0), ev(1), static_cast<int>(evi(2)));
    }
    if (name == "winquist") {
        if (args.size() != 4)
            throw std::runtime_error("winquist(a,b,q,T) expects 4 arguments");
        return winquist(ev(0), ev(1), ev(2), static_cast<int>(evi(3)));
    }
    if (name == "sift") {
        if (args.size() != 4)
            throw std::runtime_error("sift(f,n,k,T) expects 4 arguments");
        return sift(ev(0), static_cast<int>(evi(1)), static_cast<int>(evi(2)), static_cast<int>(evi(3)));
    }
    if (name == "T") {
        if (args.size() == 2)
            return T_rn(static_cast<int>(evi(0)), static_cast<int>(evi(1)), T);
        if (args.size() == 3)
            return T_rn(static_cast<int>(evi(0)), static_cast<int>(evi(1)), static_cast<int>(evi(2)));
        throw std::runtime_error("T(r,n) or T(r,n,T)");
    }
    if (name == "prodmake") {
        if (args.size() != 2)
            throw std::runtime_error("prodmake(f,T) expects 2 arguments");
        return prodmake(ev(0), static_cast<int>(evi(1)));
    }
    if (name == "etamake") {
        if (args.size() != 2)
            throw std::runtime_error("etamake(f,T) expects 2 arguments");
        return etamake(ev(0), static_cast<int>(evi(1)));
    }
    if (name == "jacprodmake") {
        if (args.size() != 2)
            throw std::runtime_error("jacprodmake(f,T) expects 2 arguments");
        return jacprodmake(ev(0), static_cast<int>(evi(1)));
    }
    if (name == "jac2prod") {
        if (args.size() != 1)
            throw std::runtime_error("jac2prod(var) expects 1 argument");
        if (args[0]->tag != Expr::Tag::Var)
            throw std::runtime_error("jac2prod expects variable name");
        auto it = env.env.find(args[0]->varName);
        if (it == env.env.end())
            throw std::runtime_error("undefined variable: " + args[0]->varName);
        if (std::holds_alternative<std::vector<JacFactor>>(it->second)) {
            std::cout << jac2prod(std::get<std::vector<JacFactor>>(it->second)) << std::endl;
            return DisplayOnly{};
        }
        throw std::runtime_error("jac2prod expects jacprodmake result");
    }
    if (name == "qfactor") {
        if (args.size() == 1)
            return qfactor(ev(0), T);
        if (args.size() == 2)
            return qfactor(ev(0), static_cast<int>(evi(1)));
        throw std::runtime_error("qfactor(f) or qfactor(f,T)");
    }
    if (name == "series") {
        if (args.size() == 1) {
            Series f = ev(0).truncTo(T);
            std::cout << f.str(std::min(100, T)) << std::endl;
            return DisplayOnly{};
        }
        if (args.size() == 2) {
            int Tr = static_cast<int>(evi(1));
            Series f = ev(0).truncTo(Tr);
            std::cout << f.str(std::min(Tr, 100)) << std::endl;
            return DisplayOnly{};
        }
        throw std::runtime_error("series(f) or series(f,T)");
    }
    if (name == "coeffs") {
        if (args.size() != 3)
            throw std::runtime_error("coeffs(f,from,to) expects 3 arguments");
        Series f = ev(0);
        int from = static_cast<int>(evi(1));
        int to = static_cast<int>(evi(2));
        auto list = f.coeffList(from, to);
        std::cout << "[";
        for (size_t i = 0; i < list.size(); ++i) {
            if (i) std::cout << ", ";
            std::cout << list[i].str();
        }
        std::cout << "]" << std::endl;
        return DisplayOnly{};
    }
    if (name == "set_trunc") {
        if (args.size() != 1)
            throw std::runtime_error("set_trunc(N) expects 1 argument");
        int N = static_cast<int>(evi(0));
        if (N <= 0)
            throw std::runtime_error("truncation must be positive");
        env.T = N;
        env.env["q"] = Series::q(N);
        return std::monostate{};
    }

    auto evalListToSeries = [&](const Expr* listExpr) {
        if (!listExpr || listExpr->tag != Expr::Tag::List)
            throw std::runtime_error("expected list of series");
        std::vector<Series> out;
        for (const auto& e : listExpr->elements)
            out.push_back(evalAsSeries(e.get(), env, sumIndices));
        return out;
    };
    auto evalListToInt = [&](const Expr* listExpr) {
        if (!listExpr || listExpr->tag != Expr::Tag::List)
            throw std::runtime_error("expected list of integers");
        std::vector<int> out;
        for (const auto& e : listExpr->elements)
            out.push_back(static_cast<int>(evalToInt(e.get(), env, sumIndices)));
        return out;
    };

    if (name == "findhom") {
        if (args.size() != 3)
            throw std::runtime_error("findhom(L,n,topshift) expects 3 arguments");
        auto L = evalListToSeries(args[0].get());
        int n = static_cast<int>(evi(1));
        int topshift = static_cast<int>(evi(2));
        auto basis = findhom(L, n, topshift);
        std::vector<std::vector<int>> exps;
        enumerate_hom_exponents(static_cast<int>(L.size()), n, {}, exps);
        return RelationKernelResult{std::move(basis), std::move(exps)};
    }
    if (name == "findnonhom") {
        if (args.size() != 3)
            throw std::runtime_error("findnonhom(L,n,topshift) expects 3 arguments");
        auto L = evalListToSeries(args[0].get());
        int n = static_cast<int>(evi(1));
        int topshift = static_cast<int>(evi(2));
        auto basis = findnonhom(L, n, topshift);
        std::vector<std::vector<int>> exps;
        enumerate_nonhom_exponents(static_cast<int>(L.size()), n, exps);
        return RelationKernelResult{std::move(basis), std::move(exps)};
    }
    if (name == "findhomcombo") {
        if (args.size() < 4 || args.size() > 5)
            throw std::runtime_error("findhomcombo(f,L,n,topshift[,etaopt]) expects 4 or 5 arguments");
        Series f = ev(0);
        auto L = evalListToSeries(args[1].get());
        int n = static_cast<int>(evi(2));
        int topshift = static_cast<int>(evi(3));
        bool etaopt = (args.size() == 5) && (evi(4) != 0);
        (void)etaopt;
        auto coeffs = findhomcombo(f, L, n, topshift, false);
        std::vector<std::vector<int>> exps;
        enumerate_hom_exponents(static_cast<int>(L.size()), n, {}, exps);
        return RelationComboResult{std::move(coeffs), std::move(exps)};
    }
    if (name == "findnonhomcombo") {
        if (args.size() < 4 || args.size() > 5)
            throw std::runtime_error("findnonhomcombo(f,L,n_list,topshift[,etaopt]) expects 4 or 5 arguments");
        Series f = ev(0);
        auto L = evalListToSeries(args[1].get());
        auto n_list = evalListToInt(args[2].get());
        int topshift = static_cast<int>(evi(3));
        bool etaopt = (args.size() == 5) && (evi(4) != 0);
        (void)etaopt;
        auto coeffs = findnonhomcombo(f, L, n_list, topshift, false);
        std::vector<std::vector<int>> exps;
        enumerate_nlist_exponents(n_list, {}, 0, exps);
        return RelationComboResult{std::move(coeffs), std::move(exps)};
    }
    if (name == "findpoly") {
        if (args.size() < 4 || args.size() > 5)
            throw std::runtime_error("findpoly(x,y,deg1,deg2[,check]) expects 4 or 5 arguments");
        Series x = ev(0);
        Series y = ev(1);
        int deg1 = static_cast<int>(evi(2));
        int deg2 = static_cast<int>(evi(3));
        std::optional<int> check = (args.size() == 5) ? std::optional<int>(static_cast<int>(evi(4))) : std::nullopt;
        auto basis = findpoly(x, y, deg1, deg2, check);
        std::vector<std::vector<int>> exps;
        for (int j = 0; j <= deg2; ++j)
            for (int i = 0; i <= deg1; ++i)
                exps.push_back({i, j});
        return RelationKernelResult{std::move(basis), std::move(exps)};
    }
    if (name == "legendre") {
        if (args.size() != 2)
            throw std::runtime_error("legendre(a,p) expects 2 arguments");
        return static_cast<int64_t>(legendre(evi(0), evi(1)));
    }
    if (name == "sigma") {
        if (args.size() == 1)
            return sigma(static_cast<int>(evi(0)), 1);
        if (args.size() == 2)
            return sigma(static_cast<int>(evi(0)), static_cast<int>(evi(1)));
        throw std::runtime_error("sigma(n) or sigma(n,k)");
    }
    if (name == "subs_q") {
        if (args.size() != 2)
            throw std::runtime_error("subs_q(f,k) expects 2 arguments");
        return ev(0).subs_q(static_cast<int>(evi(1)));
    }

    throw std::runtime_error("unknown built-in: " + name);
}

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
            return dispatchBuiltin(e->callName, e->args, env, sumIndices);
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

inline std::string formatProdmake(const std::map<int, Frac>& a, bool mapleStyle = true) {
    std::vector<std::string> num_parts, den_parts;
    for (const auto& [n, an] : a) {
        if (an.isZero()) continue;
        std::string part;
        if (mapleStyle) {
            part = (n == 1) ? "(1-q)" : "(-q" + Series::expToUnicode(n) + "+1)";
        } else {
            part = "(1-q" + (n == 1 ? "" : Series::expToUnicode(n)) + ")";
        }
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
            std::cout << formatProdmake(arg, true) << std::endl;
        } else if constexpr (std::is_same_v<T, std::vector<std::pair<int, Frac>>>) {
            std::cout << formatEtamake(arg) << std::endl;
        } else if constexpr (std::is_same_v<T, std::vector<JacFactor>>) {
            std::cout << jac2prod(arg) << std::endl;
        } else if constexpr (std::is_same_v<T, RelationKernelResult>) {
            for (const auto& r : arg.basis)
                std::cout << formatRelation(r, arg.monomialExponents, {}) << std::endl;
        } else if constexpr (std::is_same_v<T, RelationComboResult>) {
            if (arg.coeffs) std::cout << formatRelation(*arg.coeffs, arg.monomialExponents, {}) << std::endl;
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
    std::cout << R"banner(
     (\-"""-/)
        | |
     \ ^ ^ / .-.
     \_o_/ / /
     /` `\/ |
     / \ |
     \ ( ) / |
     / \_) (_/ \ /
     | (\-/) |
     \ --^o^-- /
     \ '.___.' /
     .' \-=-/ '.
     / /` `\ \
     (//./ \.\\) `"` `"`

 q-series REPL (Maple-like), version 1.0
)banner" << std::endl;

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
