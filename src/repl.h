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
#ifdef _WIN32
#include <io.h>
#define stdin_is_tty() _isatty(_fileno(stdin))
#else
#include <unistd.h>
#define stdin_is_tty() isatty(STDIN_FILENO)
#endif
#include <algorithm>
#include <sstream>
#include <chrono>
#include <iomanip>
#include <set>
#include <cctype>
#include <cstdlib>

inline std::string runtimeErr(const std::string& func, const std::string& msg) {
    return func.empty() ? msg : (func + ": " + msg);
}

#if defined(__CYGWIN__) || !defined(_WIN32)
#include <termios.h>
#include <unistd.h>
#else
#include <windows.h>
#endif

// Raw terminal mode: read char-by-char for Tab completion.
// RawModeGuard restores on scope exit. If process is killed (e.g. Ctrl+C), run "reset" to restore terminal.
#if defined(__CYGWIN__) || !defined(_WIN32)
struct RawModeGuard {
    struct termios orig;
    bool active = false;
    RawModeGuard() {
        if (tcgetattr(STDIN_FILENO, &orig) == 0) {
            struct termios raw = orig;
            raw.c_lflag &= ~(static_cast<tcflag_t>(ICANON | ECHO));
            raw.c_cc[VMIN] = 1;
            raw.c_cc[VTIME] = 0;
            if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == 0)
                active = true;
        }
    }
    ~RawModeGuard() {
        if (active)
            tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig);
    }
};
inline int readOneChar() {
    unsigned char c;
    if (read(STDIN_FILENO, &c, 1) == 1)
        return static_cast<int>(c);
    return -1;
}
#else
struct RawModeGuard {
    HANDLE hStdin = nullptr;
    DWORD origMode = 0;
    bool active = false;
    RawModeGuard() {
        hStdin = GetStdHandle(STD_INPUT_HANDLE);
        if (hStdin != INVALID_HANDLE_VALUE && GetConsoleMode(hStdin, &origMode)) {
            DWORD mode = origMode;
            mode &= ~(ENABLE_LINE_INPUT | ENABLE_ECHO_INPUT);
            if (SetConsoleMode(hStdin, mode))
                active = true;
        }
    }
    ~RawModeGuard() {
        if (active && hStdin != INVALID_HANDLE_VALUE)
            SetConsoleMode(hStdin, origMode);
    }
};
inline int readOneChar() {
    HANDLE h = GetStdHandle(STD_INPUT_HANDLE);
    unsigned char c;
    DWORD readCount = 0;
    if (ReadFile(h, &c, 1, &readCount, nullptr) && readCount == 1)
        return static_cast<int>(c);
    return -1;
}
#endif

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
    std::vector<int>,                                 // mprodmake
    std::vector<std::pair<int, Frac>>,                // etamake
    std::vector<JacFactor>,                           // jacprodmake
    RelationKernelResult,                             // findhom/findnonhom
    RelationComboResult,                              // findhomcombo/findnonhomcombo
    QFactorResult,                                    // qfactor
    CheckprodResult,                                  // checkprod
    CheckmultResult,                                  // checkmult
    FindmaxindResult,                                 // findmaxind
    int64_t,                                          // legendre/sigma
    DisplayOnly,
    std::monostate                                    // set_trunc
>;

// Help table: name -> (signature, description)
inline const std::map<std::string, std::pair<std::string, std::string>>& getHelpTable() {
    static const std::map<std::string, std::pair<std::string, std::string>> table = {
        {"add", {"add(expr, var, lo, hi)", "summation over index var from lo to hi"}},
        {"aqprod", {"aqprod(a,q,n,T)", "rising q-factorial (a;q)_n"}},
        {"coeff", {"coeff(f,n)", "coefficient of q^n in series f"}},
        {"coeffs", {"coeffs(f,from,to)", "list coefficients from exponent from to to"}},
        {"dissect", {"dissect(f,m,T)", "list all m siftings of f: sift(f,m,k,T) for k=0..m-1"}},
        {"divisors", {"divisors(n)", "sorted list of positive divisors of n"}},
        {"eisenstein", {"eisenstein(k,T)", "normalized Eisenstein series E_{2k}(q)"}},
        {"etaq", {"etaq(k) or etaq(k,T) or etaq(q,k,T)", "eta product Π(1-q^{kn})"}},
        {"etamake", {"etamake(f,T)", "identify f as eta product"}},
        {"euler_phi", {"euler_phi(n)", "Euler's totient φ(n)"}},
        {"findhom", {"findhom(L,n,topshift)", "homogeneous polynomial relations between series in L"}},
        {"findhomcombo", {"findhomcombo(f,L,n,topshift[,etaopt])", "express f as polynomial in L"}},
        {"findnonhom", {"findnonhom(L,n,topshift)", "nonhomogeneous polynomial relations"}},
        {"findnonhomcombo", {"findnonhomcombo(f,L,n_list,topshift[,etaopt])", "express f as polynomial in L (nonhom)"}},
        {"findpoly", {"findpoly(x,y,deg1,deg2[,check])", "polynomial relation between two series"}},
        {"jac2prod", {"jac2prod(var)", "display Jacobi product stored in variable"}},
        {"jacobi", {"jacobi(a,n)", "Jacobi symbol (a/n) for odd positive n"}},
        {"jacprodmake", {"jacprodmake(f,T)", "identify f as Jacobi product"}},
        {"kronecker", {"kronecker(a,n)", "Kronecker symbol (a/n) for all integer n"}},
        {"legendre", {"legendre(a,p)", "Legendre symbol (a/p)"}},
        {"partition", {"partition(n)", "partition number p(n)"}},
        {"prodmake", {"prodmake(f,T)", "Andrews' algorithm: series → infinite product"}},
        {"mobius", {"mobius(n)", "Möbius function μ(n): 0 if squared factor, (-1)^k otherwise"}},
        {"mprodmake", {"mprodmake(f,T)", "convert series to product (1+q^n1)(1+q^n2)..."}},
        {"checkprod", {"checkprod(f) or checkprod(f,T) or checkprod(f,M,T)", "check if f is a nice product (exponents |a[n]| < M)"}},
        {"checkmult", {"checkmult(f) or checkmult(f,T) or checkmult(f,T,1)", "check if coefficients are multiplicative"}},
        {"clear_cache", {"clear_cache()", "clear memoization caches (etaq)"}},
        {"qbin", {"qbin(m,n,T) or qbin(q,m,n,T)", "Gaussian polynomial [m;n]_q"}},
        {"qfactor", {"qfactor(f) or qfactor(f,T)", "factorize finite q-product"}},
        {"quinprod", {"quinprod(z,q,T)", "quintuple product"}},
        {"series", {"series(f) or series(f,T)", "display series coefficients"}},
        {"set_trunc", {"set_trunc(N)", "set default truncation"}},
        {"sigma", {"sigma(n) or sigma(n,k)", "divisor sum σ_k(n)"}},
        {"sift", {"sift(f,n,k,T)", "extract coefficients a_{ni+k}"}},
        {"subs_q", {"subs_q(f,k)", "substitute q^k for q"}},
        {"sum", {"sum(expr, var, lo, hi)", "summation over index var from lo to hi"}},
        {"T", {"T(r,n) or T(r,n,T)", "finite q-product T_{r,n}"}},
        {"theta", {"theta(z,T) or theta(z,q,T)", "generalized theta function"}},
        {"theta2", {"theta2(T) or theta2(q,T)", "theta_2"}},
        {"theta3", {"theta3(T) or theta3(q,T)", "theta_3"}},
        {"theta4", {"theta4(T) or theta4(q,T)", "theta_4"}},
        {"tripleprod", {"tripleprod(z,q,T)", "Jacobi triple product"}},
        {"version", {"version", "print package version"}},
        {"winquist", {"winquist(a,b,q,T)", "Winquist identity"}},
        {"qdegree", {"qdegree(f)", "highest exponent with nonzero coefficient"}},
        {"qdiff", {"qdiff(f)", "q-derivative: θ_q f = Σ n·a_n·q^n"}},
        {"lqdegree", {"lqdegree(f)", "lowest exponent with nonzero coefficient"}},
        {"jac2series", {"jac2series(var) or jac2series(var,T)", "convert Jacobi product (in var) to series"}},
        {"findlincombo", {"findlincombo(f,L,topshift)", "express f as linear combination of series in L"}},
        {"findmaxind", {"findmaxind(L) or findmaxind(L, topshift)", "maximal linearly independent subset of q-series in L; returns indices (1-based)"}},
        {"max", {"max(a, b, ...)", "maximum of 2 or more integers"}},
        {"min", {"min(a, b, ...)", "minimum of 2 or more integers"}},
    };
    return table;
}

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
                case BinOp::Div:
                    if (r == 0) throw std::runtime_error("division by zero");
                    return l / r;
                case BinOp::Pow: {
                    if (r < 0) throw std::runtime_error("negative exponent in integer power");
                    int64_t acc = 1;
                    for (int64_t i = 0; i < r; ++i) acc *= l;
                    return acc;
                }
            }
            __builtin_unreachable();
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

inline Series toSeries(const EvalResult& r, const char* context, int T = 50) {
    if (std::holds_alternative<Series>(r))
        return std::get<Series>(r);
    if (std::holds_alternative<int64_t>(r))
        return Series::constant(Frac(std::get<int64_t>(r)), T);
    throw std::runtime_error(std::string(context) + ": expected series expression");
}

inline int levenshteinDistance(const std::string& a, const std::string& b) {
    size_t m = a.size(), n = b.size();
    std::vector<size_t> prev(n + 1), curr(n + 1);
    for (size_t j = 0; j <= n; ++j) prev[j] = j;
    for (size_t i = 1; i <= m; ++i) {
        curr[0] = i;
        for (size_t j = 1; j <= n; ++j) {
            size_t cost = (a[i-1] == b[j-1]) ? 0 : 1;
            curr[j] = std::min({prev[j] + 1, curr[j-1] + 1, prev[j-1] + cost});
        }
        std::swap(prev, curr);
    }
    return static_cast<int>(prev[n]);
}

inline EvalResult dispatchBuiltin(const std::string& name,
    const std::vector<ExprPtr>& args, Environment& env,
    const std::map<std::string, int64_t>& sumIndices) {
    Series q = getSeriesFromEnv(env.env.at("q"));
    int T = env.T;

    auto ev = [&](size_t i) {
        try { return evalAsSeries(args[i].get(), env, sumIndices); }
        catch (const std::exception& e) { throw std::runtime_error(runtimeErr(name, e.what())); }
    };
    auto evi = [&](size_t i) {
        try { return evalToInt(args[i].get(), env, sumIndices); }
        catch (const std::exception& e) { throw std::runtime_error(runtimeErr(name, e.what())); }
    };

    if (name == "aqprod") {
        if (args.size() != 4)
            throw std::runtime_error(runtimeErr(name, "expects 4 arguments"));
        return aqprod(ev(0), ev(1), static_cast<int>(evi(2)), static_cast<int>(evi(3)));
    }
    if (name == "etaq") {
        if (args.size() == 1) {
            int k = static_cast<int>(evi(0));
            return etaq(q, k, T);
        }
        if (args.size() == 2) {
            int k = static_cast<int>(evi(0));
            int Tr = static_cast<int>(evi(1));
            return etaq(q, k, Tr);
        }
        if (args.size() == 3)
            return etaq(ev(0), static_cast<int>(evi(1)), static_cast<int>(evi(2)));
        throw std::runtime_error(runtimeErr(name, "expected etaq(k), etaq(k,T), or etaq(q,k,T), got " + std::to_string(args.size()) + " arguments"));
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
        throw std::runtime_error(runtimeErr(name, "expected theta2/3/4(T) or theta2/3/4(q,T)"));
    }
    if (name == "theta") {
        if (args.size() == 2)
            return theta(ev(0), q, static_cast<int>(evi(1)));
        if (args.size() == 3)
            return theta(ev(0), ev(1), static_cast<int>(evi(2)));
        throw std::runtime_error(runtimeErr(name, "expected theta(z,T) or theta(z,q,T)"));
    }
    if (name == "qbin") {
        if (args.size() == 3)
            return qbin(q, static_cast<int>(evi(0)), static_cast<int>(evi(1)), static_cast<int>(evi(2)));
        if (args.size() == 4)
            return qbin(ev(0), static_cast<int>(evi(1)), static_cast<int>(evi(2)), static_cast<int>(evi(3)));
        throw std::runtime_error(runtimeErr(name, "expected qbin(m,n,T) or qbin(q,m,n,T)"));
    }
    if (name == "tripleprod") {
        if (args.size() != 3)
            throw std::runtime_error(runtimeErr(name, "expects 3 arguments"));
        return tripleprod(ev(0), ev(1), static_cast<int>(evi(2)));
    }
    if (name == "quinprod") {
        if (args.size() != 3)
            throw std::runtime_error(runtimeErr(name, "expects 3 arguments"));
        return quinprod(ev(0), ev(1), static_cast<int>(evi(2)));
    }
    if (name == "winquist") {
        if (args.size() != 4)
            throw std::runtime_error(runtimeErr(name, "expects 4 arguments"));
        return winquist(ev(0), ev(1), ev(2), static_cast<int>(evi(3)));
    }
    if (name == "sift") {
        if (args.size() != 4)
            throw std::runtime_error(runtimeErr(name, "expects 4 arguments"));
        return sift(ev(0), static_cast<int>(evi(1)), static_cast<int>(evi(2)), static_cast<int>(evi(3)));
    }
    if (name == "T") {
        if (args.size() == 2)
            return T_rn(static_cast<int>(evi(0)), static_cast<int>(evi(1)), T);
        if (args.size() == 3)
            return T_rn(static_cast<int>(evi(0)), static_cast<int>(evi(1)), static_cast<int>(evi(2)));
        throw std::runtime_error(runtimeErr(name, "expected T(r,n) or T(r,n,T)"));
    }
    if (name == "prodmake") {
        if (args.size() != 2)
            throw std::runtime_error(runtimeErr(name, "expects 2 arguments"));
        return prodmake(ev(0), static_cast<int>(evi(1)));
    }
    if (name == "mprodmake") {
        if (args.size() != 2)
            throw std::runtime_error(runtimeErr(name, "expects 2 arguments"));
        return mprodmake(ev(0), static_cast<int>(evi(1)));
    }
    if (name == "checkprod") {
        if (args.size() == 1)
            return checkprod(ev(0), T);
        if (args.size() == 2)
            return checkprod(ev(0), static_cast<int>(evi(1)));
        if (args.size() == 3)
            return checkprod(ev(0), static_cast<int>(evi(1)), static_cast<int>(evi(2)));
        throw std::runtime_error(runtimeErr(name, "expects 1, 2, or 3 arguments"));
    }
    if (name == "checkmult") {
        if (args.size() == 1)
            return checkmult(ev(0), T, false);
        if (args.size() == 2)
            return checkmult(ev(0), static_cast<int>(evi(1)), false);
        if (args.size() == 3)
            return checkmult(ev(0), static_cast<int>(evi(1)), evi(2) != 0);
        throw std::runtime_error(runtimeErr(name, "expects 1, 2, or 3 arguments"));
    }
    if (name == "etamake") {
        if (args.size() != 2)
            throw std::runtime_error(runtimeErr(name, "expects 2 arguments"));
        return etamake(ev(0), static_cast<int>(evi(1)));
    }
    if (name == "jacprodmake") {
        if (args.size() != 2)
            throw std::runtime_error(runtimeErr(name, "expects 2 arguments"));
        return jacprodmake(ev(0), static_cast<int>(evi(1)));
    }
    if (name == "jac2prod") {
        if (args.size() != 1)
            throw std::runtime_error(runtimeErr(name, "expects 1 argument"));
        if (args[0]->tag != Expr::Tag::Var)
            throw std::runtime_error(runtimeErr(name, "expects variable name"));
        auto it = env.env.find(args[0]->varName);
        if (it == env.env.end())
            throw std::runtime_error(runtimeErr(name, "undefined variable: " + args[0]->varName));
        if (std::holds_alternative<std::vector<JacFactor>>(it->second)) {
            std::cout << jac2prod(std::get<std::vector<JacFactor>>(it->second)) << std::endl;
            return DisplayOnly{};
        }
        throw std::runtime_error(runtimeErr(name, "expects jacprodmake result"));
    }
    if (name == "jac2series") {
        if (args.size() < 1 || args.size() > 2)
            throw std::runtime_error(runtimeErr(name, "expects jac2series(var) or jac2series(var,T)"));
        if (args[0]->tag != Expr::Tag::Var)
            throw std::runtime_error(runtimeErr(name, "expects variable name"));
        auto it = env.env.find(args[0]->varName);
        if (it == env.env.end())
            throw std::runtime_error(runtimeErr(name, "undefined variable: " + args[0]->varName));
        if (!std::holds_alternative<std::vector<JacFactor>>(it->second))
            throw std::runtime_error(runtimeErr(name, "expects jacprodmake result"));
        int Tr = (args.size() == 2) ? static_cast<int>(evi(1)) : T;
        return jac2series(std::get<std::vector<JacFactor>>(it->second), Tr);
    }
    if (name == "qfactor") {
        if (args.size() == 1)
            return qfactor(ev(0), T);
        if (args.size() == 2)
            return qfactor(ev(0), static_cast<int>(evi(1)));
        throw std::runtime_error(runtimeErr(name, "expected qfactor(f) or qfactor(f,T)"));
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
        throw std::runtime_error(runtimeErr(name, "expected series(f) or series(f,T)"));
    }
    if (name == "coeffs") {
        if (args.size() != 3)
            throw std::runtime_error(runtimeErr(name, "expects 3 arguments"));
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
    if (name == "coeff") {
        if (args.size() != 2)
            throw std::runtime_error(runtimeErr(name, "expects 2 arguments"));
        Frac c = ev(0).coeff(static_cast<int>(evi(1)));
        std::cout << c.str() << std::endl;
        return DisplayOnly{};
    }
    if (name == "dissect") {
        if (args.size() != 3)
            throw std::runtime_error(runtimeErr(name, "expects 3 arguments"));
        Series f = ev(0);
        int m = static_cast<int>(evi(1));
        int Td = static_cast<int>(evi(2));
        if (m <= 0)
            throw std::runtime_error(runtimeErr(name, "m must be positive"));
        for (int k = 0; k < m; ++k) {
            Series s = sift(f, m, k, Td);
            std::cout << "k=" << k << ": " << s.str(30) << std::endl;
        }
        return DisplayOnly{};
    }
    if (name == "set_trunc") {
        if (args.size() != 1)
            throw std::runtime_error(runtimeErr(name, "expects 1 argument"));
        int N = static_cast<int>(evi(0));
        if (N <= 0)
            throw std::runtime_error(runtimeErr(name, "truncation must be positive"));
        env.T = N;
        env.env["q"] = Series::q(N);
        clear_etaq_cache();
        return std::monostate{};
    }
    if (name == "clear_cache") {
        if (args.size() != 0)
            throw std::runtime_error(runtimeErr(name, "expects no arguments"));
        clear_etaq_cache();
        return std::monostate{};
    }
    if (name == "version") {
        if (args.size() != 0)
            throw std::runtime_error(runtimeErr(name, "expects no arguments"));
        std::cout << "qseries 1.9\n";
        return DisplayOnly{};
    }
    if (name == "qdegree") {
        if (args.size() != 1)
            throw std::runtime_error(runtimeErr(name, "expects 1 argument"));
        Series f = ev(0);
        return static_cast<int64_t>(f.maxExp());
    }
    if (name == "lqdegree") {
        if (args.size() != 1)
            throw std::runtime_error(runtimeErr(name, "expects 1 argument"));
        Series f = ev(0);
        if (f.c.empty())
            return static_cast<int64_t>(0);
        return static_cast<int64_t>(f.minExp());
    }

    auto evalListToSeries = [&](const Expr* listExpr) {
        if (!listExpr || listExpr->tag != Expr::Tag::List)
            throw std::runtime_error(runtimeErr(name, "expected list of series"));
        std::vector<Series> out;
        for (const auto& e : listExpr->elements)
            out.push_back(evalAsSeries(e.get(), env, sumIndices));
        return out;
    };
    auto evalListToInt = [&](const Expr* listExpr) {
        if (!listExpr || listExpr->tag != Expr::Tag::List)
            throw std::runtime_error(runtimeErr(name, "expected list of integers"));
        std::vector<int> out;
        for (const auto& e : listExpr->elements)
            out.push_back(static_cast<int>(evalToInt(e.get(), env, sumIndices)));
        return out;
    };

    if (name == "findhom") {
        if (args.size() != 3)
            throw std::runtime_error(runtimeErr(name, "expects 3 arguments"));
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
            throw std::runtime_error(runtimeErr(name, "expects 3 arguments"));
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
            throw std::runtime_error(runtimeErr(name, "expects 4 or 5 arguments"));
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
    if (name == "findlincombo") {
        if (args.size() != 3)
            throw std::runtime_error(runtimeErr(name, "expects 3 arguments: findlincombo(f,L,topshift)"));
        Series f = ev(0);
        auto L = evalListToSeries(args[1].get());
        int topshift = static_cast<int>(evi(2));
        auto coeffs = findhomcombo(f, L, 1, topshift, false);
        std::vector<std::vector<int>> exps;
        enumerate_hom_exponents(static_cast<int>(L.size()), 1, {}, exps);
        return RelationComboResult{std::move(coeffs), std::move(exps)};
    }
    if (name == "findnonhomcombo") {
        if (args.size() < 4 || args.size() > 5)
            throw std::runtime_error(runtimeErr(name, "expects 4 or 5 arguments"));
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
    if (name == "findmaxind") {
        if (args.size() == 1) {
            auto L = evalListToSeries(args[0].get());
            return findmaxind(L, 0);
        }
        if (args.size() == 2) {
            auto L = evalListToSeries(args[0].get());
            int topshift = static_cast<int>(evi(1));
            return findmaxind(L, topshift);
        }
        throw std::runtime_error(runtimeErr(name, "expects 1 or 2 arguments: findmaxind(L) or findmaxind(L, topshift)"));
    }
    if (name == "findpoly") {
        if (args.size() < 4 || args.size() > 5)
            throw std::runtime_error(runtimeErr(name, "expects 4 or 5 arguments"));
        Series x = ev(0);
        Series y = ev(1);
        int deg1 = static_cast<int>(evi(2));
        int deg2 = static_cast<int>(evi(3));
        std::optional<int> check;
        if (args.size() == 5) check = static_cast<int>(evi(4));
        auto basis = findpoly(x, y, deg1, deg2, check);
        std::vector<std::vector<int>> exps;
        for (int j = 0; j <= deg2; ++j)
            for (int i = 0; i <= deg1; ++i)
                exps.push_back({i, j});
        return RelationKernelResult{std::move(basis), std::move(exps)};
    }
    if (name == "legendre") {
        if (args.size() != 2)
            throw std::runtime_error(runtimeErr(name, "expects 2 arguments"));
        return static_cast<int64_t>(legendre(evi(0), evi(1)));
    }
    if (name == "jacobi") {
        if (args.size() != 2)
            throw std::runtime_error(runtimeErr(name, "expects 2 arguments"));
        return static_cast<int64_t>(jacobi(evi(0), evi(1)));
    }
    if (name == "kronecker") {
        if (args.size() != 2)
            throw std::runtime_error(runtimeErr(name, "expects 2 arguments"));
        return static_cast<int64_t>(kronecker(evi(0), evi(1)));
    }
    if (name == "sigma") {
        if (args.size() == 1)
            return sigma(static_cast<int>(evi(0)), 1);
        if (args.size() == 2)
            return sigma(static_cast<int>(evi(0)), static_cast<int>(evi(1)));
        throw std::runtime_error(runtimeErr(name, "expected sigma(n) or sigma(n,k)"));
    }
    if (name == "divisors") {
        if (args.size() != 1)
            throw std::runtime_error(runtimeErr(name, "expects 1 argument"));
        auto d = divisors(static_cast<int>(evi(0)));
        std::cout << "[";
        for (size_t i = 0; i < d.size(); ++i) {
            if (i > 0) std::cout << ", ";
            std::cout << d[i];
        }
        std::cout << "]" << std::endl;
        return DisplayOnly{};
    }
    if (name == "mobius") {
        if (args.size() != 1)
            throw std::runtime_error(runtimeErr(name, "expects 1 argument"));
        return static_cast<int64_t>(mobius(static_cast<int>(evi(0))));
    }
    if (name == "euler_phi") {
        if (args.size() != 1)
            throw std::runtime_error(runtimeErr(name, "expects 1 argument"));
        return static_cast<int64_t>(euler_phi(static_cast<int>(evi(0))));
    }
    if (name == "min") {
        if (args.size() < 2)
            throw std::runtime_error(runtimeErr(name, "expects 2 or more arguments"));
        int64_t result = evi(0);
        for (size_t i = 1; i < args.size(); ++i)
            result = std::min(result, evi(i));
        return result;
    }
    if (name == "max") {
        if (args.size() < 2)
            throw std::runtime_error(runtimeErr(name, "expects 2 or more arguments"));
        int64_t result = evi(0);
        for (size_t i = 1; i < args.size(); ++i)
            result = std::max(result, evi(i));
        return result;
    }
    if (name == "eisenstein") {
        if (args.size() != 2)
            throw std::runtime_error(runtimeErr(name, "expects 2 arguments"));
        return eisenstein(static_cast<int>(evi(0)), static_cast<int>(evi(1)));
    }
    if (name == "partition") {
        if (args.size() != 1)
            throw std::runtime_error(runtimeErr(name, "expects 1 argument"));
        Frac p = partition_number(static_cast<int>(evi(0)));
        std::cout << p.str() << std::endl;
        return DisplayOnly{};
    }
    if (name == "qdiff") {
        if (args.size() != 1)
            throw std::runtime_error(runtimeErr(name, "expects 1 argument"));
        return qdiff(ev(0));
    }
    if (name == "subs_q") {
        if (args.size() != 2)
            throw std::runtime_error(runtimeErr(name, "expects 2 arguments"));
        return ev(0).subs_q(static_cast<int>(evi(1)));
    }

    if (name == "help") {
        const auto& table = getHelpTable();
        if (args.size() == 0) {
            std::cout << "q-series REPL. Commands: expr, var := expr, help, help(func).\nBuilt-ins: ";
            bool first = true;
            for (const auto& [n, _] : table) {
                if (!first) std::cout << ", ";
                std::cout << n;
                first = false;
            }
            std::cout << std::endl;
            return DisplayOnly{};
        }
        if (args.size() == 1 && args[0]->tag == Expr::Tag::Var) {
            const std::string& fn = args[0]->varName;
            auto it = table.find(fn);
            if (it != table.end()) {
                std::cout << it->second.first << " — " << it->second.second << std::endl;
            } else {
                std::cout << "unknown function: " << fn << std::endl;
            }
            return DisplayOnly{};
        }
        throw std::runtime_error(runtimeErr(name, "expects 0 or 1 argument"));
    }

    {
        const auto& table = getHelpTable();
        std::vector<std::pair<int, std::string>> suggestions;
        for (const auto& [key, _] : table) {
            int d = levenshteinDistance(name, key);
            if (d <= 3)
                suggestions.push_back({d, key});
        }
        std::sort(suggestions.begin(), suggestions.end());
        std::string msg = "unknown built-in '" + name + "'.";
        if (!suggestions.empty()) {
            msg += " Did you mean:";
            for (size_t i = 0; i < std::min(suggestions.size(), size_t(2)); ++i)
                msg += " " + suggestions[i].second;
            msg += "?";
        } else {
            msg += " No close matches found.";
        }
        throw std::runtime_error(msg);
    }
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
            Series l = toSeries(eval(e->left.get(), env, sumIndices), "binary op", env.T);
            Series r = toSeries(eval(e->right.get(), env, sumIndices), "binary op", env.T);
            switch (e->binOp) {
                case BinOp::Add: return l + r;
                case BinOp::Sub: return l - r;
                case BinOp::Mul: return l * r;
                case BinOp::Div:
                    if (r.c.empty()) throw std::runtime_error("division by zero");
                    return l / r;
                case BinOp::Pow: {
                    int64_t expVal = evalToInt(e->right.get(), env, sumIndices);
                    if (expVal > 10000 || expVal < -10000)
                        throw std::runtime_error("pow: exponent magnitude too large (limit 10000)");
                    return l.pow(static_cast<int>(expVal));
                }
            }
            __builtin_unreachable();
        }
        case Expr::Tag::UnOp: {
            Series s = toSeries(eval(e->operand.get(), env, sumIndices), "unary op", env.T);
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
                Series term = toSeries(eval(e->body.get(), env, idx), "sum body", env.T);
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

inline std::string formatCheckprod(const CheckprodResult& r) {
    if (r.nice)
        return "nice product (minExp=" + std::to_string(r.minExp) + ")";
    return "not a nice product (bound M=" + std::to_string(r.M) + "), minExp=" + std::to_string(r.minExp);
}

inline std::string formatFindmaxind(const FindmaxindResult& r) {
    std::string out = "indices: [";
    for (size_t i = 0; i < r.indices.size(); ++i) {
        if (i) out += ", ";
        out += std::to_string(r.indices[i]);
    }
    out += "]";
    return out;
}

inline std::string formatCheckmult(const CheckmultResult& r) {
    if (r.multiplicative) return "MULTIPLICATIVE";
    std::string s = "NOT MULTIPLICATIVE";
    for (const auto& [m, n] : r.failures)
        s += " (m=" + std::to_string(m) + ",n=" + std::to_string(n) + ")";
    return s;
}

inline std::string formatMprodmake(const std::vector<int>& S) {
    if (S.empty()) return "1";
    std::string out;
    for (int n : S) {
        if (n == 1)
            out += "(1+q)";
        else
            out += "(1+q" + Series::expToUnicode(n) + ")";
    }
    return out;
}

inline std::string trim(const std::string& s) {
    size_t i = 0, j = s.size();
    while (i < j && (s[i] == ' ' || s[i] == '\t')) ++i;
    while (j > i && (s[j-1] == ' ' || s[j-1] == '\t')) --j;
    return s.substr(i, j - i);
}

// Redraw prompt + line with cursor at pos (0 = before first char, line.size() = after last)
inline void redrawLineRaw(const std::string& line, size_t pos) {
    std::cout << "\r\033[Kqseries> " << line << std::flush;
    if (pos < line.size()) {
        size_t count = line.size() - pos;
        std::cout << "\033[" << std::to_string(count) << "D" << std::flush;
    }
}

inline std::set<std::string> getCompletionCandidates(const Environment& env) {
    std::set<std::string> out;
    for (const auto& [k, _] : getHelpTable())
        out.insert(k);
    for (const auto& [k, _] : env.env)
        out.insert(k);
    return out;
}

inline void handleTabCompletion(std::string& line, size_t& pos, const Environment& env) {
    size_t end = pos;
    size_t start = end;
    while (start > 0) {
        unsigned char c = static_cast<unsigned char>(line[start - 1]);
        if (std::isalnum(c) || c == '_')
            --start;
        else
            break;
    }
    std::string prefix = line.substr(start, end - start);
    if (prefix.empty()) return;  // Tab at start or after non-identifier: no change
    auto candidates = getCompletionCandidates(env);
    std::vector<std::string> matches;
    for (const auto& cand : candidates) {
        if (cand.size() >= prefix.size() && cand.compare(0, prefix.size(), prefix) == 0)
            matches.push_back(cand);
    }
    if (matches.empty()) return;
    if (matches.size() == 1) {
        const std::string& match = matches[0];
        line = line.substr(0, start) + match + line.substr(end);
        pos = start + match.size();
        redrawLineRaw(line, pos);
        return;
    }
    std::cout << "\n";
    for (size_t i = 0; i < matches.size(); ++i) {
        if (i) std::cout << " ";
        std::cout << matches[i];
    }
    std::cout << "\n";
    redrawLineRaw(line, pos);
}

inline std::optional<std::string> readLineRaw(Environment& env, const std::deque<std::string>& history) {
    RawModeGuard guard;
    std::string line;
    size_t pos = 0;
    size_t histIdx = history.size();
    std::string savedLine;
    for (;;) {
        int c = readOneChar();
        if (c < 0) {
            if (line.empty()) return std::nullopt;
            return line;
        }
        if (c == '\n' || c == '\r') return line;  // \r for Windows CR/LF
        if (c == 27) {  // ESC
            int c2 = readOneChar();
            if (c2 == '[') {
                int c3 = readOneChar();
                if (c3 == 65) {  // up arrow
                    if (!history.empty() && histIdx > 0) {
                        if (histIdx == history.size()) savedLine = line;
                        --histIdx;
                        line = history[histIdx];
                        pos = line.size();
                        redrawLineRaw(line, pos);
                    }
                } else if (c3 == 66) {  // down arrow
                    if (histIdx < history.size()) {
                        ++histIdx;
                        line = (histIdx == history.size()) ? savedLine : history[histIdx];
                        pos = line.size();
                        redrawLineRaw(line, pos);
                    }
                } else if (c3 == 68) {  // left
                    if (pos > 0) { --pos; redrawLineRaw(line, pos); }
                } else if (c3 == 67) {  // right
                    if (pos < line.size()) { ++pos; redrawLineRaw(line, pos); }
                }
            }
            // If first char after ESC is not '[', treat as standalone; do nothing
            continue;
        }
        if (c == '\t') {
            handleTabCompletion(line, pos, env);
            continue;
        }
        if (c == 8 || c == 127) {  // Backspace / DEL
            if (pos > 0) {
                line.erase(pos - 1, 1);
                --pos;
                redrawLineRaw(line, pos);
            }
            continue;
        }
        if (c >= 32 && c <= 126) {
            line.insert(pos, 1, static_cast<char>(c));
            ++pos;
            redrawLineRaw(line, pos);
        }
    }
}

inline void display(const EvalResult& res, Environment& env, int /*T*/) {
    std::visit([&env](auto&& arg) {
        using T = std::decay_t<decltype(arg)>;
        if constexpr (std::is_same_v<T, Series>) {
            std::cout << arg.str(30) << std::endl;
        } else if constexpr (std::is_same_v<T, std::map<int, Frac>>) {
            std::cout << formatProdmake(arg, true) << std::endl;
        } else if constexpr (std::is_same_v<T, std::vector<int>>) {
            std::cout << formatMprodmake(arg) << std::endl;
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
        } else if constexpr (std::is_same_v<T, CheckprodResult>) {
            std::cout << formatCheckprod(arg) << std::endl;
        } else if constexpr (std::is_same_v<T, CheckmultResult>) {
            std::cout << formatCheckmult(arg) << std::endl;
        } else if constexpr (std::is_same_v<T, FindmaxindResult>) {
            std::cout << formatFindmaxind(arg) << std::endl;
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
        if (std::holds_alternative<int64_t>(res)) {
            int64_t val = std::get<int64_t>(res);
            env.env[s->assignName] = Series::constant(Frac(val), env.T);
            return res;
        }
        throw std::runtime_error("assignment requires Series or Jacobi product");
    }
    return eval(s->expr.get(), env, {});
}

inline void runRepl() {
    if (stdin_is_tty()) {
        std::cout << R"banner(
 _                                           
| |                                          
| | ____ _ _ __   __ _  __ _ _ __ ___   ___  
| |/ / _` | '_ \ / _` |/ _` | '__/ _ \ / _ \
|   < (_| | | | | (_| | (_| | | | (_) | (_) |
|_|\_\__,_|_| |_|\__, |\__,_|_|  \___/ \___/ 
                  __/ |                      
                 |___/                       

 q-series REPL (Maple-like), version 1.9
)banner" << std::endl;
    }

    Environment env;
    std::deque<std::string> history;
    const size_t maxHistory = 100;
    size_t inputLineNum = 0;

    constexpr size_t maxContinuations = 100;

    for (;;) {
        std::string line;
        if (stdin_is_tty()) {
            std::cout << "qseries> " << std::flush;
            auto opt = readLineRaw(env, history);
            std::cout << std::endl;
            if (!opt) break;
            line = *opt;
        } else {
            if (!std::getline(std::cin, line)) break;
            std::cout << "qseries> " << line << std::endl;
        }

        // Backslash continuation: while line ends with \, read more lines
        size_t contCount = 0;
        for (;;) {
            // Rtrim to find last char
            while (!line.empty() && (line.back() == ' ' || line.back() == '\t'))
                line.pop_back();
            if (line.empty() || line.back() != '\\')
                break;
            line.pop_back();  // remove trailing backslash
            if (contCount >= maxContinuations)
                break;
            if (stdin_is_tty()) {
                std::cout << "  > " << std::flush;
                auto nextOpt = readLineRaw(env, history);
                std::cout << std::endl;
                if (!nextOpt) break;
                line += " " + *nextOpt;
            } else {
                std::string next;
                if (!std::getline(std::cin, next)) break;
                std::cout << "  > " << next << std::endl;
                line += " " + next;
            }
            ++contCount;
        }

        std::string trimmed = trim(line);
        if (trimmed.empty()) continue;

        bool suppress_output = false;
        if (trimmed.back() == ':') {
            trimmed.pop_back();
            trimmed = trim(trimmed);
            suppress_output = true;
        }
        if (trimmed.empty()) continue;

        ++inputLineNum;
        history.push_back(trimmed);
        if (history.size() > maxHistory) history.pop_front();

        try {
            auto t0 = std::chrono::steady_clock::now();
            StmtPtr stmt = parse(trimmed);
            EvalResult res = evalStmt(stmt.get(), env);
            if (std::holds_alternative<std::monostate>(res))
                continue;
            if (suppress_output) continue;
            display(res, env, env.T);
            auto t1 = std::chrono::steady_clock::now();
            if (stdin_is_tty()) {
                double secs = std::chrono::duration<double>(t1 - t0).count();
                std::cout << std::fixed << std::setprecision(3) << secs << "s" << std::endl;
            }
        } catch (const std::exception& e) {
            std::cerr << "error: ";
            if (!stdin_is_tty() && inputLineNum > 0)
                std::cerr << "line " << inputLineNum << ": ";
            std::cerr << e.what() << std::endl;
        }
    }
}

#endif
