#ifndef REPL_H
#define REPL_H

#include "parser.h"
#include "series.h"
#include "frac.h"
#include "convert.h"
#include "qfuncs.h"
#include "tcore.h"
#include "mock.h"
#include "crank_tables.h"
#include "bailey.h"
#include "eta_cusp.h"
#include "modforms.h"
#include "theta_ids.h"
#include "relations.h"
#include "rr_ids.h"
#include <map>
#include <vector>
#include <string>
#include <variant>
#include <optional>
#include <stdexcept>
#include <cstdint>
#include <iostream>
#include <deque>
#ifndef __EMSCRIPTEN__
#ifdef _WIN32
#include <io.h>
#define stdin_is_tty() _isatty(_fileno(stdin))
#else
#include <unistd.h>
#define stdin_is_tty() isatty(STDIN_FILENO)
#endif
#else
#define stdin_is_tty() false
#endif
#include <algorithm>
#include <sstream>
#include <fstream>
#include <chrono>
#include <iomanip>
#include <set>
#include <cctype>
#include <cstdlib>

inline std::string runtimeErr(const std::string& func, const std::string& msg) {
    return func.empty() ? msg : (func + ": " + msg);
}

#ifndef __EMSCRIPTEN__
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
#endif // __EMSCRIPTEN__

namespace ansi {
    inline bool g_color = false;

    inline void init() {
        if (!stdin_is_tty()) return;
        if (std::getenv("NO_COLOR")) return;
        g_color = true;
#if defined(_WIN32) && !defined(__CYGWIN__)
#ifndef ENABLE_VIRTUAL_TERMINAL_PROCESSING
#define ENABLE_VIRTUAL_TERMINAL_PROCESSING 0x0004
#endif
        HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
        DWORD mode = 0;
        if (GetConsoleMode(hOut, &mode))
            SetConsoleMode(hOut, mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
        HANDLE hErr = GetStdHandle(STD_ERROR_HANDLE);
        if (GetConsoleMode(hErr, &mode))
            SetConsoleMode(hErr, mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
#endif
    }

    inline const char* gold()  { return g_color ? "\033[33m" : ""; }
    inline const char* red()   { return g_color ? "\033[31m" : ""; }
    inline const char* dim()   { return g_color ? "\033[2m" : ""; }
    inline const char* reset() { return g_color ? "\033[0m" : ""; }
    inline const char* bold()  { return g_color ? "\033[1m" : ""; }
}

// Partition: non-decreasing list of positive integers
struct Partition {
    std::vector<int64_t> parts;
};

inline std::string formatPartition(const Partition& p) {
    std::string out = "[";
    for (size_t i = 0; i < p.parts.size(); ++i) {
        if (i > 0) out += ", ";
        out += std::to_string(p.parts[i]);
    }
    out += "]";
    return out;
}

// Phi1Result: [t-core, t-quotient] pair from GSK bijection
struct Phi1Result {
    Partition core;
    std::vector<Partition> quotient;
};

inline std::string formatPhi1Result(const Phi1Result& r) {
    std::string out = "Core: " + formatPartition(r.core) + "\nQuotient: [";
    for (size_t i = 0; i < r.quotient.size(); ++i) {
        if (i > 0) out += ", ";
        out += formatPartition(r.quotient[i]);
    }
    out += "]";
    return out;
}

// EnvValue: variable can hold Series, Jacobi product, Partition, or Phi1Result
using EnvValue = std::variant<Series, std::vector<JacFactor>, Partition, Phi1Result>;

struct Environment {
    std::map<std::string, EnvValue> env;
    int T = 50;

    Environment() {
        env["q"] = Series::q(T);
    }
};

// --- History persistence ---

inline std::string getHomeDir() {
#ifdef _WIN32
    const char* home = std::getenv("USERPROFILE");
    if (!home) {
        const char* drive = std::getenv("HOMEDRIVE");
        const char* path = std::getenv("HOMEPATH");
        if (drive && path) {
            static std::string buf;
            buf = std::string(drive) + path;
            return buf;
        }
        return ".";
    }
    return home;
#else
    const char* home = std::getenv("HOME");
    return home ? home : ".";
#endif
}

inline std::string getHistoryPath() {
    std::string home = getHomeDir();
    char sep = '/';
#ifdef _WIN32
    sep = '\\';
#endif
    return home + sep + ".qseries_history";
}

inline void loadHistory(std::deque<std::string>& history, size_t maxHistory) {
    std::ifstream f(getHistoryPath());
    if (!f) return;
    std::string line;
    while (std::getline(f, line)) {
        if (!line.empty()) {
            history.push_back(line);
            if (history.size() > maxHistory)
                history.pop_front();
        }
    }
}

inline void saveHistory(const std::deque<std::string>& history) {
    std::ofstream f(getHistoryPath());
    if (!f) return;
    for (const auto& line : history)
        f << line << "\n";
}

// --- Session save/load ---

inline void saveSession(const std::string& name, const Environment& env) {
    std::string path = name + ".qsession";
    std::ofstream f(path);
    if (!f)
        throw std::runtime_error("save: cannot write to " + path);
    f << "# qseries session\n";
    f << "T " << env.T << "\n";
    for (const auto& [varname, val] : env.env) {
        if (varname == "q") continue;
        if (std::holds_alternative<Series>(val)) {
            const Series& s = std::get<Series>(val);
            f << "S " << varname << " " << s.trunc;
            for (const auto& [exp, coeff] : s.c)
                f << " " << exp << ":" << coeff.str();
            f << "\n";
        } else if (std::holds_alternative<Partition>(val)) {
            const Partition& p = std::get<Partition>(val);
            f << "P " << varname;
            for (size_t i = 0; i < p.parts.size(); ++i) {
                f << (i == 0 ? " " : ",") << p.parts[i];
            }
            f << "\n";
        } else {
            const auto& jac = std::get<std::vector<JacFactor>>(val);
            f << "J " << varname;
            for (const auto& [a, b, exp] : jac)
                f << " " << a << "," << b << "," << exp.str();
            f << "\n";
        }
    }
    std::cout << "Session saved to " << path << std::endl;
}

inline Frac parseFrac(const std::string& s) {
    auto slash = s.find('/');
    if (slash == std::string::npos)
        return Frac(BigInt(s), BigInt(1));
    return Frac(BigInt(s.substr(0, slash)), BigInt(s.substr(slash + 1)));
}

inline void loadSession(const std::string& name, Environment& env) {
    std::string path = name + ".qsession";
    std::ifstream f(path);
    if (!f)
        throw std::runtime_error("load: cannot open " + path);
    std::string line;
    int loaded = 0;
    while (std::getline(f, line)) {
        if (line.empty() || line[0] == '#') continue;
        std::istringstream iss(line);
        std::string tag;
        iss >> tag;
        if (tag == "T") {
            int t;
            iss >> t;
            env.T = t;
            env.env["q"] = Series::q(t);
        } else if (tag == "S") {
            std::string varname;
            int trunc;
            iss >> varname >> trunc;
            Series s;
            s.trunc = trunc;
            std::string token;
            while (iss >> token) {
                auto colon = token.find(':');
                if (colon == std::string::npos) continue;
                int exp = std::stoi(token.substr(0, colon));
                Frac coeff = parseFrac(token.substr(colon + 1));
                s.setCoeff(exp, coeff);
            }
            env.env[varname] = s;
            ++loaded;
        } else if (tag == "P") {
            std::string varname;
            iss >> varname;
            Partition ptn;
            std::string rest;
            if (std::getline(iss, rest)) {
                size_t start = rest.find_first_not_of(' ');
                if (start != std::string::npos) {
                    std::istringstream pss(rest.substr(start));
                    std::string tok;
                    while (std::getline(pss, tok, ',')) {
                        if (!tok.empty())
                            ptn.parts.push_back(std::stoll(tok));
                    }
                }
            }
            env.env[varname] = ptn;
            ++loaded;
        } else if (tag == "J") {
            std::string varname;
            iss >> varname;
            std::vector<JacFactor> jac;
            std::string token;
            while (iss >> token) {
                auto c1 = token.find(',');
                auto c2 = token.find(',', c1 + 1);
                if (c1 == std::string::npos || c2 == std::string::npos) continue;
                int a = std::stoi(token.substr(0, c1));
                int b = std::stoi(token.substr(c1 + 1, c2 - c1 - 1));
                Frac exp = parseFrac(token.substr(c2 + 1));
                jac.push_back({a, b, exp});
            }
            env.env[varname] = jac;
            ++loaded;
        }
    }
    std::cout << "Session loaded from " << path << " (" << loaded << " variables)" << std::endl;
}

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
    std::monostate,                                   // set_trunc
    Partition,                                        // single partition
    std::vector<Partition>,                           // list of partitions
    Phi1Result                                        // GSK [core, quotient]
>;

// Help table: name -> (signature, description)
inline const std::map<std::string, std::pair<std::string, std::string>>& getHelpTable() {
    static const std::map<std::string, std::pair<std::string, std::string>> table = {
        {"add", {"add(expr, var, lo, hi)", "summation over index var from lo to hi"}},
        {"aqprod", {"aqprod(a,q,n,T)", "rising q-factorial (a;q)_n"}},
        {"coeff", {"coeff(f,n)", "coefficient of q^n in series f"}},
        {"coeffs", {"coeffs(f,from,to)", "list coefficients from exponent from to to"}},
        {"conjpart", {"conjpart(ptn)", "conjugate partition"}},
        {"crankgf", {"crankgf(m,T)", "GF for partitions with crank m: \xCE\xA3 M(m,n) q^n"}},
        {"dissect", {"dissect(f,m,T)", "list all m siftings of f: sift(f,m,k,T) for k=0..m-1"}},
        {"dilly", {"dilly(f,d)", "q-dilation: substitute q \u2192 q^d"}},
        {"divisors", {"divisors(n)", "sorted list of positive divisors of n"}},
        {"DELTA12", {"DELTA12(T)", "discriminant cusp form q·η(q)^24"}},
        {"EISENq", {"EISENq(d,T)", "Eisenstein series E_d(q), d even (2,4,6,...)"}},
        {"makebasisM", {"makebasisM(k,T)", "basis of M_k(SL_2(Z)) using E4, E6"}},
        {"makebasisPX", {"makebasisPX(k,T)", "basis P*X[k] using partition function P"}},
        {"QP2", {"QP2(num, den)", "second periodic Bernoulli: fpart^2-fpart+1/6"}},
        {"getacuspord", {"getacuspord(n, r, a, c)", "order of η_{n,r} at cusp a/c"}},
        {"cuspmake1", {"cuspmake1(N)", "cusps for Gamma_1(N)"}},
        {"Gamma1ModFunc", {"Gamma1ModFunc(L, N)", "check generalized eta product modular on Gamma_1(N)"}},
        {"eta2jac", {"eta2jac(getalist)", "convert GETA list to Jacobi product"}},
        {"jac2eprod", {"jac2eprod(var)", "convert Jacobi product to GETA form"}},
        {"eisenstein", {"eisenstein(k,T)", "normalized Eisenstein series E_{2k}(q)"}},
        {"etaq", {"etaq(k) or etaq(k,T) or etaq(q,k,T)", "eta product Π(1-q^{kn})"}},
        {"etamake", {"etamake(f,T)", "identify f as eta product"}},
        {"euler_phi", {"euler_phi(n)", "Euler's totient φ(n)"}},
        {"findhom", {"findhom(L,n,topshift)", "homogeneous polynomial relations between series in L"}},
        {"findhommodp", {"findhommodp(L,p,n,topshift)", "homogeneous polynomial relations mod p"}},
        {"findhomcombo", {"findhomcombo(f,L,n,topshift[,etaopt])", "express f as polynomial in L"}},
        {"agcrank", {"agcrank(ptn)", "Andrews-Garvan crank of a partition"}},
        {"drank", {"drank(ptn)", "Dyson rank: largest part minus number of parts"}},
        {"findnonhom", {"findnonhom(L,n,topshift)", "nonhomogeneous polynomial relations"}},
        {"findnonhomcombo", {"findnonhomcombo(f,L,n_list,topshift[,etaopt])", "express f as polynomial in L (nonhom)"}},
        {"findpoly", {"findpoly(x,y,deg1,deg2[,check])", "polynomial relation between two series"}},
        {"jac2prod", {"jac2prod(var)", "display Jacobi product stored in variable"}},
        {"invphi1", {"invphi1(phi1result,t)", "inverse GSK bijection: [core,quotient] \xe2\x86\x92 partition"}},
        {"istcore", {"istcore(ptn,t)", "test if partition is a t-core (returns 1 or 0)"}},
        {"jacobi", {"jacobi(a,n)", "Jacobi symbol (a/n) for odd positive n"}},
        {"jacprodmake", {"jacprodmake(f,T)", "identify f as Jacobi product"}},
        {"kapPD", {"kapPD(ptn)", "count of parts > largest even part (distinct-part partition)"}},
        {"kronecker", {"kronecker(a,n)", "Kronecker symbol (a/n) for all integer n"}},
        {"lamPD", {"lamPD(ptn)", "largest part minus smallest part (distinct-part partition)"}},
        {"legendre", {"legendre(a,p)", "Legendre symbol (a/p)"}},
        {"lp", {"lp(ptn)", "largest part of partition"}},
        {"overptncrank", {"overptncrank(dptn,ptn)", "overpartition crank"}},
        {"overptnrank", {"overptnrank(dptn,ptn)", "overpartition rank"}},
        {"overptns", {"overptns(n)", "enumerate all overpartitions of n as [dptn, ptn] pairs"}},
        {"partition", {"partition(n)", "partition number p(n)"}},
        {"partitions", {"partitions(n)", "all partitions of n as list of integer lists"}},
        {"PDP", {"PDP(n)", "count partitions of n into distinct parts"}},
        {"POE", {"POE(n)", "count partitions of n with even parts < odd parts"}},
        {"PRR", {"PRR(n)", "count Rogers-Ramanujan partitions of n (parts differ by ≥2)"}},
        {"PSCHUR", {"PSCHUR(n)", "count Schur partitions of n (distinct, no consecutive)"}},
        {"ptnCC", {"ptnCC(ptn)", "test: parts differ by at least 3"}},
        {"ptnDP", {"ptnDP(ptn)", "test: all parts distinct"}},
        {"ptnOE", {"ptnOE(ptn)", "test: even parts all smaller than odd parts"}},
        {"ptnOP", {"ptnOP(ptn)", "test: all parts odd"}},
        {"ptnRR", {"ptnRR(ptn)", "test: parts differ by at least 2 (Rogers-Ramanujan type)"}},
        {"ptnSCHUR", {"ptnSCHUR(ptn)", "test: distinct parts, no two consecutive"}},
        {"phi1", {"phi1(ptn,t)", "GSK bijection: partition \xe2\x86\x92 [t-core, t-quotient]"}},
        {"GFDM2N", {"GFDM2N(k1,k2,t,r[,T])", "GF for M2N(k1,t,n*t+r)-M2N(k2,t,n*t+r)"}},
        {"M2N", {"M2N(m,n) or M2N(k,r,n)", "count partitions of n (no repeated odd parts) with M2-rank m"}},
        {"MBAR", {"MBAR(m,n) or MBAR(k,r,n)", "overpartition crank count"}},
        {"NS", {"NS(m,n) or NS(k,r,n)", "SPT-crank count"}},
        {"Phiq", {"Phiq(j,T)", "divisor sum GF Σ σ_j(n) q^n"}},
        {"polyfind", {"polyfind(n0,n1,n2,T)", "quadratic p(x) with p(T)=n0, p(T+1)=n1, p(T+2)=n2"}},
        {"prodmake", {"prodmake(f,T)", "Andrews' algorithm: series → infinite product"}},
        {"RRG", {"RRG(n) or RRG(n,T)", "Rogers-Ramanujan G(n): sum form for n=1, Geta for n>1"}},
        {"RRH", {"RRH(n) or RRH(n,T)", "Rogers-Ramanujan H(n): sum form for n=1, Geta for n>1"}},
        {"RRGstar", {"RRGstar(n) or RRGstar(n,T)", "Göllnitz-Gordon G*(n)"}},
        {"RRHstar", {"RRHstar(n) or RRHstar(n,T)", "Göllnitz-Gordon H*(n)"}},
        {"geta", {"geta(g,d,n,T)", "Geta(g,d,n) = q^(n·QP2(g/d)·d/2)·JAC(ng,nd,∞)/JAC(0,nd,∞)"}},
        {"checkid", {"checkid(expr,T) or checkid(expr,T,acc)", "check if expr is eta/theta product (CHECKRAMIDF)"}},
        {"sieveqcheck", {"sieveqcheck(f,p)", "true if all exponents ≡ same residue mod p"}},
        {"siftfindrange", {"siftfindrange(f,p,T)", "find residue class with fewest terms"}},
        {"sptcrankresnum", {"sptcrankresnum(k,r,n)", "partitions of n with SPT-crank ≡ k mod r"}},
        {"ocrankresnum", {"ocrankresnum(k,r,n)", "overpartitions of n with crank ≡ k mod r"}},
        {"pbar", {"pbar(n)", "overpartition number"}},
        {"betafind", {"betafind(n,T) or betafind(n)", "beta for unit Bailey pair at index n"}},
        {"alphaup", {"alphaup(alpha,T)", "Bailey chain: move alpha right"}},
        {"alphadown", {"alphadown(alpha,T)", "Bailey chain: move alpha left"}},
        {"bailey_sum", {"bailey_sum(pair,N,T)", "sum of beta[0..N]; pair: 0=unit, 1=RR, 2=GG"}},
        {"cuspmake", {"cuspmake(N)", "inequivalent cusps for Gamma_0(N)"}},
        {"cuspord", {"cuspord(GP, cusp)", "Ligozat order of eta-quotient at cusp"}},
        {"gammacheck", {"gammacheck(GP, N)", "Newman's 5 conditions for Gamma_0(N)"}},
        {"etaprodtoqseries", {"etaprodtoqseries(GP, T)", "q-expansion of eta-quotient from GP"}},
        {"vp", {"vp(n, p)", "p-adic valuation"}},
        {"provemodfuncGAMMA0id", {"provemodfuncGAMMA0id(etaid, N)", "prove eta identity on Gamma_0(N) via Sturm"}},
        {"provemodfuncGAMMA0idBATCH", {"provemodfuncGAMMA0idBATCH(etaids, N)", "batch prove multiple eta identities"}},
        {"provemodfuncid", {"provemodfuncid(jacid, N)", "prove Jacobi theta identity on Gamma_1(N) via Sturm"}},
        {"mod", {"mod(a,b)", "integer remainder a mod b"}},
        {"modp", {"modp(f,p)", "reduce series coefficients modulo prime p"}},
        {"mockdesorder", {"mockdesorder(m)", "list mock theta function names of order m (2,3,5,6,7,8,10)"}},
        {"mockqs", {"mockqs(name, order, T)", "q-series expansion of mock theta function (e.g. mockqs(f3, 3, 20))"}},
        {"newprodmake", {"newprodmake(f,T)", "Andrews' prodmake with arbitrary leading term"}},
        {"mobius", {"mobius(n)", "Möbius function μ(n): 0 if squared factor, (-1)^k otherwise"}},
        {"mprodmake", {"mprodmake(f,T)", "convert series to product (1+q^n1)(1+q^n2)..."}},
        {"makebiw", {"makebiw(ptn,t,m)", "display bi-infinite words W0..W(t-1) for positions -m..m"}},
        {"nep", {"nep(ptn)", "number of even parts"}},
        {"np_parts", {"np_parts(ptn)", "number of parts"}},
        {"nterms", {"nterms(f)", "count non-zero coefficients in series"}},
        {"numLE", {"numLE(ptn)", "count of times largest even part occurs"}},
        {"nvec2alphavec", {"nvec2alphavec(nvec)", "convert n-vector to alpha-vector (t=5 or t=7)"}},
        {"nvec2ptn", {"nvec2ptn(nvec)", "reconstruct t-core from n-vector"}},
        {"ptn2nvec", {"ptn2nvec(ptn,t)", "n-vector of partition (t-core theory)"}},
        {"ptn2rvec", {"ptn2rvec(ptn,t)", "r-vector (residue counts) of partition"}},
        {"ptnnorm", {"ptnnorm(ptn)", "sum of parts of partition"}},
        {"tcrank", {"tcrank(ptn,t)", "t-core crank statistic (mod t, t odd prime)"}},
        {"tresdiag", {"tresdiag(ptn,t)", "display t-residue diagram of partition"}},
        {"checkprod", {"checkprod(f) or checkprod(f,T) or checkprod(f,M,T)", "check if f is a nice product (exponents |a[n]| < M)"}},
        {"checkmult", {"checkmult(f) or checkmult(f,T) or checkmult(f,T,1)", "check if coefficients are multiplicative"}},
        {"clear_cache", {"clear_cache()", "clear memoization caches (etaq)"}},
        {"qbin", {"qbin(m,n,T) or qbin(q,m,n,T)", "Gaussian polynomial [m;n]_q"}},
        {"qfactor", {"qfactor(f) or qfactor(f,T)", "factorize finite q-product"}},
        {"quinprod", {"quinprod(z,q,T)", "quintuple product"}},
        {"rankgf", {"rankgf(m,T)", "GF for partitions with rank m: \xCE\xA3 N(m,n) q^n"}},
        {"rvec", {"rvec(ptn,t,k)", "count of nodes colored k in t-residue diagram"}},
        {"series", {"series(f) or series(f,T)", "display series coefficients"}},
        {"set_trunc", {"set_trunc(N)", "set default truncation"}},
        {"sigma", {"sigma(n) or sigma(n,k)", "divisor sum σ_k(n)"}},
        {"sift", {"sift(f,n,k,T)", "extract coefficients a_{ni+k}"}},
        {"subs_q", {"subs_q(f,k)", "substitute q^k for q"}},
        {"sum", {"sum(expr, var, lo, hi)", "summation over index var from lo to hi"}},
        {"T", {"T(r,n) or T(r,n,T)", "finite q-product T_{r,n}"}},
        {"tcoreofptn", {"tcoreofptn(ptn,t)", "compute the t-core of a partition"}},
        {"tcores", {"tcores(t,n)", "list all t-cores of n"}},
        {"tquot", {"tquot(ptn,t)", "t-quotient of partition (list of t partitions)"}},
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
        {"load", {"load(name)", "restore session from name.qsession file"}},
        {"max", {"max(a, b, ...)", "maximum of 2 or more integers"}},
        {"min", {"min(a, b, ...)", "minimum of 2 or more integers"}},
        {"save", {"save(name)", "save current session to name.qsession file"}},
    };
    return table;
}

// Helper: get Series from EnvValue (for variable lookup in arithmetic)
inline Series getSeriesFromEnv(const EnvValue& v) {
    if (std::holds_alternative<Series>(v))
        return std::get<Series>(v);
    if (std::holds_alternative<Partition>(v))
        throw std::runtime_error("variable holds partition, not series");
    if (std::holds_alternative<Phi1Result>(v))
        throw std::runtime_error("variable holds phi1 result, not series");
    throw std::runtime_error("variable holds Jacobi product, not series");
}

inline EvalResult dispatchBuiltin(const std::string& name,
    const std::vector<ExprPtr>& args, Environment& env,
    const std::map<std::string, int64_t>& sumIndices);

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
                    if (l % r != 0) throw std::runtime_error("non-integer division");
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
        case Expr::Tag::Call: {
            EvalResult r = dispatchBuiltin(e->callName, e->args, env, sumIndices);
            if (std::holds_alternative<int64_t>(r))
                return std::get<int64_t>(r);
            throw std::runtime_error("expected integer from " + e->callName + "()");
        }
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

inline std::string formatProdmake(const std::map<int, Frac>& a, bool mapleStyle = true);

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
    if (name == "RRG") {
        int n = static_cast<int>(evi(0));
        int Tr = (args.size() == 2) ? static_cast<int>(evi(1)) : T;
        return RRG(n, Tr);
    }
    if (name == "RRH") {
        int n = static_cast<int>(evi(0));
        int Tr = (args.size() == 2) ? static_cast<int>(evi(1)) : T;
        return RRH(n, Tr);
    }
    if (name == "RRGstar") {
        int n = static_cast<int>(evi(0));
        int Tr = (args.size() == 2) ? static_cast<int>(evi(1)) : T;
        return RRGstar(n, Tr);
    }
    if (name == "RRHstar") {
        int n = static_cast<int>(evi(0));
        int Tr = (args.size() == 2) ? static_cast<int>(evi(1)) : T;
        return RRHstar(n, Tr);
    }
    if (name == "geta") {
        if (args.size() != 4)
            throw std::runtime_error(runtimeErr(name, "expects geta(g,d,n,T)"));
        return geta(static_cast<int>(evi(0)), static_cast<int>(evi(1)),
                   static_cast<int>(evi(2)), static_cast<int>(evi(3)));
    }
    if (name == "checkid") {
        if (args.size() < 2 || args.size() > 3)
            throw std::runtime_error(runtimeErr(name, "expects checkid(expr,T) or checkid(expr,T,acc)"));
        Series f = ev(0);
        int Tr = static_cast<int>(evi(1));
        int acc = (args.size() == 3) ? static_cast<int>(evi(2)) : 10;
        auto r = checkid(f, Tr, acc);
        if (!r.ok) {
            std::cout << "not an eta product" << std::endl;
        } else {
            std::string prefix;
            if (r.ldq != 0)
                prefix = "q^" + std::to_string(r.ldq) + " * ";
            else
                prefix = "";
            std::cout << prefix << formatEtamake(r.eta) << std::endl;
        }
        return DisplayOnly{};
    }
    if (name == "newprodmake") {
        if (args.size() != 2)
            throw std::runtime_error(runtimeErr(name, "expects 2 arguments"));
        auto r = newprodmake(ev(0), static_cast<int>(evi(1)));
        std::ostringstream lead;
        if (r.lead_exp == 0)
            lead << r.lead_coeff.str();
        else if (r.lead_coeff == Frac(1))
            lead << "q^" << r.lead_exp;
        else if (r.lead_coeff == Frac(-1))
            lead << "-q^" << r.lead_exp;
        else
            lead << r.lead_coeff.str() << "*q^" << r.lead_exp;
        std::cout << lead.str() << " * " << formatProdmake(r.a, true) << std::endl;
        return DisplayOnly{};
    }
    if (name == "dilly") {
        if (args.size() != 2)
            throw std::runtime_error(runtimeErr(name, "expects 2 arguments"));
        return dilly(ev(0), static_cast<int>(evi(1)));
    }
    if (name == "sieveqcheck") {
        if (args.size() != 2)
            throw std::runtime_error(runtimeErr(name, "expects 2 arguments"));
        return static_cast<int64_t>(sieveqcheck(ev(0), static_cast<int>(evi(1))) ? 1 : 0);
    }
    if (name == "siftfindrange") {
        if (args.size() != 3)
            throw std::runtime_error(runtimeErr(name, "expects 3 arguments"));
        auto r = siftfindrange(ev(0), static_cast<int>(evi(1)), static_cast<int>(evi(2)));
        std::cout << "minnops=" << r.min_count << " minres=" << r.min_res
                  << " average=" << r.avg << " maxnops=" << r.max_count << std::endl;
        return DisplayOnly{};
    }
    if (name == "polyfind") {
        if (args.size() != 4)
            throw std::runtime_error(runtimeErr(name, "expects 4 arguments"));
        auto r = polyfind(evi(0), evi(1), evi(2), evi(3));
        if (!r.ok) {
            std::cout << "polyfind: no integer quadratic fit" << std::endl;
            return DisplayOnly{};
        }
        std::cout << "a b c: " << r.a.str() << " " << r.b.str() << " " << r.c.str() << std::endl;
        return DisplayOnly{};
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
        if (args.size() == 2)
            return etamake(ev(0), static_cast<int>(evi(1)));
        if (args.size() == 3)
            return etamake(ev(0), static_cast<int>(evi(2)));
        throw std::runtime_error(runtimeErr(name, "expects 2 or 3 arguments"));
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
        std::cout << "qseries 4.1\n";
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
    auto evalToPartition = [&](const Expr* e) -> Partition {
        if (e && e->tag == Expr::Tag::List) {
            Partition ptn;
            for (const auto& elem : e->elements)
                ptn.parts.push_back(evalToInt(elem.get(), env, sumIndices));
            return ptn;
        }
        if (e && e->tag == Expr::Tag::Var) {
            auto it = env.env.find(e->varName);
            if (it != env.env.end() && std::holds_alternative<Partition>(it->second))
                return std::get<Partition>(it->second);
        }
        EvalResult r = eval(e, env, sumIndices);
        if (std::holds_alternative<Partition>(r))
            return std::get<Partition>(r);
        throw std::runtime_error(runtimeErr(name, "expected partition (integer list)"));
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
    if (name == "findhommodp") {
        if (args.size() != 4)
            throw std::runtime_error(runtimeErr(name, "expects 4 arguments"));
        auto L = evalListToSeries(args[0].get());
        int p = static_cast<int>(evi(1));
        int n = static_cast<int>(evi(2));
        int topshift = static_cast<int>(evi(3));
        auto basis = findhommodp(L, p, n, topshift);
        std::vector<std::vector<int>> exps;
        enumerate_hom_exponents(static_cast<int>(L.size()), n, {}, exps);
        std::vector<std::vector<Frac>> frac_basis;
        for (const auto& v : basis) {
            std::vector<Frac> fv;
            for (int64_t x : v) fv.push_back(Frac(x));
            frac_basis.push_back(std::move(fv));
        }
        return RelationKernelResult{std::move(frac_basis), std::move(exps)};
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
    if (name == "modp") {
        if (args.size() != 2)
            throw std::runtime_error(runtimeErr(name, "expects 2 arguments"));
        Series f = ev(0);
        int p = static_cast<int>(evi(1));
        return f.modp(p);
    }
    if (name == "nterms") {
        if (args.size() != 1)
            throw std::runtime_error(runtimeErr(name, "expects 1 argument"));
        Series f = ev(0);
        return static_cast<int64_t>(f.c.size());
    }
    if (name == "mod") {
        if (args.size() != 2)
            throw std::runtime_error(runtimeErr(name, "expects 2 arguments"));
        int64_t a = evi(0), b = evi(1);
        if (b == 0) throw std::runtime_error(runtimeErr(name, "modulus cannot be zero"));
        int64_t r = a % b;
        if (r < 0) r += (b < 0 ? -b : b);
        return r;
    }
    if (name == "mobius") {
        if (args.size() != 1)
            throw std::runtime_error(runtimeErr(name, "expects 1 argument"));
        return static_cast<int64_t>(mobius(static_cast<int>(evi(0))));
    }
    if (name == "mockqs") {
        if (args.size() != 3)
            throw std::runtime_error(runtimeErr(name, "expects 3 arguments: mockqs(name, order, T)"));
        if (args[0]->tag != Expr::Tag::Var)
            throw std::runtime_error(runtimeErr(name, "first argument must be a mock theta function name (e.g. f3, phi3)"));
        std::string fn = args[0]->varName;
        int ord = static_cast<int>(evi(1));
        int Tr = static_cast<int>(evi(2));
        if (!mock_valid(fn, ord))
            throw std::runtime_error(runtimeErr(name, "unknown mock theta function " + fn + " of order " + std::to_string(ord)));
        return mockqs(fn, ord, Tr);
    }
    if (name == "mockdesorder") {
        if (args.size() != 1)
            throw std::runtime_error(runtimeErr(name, "expects 1 argument"));
        int m = static_cast<int>(evi(0));
        auto list = mockdesorder(m);
        if (list.empty())
            throw std::runtime_error(runtimeErr(name, "order must be 2, 3, 5, 6, 7, 8, or 10"));
        std::cout << "There are " << list.size() << " mock theta functions of order " << m << ":" << std::endl;
        for (size_t i = 0; i < list.size(); ++i)
            std::cout << (i ? ", " : "") << list[i];
        std::cout << std::endl;
        return DisplayOnly{};
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
    if (name == "DELTA12") {
        if (args.size() != 1) throw std::runtime_error(runtimeErr(name, "expects DELTA12(T)"));
        return DELTA12(static_cast<int>(evi(0)));
    }
    if (name == "EISENq") {
        if (args.size() != 2)
            throw std::runtime_error(runtimeErr(name, "expects 2 arguments"));
        return EISENq(static_cast<int>(evi(0)), static_cast<int>(evi(1)));
    }
    if (name == "makebasisM") {
        if (args.size() != 2) throw std::runtime_error(runtimeErr(name, "expects makebasisM(k, T)"));
        auto basis = makebasisM(static_cast<int>(evi(0)), static_cast<int>(evi(1)));
        std::cout << "[" << basis.size() << " basis elements]" << std::endl;
        for (size_t i = 0; i < basis.size(); ++i)
            std::cout << "  " << (i + 1) << ": " << basis[i].str() << std::endl;
        return DisplayOnly{};
    }
    if (name == "makebasisPX") {
        if (args.size() != 2) throw std::runtime_error(runtimeErr(name, "expects makebasisPX(k, T)"));
        auto basis = makebasisPX(static_cast<int>(evi(0)), static_cast<int>(evi(1)));
        std::cout << "[" << basis.size() << " basis elements]" << std::endl;
        for (size_t i = 0; i < basis.size(); ++i)
            std::cout << "  " << (i + 1) << ": " << basis[i].str() << std::endl;
        return DisplayOnly{};
    }
    if (name == "QP2") {
        if (args.size() != 2) throw std::runtime_error(runtimeErr(name, "expects QP2(num, den)"));
        Frac x(static_cast<int64_t>(evi(0)), static_cast<int64_t>(evi(1)));
        std::cout << QP2(x).str() << std::endl;
        return DisplayOnly{};
    }
    if (name == "getacuspord") {
        if (args.size() != 4) throw std::runtime_error(runtimeErr(name, "expects getacuspord(n, r, a, c)"));
        Frac ord = getacuspord(static_cast<int>(evi(0)), static_cast<int>(evi(1)),
                              static_cast<int>(evi(2)), static_cast<int>(evi(3)));
        std::cout << ord.str() << std::endl;
        return DisplayOnly{};
    }
    if (name == "cuspmake1") {
        if (args.size() != 1) throw std::runtime_error(runtimeErr(name, "expects cuspmake1(N)"));
        auto cusps = cuspmake1(static_cast<int>(evi(0)));
        std::cout << "{";
        bool first = true;
        for (const auto& [a, c] : cusps) {
            if (!first) std::cout << ", ";
            if (c == 0) std::cout << "oo";
            else std::cout << a << "/" << c;
            first = false;
        }
        std::cout << "}" << std::endl;
        return DisplayOnly{};
    }
    if (name == "Gamma1ModFunc") {
        if (args.size() != 2) throw std::runtime_error(runtimeErr(name, "expects Gamma1ModFunc(L, N)"));
        const Expr* listExpr = args[0].get();
        if (!listExpr || listExpr->tag != Expr::Tag::List)
            throw std::runtime_error(runtimeErr(name, "L must be list of [n,a,c]"));
        std::vector<std::vector<int>> L;
        for (const auto& elem : listExpr->elements) {
            if (!elem || elem->tag != Expr::Tag::List || elem->elements.size() < 3)
                throw std::runtime_error(runtimeErr(name, "each element must be [n,a,c]"));
            L.push_back({static_cast<int>(evalToInt(elem->elements[0].get(), env, sumIndices)),
                         static_cast<int>(evalToInt(elem->elements[1].get(), env, sumIndices)),
                         static_cast<int>(evalToInt(elem->elements[2].get(), env, sumIndices))});
        }
        int N = static_cast<int>(evi(1));
        int r = Gamma1ModFunc(L, N);
        std::cout << (r ? "1" : "0") << std::endl;
        return static_cast<int64_t>(r);
    }
    if (name == "jac2eprod") {
        if (args.size() != 1) throw std::runtime_error(runtimeErr(name, "expects jac2eprod(var)"));
        std::string vname = args[0]->tag == Expr::Tag::Var ? args[0]->varName : "";
        if (vname.empty()) throw std::runtime_error(runtimeErr(name, "argument must be variable name"));
        auto it = env.env.find(vname);
        if (it == env.env.end()) throw std::runtime_error("undefined variable: " + vname);
        if (!std::holds_alternative<std::vector<JacFactor>>(it->second))
            throw std::runtime_error(vname + " must hold Jacobi product (from jacprodmake)");
        auto geta = jac2eprod(std::get<std::vector<JacFactor>>(it->second));
        std::ostringstream os;
        for (size_t i = 0; i < geta.size(); ++i) {
            auto [n, a, c] = geta[i];
            if (i) os << " ";
            if (a == 0) os << "EETA(" << n << ")^" << c.str();
            else os << "GETA(" << n << "," << a << ")^" << c.str();
        }
        std::cout << os.str() << std::endl;
        return DisplayOnly{};
    }
    if (name == "eta2jac") {
        if (args.size() != 1) throw std::runtime_error(runtimeErr(name, "expects eta2jac(getalist)"));
        const Expr* listExpr = args[0].get();
        if (!listExpr || listExpr->tag != Expr::Tag::List)
            throw std::runtime_error(runtimeErr(name, "getalist must be list of [n,a,c]"));
        std::vector<std::tuple<int, int, Frac>> geta;
        for (const auto& elem : listExpr->elements) {
            if (!elem || elem->tag != Expr::Tag::List || elem->elements.size() < 3)
                throw std::runtime_error(runtimeErr(name, "each element must be [n,a,c]"));
            int n = static_cast<int>(evalToInt(elem->elements[0].get(), env, sumIndices));
            int a = static_cast<int>(evalToInt(elem->elements[1].get(), env, sumIndices));
            Frac c(static_cast<int64_t>(evalToInt(elem->elements[2].get(), env, sumIndices)));
            geta.push_back({n, a, c});
        }
        std::vector<JacFactor> jac = eta2jac(geta);
        std::cout << jac2prod(jac) << std::endl;
        return jac;
    }
    if (name == "Phiq") {
        if (args.size() != 2)
            throw std::runtime_error(runtimeErr(name, "expects 2 arguments"));
        return Phiq(static_cast<int>(evi(0)), static_cast<int>(evi(1)));
    }
    if (name == "rankgf") {
        if (args.size() != 2)
            throw std::runtime_error(runtimeErr(name, "expects 2 arguments: rankgf(m, T)"));
        return rankgf(static_cast<int>(evi(0)), static_cast<int>(evi(1)));
    }
    if (name == "crankgf") {
        if (args.size() != 2)
            throw std::runtime_error(runtimeErr(name, "expects 2 arguments: crankgf(m, T)"));
        return crankgf(static_cast<int>(evi(0)), static_cast<int>(evi(1)));
    }
    if (name == "NS") {
        if (args.size() == 2)
            return static_cast<int64_t>(NS(static_cast<int>(evi(0)), static_cast<int>(evi(1))));
        if (args.size() == 3)
            return static_cast<int64_t>(sptcrankresnum(static_cast<int>(evi(0)), static_cast<int>(evi(1)), static_cast<int>(evi(2))));
        throw std::runtime_error(runtimeErr(name, "expects NS(m,n) or NS(k,r,n)"));
    }
    if (name == "MBAR") {
        if (args.size() == 2)
            return static_cast<int64_t>(MBAR(static_cast<int>(evi(0)), static_cast<int>(evi(1))));
        if (args.size() == 3)
            return static_cast<int64_t>(ocrankresnum(static_cast<int>(evi(0)), static_cast<int>(evi(1)), static_cast<int>(evi(2))));
        throw std::runtime_error(runtimeErr(name, "expects MBAR(m,n) or MBAR(k,r,n)"));
    }
    if (name == "M2N") {
        if (args.size() == 2)
            return static_cast<int64_t>(M2N(static_cast<int>(evi(0)), static_cast<int>(evi(1))));
        if (args.size() == 3)
            return static_cast<int64_t>(m2rankresnum(static_cast<int>(evi(0)), static_cast<int>(evi(1)), static_cast<int>(evi(2))));
        throw std::runtime_error(runtimeErr(name, "expects M2N(m,n) or M2N(k,r,n)"));
    }
    if (name == "GFDM2N") {
        if (args.size() < 4 || args.size() > 5)
            throw std::runtime_error(runtimeErr(name, "expects GFDM2N(k1,k2,t,r) or GFDM2N(k1,k2,t,r,T)"));
        int k1 = static_cast<int>(evi(0)), k2 = static_cast<int>(evi(1));
        int t = static_cast<int>(evi(2)), r = static_cast<int>(evi(3));
        int Tr = (args.size() == 5) ? static_cast<int>(evi(4)) : 50;
        return GFDM2N(k1, k2, t, r, Tr);
    }
    if (name == "sptcrankresnum") {
        if (args.size() != 3) throw std::runtime_error(runtimeErr(name, "expects 3 arguments"));
        return static_cast<int64_t>(sptcrankresnum(static_cast<int>(evi(0)), static_cast<int>(evi(1)), static_cast<int>(evi(2))));
    }
    if (name == "ocrankresnum") {
        if (args.size() != 3) throw std::runtime_error(runtimeErr(name, "expects 3 arguments"));
        return static_cast<int64_t>(ocrankresnum(static_cast<int>(evi(0)), static_cast<int>(evi(1)), static_cast<int>(evi(2))));
    }
    if (name == "pbar") {
        if (args.size() != 1) throw std::runtime_error(runtimeErr(name, "expects 1 argument"));
        return static_cast<int64_t>(pbar(static_cast<int>(evi(0))));
    }
    if (name == "betafind") {
        if (args.size() < 1 || args.size() > 2) throw std::runtime_error(runtimeErr(name, "expects betafind(n) or betafind(n,T)"));
        int n = static_cast<int>(evi(0));
        int Tr = (args.size() >= 2) ? static_cast<int>(evi(1)) : T;
        Series a = Series::one(Tr);
        return betafind(unit_alpha, a, q, n, Tr);
    }
    if (name == "alphaup") {
        if (args.size() != 2) throw std::runtime_error(runtimeErr(name, "expects alphaup(alpha,T)"));
        Series alpha = ev(0);
        int Tr = static_cast<int>(evi(1));
        Series a = Series::one(Tr);
        return alphaup(alpha, a, q, Tr);
    }
    if (name == "alphadown") {
        if (args.size() != 2) throw std::runtime_error(runtimeErr(name, "expects alphadown(alpha,T)"));
        Series alpha = ev(0);
        int Tr = static_cast<int>(evi(1));
        Series a = Series::one(Tr);
        return alphadown(alpha, a, q, Tr);
    }
    if (name == "bailey_sum") {
        if (args.size() != 3) throw std::runtime_error(runtimeErr(name, "expects bailey_sum(pair,N,T)"));
        int pair = static_cast<int>(evi(0));
        int N = static_cast<int>(evi(1));
        int Tr = static_cast<int>(evi(2));
        Series a = Series::one(Tr);
        BetaFunc bf = (pair == 1) ? rr_beta : (pair == 2) ? gg_beta : unit_beta;
        return bailey_sum(bf, a, q, N, Tr);
    }
    auto evalListToIntVec = [&](const Expr* listExpr) {
        if (!listExpr || listExpr->tag != Expr::Tag::List)
            throw std::runtime_error(runtimeErr(name, "expected list of integers"));
        std::vector<int> out;
        for (const auto& e : listExpr->elements)
            out.push_back(static_cast<int>(evalToInt(e.get(), env, sumIndices)));
        return out;
    };
    if (name == "cuspmake") {
        if (args.size() != 1) throw std::runtime_error(runtimeErr(name, "expects cuspmake(N)"));
        int N = static_cast<int>(evi(0));
        auto cusps = cuspmake(N);
        std::cout << "{";
        bool first = true;
        for (const auto& [d, c] : cusps) {
            if (!first) std::cout << ", ";
            if (c == 1) std::cout << d; else std::cout << d << "/" << c;
            first = false;
        }
        std::cout << "}" << std::endl;
        return DisplayOnly{};
    }
    if (name == "cuspord") {
        if (args.size() != 2) throw std::runtime_error(runtimeErr(name, "expects cuspord(GP, cusp)"));
        std::vector<int> GP = evalListToIntVec(args[0].get());
        std::vector<int> cusp = evalListToIntVec(args[1].get());
        if (cusp.size() < 2) throw std::runtime_error(runtimeErr(name, "cusp must be [num,den]"));
        Frac ord = cuspord(GP, cusp[0], cusp[1]);
        std::cout << ord.str() << std::endl;
        return DisplayOnly{};
    }
    if (name == "gammacheck") {
        if (args.size() != 2) throw std::runtime_error(runtimeErr(name, "expects gammacheck(GP, N)"));
        std::vector<int> GP = evalListToIntVec(args[0].get());
        int N = static_cast<int>(evi(1));
        return static_cast<int64_t>(gammacheck(GP, N));
    }
    if (name == "etaprodtoqseries") {
        if (args.size() != 2) throw std::runtime_error(runtimeErr(name, "expects etaprodtoqseries(GP, T)"));
        std::vector<int> GP = evalListToIntVec(args[0].get());
        int Tr = static_cast<int>(evi(1));
        return etaprodtoqseries(GP, Tr);
    }
    if (name == "vp") {
        if (args.size() != 2) throw std::runtime_error(runtimeErr(name, "expects vp(n, p)"));
        return static_cast<int64_t>(vp(static_cast<int>(evi(0)), static_cast<int>(evi(1))));
    }
    if (name == "provemodfuncGAMMA0id") {
        if (args.size() != 2) throw std::runtime_error(runtimeErr(name, "expects provemodfuncGAMMA0id(etaid, N)"));
        const Expr* listExpr = args[0].get();
        if (!listExpr || listExpr->tag != Expr::Tag::List)
            throw std::runtime_error(runtimeErr(name, "etaid must be list of [c, GP] terms"));
        std::vector<std::pair<Frac, std::vector<int>>> etaid;
        for (const auto& term : listExpr->elements) {
            if (!term || term->tag != Expr::Tag::List || term->elements.size() < 1)
                throw std::runtime_error(runtimeErr(name, "each term must be [c] or [c, t1, r1, ...]"));
            Frac c = Frac(static_cast<int64_t>(evalToInt(term->elements[0].get(), env, sumIndices)));
            std::vector<int> GP;
            if (term->elements.size() >= 2) {
                for (size_t i = 1; i < term->elements.size(); ++i)
                    GP.push_back(static_cast<int>(evalToInt(term->elements[i].get(), env, sumIndices)));
            }
            etaid.push_back({c, GP});
        }
        int N = static_cast<int>(evi(1));
        ProveModfuncResult res = provemodfuncGAMMA0id(etaid, N);
        std::cout << "sturm_bound=" << res.sturm_bound << " proven=" << res.proven << " " << res.message << std::endl;
        return static_cast<int64_t>(res.proven);
    }
    if (name == "provemodfuncGAMMA0idBATCH") {
        if (args.size() != 2) throw std::runtime_error(runtimeErr(name, "expects provemodfuncGAMMA0idBATCH(etaids, N)"));
        const Expr* listExpr = args[0].get();
        if (!listExpr || listExpr->tag != Expr::Tag::List)
            throw std::runtime_error(runtimeErr(name, "etaids must be list of etaid terms"));
        std::vector<std::vector<std::pair<Frac, std::vector<int>>>> etaids;
        for (const auto& etaidExpr : listExpr->elements) {
            if (!etaidExpr || etaidExpr->tag != Expr::Tag::List)
                throw std::runtime_error(runtimeErr(name, "each etaid must be list of [c, GP] terms"));
            std::vector<std::pair<Frac, std::vector<int>>> etaid;
            for (const auto& term : etaidExpr->elements) {
                if (!term || term->tag != Expr::Tag::List || term->elements.size() < 1)
                    throw std::runtime_error(runtimeErr(name, "each term must be [c] or [c, t1, r1, ...]"));
                Frac c = Frac(static_cast<int64_t>(evalToInt(term->elements[0].get(), env, sumIndices)));
                std::vector<int> GP;
                if (term->elements.size() >= 2) {
                    for (size_t i = 1; i < term->elements.size(); ++i)
                        GP.push_back(static_cast<int>(evalToInt(term->elements[i].get(), env, sumIndices)));
                }
                etaid.push_back({c, GP});
            }
            etaids.push_back(std::move(etaid));
        }
        int N = static_cast<int>(evi(1));
        auto results = provemodfuncGAMMA0idBATCH(etaids, N);
        int all_proven = 1;
        for (size_t i = 0; i < results.size(); ++i) {
            std::cout << "id[" << (i+1) << "] sturm_bound=" << results[i].sturm_bound
                      << " proven=" << results[i].proven << " " << results[i].message << std::endl;
            if (results[i].proven != 1) all_proven = 0;
        }
        return static_cast<int64_t>(all_proven);
    }
    if (name == "provemodfuncid") {
        if (args.size() != 2) throw std::runtime_error(runtimeErr(name, "expects provemodfuncid(jacid, N)"));
        const Expr* listExpr = args[0].get();
        if (!listExpr || listExpr->tag != Expr::Tag::List)
            throw std::runtime_error(runtimeErr(name, "jacid must be list of [coeff, jaclist] terms"));
        std::vector<std::pair<Frac, std::vector<JacFactor>>> jacid;
        for (const auto& term : listExpr->elements) {
            if (!term || term->tag != Expr::Tag::List || term->elements.size() < 1)
                throw std::runtime_error(runtimeErr(name, "each term must be [coeff] or [coeff, [a,b,exp], ...]"));
            Frac c = Frac(static_cast<int64_t>(evalToInt(term->elements[0].get(), env, sumIndices)));
            std::vector<JacFactor> jac;
            if (term->elements.size() >= 2) {
                for (size_t i = 1; i < term->elements.size(); ++i) {
                    const Expr* fac = term->elements[i].get();
                    if (!fac || fac->tag != Expr::Tag::List || fac->elements.size() < 3)
                        throw std::runtime_error(runtimeErr(name, "each JacFactor must be [a, b, exp]"));
                    int a = static_cast<int>(evalToInt(fac->elements[0].get(), env, sumIndices));
                    int b = static_cast<int>(evalToInt(fac->elements[1].get(), env, sumIndices));
                    Frac e = Frac(static_cast<int64_t>(evalToInt(fac->elements[2].get(), env, sumIndices)));
                    jac.push_back({a, b, e});
                }
            }
            jacid.push_back({c, jac});
        }
        int N = static_cast<int>(evi(1));
        ProveModfuncIdResult res = provemodfuncid(jacid, N);
        std::cout << "sturm_bound=" << res.sturm_bound << " proven=" << res.proven << " " << res.message << std::endl;
        return static_cast<int64_t>(res.proven);
    }
    if (name == "partition") {
        if (args.size() != 1)
            throw std::runtime_error(runtimeErr(name, "expects 1 argument"));
        Frac p = partition_number(static_cast<int>(evi(0)));
        std::cout << p.str() << std::endl;
        return DisplayOnly{};
    }
    if (name == "partitions") {
        if (args.size() != 1)
            throw std::runtime_error(runtimeErr(name, "expects 1 argument"));
        int n = static_cast<int>(evi(0));
        if (n > 100)
            throw std::runtime_error(runtimeErr(name, "n too large (limit 100)"));
        auto raw = enumerate_partitions(n);
        std::vector<Partition> result;
        result.reserve(raw.size());
        for (auto& v : raw) {
            Partition ptn;
            ptn.parts = std::move(v);
            result.push_back(std::move(ptn));
        }
        return result;
    }
    if (name == "conjpart") {
        if (args.size() != 1)
            throw std::runtime_error(runtimeErr(name, "expects 1 argument"));
        Partition ptn = evalToPartition(args[0].get());
        Partition conj;
        conj.parts = conjugate_partition(ptn.parts);
        return conj;
    }
    if (name == "lp") {
        if (args.size() != 1)
            throw std::runtime_error(runtimeErr(name, "expects 1 argument"));
        Partition ptn = evalToPartition(args[0].get());
        if (ptn.parts.empty()) return static_cast<int64_t>(0);
        return *std::max_element(ptn.parts.begin(), ptn.parts.end());
    }
    if (name == "np_parts") {
        if (args.size() != 1)
            throw std::runtime_error(runtimeErr(name, "expects 1 argument"));
        Partition ptn = evalToPartition(args[0].get());
        return static_cast<int64_t>(ptn.parts.size());
    }
    if (name == "nep") {
        if (args.size() != 1)
            throw std::runtime_error(runtimeErr(name, "expects 1 argument"));
        Partition ptn = evalToPartition(args[0].get());
        int64_t count = 0;
        for (int64_t p : ptn.parts)
            if (p % 2 == 0) ++count;
        return count;
    }
    if (name == "ptnnorm") {
        if (args.size() != 1)
            throw std::runtime_error(runtimeErr(name, "expects 1 argument"));
        Partition ptn = evalToPartition(args[0].get());
        int64_t sum = 0;
        for (int64_t p : ptn.parts) sum += p;
        return sum;
    }

    // ---- partition statistics (Phase 76) ----
    if (name == "drank") {
        if (args.size() != 1) throw std::runtime_error(runtimeErr(name, "expects 1 argument"));
        Partition ptn = evalToPartition(args[0].get());
        return ptn_drank(ptn.parts);
    }
    if (name == "agcrank") {
        if (args.size() != 1) throw std::runtime_error(runtimeErr(name, "expects 1 argument"));
        Partition ptn = evalToPartition(args[0].get());
        return ptn_agcrank(ptn.parts);
    }
    if (name == "kapPD") {
        if (args.size() != 1) throw std::runtime_error(runtimeErr(name, "expects 1 argument"));
        Partition ptn = evalToPartition(args[0].get());
        return ptn_kapPD(ptn.parts);
    }
    if (name == "lamPD") {
        if (args.size() != 1) throw std::runtime_error(runtimeErr(name, "expects 1 argument"));
        Partition ptn = evalToPartition(args[0].get());
        return ptn_lamPD(ptn.parts);
    }
    if (name == "numLE") {
        if (args.size() != 1) throw std::runtime_error(runtimeErr(name, "expects 1 argument"));
        Partition ptn = evalToPartition(args[0].get());
        return ptn_numLE(ptn.parts);
    }
    if (name == "ptnDP") {
        if (args.size() != 1) throw std::runtime_error(runtimeErr(name, "expects 1 argument"));
        Partition ptn = evalToPartition(args[0].get());
        return static_cast<int64_t>(ptn_is_distinct(ptn.parts) ? 1 : 0);
    }
    if (name == "ptnOP") {
        if (args.size() != 1) throw std::runtime_error(runtimeErr(name, "expects 1 argument"));
        Partition ptn = evalToPartition(args[0].get());
        return static_cast<int64_t>(ptn_is_odd_parts(ptn.parts) ? 1 : 0);
    }
    if (name == "ptnRR") {
        if (args.size() != 1) throw std::runtime_error(runtimeErr(name, "expects 1 argument"));
        Partition ptn = evalToPartition(args[0].get());
        return static_cast<int64_t>(ptn_is_RR(ptn.parts) ? 1 : 0);
    }
    if (name == "ptnCC") {
        if (args.size() != 1) throw std::runtime_error(runtimeErr(name, "expects 1 argument"));
        Partition ptn = evalToPartition(args[0].get());
        return static_cast<int64_t>(ptn_is_CC(ptn.parts) ? 1 : 0);
    }
    if (name == "ptnSCHUR") {
        if (args.size() != 1) throw std::runtime_error(runtimeErr(name, "expects 1 argument"));
        Partition ptn = evalToPartition(args[0].get());
        return static_cast<int64_t>(ptn_is_SCHUR(ptn.parts) ? 1 : 0);
    }
    if (name == "ptnOE") {
        if (args.size() != 1) throw std::runtime_error(runtimeErr(name, "expects 1 argument"));
        Partition ptn = evalToPartition(args[0].get());
        return static_cast<int64_t>(ptn_is_OE(ptn.parts) ? 1 : 0);
    }
    if (name == "PDP") {
        if (args.size() != 1) throw std::runtime_error(runtimeErr(name, "expects 1 argument"));
        return count_PDP(static_cast<int>(evi(0)));
    }
    if (name == "POE") {
        if (args.size() != 1) throw std::runtime_error(runtimeErr(name, "expects 1 argument"));
        return count_POE(static_cast<int>(evi(0)));
    }
    if (name == "PRR") {
        if (args.size() != 1) throw std::runtime_error(runtimeErr(name, "expects 1 argument"));
        return count_PRR(static_cast<int>(evi(0)));
    }
    if (name == "PSCHUR") {
        if (args.size() != 1) throw std::runtime_error(runtimeErr(name, "expects 1 argument"));
        return count_PSCHUR(static_cast<int>(evi(0)));
    }
    if (name == "overptns") {
        if (args.size() != 1) throw std::runtime_error(runtimeErr(name, "expects 1 argument"));
        int n = static_cast<int>(evi(0));
        auto ops = enumerate_overpartitions(n);
        for (const auto& [dp, p] : ops) {
            std::cout << "[" << formatPartition({dp}) << ", " << formatPartition({p}) << "]" << std::endl;
        }
        std::cout << "Total: " << ops.size() << " overpartitions" << std::endl;
        return DisplayOnly{};
    }
    if (name == "overptnrank") {
        if (args.size() != 2) throw std::runtime_error(runtimeErr(name, "expects 2 arguments: overptnrank(dptn, ptn)"));
        Partition dptn = evalToPartition(args[0].get());
        Partition ptn = evalToPartition(args[1].get());
        return overptn_rank(dptn.parts, ptn.parts);
    }
    if (name == "overptncrank") {
        if (args.size() != 2) throw std::runtime_error(runtimeErr(name, "expects 2 arguments: overptncrank(dptn, ptn)"));
        Partition dptn = evalToPartition(args[0].get());
        Partition ptn = evalToPartition(args[1].get());
        return overptn_crank(dptn.parts, ptn.parts);
    }

    // ---- t-core functions ----
    if (name == "rvec") {
        if (args.size() != 3)
            throw std::runtime_error(runtimeErr(name, "expects 3 arguments: rvec(ptn, t, k)"));
        Partition ptn = evalToPartition(args[0].get());
        int t = static_cast<int>(evi(1));
        int k = static_cast<int>(evi(2));
        if (t <= 0) throw std::runtime_error(runtimeErr(name, "t must be positive"));
        return tcore_rvec(ptn.parts, t, k);
    }
    if (name == "istcore") {
        if (args.size() != 2)
            throw std::runtime_error(runtimeErr(name, "expects 2 arguments: istcore(ptn, t)"));
        Partition ptn = evalToPartition(args[0].get());
        int t = static_cast<int>(evi(1));
        if (t <= 0) throw std::runtime_error(runtimeErr(name, "t must be positive"));
        return static_cast<int64_t>(tcore_istcore(ptn.parts, t) ? 1 : 0);
    }
    if (name == "tcoreofptn") {
        if (args.size() != 2)
            throw std::runtime_error(runtimeErr(name, "expects 2 arguments: tcoreofptn(ptn, t)"));
        Partition ptn = evalToPartition(args[0].get());
        int t = static_cast<int>(evi(1));
        if (t <= 0) throw std::runtime_error(runtimeErr(name, "t must be positive"));
        Partition core;
        core.parts = tcore_tcoreofptn(ptn.parts, t);
        return core;
    }
    if (name == "tcores") {
        if (args.size() != 2)
            throw std::runtime_error(runtimeErr(name, "expects 2 arguments: tcores(t, n)"));
        int t = static_cast<int>(evi(0));
        int n = static_cast<int>(evi(1));
        if (t <= 0) throw std::runtime_error(runtimeErr(name, "t must be positive"));
        if (n > 100) throw std::runtime_error(runtimeErr(name, "n too large (limit 100)"));
        auto all = enumerate_partitions(n);
        auto cores = tcore_tcores(t, n, all);
        std::vector<Partition> result;
        result.reserve(cores.size());
        for (auto& v : cores) {
            Partition p;
            p.parts = std::move(v);
            result.push_back(std::move(p));
        }
        return result;
    }
    if (name == "ptn2nvec") {
        if (args.size() != 2)
            throw std::runtime_error(runtimeErr(name, "expects 2 arguments: ptn2nvec(ptn, t)"));
        Partition ptn = evalToPartition(args[0].get());
        int t = static_cast<int>(evi(1));
        if (t <= 0) throw std::runtime_error(runtimeErr(name, "t must be positive"));
        Partition nvec;
        nvec.parts = tcore_ptn2nvec(ptn.parts, t);
        return nvec;
    }
    if (name == "ptn2rvec") {
        if (args.size() != 2)
            throw std::runtime_error(runtimeErr(name, "expects 2 arguments: ptn2rvec(ptn, t)"));
        Partition ptn = evalToPartition(args[0].get());
        int t = static_cast<int>(evi(1));
        if (t <= 0) throw std::runtime_error(runtimeErr(name, "t must be positive"));
        Partition rvec;
        rvec.parts = tcore_ptn2rvec(ptn.parts, t);
        return rvec;
    }
    if (name == "nvec2ptn") {
        if (args.size() != 1)
            throw std::runtime_error(runtimeErr(name, "expects 1 argument: nvec2ptn(nvec)"));
        Partition nvec = evalToPartition(args[0].get());
        Partition result;
        result.parts = tcore_nvec2ptn(nvec.parts);
        return result;
    }
    if (name == "nvec2alphavec") {
        if (args.size() != 1)
            throw std::runtime_error(runtimeErr(name, "expects 1 argument: nvec2alphavec(nvec)"));
        Partition nvec = evalToPartition(args[0].get());
        int t = static_cast<int>(nvec.parts.size());
        if (t == 5) {
            int64_t n0 = nvec.parts[0], n1 = nvec.parts[1], n2 = nvec.parts[2],
                    n3 = nvec.parts[3];
            // Hardcoded rational linear formulas from Garvan's tcore package
            Frac a3 = Frac(1,5)*Frac(n0) + Frac(1,5) + Frac(3,5)*Frac(n2)
                     + Frac(2,5)*Frac(n1) + Frac(4,5)*Frac(n3);
            Frac a0 = Frac(-1,5)*Frac(n3) + Frac(1,5)*Frac(n0) + Frac(1,5)
                     - Frac(2,5)*Frac(n2) - Frac(3,5)*Frac(n1);
            Frac a2 = Frac(-1,5)*Frac(n1) - Frac(2,5)*Frac(n3) - Frac(3,5)*Frac(n0)
                     + Frac(2,5) + Frac(1,5)*Frac(n2);
            Frac a1 = Frac(-4,5)*Frac(n2) - Frac(1,5)*Frac(n1) - Frac(2,5)*Frac(n3)
                     - Frac(3,5)*Frac(n0) + Frac(2,5);
            Frac a4 = Frac(1) - a0 - a1 - a2 - a3;
            std::cout << "[" << a0.str() << ", " << a1.str() << ", " << a2.str()
                      << ", " << a3.str() << ", " << a4.str() << "]" << std::endl;
            return DisplayOnly{};
        }
        if (t == 7) {
            int64_t n0 = nvec.parts[0], n1 = nvec.parts[1], n2 = nvec.parts[2],
                    n3 = nvec.parts[3], n4 = nvec.parts[4], n5 = nvec.parts[5];
            Frac a3 = Frac(6,7)*Frac(n1) + Frac(5,7)*Frac(n3) + Frac(1,7)*Frac(n4)
                     + Frac(3,7)*Frac(n0) - Frac(1,7) + Frac(2,7)*Frac(n2) + Frac(4,7)*Frac(n5);
            Frac a4 = Frac(-1,7)*Frac(n4) - Frac(3,7)*Frac(n0) + Frac(1,7)
                     - Frac(2,7)*Frac(n2) - Frac(4,7)*Frac(n5) + Frac(2,7)*Frac(n3) + Frac(1,7)*Frac(n1);
            Frac a0 = Frac(-1,7)*Frac(n5) + Frac(1,7)*Frac(n0) - Frac(2,7)*Frac(n4)
                     + Frac(2,7) - Frac(4,7)*Frac(n2) - Frac(3,7)*Frac(n3) - Frac(5,7)*Frac(n1);
            Frac a2 = Frac(3,7)*Frac(n2) + Frac(6,7)*Frac(n5) + Frac(1,7)*Frac(n0)
                     + Frac(5,7)*Frac(n4) + Frac(2,7) + Frac(4,7)*Frac(n3) + Frac(2,7)*Frac(n1);
            Frac a1 = Frac(-2,7)*Frac(n0) + Frac(3,7) + Frac(1,7)*Frac(n2)
                     + Frac(2,7)*Frac(n5) - Frac(3,7)*Frac(n4) - Frac(1,7)*Frac(n3) - Frac(4,7)*Frac(n1);
            Frac a5 = Frac(-5,7)*Frac(n3) - Frac(1,7)*Frac(n4) - Frac(3,7)*Frac(n0)
                     + Frac(1,7) - Frac(2,7)*Frac(n2) - Frac(4,7)*Frac(n5) + Frac(1,7)*Frac(n1);
            Frac a6 = Frac(1) - a0 - a1 - a2 - a3 - a4 - a5;
            std::cout << "[" << a0.str() << ", " << a1.str() << ", " << a2.str()
                      << ", " << a3.str() << ", " << a4.str() << ", " << a5.str()
                      << ", " << a6.str() << "]" << std::endl;
            return DisplayOnly{};
        }
        throw std::runtime_error(runtimeErr(name, "only implemented for t=5 and t=7"));
    }
    if (name == "tquot") {
        if (args.size() != 2)
            throw std::runtime_error(runtimeErr(name, "expects 2 arguments: tquot(ptn, t)"));
        Partition ptn = evalToPartition(args[0].get());
        int t = static_cast<int>(evi(1));
        if (t <= 0) throw std::runtime_error(runtimeErr(name, "t must be positive"));
        auto raw = tcore_tquot(ptn.parts, t);
        std::vector<Partition> result;
        for (auto& v : raw) {
            Partition p;
            p.parts = std::move(v);
            result.push_back(std::move(p));
        }
        return result;
    }
    if (name == "phi1") {
        if (args.size() != 2)
            throw std::runtime_error(runtimeErr(name, "expects 2 arguments: phi1(ptn, t)"));
        Partition ptn = evalToPartition(args[0].get());
        int t = static_cast<int>(evi(1));
        if (t <= 0) throw std::runtime_error(runtimeErr(name, "t must be positive"));
        Phi1Result res;
        res.core.parts = tcore_tcoreofptn(ptn.parts, t);
        auto raw_quot = tcore_tquot(ptn.parts, t);
        for (auto& v : raw_quot) {
            Partition p;
            p.parts = std::move(v);
            res.quotient.push_back(std::move(p));
        }
        return res;
    }
    if (name == "invphi1") {
        if (args.size() != 2)
            throw std::runtime_error(runtimeErr(name, "expects 2 arguments: invphi1(phi1result, t)"));
        int t = static_cast<int>(evi(1));
        if (t <= 0) throw std::runtime_error(runtimeErr(name, "t must be positive"));
        // First arg: Phi1Result from phi1() or variable
        EvalResult r = eval(args[0].get(), env, sumIndices);
        if (!std::holds_alternative<Phi1Result>(r))
            throw std::runtime_error(runtimeErr(name, "first argument must be a phi1 result"));
        const auto& phi1r = std::get<Phi1Result>(r);
        std::vector<std::vector<int64_t>> quot_raw;
        for (const auto& p : phi1r.quotient)
            quot_raw.push_back(p.parts);
        Partition result;
        result.parts = tcore_invphi1(phi1r.core.parts, quot_raw, t);
        return result;
    }
    if (name == "tcrank") {
        if (args.size() != 2)
            throw std::runtime_error(runtimeErr(name, "expects 2 arguments: tcrank(ptn, t)"));
        Partition ptn = evalToPartition(args[0].get());
        int64_t t = evi(1);
        if (t < 3 || t % 2 == 0)
            throw std::runtime_error(runtimeErr(name, "t must be an odd prime >= 3"));
        int64_t val = tcore_tcrank(ptn.parts, t);
        std::cout << val << std::endl;
        return DisplayOnly{};
    }
    if (name == "tresdiag") {
        if (args.size() != 2)
            throw std::runtime_error(runtimeErr(name, "expects 2 arguments: tresdiag(ptn, t)"));
        Partition ptn = evalToPartition(args[0].get());
        int t = static_cast<int>(evi(1));
        if (t <= 0) throw std::runtime_error(runtimeErr(name, "t must be positive"));
        for (size_t row = 0; row < ptn.parts.size(); ++row) {
            for (int64_t col = 1; col <= ptn.parts[row]; ++col) {
                if (col > 1) std::cout << " ";
                std::cout << safe_mod(col - static_cast<int64_t>(row) - 1, t);
            }
            std::cout << "\n";
        }
        return DisplayOnly{};
    }
    if (name == "makebiw") {
        if (args.size() != 3)
            throw std::runtime_error(runtimeErr(name, "expects 3 arguments: makebiw(ptn, t, m)"));
        Partition ptn = evalToPartition(args[0].get());
        int t = static_cast<int>(evi(1));
        int mj = static_cast<int>(evi(2));
        if (t <= 0) throw std::runtime_error(runtimeErr(name, "t must be positive"));
        if (mj <= 0) throw std::runtime_error(runtimeErr(name, "m must be positive"));
        int np = static_cast<int>(ptn.parts.size());
        int pad = (mj + 1) * t;
        std::vector<int64_t> ip;
        for (int k = 0; k < np; ++k) ip.push_back(ptn.parts[k]);
        for (int k = 0; k < pad; ++k) ip.push_back(0);
        std::map<std::pair<int,int>, char> reg;
        for (int i = -100; i <= 100; ++i)
            for (int k = 0; k < t; ++k)
                reg[{i, k}] = 'N';
        for (size_t j = 0; j < ip.size(); ++j) {
            int64_t a = ip[j];
            int64_t maple_j = static_cast<int64_t>(j) + 1;
            int b = static_cast<int>(safe_mod(a - maple_j, t));
            int r = static_cast<int>(floor_div(a - maple_j, t)) + 1;
            if (r >= -100 && r <= 100) reg[{r, b}] = 'E';
        }
        // Header
        std::cout << "   ";
        for (int r = -mj; r <= mj; ++r) {
            if (r < 0) std::cout << r;
            else std::cout << " " << r;
        }
        std::cout << "\n";
        // Words
        for (int i = 0; i < t; ++i) {
            std::cout << "W" << i << " ";
            for (int r = -mj; r <= mj; ++r) {
                std::cout << " " << reg[{r, i}];
            }
            std::cout << "\n";
        }
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
                std::cout << ansi::gold() << n << ansi::reset();
                first = false;
            }
            std::cout << std::endl;
            return DisplayOnly{};
        }
        if (args.size() == 1 && args[0]->tag == Expr::Tag::Var) {
            const std::string& fn = args[0]->varName;
            auto it = table.find(fn);
            if (it != table.end()) {
                std::cout << ansi::gold() << it->second.first << ansi::reset() << " — " << it->second.second << std::endl;
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
            if (std::holds_alternative<Partition>(ev->second))
                return std::get<Partition>(ev->second);
            if (std::holds_alternative<Phi1Result>(ev->second))
                return std::get<Phi1Result>(ev->second);
            return getSeriesFromEnv(ev->second);
        }
        case Expr::Tag::BinOp: {
            if (e->binOp == BinOp::Pow && isQLike(e->left.get())) {
                try {
                    int64_t iv = evalToInt(e->right.get(), env, sumIndices);
                    return Series::qpow(static_cast<int>(iv), env.T);
                } catch (...) {}
                Series rs = toSeries(eval(e->right.get(), env, sumIndices), "q^exponent", env.T);
                if (rs.c.empty()) return Series::one(env.T);
                if (rs.c.size() == 1 && rs.c.count(0)) {
                    Frac fv = rs.c.at(0);
                    if (fv.den == BigInt(1)) {
                        int iv = fv.num.d.empty() ? 0 : static_cast<int>(fv.num.d[0]);
                        if (fv.num.neg) iv = -iv;
                        return Series::qpow(iv, env.T);
                    }
                    Series s = Series::one(env.T);
                    s.q_shift = fv;
                    return s;
                }
                throw std::runtime_error("q^(expr): exponent must be a constant");
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
                    try {
                        int64_t expVal = evalToInt(e->right.get(), env, sumIndices);
                        if (expVal > 10000 || expVal < -10000)
                            throw std::runtime_error("pow: exponent magnitude too large (limit 10000)");
                        return l.pow(static_cast<int>(expVal));
                    } catch (...) {
                        EvalResult rv = eval(e->right.get(), env, sumIndices);
                        Series rs = toSeries(rv, "pow exponent", env.T);
                        if (rs.c.size() == 1 && rs.c.count(0) && rs.q_shift.isZero()) {
                            return l.powFrac(rs.c.at(0));
                        }
                        throw std::runtime_error("pow: exponent must be an integer or rational constant");
                    }
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
        case Expr::Tag::List: {
            Partition ptn;
            for (const auto& elem : e->elements)
                ptn.parts.push_back(evalToInt(elem.get(), env, sumIndices));
            return ptn;
        }
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

inline std::string formatProdmake(const std::map<int, Frac>& a, bool mapleStyle) {
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
    std::string scalar_str;
    Frac input_q_shift(0);
    Frac eta_q_power(0);
    std::vector<std::string> num_parts, den_parts;
    for (const auto& [k, e] : eta) {
        if (e.isZero()) continue;
        if (k == -1) {
            input_q_shift = e;
            continue;
        }
        if (k == 0) {
            if (!(e == Frac(1))) scalar_str = e.str();
            continue;
        }
        std::string part = (k == 1)
            ? "\xCE\xB7(\xCF\x84)"
            : "\xCE\xB7(" + std::to_string(k) + "\xCF\x84)";
        int ex = 0;
        if (e.den == BigInt(1) && e.num.d.size() == 1 && e.num.d[0] <= 100)
            ex = e.num.neg ? -static_cast<int>(e.num.d[0]) : static_cast<int>(e.num.d[0]);
        if (ex == 0) continue;
        eta_q_power = eta_q_power + Frac(k * ex, 24);
        int abs_ex = ex < 0 ? -ex : ex;
        if (abs_ex != 1) part += Series::expToUnicode(abs_ex);
        if (ex > 0)
            num_parts.push_back(part);
        else
            den_parts.push_back(part);
    }
    // Compensating q-power: difference between input shift and what etas imply
    Frac comp = input_q_shift - eta_q_power;

    std::string num_str, den_str;
    for (const auto& s : num_parts) num_str += (num_str.empty() ? "" : " ") + s;
    for (const auto& s : den_parts) den_str += (den_str.empty() ? "" : " ") + s;
    std::string prefix = scalar_str.empty() ? "" : scalar_str + " ";

    // Add compensating q-power to numerator or denominator
    if (!(comp == Frac(0))) {
        Frac neg_comp = Frac(0) - comp;
        if (comp < Frac(0)) {
            std::string qstr = "q" + Series::fracExpStr(neg_comp);
            den_parts.insert(den_parts.begin(), qstr);
            den_str = "";
            for (const auto& s : den_parts) den_str += (den_str.empty() ? "" : " ") + s;
        } else {
            std::string qstr = "q" + Series::fracExpStr(comp);
            prefix = prefix + qstr + " ";
        }
    }

    if (num_str.empty() && den_str.empty()) return prefix.empty() ? "1" : prefix;
    if (den_str.empty()) return prefix + num_str;
    bool need_parens = den_parts.size() > 1;
    std::string denom = need_parens ? "(" + den_str + ")" : den_str;
    if (num_str.empty()) return prefix + "1 / " + denom;
    return prefix + num_str + " / " + denom;
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

#ifndef __EMSCRIPTEN__
// Redraw prompt + line with cursor at pos (0 = before first char, line.size() = after last)
inline void redrawLineRaw(const std::string& line, size_t pos) {
    std::cout << "\r\033[K" << ansi::gold() << "qseries> " << ansi::reset() << line << std::flush;
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
        std::string match = matches[0];
        bool isFunc = getHelpTable().count(match) > 0;
        if (isFunc) match += "(";
        line = line.substr(0, start) + match + line.substr(end);
        pos = start + match.size();
        redrawLineRaw(line, pos);
        return;
    }
    // Compute longest common prefix (LCP) across all matches
    std::string lcp = matches[0];
    for (size_t i = 1; i < matches.size(); ++i) {
        size_t j = 0;
        while (j < lcp.size() && j < matches[i].size() && lcp[j] == matches[i][j])
            ++j;
        lcp = lcp.substr(0, j);
    }
    if (lcp.size() > prefix.size()) {
        line = line.substr(0, start) + lcp + line.substr(end);
        pos = start + lcp.size();
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
        if (c == 12) {  // Ctrl+L
            std::cout << "\033[2J\033[H" << std::flush;
            redrawLineRaw(line, pos);
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
#endif // __EMSCRIPTEN__

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
        } else if constexpr (std::is_same_v<T, Partition>) {
            std::cout << formatPartition(arg) << std::endl;
        } else if constexpr (std::is_same_v<T, std::vector<Partition>>) {
            for (const auto& p : arg)
                std::cout << formatPartition(p) << std::endl;
        } else if constexpr (std::is_same_v<T, Phi1Result>) {
            std::cout << formatPhi1Result(arg) << std::endl;
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
        if (std::holds_alternative<Partition>(res)) {
            env.env[s->assignName] = std::get<Partition>(res);
            return res;
        }
        if (std::holds_alternative<Phi1Result>(res)) {
            env.env[s->assignName] = std::get<Phi1Result>(res);
            return res;
        }
        throw std::runtime_error("assignment requires Series, Jacobi product, or Partition");
    }
    return eval(s->expr.get(), env, {});
}

#ifndef __EMSCRIPTEN__
inline void runRepl() {
    ansi::init();

    if (stdin_is_tty()) {
        std::cout << ansi::gold() << R"banner(
 _                                           
| |                                          
| | ____ _ _ __   __ _  __ _ _ __ ___   ___  
| |/ / _` | '_ \ / _` |/ _` | '__/ _ \ / _ \
|   < (_| | | | | (_| | (_| | | | (_) | (_) |
|_|\_\__,_|_| |_|\__, |\__,_|_|  \___/ \___/ 
                  __/ |                      
                 |___/                       

 q-series REPL (Maple-like), version 4.1
)banner" << ansi::reset() << std::endl;
    }

    Environment env;
    std::deque<std::string> history;
    const size_t maxHistory = 1000;
    size_t inputLineNum = 0;

    if (stdin_is_tty())
        loadHistory(history, maxHistory);

    constexpr size_t maxContinuations = 100;

    for (;;) {
        std::string line;
        if (stdin_is_tty()) {
            std::cout << ansi::gold() << "qseries> " << ansi::reset() << std::flush;
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
                std::cout << ansi::gold() << "  > " << ansi::reset() << std::flush;
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

        if (trimmed == "clear") {
            if (stdin_is_tty())
                std::cout << "\033[2J\033[H" << std::flush;
            continue;
        }

        if (trimmed.size() > 6 && trimmed.substr(0, 5) == "save(" && trimmed.back() == ')') {
            std::string arg = trimmed.substr(5, trimmed.size() - 6);
            while (!arg.empty() && (arg.front() == ' ' || arg.front() == '"' || arg.front() == '\'')) arg.erase(arg.begin());
            while (!arg.empty() && (arg.back() == ' ' || arg.back() == '"' || arg.back() == '\'')) arg.pop_back();
            if (arg.empty()) {
                std::cerr << ansi::red() << "error: " << ansi::reset() << "save: name required" << std::endl;
            } else {
                try { saveSession(arg, env); }
                catch (const std::exception& e) { std::cerr << ansi::red() << "error: " << ansi::reset() << e.what() << std::endl; }
            }
            continue;
        }

        if (trimmed.size() > 6 && trimmed.substr(0, 5) == "load(" && trimmed.back() == ')') {
            std::string arg = trimmed.substr(5, trimmed.size() - 6);
            while (!arg.empty() && (arg.front() == ' ' || arg.front() == '"' || arg.front() == '\'')) arg.erase(arg.begin());
            while (!arg.empty() && (arg.back() == ' ' || arg.back() == '"' || arg.back() == '\'')) arg.pop_back();
            if (arg.empty()) {
                std::cerr << ansi::red() << "error: " << ansi::reset() << "load: name required" << std::endl;
            } else {
                try { loadSession(arg, env); }
                catch (const std::exception& e) { std::cerr << ansi::red() << "error: " << ansi::reset() << e.what() << std::endl; }
            }
            continue;
        }

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
                std::cout << ansi::dim() << std::fixed << std::setprecision(3) << secs << "s" << ansi::reset() << std::endl;
            }
        } catch (const std::exception& e) {
            std::cerr << ansi::red() << "error: " << ansi::reset();
            if (!stdin_is_tty() && inputLineNum > 0)
                std::cerr << "line " << inputLineNum << ": ";
            std::cerr << e.what() << std::endl;
        }
    }

    if (stdin_is_tty())
        saveHistory(history);
}
#endif // __EMSCRIPTEN__

#endif
