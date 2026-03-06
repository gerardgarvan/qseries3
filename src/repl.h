#ifndef REPL_H
#define REPL_H

#include "parser.h"
#include "series.h"
#include "omega3.h"
#include "series_omega.h"
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
#include <tuple>
#include <cctype>
#include <cstdlib>
#include <functional>
#include <unordered_map>

inline std::string runtimeErr(const std::string& func, const std::string& msg) {
    return func.empty() ? ("Error, " + msg) : ("Error, (in " + func + ") " + msg);
}
inline std::string ordinal(int n) {
    if (n <= 0) return std::to_string(n);
    int d = n % 10, t = (n / 10) % 10;
    if (t != 1 && d == 1) return std::to_string(n) + "st";
    if (t != 1 && d == 2) return std::to_string(n) + "nd";
    if (t != 1 && d == 3) return std::to_string(n) + "rd";
    return std::to_string(n) + "th";
}
inline std::string expectArg(int n, const std::string& name,
    const std::string& expected, const std::string& received) {
    return "invalid input: expects its " + ordinal(n) + " argument, " + name +
        ", to be " + expected + ", but received " + received;
}

// Parse "parser: line N, col M: message" — return true if successful
inline bool parseParserMessage(const std::string& msg, int& line, int& col, std::string& rest) {
    if (msg.size() < 13 || msg.compare(0, 8, "parser: ") != 0) return false;
    size_t i = 8;
    if (msg.compare(i, 5, "line ") != 0) return false;
    i += 5;
    line = 0;
    while (i < msg.size() && std::isdigit(static_cast<unsigned char>(msg[i])))
        line = line * 10 + (msg[i++] - '0');
    if (i + 6 > msg.size() || msg.compare(i, 6, ", col ") != 0) return false;
    i += 6;
    col = 0;
    while (i < msg.size() && std::isdigit(static_cast<unsigned char>(msg[i])))
        col = col * 10 + (msg[i++] - '0');
    if (i < msg.size() && msg[i] == ':') { ++i; if (i < msg.size() && msg[i] == ' ') ++i; }
    rest = (i < msg.size()) ? msg.substr(i) : "";
    return line > 0 && col > 0;
}

inline std::string getLineAt(const std::string& input, int lineNum) {
    int cur = 1;
    for (size_t i = 0; i < input.size(); ++i) {
        if (cur == lineNum) {
            size_t end = input.find('\n', i);
            if (end == std::string::npos) end = input.size();
            return input.substr(i, end - i);
        }
        if (input[i] == '\n') ++cur;
    }
    return "";
}

inline std::string caretLine(int col) {
    return std::string(col > 0 ? static_cast<size_t>(col - 1) : 0, ' ') + "^";
}

inline std::string formatParseErrorWithSnippet(const std::string& input, const std::string& msg) {
    int line = 0, col = 0;
    std::string rest;
    if (parseParserMessage(msg, line, col, rest)) {
        std::string srcLine = getLineAt(input, line);
        std::string out = "  " + srcLine + "\n  " + caretLine(col) + "\nError, (in parser) line " +
            std::to_string(line) + ", col " + std::to_string(col) + ": " + rest;
        return out;
    }
    if (msg.size() >= 8 && msg.compare(0, 8, "parser: ") == 0)
        return "Error, (in parser) " + msg.substr(8);
    return msg;
}

#ifndef __EMSCRIPTEN__
#if defined(__CYGWIN__) || !defined(_WIN32)
#define _POSIX_C_SOURCE 200809L
#include <termios.h>
#include <unistd.h>
#include <csignal>
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
static struct termios g_repl_orig_termios;
static void repl_sigint_handler(int) {
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &g_repl_orig_termios);
    signal(SIGINT, SIG_DFL);
    raise(SIGINT);
}
static void repl_install_sigint_handler() {
    if (tcgetattr(STDIN_FILENO, &g_repl_orig_termios) != 0) return;
    signal(SIGINT, repl_sigint_handler);
}
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

// REPL configuration constants (Phase 118 TD-06)
static constexpr int64_t kPowExponentLimit = 10000;
static constexpr size_t kMaxHistory = 1000;
static constexpr size_t kMaxContinuations = 100;
static constexpr size_t kMaxBracketContinuations = 100;
static constexpr int kLevenshteinSuggestionThreshold = 3;

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

// RelationKernelResult: findhom/findnonhom result (basis + monomial exponents)
struct RelationKernelResult {
    std::vector<std::vector<Frac>> basis;
    std::vector<std::vector<int>> monomialExponents;
};

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

// EnvValue: variable can hold Series, Jacobi product, Partition, Phi1Result, RelationKernelResult, Omega3, or SeriesOmega
using EnvValue = std::variant<Series, std::vector<JacFactor>, Partition, Phi1Result, RelationKernelResult, Omega3, SeriesOmega>;

struct Environment {
    std::map<std::string, EnvValue> env;
    int T = 50;

    Environment() {
        env["q"] = Series::q(T);
        env["omega"] = Omega3::omega();
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
        if (std::holds_alternative<RelationKernelResult>(val))
            continue;  // skip: relations are typically recomputed
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

struct RelationComboResult {
    std::optional<std::vector<Frac>> coeffs;
    std::vector<std::vector<int>> monomialExponents;
};

struct FindlincombomodpResult {
    std::optional<std::vector<int64_t>> coeffs;
    int p;
    size_t k;
};

using EvalResult = std::variant<
    Series,
    Omega3,                                           // omega, RootOf(3)
    SeriesOmega,                                      // omega * Series
    std::map<int, Frac>,                              // prodmake
    std::vector<int>,                                 // mprodmake
    std::vector<std::pair<int, Frac>>,                // etamake
    std::vector<JacFactor>,                           // jacprodmake
    RelationKernelResult,                             // findhom/findnonhom
    RelationComboResult,                              // findhomcombo/findnonhomcombo
    FindlincombomodpResult,                           // findlincombomodp
    QFactorResult,                                    // qfactor
    FactorResult,                                     // factor
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

// DispatchContext: passed to built-in handlers for ev/evi and helpers
struct DispatchContext {
    const std::string& name;
    const std::vector<ExprPtr>& args;
    Environment& env;
    const std::map<std::string, int64_t>& sumIndices;
    Series q;
    int T;
    std::function<Series(size_t)> ev;
    std::function<int64_t(size_t)> evi;
    std::function<std::vector<Series>(const Expr*)> evalListToSeries;
    std::function<std::vector<int>(const Expr*)> evalListToInt;
    std::function<Partition(const Expr*)> evalToPartition;
};

// expectArgCount: validate ctx.args.size() against allowed counts
inline void expectArgCount(const DispatchContext& ctx, size_t n) {
    if (ctx.args.size() != n)
        throw std::runtime_error(runtimeErr(ctx.name, expectArg(1, "arguments", std::to_string(n), std::to_string(ctx.args.size()) + " arguments")));
}
inline void expectArgCount(const DispatchContext& ctx, std::initializer_list<size_t> allowed) {
    for (size_t a : allowed)
        if (ctx.args.size() == a) return;
    std::string allowedStr;
    if (allowed.size() == 1) allowedStr = std::to_string(*allowed.begin());
    else if (allowed.size() == 2) allowedStr = std::to_string(*allowed.begin()) + " or " + std::to_string(*(allowed.begin() + 1));
    else {
        auto it = allowed.begin();
        for (size_t i = 0; i < allowed.size(); ++i, ++it) {
            if (i > 0) allowedStr += (i + 1 == allowed.size()) ? ", or " : ", ";
            allowedStr += std::to_string(*it);
        }
    }
    throw std::runtime_error(runtimeErr(ctx.name, expectArg(1, "arguments", allowedStr, std::to_string(ctx.args.size()) + " arguments")));
}

using BuiltinHandler = std::function<EvalResult(DispatchContext&)>;

// Help table: name -> (signature, description, examples, seeAlso)
struct HelpEntry {
    std::string sig;
    std::string desc;
    std::vector<std::string> examples;
    std::vector<std::string> seeAlso;
};

struct BuiltinEntry {
    HelpEntry help;
    BuiltinHandler handler;
};

inline void formatHelpEntry([[maybe_unused]] const std::string& name, const HelpEntry& e) {
    std::cout << "  SYNOPSIS  " << ansi::gold() << e.sig << ansi::reset() << " — " << e.desc << std::endl;
    if (!e.examples.empty()) {
        std::cout << "  EXAMPLES" << std::endl;
        for (const auto& ex : e.examples)
            std::cout << "  qseries> " << ex << std::endl;
    }
    if (!e.seeAlso.empty()) {
        std::cout << "  SEE ALSO  ";
        for (size_t i = 0; i < e.seeAlso.size(); ++i) {
            if (i > 0) std::cout << ", ";
            std::cout << e.seeAlso[i];
        }
        std::cout << std::endl;
    }
}

// Forward declaration for handlers that call eval (e.g. invphi1)
inline EvalResult eval(const Expr* e, Environment& env,
    const std::map<std::string, int64_t>& sumIndices);

inline const std::map<std::string, HelpEntry>& getHelpTable() {
    static const std::map<std::string, HelpEntry> table = []() {
        std::map<std::string, HelpEntry> t;
        for (const auto& [name, entry] : getBuiltinRegistry())
            t[name] = entry.help;
        return t;
    }();
    return table;
}

inline const std::unordered_map<std::string, BuiltinEntry>& getBuiltinRegistry() {
    static const std::unordered_map<std::string, BuiltinEntry> reg = []() {
        std::unordered_map<std::string, BuiltinEntry> m;
        auto regBuiltin = [&m](const char* n, HelpEntry h, BuiltinHandler f) {
            m[n] = BuiltinEntry{std::move(h), std::move(f)};
        };
#define H(sig, desc, ex, see) HelpEntry{sig, desc, ex, see}

        regBuiltin("RootOf", H("RootOf(3) or RootOf([1,1,1])", "primitive cube root of unity omega", {}, {}), [](DispatchContext& ctx) {
            if (ctx.args.size() != 1) throw std::runtime_error(runtimeErr(ctx.name, "expects 1 argument"));
            const Expr* a = ctx.args[0].get();
            if (a->tag == Expr::Tag::IntLit) {
                if (a->intVal == 3) return EvalResult(Omega3::omega());
                throw std::runtime_error(runtimeErr(ctx.name, "RootOf(n) only supports n=3 for omega"));
            }
            if (a->tag == Expr::Tag::List) {
                if (a->elements.size() != 3) throw std::runtime_error(runtimeErr(ctx.name, "RootOf([a,b,c]) expects 3 coefficients"));
                int c0 = static_cast<int>(evalToInt(a->elements[0].get(), ctx.env, ctx.sumIndices));
                int c1 = static_cast<int>(evalToInt(a->elements[1].get(), ctx.env, ctx.sumIndices));
                int c2 = static_cast<int>(evalToInt(a->elements[2].get(), ctx.env, ctx.sumIndices));
                if (c0 == 1 && c1 == 1 && c2 == 1) return EvalResult(Omega3::omega());
                throw std::runtime_error(runtimeErr(ctx.name, "RootOf([a,b,c]) only supports [1,1,1] for omega"));
            }
            throw std::runtime_error(runtimeErr(ctx.name, "expects int or list argument"));
        });

        regBuiltin("aqprod", H("aqprod(a,q,n,T)", "rising q-factorial (a;q)_n", {"aqprod(q,q,5,50)"}, {"etaq", "prodmake"}), [](DispatchContext& ctx) {
            if (ctx.args.size() != 4) throw std::runtime_error(runtimeErr(ctx.name, "expects 4 arguments"));
            return aqprod(ctx.ev(0), ctx.ev(1), static_cast<int>(ctx.evi(2)), static_cast<int>(ctx.evi(3)));
        });

        regBuiltin("etaq", H("etaq(k) or etaq(k,T) or etaq(q,k,T)", "eta product \u03A0(1-q^{kn})", {"etaq(1, 20)"}, {"etamake", "aqprod"}), [](DispatchContext& ctx) {
            expectArgCount(ctx, {1, 2, 3});
            if (ctx.args.size() == 1) return etaq(ctx.q, static_cast<int>(ctx.evi(0)), ctx.T);
            if (ctx.args.size() == 2) return etaq(ctx.q, static_cast<int>(ctx.evi(0)), static_cast<int>(ctx.evi(1)));
            return etaq(ctx.ev(0), static_cast<int>(ctx.evi(1)), static_cast<int>(ctx.evi(2)));
        });

        regBuiltin("theta2", H("theta2(T) or theta2(q,T)", "theta_2", {}, {}), [](DispatchContext& ctx) {
            if (ctx.args.size() == 1) return theta2(ctx.q, static_cast<int>(ctx.evi(0)));
            if (ctx.args.size() == 2) return theta2(ctx.ev(0), static_cast<int>(ctx.evi(1)));
            throw std::runtime_error(runtimeErr(ctx.name, "expected theta2/3/4(T) or theta2/3/4(q,T)"));
        });
        regBuiltin("theta3", H("theta3(T) or theta3(q,T)", "theta_3", {"theta3(20)"}, {"etaq", "etamake", "theta", "theta4"}), [](DispatchContext& ctx) {
            if (ctx.args.size() == 1) return theta3(ctx.q, static_cast<int>(ctx.evi(0)));
            if (ctx.args.size() == 2) return theta3(ctx.ev(0), static_cast<int>(ctx.evi(1)));
            throw std::runtime_error(runtimeErr(ctx.name, "expected theta2/3/4(T) or theta2/3/4(q,T)"));
        });
        regBuiltin("theta4", H("theta4(T) or theta4(q,T)", "theta_4", {}, {}), [](DispatchContext& ctx) {
            if (ctx.args.size() == 1) return theta4(ctx.q, static_cast<int>(ctx.evi(0)));
            if (ctx.args.size() == 2) return theta4(ctx.ev(0), static_cast<int>(ctx.evi(1)));
            throw std::runtime_error(runtimeErr(ctx.name, "expected theta2/3/4(T) or theta2/3/4(q,T)"));
        });

        regBuiltin("theta", H("theta(z,T) or theta(z,q,T)", "generalized theta function", {}, {}), [](DispatchContext& ctx) {
            if (ctx.args.size() == 2) return theta(ctx.ev(0), ctx.q, static_cast<int>(ctx.evi(1)));
            if (ctx.args.size() == 3) return theta(ctx.ev(0), ctx.ev(1), static_cast<int>(ctx.evi(2)));
            throw std::runtime_error(runtimeErr(ctx.name, "expected theta(z,T) or theta(z,q,T)"));
        });
        regBuiltin("qbin", H("qbin(m,n,T) or qbin(q,m,n,T)", "Gaussian polynomial [m;n]_q", {}, {}), [](DispatchContext& ctx) {
            if (ctx.args.size() == 3) return qbin(ctx.q, static_cast<int>(ctx.evi(0)), static_cast<int>(ctx.evi(1)), static_cast<int>(ctx.evi(2)));
            if (ctx.args.size() == 4) return qbin(ctx.ev(0), static_cast<int>(ctx.evi(1)), static_cast<int>(ctx.evi(2)), static_cast<int>(ctx.evi(3)));
            throw std::runtime_error(runtimeErr(ctx.name, "expected qbin(m,n,T) or qbin(q,m,n,T)"));
        });
        regBuiltin("tripleprod", H("tripleprod(z,q,T)", "Jacobi triple product", {}, {}), [](DispatchContext& ctx) {
            if (ctx.args.size() != 3) throw std::runtime_error(runtimeErr(ctx.name, "expects 3 arguments"));
            if (ctx.args[0]->tag == Expr::Tag::Var && ctx.env.env.find(ctx.args[0]->varName) == ctx.env.env.end()) {
                Series qv = ctx.ev(1);
                int Tr = static_cast<int>(ctx.evi(2));
                auto b = tripleprod_symbolic(qv, Tr);
                std::cout << formatBivariate(b) << std::endl;
                return EvalResult(DisplayOnly{});
            }
            return tripleprod(ctx.ev(0), ctx.ev(1), static_cast<int>(ctx.evi(2)));
        });
        regBuiltin("quinprod", H("quinprod(z,q,T) or quinprod(z,q,prodid) or quinprod(z,q,seriesid)", "quintuple product", {}, {}), [](DispatchContext& ctx) {
            if (ctx.args.size() != 3) throw std::runtime_error(runtimeErr(ctx.name, "expects 3 arguments"));
            if (ctx.args[2]->tag == Expr::Tag::Var) {
                const std::string& v = ctx.args[2]->varName;
                if (v == "prodid") { std::cout << "(-z;q)_\u221E (-q/z;q)_\u221E (z\u00B2q;q\u00B2)_\u221E (q/z\u00B2;q\u00B2)_\u221E (q;q)_\u221E" << std::endl; return DisplayOnly{}; }
                if (v == "seriesid") { std::cout << "\u03A3 ((-z)^(-3n) - (-z)^(3n+1)) q^(n(3n+1)/2)" << std::endl; return DisplayOnly{}; }
            }
            if (ctx.args[0]->tag == Expr::Tag::Var && ctx.env.env.find(ctx.args[0]->varName) == ctx.env.env.end()) {
                Series qv = ctx.ev(1);
                int Tr = static_cast<int>(ctx.evi(2));
                auto b = quinprod_symbolic(qv, Tr);
                std::cout << formatBivariate(b) << std::endl;
                return DisplayOnly{};
            }
            return quinprod(ctx.ev(0), ctx.ev(1), static_cast<int>(ctx.evi(2)));
        });
        regBuiltin("winquist", H("winquist(a,b,q,T)", "Winquist identity", {}, {}), [](DispatchContext& ctx) {
            if (ctx.args.size() != 4) throw std::runtime_error(runtimeErr(ctx.name, "expects 4 arguments"));
            return winquist(ctx.ev(0), ctx.ev(1), ctx.ev(2), static_cast<int>(ctx.evi(3)));
        });
        regBuiltin("sift", H("sift(f,n,k,T)", "extract coefficients a_{ni+k}", {}, {}), [](DispatchContext& ctx) {
            if (ctx.args.size() != 4) throw std::runtime_error(runtimeErr(ctx.name, "expects 4 arguments"));
            return sift(ctx.ev(0), static_cast<int>(ctx.evi(1)), static_cast<int>(ctx.evi(2)), static_cast<int>(ctx.evi(3)));
        });
        regBuiltin("T", H("T(r,n) or T(r,n,T)", "finite q-product T_{r,n}", {}, {}), [](DispatchContext& ctx) {
            if (ctx.args.size() == 2) return T_rn(static_cast<int>(ctx.evi(0)), static_cast<int>(ctx.evi(1)), ctx.T);
            if (ctx.args.size() == 3) return T_rn(static_cast<int>(ctx.evi(0)), static_cast<int>(ctx.evi(1)), static_cast<int>(ctx.evi(2)));
            throw std::runtime_error(runtimeErr(ctx.name, "expected T(r,n) or T(r,n,T)"));
        });
        regBuiltin("prodmake", H("prodmake(f,T)", "Andrews' algorithm: series \u2192 infinite product", {"rr := sum(q^(n^2)/aqprod(q,q,n,50), n, 0, 8)", "prodmake(rr, 40)"}, {"etamake", "jacprodmake"}), [](DispatchContext& ctx) {
            expectArgCount(ctx, 2);
            return prodmake(ctx.ev(0), static_cast<int>(ctx.evi(1)));
        });
        regBuiltin("RRG", H("RRG(n) or RRG(n,T)", "Rogers-Ramanujan G(n)", {}, {}), [](DispatchContext& ctx) {
            int n = static_cast<int>(ctx.evi(0));
            int Tr = (ctx.args.size() == 2) ? static_cast<int>(ctx.evi(1)) : ctx.T;
            return RRG(n, Tr);
        });
        regBuiltin("RRH", H("RRH(n) or RRH(n,T)", "Rogers-Ramanujan H(n)", {}, {}), [](DispatchContext& ctx) {
            int n = static_cast<int>(ctx.evi(0));
            int Tr = (ctx.args.size() == 2) ? static_cast<int>(ctx.evi(1)) : ctx.T;
            return RRH(n, Tr);
        });
        regBuiltin("RRGstar", H("RRGstar(n) or RRGstar(n,T)", "G\u00F6llnitz-Gordon G*(n)", {}, {}), [](DispatchContext& ctx) {
            int n = static_cast<int>(ctx.evi(0));
            int Tr = (ctx.args.size() == 2) ? static_cast<int>(ctx.evi(1)) : ctx.T;
            return RRGstar(n, Tr);
        });
        regBuiltin("RRHstar", H("RRHstar(n) or RRHstar(n,T)", "G\u00F6llnitz-Gordon H*(n)", {}, {}), [](DispatchContext& ctx) {
            int n = static_cast<int>(ctx.evi(0));
            int Tr = (ctx.args.size() == 2) ? static_cast<int>(ctx.evi(1)) : ctx.T;
            return RRHstar(n, Tr);
        });
        regBuiltin("geta", H("geta(g,d,n,T)", "Geta(g,d,n)", {}, {}), [](DispatchContext& ctx) {
            if (ctx.args.size() != 4) throw std::runtime_error(runtimeErr(ctx.name, "expects geta(g,d,n,T)"));
            return geta(static_cast<int>(ctx.evi(0)), static_cast<int>(ctx.evi(1)), static_cast<int>(ctx.evi(2)), static_cast<int>(ctx.evi(3)));
        });
        regBuiltin("checkid", H("checkid(expr,T) or checkid(expr,T,acc)", "check if expr is eta/theta product", {}, {}), [](DispatchContext& ctx) {
            if (ctx.args.size() < 2 || ctx.args.size() > 3) throw std::runtime_error(runtimeErr(ctx.name, "expects checkid(expr,T) or checkid(expr,T,acc)"));
            Series f = ctx.ev(0);
            int Tr = static_cast<int>(ctx.evi(1));
            int acc = (ctx.args.size() == 3) ? static_cast<int>(ctx.evi(2)) : 10;
            auto r = checkid(f, Tr, acc);
            if (!r.ok) std::cout << "not an eta product" << std::endl;
            else { std::string prefix = (r.ldq != 0) ? ("q^" + std::to_string(r.ldq) + " * ") : ""; std::cout << prefix << formatEtamake(r.eta) << std::endl; }
            return DisplayOnly{};
        });
        regBuiltin("findids", H("findids(type) or findids(type,T)", "search RR identities", {}, {}), [](DispatchContext& ctx) {
            if (ctx.args.size() < 1 || ctx.args.size() > 2) throw std::runtime_error(runtimeErr(ctx.name, "expects findids(type) or findids(type,T)"));
            int typeVal = static_cast<int>(ctx.evi(0));
            int Tr = (ctx.args.size() == 2) ? static_cast<int>(ctx.evi(1)) : ctx.T;
            auto ids = findids(typeVal, Tr);
            for (const auto& [a, b, c1] : ids) std::cout << "[" << a << ", " << b << ", " << c1 << "]" << std::endl;
            return DisplayOnly{};
        });
        regBuiltin("newprodmake", H("newprodmake(f,T)", "Andrews' prodmake with arbitrary leading term", {}, {}), [](DispatchContext& ctx) {
            if (ctx.args.size() != 2) throw std::runtime_error(runtimeErr(ctx.name, "expects 2 arguments"));
            auto r = newprodmake(ctx.ev(0), static_cast<int>(ctx.evi(1)));
            std::ostringstream lead;
            if (r.lead_exp == 0) lead << r.lead_coeff.str();
            else if (r.lead_coeff == Frac(1)) lead << "q^" << r.lead_exp;
            else if (r.lead_coeff == Frac(-1)) lead << "-q^" << r.lead_exp;
            else lead << r.lead_coeff.str() << "*q^" << r.lead_exp;
            std::cout << lead.str() << " * " << formatProdmake(r.a, true) << std::endl;
            return DisplayOnly{};
        });
        regBuiltin("dilly", H("dilly(f,d)", "q-dilation: substitute q \u2192 q^d", {}, {}), [](DispatchContext& ctx) {
            if (ctx.args.size() != 2) throw std::runtime_error(runtimeErr(ctx.name, "expects 2 arguments"));
            return dilly(ctx.ev(0), static_cast<int>(ctx.evi(1)));
        });
        regBuiltin("sieveqcheck", H("sieveqcheck(f,p)", "true if all exponents \u2261 same residue mod p", {}, {}), [](DispatchContext& ctx) {
            if (ctx.args.size() != 2) throw std::runtime_error(runtimeErr(ctx.name, "expects 2 arguments"));
            return static_cast<int64_t>(sieveqcheck(ctx.ev(0), static_cast<int>(ctx.evi(1))) ? 1 : 0);
        });
        regBuiltin("siftfindrange", H("siftfindrange(f,p,T)", "find residue class with fewest terms", {}, {}), [](DispatchContext& ctx) {
            if (ctx.args.size() != 3) throw std::runtime_error(runtimeErr(ctx.name, "expects 3 arguments"));
            auto r = siftfindrange(ctx.ev(0), static_cast<int>(ctx.evi(1)), static_cast<int>(ctx.evi(2)));
            std::cout << "minnops=" << r.min_count << " minres=" << r.min_res << " average=" << r.avg << " maxnops=" << r.max_count << std::endl;
            return DisplayOnly{};
        });
        regBuiltin("polyfind", H("polyfind(n0,n1,n2,T)", "quadratic p(x) with p(T)=n0, p(T+1)=n1, p(T+2)=n2", {}, {}), [](DispatchContext& ctx) {
            if (ctx.args.size() != 4) throw std::runtime_error(runtimeErr(ctx.name, "expects 4 arguments"));
            auto r = polyfind(ctx.evi(0), ctx.evi(1), ctx.evi(2), ctx.evi(3));
            if (!r.ok) { std::cout << "polyfind: no integer quadratic fit" << std::endl; return DisplayOnly{}; }
            std::cout << "a b c: " << r.a.str() << " " << r.b.str() << " " << r.c.str() << std::endl;
            return DisplayOnly{};
        });
        regBuiltin("mprodmake", H("mprodmake(f,T)", "convert series to product (1+q^n1)(1+q^n2)...", {}, {}), [](DispatchContext& ctx) {
            if (ctx.args.size() != 2) throw std::runtime_error(runtimeErr(ctx.name, "expects 2 arguments"));
            return mprodmake(ctx.ev(0), static_cast<int>(ctx.evi(1)));
        });
        regBuiltin("checkprod", H("checkprod(f) or checkprod(f,T) or checkprod(f,M,T)", "check if f is a nice product", {}, {}), [](DispatchContext& ctx) {
            if (ctx.args.size() == 1) return checkprod(ctx.ev(0), ctx.T);
            if (ctx.args.size() == 2) return checkprod(ctx.ev(0), static_cast<int>(ctx.evi(1)));
            if (ctx.args.size() == 3) return checkprod(ctx.ev(0), static_cast<int>(ctx.evi(1)), static_cast<int>(ctx.evi(2)));
            throw std::runtime_error(runtimeErr(ctx.name, "expects 1, 2, or 3 arguments"));
        });
        regBuiltin("checkmult", H("checkmult(f) or checkmult(f,T) or checkmult(f,T,1)", "check if coefficients are multiplicative", {}, {}), [](DispatchContext& ctx) {
            if (ctx.args.size() == 1) return checkmult(ctx.ev(0), ctx.T, false);
            if (ctx.args.size() == 2) return checkmult(ctx.ev(0), static_cast<int>(ctx.evi(1)), false);
            if (ctx.args.size() == 3) return checkmult(ctx.ev(0), static_cast<int>(ctx.evi(1)), ctx.evi(2) != 0);
            throw std::runtime_error(runtimeErr(ctx.name, "expects 1, 2, or 3 arguments"));
        });
        regBuiltin("etamake", H("etamake(f,T)", "identify f as eta product", {}, {"prodmake", "etaq", "jacprodmake"}), [](DispatchContext& ctx) {
            expectArgCount(ctx, {2, 3});
            if (ctx.args.size() == 2) return etamake(ctx.ev(0), static_cast<int>(ctx.evi(1)));
            return etamake(ctx.ev(0), static_cast<int>(ctx.evi(2)));
        });
        regBuiltin("jacprodmake", H("jacprodmake(f,T)", "identify f as Jacobi product", {}, {"prodmake", "etamake", "jac2prod"}), [](DispatchContext& ctx) {
            if (ctx.args.size() != 2) throw std::runtime_error(runtimeErr(ctx.name, "expects 2 arguments"));
            return jacprodmake(ctx.ev(0), static_cast<int>(ctx.evi(1)));
        });
        regBuiltin("jac2prod", H("jac2prod(var)", "display Jacobi product stored in variable", {}, {}), [](DispatchContext& ctx) {
            if (ctx.args.size() != 1) throw std::runtime_error(runtimeErr(ctx.name, "expects 1 argument"));
            if (ctx.args[0]->tag != Expr::Tag::Var) throw std::runtime_error(runtimeErr(ctx.name, "expects variable name"));
            auto it = ctx.env.env.find(ctx.args[0]->varName);
            if (it == ctx.env.env.end()) throw std::runtime_error(runtimeErr(ctx.name, formatUndefinedVariableMsg(ctx.env, ctx.args[0]->varName)));
            if (std::holds_alternative<std::vector<JacFactor>>(it->second)) { std::cout << jac2prod(std::get<std::vector<JacFactor>>(it->second)) << std::endl; return DisplayOnly{}; }
            throw std::runtime_error(runtimeErr(ctx.name, "expects jacprodmake result"));
        });
        regBuiltin("jac2series", H("jac2series(var) or jac2series(var,T)", "convert Jacobi product to series", {}, {}), [](DispatchContext& ctx) {
            if (ctx.args.size() < 1 || ctx.args.size() > 2) throw std::runtime_error(runtimeErr(ctx.name, "expects jac2series(var) or jac2series(var,T)"));
            if (ctx.args[0]->tag != Expr::Tag::Var) throw std::runtime_error(runtimeErr(ctx.name, "expects variable name"));
            auto it = ctx.env.env.find(ctx.args[0]->varName);
            if (it == ctx.env.env.end()) throw std::runtime_error(runtimeErr(ctx.name, formatUndefinedVariableMsg(ctx.env, ctx.args[0]->varName)));
            if (!std::holds_alternative<std::vector<JacFactor>>(it->second)) throw std::runtime_error(runtimeErr(ctx.name, "expects jacprodmake result"));
            int Tr = (ctx.args.size() == 2) ? static_cast<int>(ctx.evi(1)) : ctx.T;
            return jac2series(std::get<std::vector<JacFactor>>(it->second), Tr);
        });
        regBuiltin("qfactor", H("qfactor(f) or qfactor(f,T)", "factorize finite q-product", {}, {}), [](DispatchContext& ctx) {
            if (ctx.args.size() == 1) return qfactor(ctx.ev(0), ctx.T);
            if (ctx.args.size() == 2) return qfactor(ctx.ev(0), static_cast<int>(ctx.evi(1)));
            throw std::runtime_error(runtimeErr(ctx.name, "expected qfactor(f) or qfactor(f,T)"));
        });
        regBuiltin("factor", H("factor(f) or factor(f,T)", "factor polynomial in q into cyclotomic form", {}, {}), [](DispatchContext& ctx) {
            if (ctx.args.size() == 1) return factor(ctx.ev(0), ctx.T);
            if (ctx.args.size() == 2) return factor(ctx.ev(0), static_cast<int>(ctx.evi(1)));
            throw std::runtime_error(runtimeErr(ctx.name, "expected factor(f) or factor(f,T)"));
        });
        regBuiltin("series", H("series(f) or series(f,T)", "display series coefficients", {}, {}), [](DispatchContext& ctx) {
            if (ctx.args.size() == 1) { Series f = ctx.ev(0).truncTo(ctx.T); std::cout << f.str(std::min(100, ctx.T)) << std::endl; return DisplayOnly{}; }
            if (ctx.args.size() == 2) { int Tr = static_cast<int>(ctx.evi(1)); Series f = ctx.ev(0).truncTo(Tr); std::cout << f.str(std::min(Tr, 100)) << std::endl; return DisplayOnly{}; }
            throw std::runtime_error(runtimeErr(ctx.name, "expected series(f) or series(f,T)"));
        });
        regBuiltin("coeffs", H("coeffs(f,from,to)", "list coefficients from exponent from to to", {}, {}), [](DispatchContext& ctx) {
            if (ctx.args.size() != 3) throw std::runtime_error(runtimeErr(ctx.name, "expects 3 arguments"));
            Series f = ctx.ev(0);
            int from = static_cast<int>(ctx.evi(1)), to = static_cast<int>(ctx.evi(2));
            auto list = f.coeffList(from, to);
            std::cout << "["; for (size_t i = 0; i < list.size(); ++i) { if (i) std::cout << ", "; std::cout << list[i].str(); } std::cout << "]" << std::endl;
            return DisplayOnly{};
        });
        regBuiltin("coeff", H("coeff(f,n)", "coefficient of q^n in series f", {}, {}), [](DispatchContext& ctx) {
            if (ctx.args.size() != 2) throw std::runtime_error(runtimeErr(ctx.name, "expects 2 arguments"));
            std::cout << ctx.ev(0).coeff(static_cast<int>(ctx.evi(1))).str() << std::endl;
            return DisplayOnly{};
        });
        regBuiltin("dissect", H("dissect(f,m,T)", "list all m siftings of f", {}, {}), [](DispatchContext& ctx) {
            if (ctx.args.size() != 3) throw std::runtime_error(runtimeErr(ctx.name, "expects 3 arguments"));
            Series f = ctx.ev(0); int m = static_cast<int>(ctx.evi(1)), Td = static_cast<int>(ctx.evi(2));
            if (m <= 0) throw std::runtime_error(runtimeErr(ctx.name, "m must be positive"));
            for (int k = 0; k < m; ++k) std::cout << "k=" << k << ": " << sift(f, m, k, Td).str(30) << std::endl;
            return DisplayOnly{};
        });
        regBuiltin("set_trunc", H("set_trunc(N)", "set default truncation", {}, {}), [](DispatchContext& ctx) {
            if (ctx.args.size() != 1) throw std::runtime_error(runtimeErr(ctx.name, "expects 1 argument"));
            int N = static_cast<int>(ctx.evi(0));
            if (N <= 0) throw std::runtime_error(runtimeErr(ctx.name, "truncation must be positive"));
            ctx.env.T = N;
            ctx.env.env["q"] = Series::q(N);
            clear_etaq_cache();
            return std::monostate{};
        });
        regBuiltin("clear_cache", H("clear_cache()", "clear memoization caches (etaq)", {}, {}), [](DispatchContext& ctx) {
            if (ctx.args.size() != 0) throw std::runtime_error(runtimeErr(ctx.name, "expects no arguments"));
            clear_etaq_cache();
            return std::monostate{};
        });
        regBuiltin("version", H("version", "print package version", {}, {}), [](DispatchContext& ctx) {
            if (ctx.args.size() != 0) throw std::runtime_error(runtimeErr(ctx.name, "expects no arguments"));
            std::cout << "qseries 4.1\n";
            return DisplayOnly{};
        });
        regBuiltin("qdegree", H("qdegree(f)", "highest exponent with nonzero coefficient", {}, {}), [](DispatchContext& ctx) {
            if (ctx.args.size() != 1) throw std::runtime_error(runtimeErr(ctx.name, "expects 1 argument"));
            return static_cast<int64_t>(ctx.ev(0).maxExp());
        });
        regBuiltin("lqdegree", H("lqdegree(f)", "lowest exponent with nonzero coefficient", {}, {}), [](DispatchContext& ctx) {
            if (ctx.args.size() != 1) throw std::runtime_error(runtimeErr(ctx.name, "expects 1 argument"));
            Series f = ctx.ev(0);
            return static_cast<int64_t>(f.c.empty() ? 0 : f.minExp());
        });
#define H2(sig, desc, ex, see) HelpEntry{sig, desc, ex, see}
        regBuiltin("findhom", H2("findhom(L,n,topshift)", "homogeneous polynomial relations between series in L", {}, {}), [](DispatchContext& ctx) {
            if (ctx.args.size() != 3) throw std::runtime_error(runtimeErr(ctx.name, "expects 3 arguments"));
            auto L = ctx.evalListToSeries(ctx.args[0].get());
            int n = static_cast<int>(ctx.evi(1)), topshift = static_cast<int>(ctx.evi(2));
            auto basis = findhom(L, n, topshift);
            std::vector<std::vector<int>> exps;
            enumerate_hom_exponents(static_cast<int>(L.size()), n, {}, exps);
            return RelationKernelResult{std::move(basis), std::move(exps)};
        });
        regBuiltin("findhommodp", H2("findhommodp(L,p,n,topshift)", "homogeneous polynomial relations mod p", {}, {}), [](DispatchContext& ctx) {
            if (ctx.args.size() != 4) throw std::runtime_error(runtimeErr(ctx.name, "expects 4 arguments"));
            auto L = ctx.evalListToSeries(ctx.args[0].get());
            int p = static_cast<int>(ctx.evi(1)), n = static_cast<int>(ctx.evi(2)), topshift = static_cast<int>(ctx.evi(3));
            auto basis = findhommodp(L, p, n, topshift);
            std::vector<std::vector<int>> exps;
            enumerate_hom_exponents(static_cast<int>(L.size()), n, {}, exps);
            std::vector<std::vector<Frac>> frac_basis;
            for (const auto& v : basis) { std::vector<Frac> fv; for (int64_t x : v) fv.push_back(Frac(x)); frac_basis.push_back(std::move(fv)); }
            return RelationKernelResult{std::move(frac_basis), std::move(exps)};
        });
        regBuiltin("findnonhom", H2("findnonhom(L,n,topshift)", "nonhomogeneous polynomial relations", {}, {}), [](DispatchContext& ctx) {
            if (ctx.args.size() != 3) throw std::runtime_error(runtimeErr(ctx.name, "expects 3 arguments"));
            auto L = ctx.evalListToSeries(ctx.args[0].get());
            int n = static_cast<int>(ctx.evi(1)), topshift = static_cast<int>(ctx.evi(2));
            auto basis = findnonhom(L, n, topshift);
            std::vector<std::vector<int>> exps;
            enumerate_nonhom_exponents(static_cast<int>(L.size()), n, exps);
            return RelationKernelResult{std::move(basis), std::move(exps)};
        });
        regBuiltin("findhomcombo", H2("findhomcombo(f,L,n,topshift[,etaopt])", "express f as polynomial in L", {}, {}), [](DispatchContext& ctx) {
            if (ctx.args.size() < 4 || ctx.args.size() > 5) throw std::runtime_error(runtimeErr(ctx.name, "expects 4 or 5 arguments"));
            Series f = ctx.ev(0); auto L = ctx.evalListToSeries(ctx.args[1].get());
            int n = static_cast<int>(ctx.evi(2)), topshift = static_cast<int>(ctx.evi(3));
            auto coeffs = findhomcombo(f, L, n, topshift, false);
            std::vector<std::vector<int>> exps;
            enumerate_hom_exponents(static_cast<int>(L.size()), n, {}, exps);
            return RelationComboResult{std::move(coeffs), std::move(exps)};
        });
        regBuiltin("findlincombo", H2("findlincombo(f,L,topshift)", "express f as linear combination of series in L", {}, {}), [](DispatchContext& ctx) {
            if (ctx.args.size() != 3) throw std::runtime_error(runtimeErr(ctx.name, "expects 3 arguments: findlincombo(f,L,topshift)"));
            Series f = ctx.ev(0); auto L = ctx.evalListToSeries(ctx.args[1].get());
            int topshift = static_cast<int>(ctx.evi(2));
            auto coeffs = findhomcombo(f, L, 1, topshift, false);
            std::vector<std::vector<int>> exps;
            enumerate_hom_exponents(static_cast<int>(L.size()), 1, {}, exps);
            return RelationComboResult{std::move(coeffs), std::move(exps)};
        });
        regBuiltin("findlincombomodp", H2("findlincombomodp(f,L,p[,topshift])", "express f as linear combination of L mod p", {}, {}), [](DispatchContext& ctx) {
            if (ctx.args.size() != 3 && ctx.args.size() != 4) throw std::runtime_error(runtimeErr(ctx.name, "expects 3 or 4 arguments"));
            Series f = ctx.ev(0); auto L = ctx.evalListToSeries(ctx.args[1].get());
            int p = static_cast<int>(ctx.evi(2));
            int topshift = (ctx.args.size() == 4) ? static_cast<int>(ctx.evi(3)) : 0;
            auto coeffs = findlincombomodp(f, L, p, topshift);
            return FindlincombomodpResult{std::move(coeffs), p, L.size()};
        });
        regBuiltin("findnonhomcombo", H2("findnonhomcombo(f,L,n_list,topshift[,etaopt])", "express f as polynomial in L (nonhom)", {}, {}), [](DispatchContext& ctx) {
            if (ctx.args.size() < 4 || ctx.args.size() > 5) throw std::runtime_error(runtimeErr(ctx.name, "expects 4 or 5 arguments"));
            Series f = ctx.ev(0); auto L = ctx.evalListToSeries(ctx.args[1].get());
            auto n_list = ctx.evalListToInt(ctx.args[2].get());
            int topshift = static_cast<int>(ctx.evi(3));
            auto coeffs = findnonhomcombo(f, L, n_list, topshift, false);
            std::vector<std::vector<int>> exps;
            enumerate_nlist_exponents(n_list, {}, 0, exps);
            return RelationComboResult{std::move(coeffs), std::move(exps)};
        });
        regBuiltin("findmaxind", H2("findmaxind(L) or findmaxind(L, topshift)", "maximal linearly independent subset", {}, {}), [](DispatchContext& ctx) {
            if (ctx.args.size() == 1) return findmaxind(ctx.evalListToSeries(ctx.args[0].get()), 0);
            if (ctx.args.size() == 2) return findmaxind(ctx.evalListToSeries(ctx.args[0].get()), static_cast<int>(ctx.evi(1)));
            throw std::runtime_error(runtimeErr(ctx.name, "expects 1 or 2 arguments"));
        });
        regBuiltin("findpoly", H2("findpoly(x,y,deg1,deg2[,check])", "polynomial relation between two series", {}, {}), [](DispatchContext& ctx) {
            if (ctx.args.size() < 4 || ctx.args.size() > 5) throw std::runtime_error(runtimeErr(ctx.name, "expects 4 or 5 arguments"));
            Series x = ctx.ev(0), y = ctx.ev(1);
            int deg1 = static_cast<int>(ctx.evi(2)), deg2 = static_cast<int>(ctx.evi(3));
            std::optional<int> check; if (ctx.args.size() == 5) check = static_cast<int>(ctx.evi(4));
            auto basis = findpoly(x, y, deg1, deg2, check);
            std::vector<std::vector<int>> exps;
            for (int j = 0; j <= deg2; ++j) for (int i = 0; i <= deg1; ++i) exps.push_back({i, j});
            return RelationKernelResult{std::move(basis), std::move(exps)};
        });
        regBuiltin("legendre", H2("legendre(a,p)", "Legendre symbol (a/p)", {}, {}), [](DispatchContext& ctx) {
            if (ctx.args.size() != 2) throw std::runtime_error(runtimeErr(ctx.name, "expects 2 arguments"));
            return static_cast<int64_t>(legendre(ctx.evi(0), ctx.evi(1)));
        });
        regBuiltin("jacobi", H2("jacobi(a,n)", "Jacobi symbol (a/n)", {}, {}), [](DispatchContext& ctx) {
            if (ctx.args.size() != 2) throw std::runtime_error(runtimeErr(ctx.name, "expects 2 arguments"));
            return static_cast<int64_t>(jacobi(ctx.evi(0), ctx.evi(1)));
        });
        regBuiltin("kronecker", H2("kronecker(a,n)", "Kronecker symbol (a/n)", {}, {}), [](DispatchContext& ctx) {
            if (ctx.args.size() != 2) throw std::runtime_error(runtimeErr(ctx.name, "expects 2 arguments"));
            return static_cast<int64_t>(kronecker(ctx.evi(0), ctx.evi(1)));
        });
        regBuiltin("sigma", H2("sigma(n) or sigma(n,k)", "divisor sum \u03C3_k(n)", {}, {}), [](DispatchContext& ctx) {
            if (ctx.args.size() == 1) return sigma(static_cast<int>(ctx.evi(0)), 1);
            if (ctx.args.size() == 2) return sigma(static_cast<int>(ctx.evi(0)), static_cast<int>(ctx.evi(1)));
            throw std::runtime_error(runtimeErr(ctx.name, "expected sigma(n) or sigma(n,k)"));
        });
        regBuiltin("divisors", H2("divisors(n)", "sorted list of positive divisors of n", {}, {}), [](DispatchContext& ctx) {
            if (ctx.args.size() != 1) throw std::runtime_error(runtimeErr(ctx.name, "expects 1 argument"));
            auto d = divisors(static_cast<int>(ctx.evi(0)));
            std::cout << "["; for (size_t i = 0; i < d.size(); ++i) { if (i) std::cout << ", "; std::cout << d[i]; } std::cout << "]" << std::endl;
            return DisplayOnly{};
        });
        regBuiltin("modp", H2("modp(f,p)", "reduce series coefficients modulo prime p", {}, {}), [](DispatchContext& ctx) {
            if (ctx.args.size() != 2) throw std::runtime_error(runtimeErr(ctx.name, "expects 2 arguments"));
            return ctx.ev(0).modp(static_cast<int>(ctx.evi(1)));
        });
        regBuiltin("nterms", H2("nterms(f)", "count non-zero coefficients in series", {}, {}), [](DispatchContext& ctx) {
            if (ctx.args.size() != 1) throw std::runtime_error(runtimeErr(ctx.name, "expects 1 argument"));
            return static_cast<int64_t>(ctx.ev(0).c.size());
        });
        regBuiltin("mod", H2("mod(a,b)", "integer remainder a mod b", {}, {}), [](DispatchContext& ctx) {
            if (ctx.args.size() != 2) throw std::runtime_error(runtimeErr(ctx.name, "expects 2 arguments"));
            int64_t a = ctx.evi(0), b = ctx.evi(1);
            if (b == 0) throw std::runtime_error(runtimeErr(ctx.name, "modulus cannot be zero"));
            int64_t r = a % b;
            if (r < 0) r += (b < 0 ? -b : b);
            return r;
        });
        regBuiltin("mobius", H2("mobius(n)", "M\u00F6bius function \u03BC(n)", {}, {}), [](DispatchContext& ctx) {
            if (ctx.args.size() != 1) throw std::runtime_error(runtimeErr(ctx.name, "expects 1 argument"));
            return static_cast<int64_t>(mobius(static_cast<int>(ctx.evi(0))));
        });
        regBuiltin("mockqs", H2("mockqs(name, order, T)", "q-series expansion of mock theta function", {}, {}), [](DispatchContext& ctx) {
            if (ctx.args.size() != 3) throw std::runtime_error(runtimeErr(ctx.name, "expects 3 arguments: mockqs(name, order, T)"));
            if (ctx.args[0]->tag != Expr::Tag::Var) throw std::runtime_error(runtimeErr(ctx.name, "first argument must be a mock theta function name"));
            std::string fn = ctx.args[0]->varName;
            int ord = static_cast<int>(ctx.evi(1)), Tr = static_cast<int>(ctx.evi(2));
            if (!mock_valid(fn, ord)) throw std::runtime_error(runtimeErr(ctx.name, "unknown mock theta function " + fn + " of order " + std::to_string(ord)));
            return mockqs(fn, ord, Tr);
        });
        regBuiltin("mockdesorder", H2("mockdesorder(m)", "list mock theta function names of order m", {}, {}), [](DispatchContext& ctx) {
            if (ctx.args.size() != 1) throw std::runtime_error(runtimeErr(ctx.name, "expects 1 argument"));
            int m = static_cast<int>(ctx.evi(0));
            auto list = mockdesorder(m);
            if (list.empty()) throw std::runtime_error(runtimeErr(ctx.name, "order must be 2, 3, 5, 6, 7, 8, or 10"));
            std::cout << "There are " << list.size() << " mock theta functions of order " << m << ":" << std::endl;
            for (size_t i = 0; i < list.size(); ++i) std::cout << (i ? ", " : "") << list[i];
            std::cout << std::endl;
            return DisplayOnly{};
        });
        regBuiltin("euler_phi", H2("euler_phi(n)", "Euler's totient \u03C6(n)", {}, {}), [](DispatchContext& ctx) {
            if (ctx.args.size() != 1) throw std::runtime_error(runtimeErr(ctx.name, "expects 1 argument"));
            return static_cast<int64_t>(euler_phi(static_cast<int>(ctx.evi(0))));
        });
        regBuiltin("min", H2("min(a, b, ...)", "minimum of 2 or more integers", {}, {}), [](DispatchContext& ctx) {
            if (ctx.args.size() < 2) throw std::runtime_error(runtimeErr(ctx.name, "expects 2 or more arguments"));
            int64_t result = ctx.evi(0);
            for (size_t i = 1; i < ctx.args.size(); ++i) result = std::min(result, ctx.evi(i));
            return result;
        });
        regBuiltin("max", H2("max(a, b, ...)", "maximum of 2 or more integers", {}, {}), [](DispatchContext& ctx) {
            if (ctx.args.size() < 2) throw std::runtime_error(runtimeErr(ctx.name, "expects 2 or more arguments"));
            int64_t result = ctx.evi(0);
            for (size_t i = 1; i < ctx.args.size(); ++i) result = std::max(result, ctx.evi(i));
            return result;
        });
        regBuiltin("add", H2("add(expr, var, lo, hi)", "summation (use sum)", {}, {}), [](DispatchContext&) {
            throw std::runtime_error("add and sum use special syntax sum(expr, var, lo, hi); they are not called as regular functions");
        });
        regBuiltin("sum", H2("sum(expr, var, lo, hi)", "summation over index var from lo to hi", {}, {}), [](DispatchContext&) {
            throw std::runtime_error("sum uses special syntax sum(expr, var, lo, hi); it is not called as a regular function");
        });
        regBuiltin("load", H2("load(name)", "restore session from name.qsession file", {}, {}), [](DispatchContext& ctx) {
            if (ctx.args.size() != 1) throw std::runtime_error(runtimeErr(ctx.name, "expects 1 argument"));
            std::string nm;
            if (ctx.args[0]->tag == Expr::Tag::Var) nm = ctx.args[0]->varName;
            else if (ctx.args[0]->tag == Expr::Tag::IntLit) nm = std::to_string(ctx.args[0]->intVal);
            else throw std::runtime_error(runtimeErr(ctx.name, "load expects variable or integer name"));
            loadSession(nm, ctx.env);
            return DisplayOnly{};
        });
        regBuiltin("save", H2("save(name)", "save current session to name.qsession file", {}, {}), [](DispatchContext& ctx) {
            if (ctx.args.size() != 1) throw std::runtime_error(runtimeErr(ctx.name, "expects 1 argument"));
            std::string nm;
            if (ctx.args[0]->tag == Expr::Tag::Var) nm = ctx.args[0]->varName;
            else if (ctx.args[0]->tag == Expr::Tag::IntLit) nm = std::to_string(ctx.args[0]->intVal);
            else throw std::runtime_error(runtimeErr(ctx.name, "save expects variable or integer name"));
            saveSession(nm, ctx.env);
            return DisplayOnly{};
        });
        regBuiltin("eisenstein", H2("eisenstein(k,T)", "normalized Eisenstein series", {}, {}), [](DispatchContext& ctx) {
            if (ctx.args.size() != 2) throw std::runtime_error(runtimeErr(ctx.name, "expects 2 arguments"));
            return eisenstein(static_cast<int>(ctx.evi(0)), static_cast<int>(ctx.evi(1)));
        });
        regBuiltin("DELTA12", H2("DELTA12(T)", "discriminant cusp form q*\u03B7(q)^24", {}, {}), [](DispatchContext& ctx) {
            if (ctx.args.size() != 1) throw std::runtime_error(runtimeErr(ctx.name, "expects DELTA12(T)"));
            return DELTA12(static_cast<int>(ctx.evi(0)));
        });
        regBuiltin("EISENq", H2("EISENq(d,T)", "Eisenstein series E_d(q)", {}, {}), [](DispatchContext& ctx) {
            if (ctx.args.size() != 2) throw std::runtime_error(runtimeErr(ctx.name, "expects 2 arguments"));
            return EISENq(static_cast<int>(ctx.evi(0)), static_cast<int>(ctx.evi(1)));
        });
        regBuiltin("Phiq", H2("Phiq(j,T)", "divisor sum GF", {}, {}), [](DispatchContext& ctx) {
            if (ctx.args.size() != 2) throw std::runtime_error(runtimeErr(ctx.name, "expects 2 arguments"));
            return Phiq(static_cast<int>(ctx.evi(0)), static_cast<int>(ctx.evi(1)));
        });
        regBuiltin("rankgf", H2("rankgf(m,T)", "GF for partitions with rank m", {}, {}), [](DispatchContext& ctx) {
            if (ctx.args.size() != 2) throw std::runtime_error(runtimeErr(ctx.name, "expects 2 arguments"));
            return rankgf(static_cast<int>(ctx.evi(0)), static_cast<int>(ctx.evi(1)));
        });
        regBuiltin("crankgf", H2("crankgf(m,T)", "GF for partitions with crank m", {}, {}), [](DispatchContext& ctx) {
            if (ctx.args.size() != 2) throw std::runtime_error(runtimeErr(ctx.name, "expects 2 arguments"));
            return crankgf(static_cast<int>(ctx.evi(0)), static_cast<int>(ctx.evi(1)));
        });
        regBuiltin("vp", H2("vp(n, p)", "p-adic valuation", {}, {}), [](DispatchContext& ctx) {
            if (ctx.args.size() != 2) throw std::runtime_error(runtimeErr(ctx.name, "expects vp(n, p)"));
            return static_cast<int64_t>(vp(static_cast<int>(ctx.evi(0)), static_cast<int>(ctx.evi(1))));
        });
        regBuiltin("qdiff", H2("qdiff(f)", "q-derivative", {}, {}), [](DispatchContext& ctx) {
            if (ctx.args.size() != 1) throw std::runtime_error(runtimeErr(ctx.name, "expects 1 argument"));
            return qdiff(ctx.ev(0));
        });
        regBuiltin("subs_q", H2("subs_q(f,k)", "substitute q^k for q", {}, {}), [](DispatchContext& ctx) {
            if (ctx.args.size() != 2) throw std::runtime_error(runtimeErr(ctx.name, "expects 2 arguments"));
            return ctx.ev(0).subs_q(static_cast<int>(ctx.evi(1)));
        });

        regBuiltin("makebasisM", H2("makebasisM(k,T)", "basis of M_k(SL_2(Z))", {}, {}), [](DispatchContext& ctx) {
            if (ctx.args.size() != 2) throw std::runtime_error(runtimeErr(ctx.name, "expects makebasisM(k, T)"));
            auto basis = makebasisM(static_cast<int>(ctx.evi(0)), static_cast<int>(ctx.evi(1)));
            std::cout << "[" << basis.size() << " basis elements]" << std::endl;
            for (size_t i = 0; i < basis.size(); ++i)
                std::cout << "  " << (i + 1) << ": " << basis[i].str() << std::endl;
            return DisplayOnly{};
        });
        regBuiltin("makeALTbasisM", H2("makeALTbasisM(k,T)", "alternate basis of M_k(SL_2(Z))", {}, {}), [](DispatchContext& ctx) {
            if (ctx.args.size() != 2) throw std::runtime_error(runtimeErr(ctx.name, "expects makeALTbasisM(k, T)"));
            auto basis = makeALTbasisM(static_cast<int>(ctx.evi(0)), static_cast<int>(ctx.evi(1)));
            std::cout << "[" << basis.size() << " basis elements]" << std::endl;
            for (size_t i = 0; i < basis.size(); ++i)
                std::cout << "  " << (i + 1) << ": " << basis[i].str() << std::endl;
            return DisplayOnly{};
        });
        regBuiltin("makebasisPX", H2("makebasisPX(k,T)", "basis P*X[k]", {}, {}), [](DispatchContext& ctx) {
            if (ctx.args.size() != 2) throw std::runtime_error(runtimeErr(ctx.name, "expects makebasisPX(k, T)"));
            auto basis = makebasisPX(static_cast<int>(ctx.evi(0)), static_cast<int>(ctx.evi(1)));
            std::cout << "[" << basis.size() << " basis elements]" << std::endl;
            for (size_t i = 0; i < basis.size(); ++i)
                std::cout << "  " << (i + 1) << ": " << basis[i].str() << std::endl;
            return DisplayOnly{};
        });
        regBuiltin("QP2", H2("QP2(num, den)", "second periodic Bernoulli", {}, {}), [](DispatchContext& ctx) {
            if (ctx.args.size() != 2) throw std::runtime_error(runtimeErr(ctx.name, "expects QP2(num, den)"));
            Frac x(static_cast<int64_t>(ctx.evi(0)), static_cast<int64_t>(ctx.evi(1)));
            std::cout << QP2(x).str() << std::endl;
            return DisplayOnly{};
        });
        regBuiltin("getacuspord", H2("getacuspord(n,r,a,c)", "order of eta at cusp", {}, {}), [](DispatchContext& ctx) {
            if (ctx.args.size() != 4) throw std::runtime_error(runtimeErr(ctx.name, "expects getacuspord(n, r, a, c)"));
            Frac ord = getacuspord(static_cast<int>(ctx.evi(0)), static_cast<int>(ctx.evi(1)),
                                  static_cast<int>(ctx.evi(2)), static_cast<int>(ctx.evi(3)));
            std::cout << ord.str() << std::endl;
            return DisplayOnly{};
        });
        regBuiltin("cuspmake1", H2("cuspmake1(N)", "cusps for Gamma_1(N)", {}, {}), [](DispatchContext& ctx) {
            if (ctx.args.size() != 1) throw std::runtime_error(runtimeErr(ctx.name, "expects cuspmake1(N)"));
            auto cusps = cuspmake1(static_cast<int>(ctx.evi(0)));
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
        });
        regBuiltin("Gamma1ModFunc", H2("Gamma1ModFunc(L,N)", "check generalized eta product modular on Gamma_1(N)", {}, {}), [](DispatchContext& ctx) {
            if (ctx.args.size() != 2) throw std::runtime_error(runtimeErr(ctx.name, "expects Gamma1ModFunc(L, N)"));
            const Expr* listExpr = ctx.args[0].get();
            if (!listExpr || listExpr->tag != Expr::Tag::List)
                throw std::runtime_error(runtimeErr(ctx.name, "L must be list of [n,a,c]"));
            std::vector<std::vector<int>> L;
            for (const auto& elem : listExpr->elements) {
                if (!elem || elem->tag != Expr::Tag::List || elem->elements.size() < 3)
                    throw std::runtime_error(runtimeErr(ctx.name, "each element must be [n,a,c]"));
                L.push_back({static_cast<int>(evalToInt(elem->elements[0].get(), ctx.env, ctx.sumIndices)),
                             static_cast<int>(evalToInt(elem->elements[1].get(), ctx.env, ctx.sumIndices)),
                             static_cast<int>(evalToInt(elem->elements[2].get(), ctx.env, ctx.sumIndices))});
            }
            int N = static_cast<int>(ctx.evi(1));
            int r = Gamma1ModFunc(L, N);
            std::cout << (r ? "1" : "0") << std::endl;
            return static_cast<int64_t>(r);
        });
        regBuiltin("jac2eprod", H2("jac2eprod(var)", "convert Jacobi product to GETA form", {}, {}), [](DispatchContext& ctx) {
            if (ctx.args.size() != 1) throw std::runtime_error(runtimeErr(ctx.name, "expects jac2eprod(var)"));
            std::string vname = ctx.args[0]->tag == Expr::Tag::Var ? ctx.args[0]->varName : "";
            if (vname.empty()) throw std::runtime_error(runtimeErr(ctx.name, "argument must be variable name"));
            auto it = ctx.env.env.find(vname);
            if (it == ctx.env.env.end()) throw std::runtime_error(formatUndefinedVariableMsg(ctx.env, vname));
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
        });
        regBuiltin("eta2jac", H2("eta2jac(getalist)", "convert GETA list to Jacobi product", {}, {}), [](DispatchContext& ctx) {
            if (ctx.args.size() != 1) throw std::runtime_error(runtimeErr(ctx.name, "expects eta2jac(getalist)"));
            const Expr* listExpr = ctx.args[0].get();
            if (!listExpr || listExpr->tag != Expr::Tag::List)
                throw std::runtime_error(runtimeErr(ctx.name, "getalist must be list of [n,a,c]"));
            std::vector<std::tuple<int, int, Frac>> geta;
            for (const auto& elem : listExpr->elements) {
                if (!elem || elem->tag != Expr::Tag::List || elem->elements.size() < 3)
                    throw std::runtime_error(runtimeErr(ctx.name, "each element must be [n,a,c]"));
                int n = static_cast<int>(evalToInt(elem->elements[0].get(), ctx.env, ctx.sumIndices));
                int a = static_cast<int>(evalToInt(elem->elements[1].get(), ctx.env, ctx.sumIndices));
                Frac c(static_cast<int64_t>(evalToInt(elem->elements[2].get(), ctx.env, ctx.sumIndices)));
                geta.push_back({n, a, c});
            }
            std::vector<JacFactor> jac = eta2jac(geta);
            std::cout << jac2prod(jac) << std::endl;
            return jac;
        });
        regBuiltin("NS", H2("NS(m,n) or NS(k,r,n)", "spt crank rank multiplicity", {}, {}), [](DispatchContext& ctx) {
            if (ctx.args.size() == 2)
                return static_cast<int64_t>(NS(static_cast<int>(ctx.evi(0)), static_cast<int>(ctx.evi(1))));
            if (ctx.args.size() == 3)
                return static_cast<int64_t>(sptcrankresnum(static_cast<int>(ctx.evi(0)), static_cast<int>(ctx.evi(1)), static_cast<int>(ctx.evi(2))));
            throw std::runtime_error(runtimeErr(ctx.name, "expects NS(m,n) or NS(k,r,n)"));
        });
        regBuiltin("MBAR", H2("MBAR(m,n) or MBAR(k,r,n)", "overcrank multiplicity", {}, {}), [](DispatchContext& ctx) {
            if (ctx.args.size() == 2)
                return static_cast<int64_t>(MBAR(static_cast<int>(ctx.evi(0)), static_cast<int>(ctx.evi(1))));
            if (ctx.args.size() == 3)
                return static_cast<int64_t>(ocrankresnum(static_cast<int>(ctx.evi(0)), static_cast<int>(ctx.evi(1)), static_cast<int>(ctx.evi(2))));
            throw std::runtime_error(runtimeErr(ctx.name, "expects MBAR(m,n) or MBAR(k,r,n)"));
        });
        regBuiltin("M2N", H2("M2N(m,n) or M2N(k,r,n)", "M2-rank multiplicity", {}, {}), [](DispatchContext& ctx) {
            if (ctx.args.size() == 2)
                return static_cast<int64_t>(M2N(static_cast<int>(ctx.evi(0)), static_cast<int>(ctx.evi(1))));
            if (ctx.args.size() == 3)
                return static_cast<int64_t>(m2rankresnum(static_cast<int>(ctx.evi(0)), static_cast<int>(ctx.evi(1)), static_cast<int>(ctx.evi(2))));
            throw std::runtime_error(runtimeErr(ctx.name, "expects M2N(m,n) or M2N(k,r,n)"));
        });
        regBuiltin("GFDM2N", H2("GFDM2N(k1,k2,t,r[,T])", "GF for M2-rank", {}, {}), [](DispatchContext& ctx) {
            if (ctx.args.size() < 4 || ctx.args.size() > 5)
                throw std::runtime_error(runtimeErr(ctx.name, "expects GFDM2N(k1,k2,t,r) or GFDM2N(k1,k2,t,r,T)"));
            int k1 = static_cast<int>(ctx.evi(0)), k2 = static_cast<int>(ctx.evi(1));
            int t = static_cast<int>(ctx.evi(2)), r = static_cast<int>(ctx.evi(3));
            int Tr = (ctx.args.size() == 5) ? static_cast<int>(ctx.evi(4)) : 50;
            return GFDM2N(k1, k2, t, r, Tr);
        });
        regBuiltin("sptcrankresnum", H2("sptcrankresnum(k,r,n)", "spt crank residue number", {}, {}), [](DispatchContext& ctx) {
            if (ctx.args.size() != 3) throw std::runtime_error(runtimeErr(ctx.name, "expects 3 arguments"));
            return static_cast<int64_t>(sptcrankresnum(static_cast<int>(ctx.evi(0)), static_cast<int>(ctx.evi(1)), static_cast<int>(ctx.evi(2))));
        });
        regBuiltin("ocrankresnum", H2("ocrankresnum(k,r,n)", "overcrank residue number", {}, {}), [](DispatchContext& ctx) {
            if (ctx.args.size() != 3) throw std::runtime_error(runtimeErr(ctx.name, "expects 3 arguments"));
            return static_cast<int64_t>(ocrankresnum(static_cast<int>(ctx.evi(0)), static_cast<int>(ctx.evi(1)), static_cast<int>(ctx.evi(2))));
        });
        regBuiltin("pbar", H2("pbar(n)", "overpartition count", {}, {}), [](DispatchContext& ctx) {
            if (ctx.args.size() != 1) throw std::runtime_error(runtimeErr(ctx.name, "expects 1 argument"));
            return static_cast<int64_t>(pbar(static_cast<int>(ctx.evi(0))));
        });
        regBuiltin("betafind", H2("betafind(n[,T])", "find beta from unit alpha", {}, {}), [](DispatchContext& ctx) {
            if (ctx.args.size() < 1 || ctx.args.size() > 2) throw std::runtime_error(runtimeErr(ctx.name, "expects betafind(n) or betafind(n,T)"));
            int n = static_cast<int>(ctx.evi(0));
            int Tr = (ctx.args.size() >= 2) ? static_cast<int>(ctx.evi(1)) : ctx.T;
            Series a = Series::one(Tr);
            return betafind(unit_alpha, a, ctx.q, n, Tr);
        });
        regBuiltin("alphaup", H2("alphaup(alpha,T)", "Bailey alpha up", {}, {}), [](DispatchContext& ctx) {
            if (ctx.args.size() != 2) throw std::runtime_error(runtimeErr(ctx.name, "expects alphaup(alpha,T)"));
            Series alpha = ctx.ev(0);
            int Tr = static_cast<int>(ctx.evi(1));
            Series a = Series::one(Tr);
            return alphaup(alpha, a, ctx.q, Tr);
        });
        regBuiltin("alphadown", H2("alphadown(alpha,T)", "Bailey alpha down", {}, {}), [](DispatchContext& ctx) {
            if (ctx.args.size() != 2) throw std::runtime_error(runtimeErr(ctx.name, "expects alphadown(alpha,T)"));
            Series alpha = ctx.ev(0);
            int Tr = static_cast<int>(ctx.evi(1));
            Series a = Series::one(Tr);
            return alphadown(alpha, a, ctx.q, Tr);
        });
        regBuiltin("bailey_sum", H2("bailey_sum(pair,N,T)", "Bailey pair sum", {}, {}), [](DispatchContext& ctx) {
            if (ctx.args.size() != 3) throw std::runtime_error(runtimeErr(ctx.name, "expects bailey_sum(pair,N,T)"));
            int pair = static_cast<int>(ctx.evi(0));
            int N = static_cast<int>(ctx.evi(1));
            int Tr = static_cast<int>(ctx.evi(2));
            Series a = Series::one(Tr);
            BetaFunc bf = (pair == 1) ? rr_beta : (pair == 2) ? gg_beta : unit_beta;
            return bailey_sum(bf, a, ctx.q, N, Tr);
        });
        regBuiltin("cuspmake", H2("cuspmake(N)", "cusps for Gamma_0(N)", {}, {}), [](DispatchContext& ctx) {
            if (ctx.args.size() != 1) throw std::runtime_error(runtimeErr(ctx.name, "expects cuspmake(N)"));
            int N = static_cast<int>(ctx.evi(0));
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
        });
        regBuiltin("cuspord", H2("cuspord(GP,cusp)", "order of cusp", {}, {}), [](DispatchContext& ctx) {
            if (ctx.args.size() != 2) throw std::runtime_error(runtimeErr(ctx.name, "expects cuspord(GP, cusp)"));
            std::vector<int> GP = ctx.evalListToInt(ctx.args[0].get());
            std::vector<int> cusp = ctx.evalListToInt(ctx.args[1].get());
            if (cusp.size() < 2) throw std::runtime_error(runtimeErr(ctx.name, "cusp must be [num,den]"));
            Frac ord = cuspord(GP, cusp[0], cusp[1]);
            std::cout << ord.str() << std::endl;
            return DisplayOnly{};
        });
        regBuiltin("gammacheck", H2("gammacheck(GP,N)", "check modularity on Gamma", {}, {}), [](DispatchContext& ctx) {
            if (ctx.args.size() != 2) throw std::runtime_error(runtimeErr(ctx.name, "expects gammacheck(GP, N)"));
            std::vector<int> GP = ctx.evalListToInt(ctx.args[0].get());
            int N = static_cast<int>(ctx.evi(1));
            return static_cast<int64_t>(gammacheck(GP, N));
        });
        regBuiltin("etaprodtoqseries", H2("etaprodtoqseries(GP,T)", "eta product to q-series", {}, {}), [](DispatchContext& ctx) {
            if (ctx.args.size() != 2) throw std::runtime_error(runtimeErr(ctx.name, "expects etaprodtoqseries(GP, T)"));
            std::vector<int> GP = ctx.evalListToInt(ctx.args[0].get());
            int Tr = static_cast<int>(ctx.evi(1));
            return etaprodtoqseries(GP, Tr);
        });
        regBuiltin("provemodfuncGAMMA0id", H2("provemodfuncGAMMA0id(etaid,N)", "prove eta product identity on Gamma_0(N)", {}, {}), [](DispatchContext& ctx) {
            if (ctx.args.size() != 2) throw std::runtime_error(runtimeErr(ctx.name, "expects provemodfuncGAMMA0id(etaid, N)"));
            const Expr* listExpr = ctx.args[0].get();
            if (!listExpr || listExpr->tag != Expr::Tag::List)
                throw std::runtime_error(runtimeErr(ctx.name, "etaid must be list of [c, GP] terms"));
            std::vector<std::pair<Frac, std::vector<int>>> etaid;
            for (const auto& term : listExpr->elements) {
                if (!term || term->tag != Expr::Tag::List || term->elements.size() < 1)
                    throw std::runtime_error(runtimeErr(ctx.name, "each term must be [c] or [c, t1, r1, ...]"));
                Frac c = Frac(static_cast<int64_t>(evalToInt(term->elements[0].get(), ctx.env, ctx.sumIndices)));
                std::vector<int> GP;
                if (term->elements.size() >= 2) {
                    for (size_t i = 1; i < term->elements.size(); ++i)
                        GP.push_back(static_cast<int>(evalToInt(term->elements[i].get(), ctx.env, ctx.sumIndices)));
                }
                etaid.push_back({c, GP});
            }
            int N = static_cast<int>(ctx.evi(1));
            ProveModfuncResult res = provemodfuncGAMMA0id(etaid, N);
            std::cout << "sturm_bound=" << res.sturm_bound << " proven=" << res.proven << " " << res.message << std::endl;
            return static_cast<int64_t>(res.proven);
        });
        regBuiltin("provemodfuncGAMMA0idBATCH", H2("provemodfuncGAMMA0idBATCH(etaids,N)", "batch prove eta identities", {}, {}), [](DispatchContext& ctx) {
            if (ctx.args.size() != 2) throw std::runtime_error(runtimeErr(ctx.name, "expects provemodfuncGAMMA0idBATCH(etaids, N)"));
            const Expr* listExpr = ctx.args[0].get();
            if (!listExpr || listExpr->tag != Expr::Tag::List)
                throw std::runtime_error(runtimeErr(ctx.name, "etaids must be list of etaid terms"));
            std::vector<std::vector<std::pair<Frac, std::vector<int>>>> etaids;
            for (const auto& etaidExpr : listExpr->elements) {
                if (!etaidExpr || etaidExpr->tag != Expr::Tag::List)
                    throw std::runtime_error(runtimeErr(ctx.name, "each etaid must be list of [c, GP] terms"));
                std::vector<std::pair<Frac, std::vector<int>>> etaid;
                for (const auto& term : etaidExpr->elements) {
                    if (!term || term->tag != Expr::Tag::List || term->elements.size() < 1)
                        throw std::runtime_error(runtimeErr(ctx.name, "each term must be [c] or [c, t1, r1, ...]"));
                    Frac c = Frac(static_cast<int64_t>(evalToInt(term->elements[0].get(), ctx.env, ctx.sumIndices)));
                    std::vector<int> GP;
                    if (term->elements.size() >= 2) {
                        for (size_t i = 1; i < term->elements.size(); ++i)
                            GP.push_back(static_cast<int>(evalToInt(term->elements[i].get(), ctx.env, ctx.sumIndices)));
                    }
                    etaid.push_back({c, GP});
                }
                etaids.push_back(std::move(etaid));
            }
            int N = static_cast<int>(ctx.evi(1));
            auto results = provemodfuncGAMMA0idBATCH(etaids, N);
            int all_proven = 1;
            for (size_t i = 0; i < results.size(); ++i) {
                std::cout << "id[" << (i+1) << "] sturm_bound=" << results[i].sturm_bound
                          << " proven=" << results[i].proven << " " << results[i].message << std::endl;
                if (results[i].proven != 1) all_proven = 0;
            }
            return static_cast<int64_t>(all_proven);
        });
        regBuiltin("provemodfuncid", H2("provemodfuncid(jacid,N)", "prove Jacobi product identity", {}, {}), [](DispatchContext& ctx) {
            if (ctx.args.size() != 2) throw std::runtime_error(runtimeErr(ctx.name, "expects provemodfuncid(jacid, N)"));
            const Expr* listExpr = ctx.args[0].get();
            if (!listExpr || listExpr->tag != Expr::Tag::List)
                throw std::runtime_error(runtimeErr(ctx.name, "jacid must be list of [coeff, jaclist] terms"));
            std::vector<std::pair<Frac, std::vector<JacFactor>>> jacid;
            for (const auto& term : listExpr->elements) {
                if (!term || term->tag != Expr::Tag::List || term->elements.size() < 1)
                    throw std::runtime_error(runtimeErr(ctx.name, "each term must be [coeff] or [coeff, [a,b,exp], ...]"));
                Frac c = Frac(static_cast<int64_t>(evalToInt(term->elements[0].get(), ctx.env, ctx.sumIndices)));
                std::vector<JacFactor> jac;
                if (term->elements.size() >= 2) {
                    for (size_t i = 1; i < term->elements.size(); ++i) {
                        const Expr* fac = term->elements[i].get();
                        if (!fac || fac->tag != Expr::Tag::List || fac->elements.size() < 3)
                            throw std::runtime_error(runtimeErr(ctx.name, "each JacFactor must be [a, b, exp]"));
                        int a = static_cast<int>(evalToInt(fac->elements[0].get(), ctx.env, ctx.sumIndices));
                        int b = static_cast<int>(evalToInt(fac->elements[1].get(), ctx.env, ctx.sumIndices));
                        Frac e = Frac(static_cast<int64_t>(evalToInt(fac->elements[2].get(), ctx.env, ctx.sumIndices)));
                        jac.push_back({a, b, e});
                    }
                }
                jacid.push_back({c, jac});
            }
            int N = static_cast<int>(ctx.evi(1));
            ProveModfuncIdResult res = provemodfuncid(jacid, N);
            std::cout << "sturm_bound=" << res.sturm_bound << " proven=" << res.proven << " " << res.message << std::endl;
            return static_cast<int64_t>(res.proven);
        });
        regBuiltin("provemodfuncidBATCH", H2("provemodfuncidBATCH(jacids,N)", "batch prove Jacobi identities", {}, {}), [](DispatchContext& ctx) {
            if (ctx.args.size() != 2) throw std::runtime_error(runtimeErr(ctx.name, "expects provemodfuncidBATCH(jacids, N)"));
            const Expr* listExpr = ctx.args[0].get();
            if (!listExpr || listExpr->tag != Expr::Tag::List)
                throw std::runtime_error(runtimeErr(ctx.name, "jacids must be list of jacid"));
            std::vector<std::vector<std::pair<Frac, std::vector<JacFactor>>>> jacids;
            for (const auto& jacidExpr : listExpr->elements) {
                if (!jacidExpr || jacidExpr->tag != Expr::Tag::List)
                    throw std::runtime_error(runtimeErr(ctx.name, "each jacid must be list of [coeff, jaclist] terms"));
                std::vector<std::pair<Frac, std::vector<JacFactor>>> jacid;
                for (const auto& term : jacidExpr->elements) {
                    if (!term || term->tag != Expr::Tag::List || term->elements.size() < 1)
                        throw std::runtime_error(runtimeErr(ctx.name, "each term must be [coeff] or [coeff, [a,b,exp], ...]"));
                    Frac c = Frac(static_cast<int64_t>(evalToInt(term->elements[0].get(), ctx.env, ctx.sumIndices)));
                    std::vector<JacFactor> jac;
                    if (term->elements.size() >= 2) {
                        for (size_t i = 1; i < term->elements.size(); ++i) {
                            const Expr* fac = term->elements[i].get();
                            if (!fac || fac->tag != Expr::Tag::List || fac->elements.size() < 3)
                                throw std::runtime_error(runtimeErr(ctx.name, "each JacFactor must be [a, b, exp]"));
                            int a = static_cast<int>(evalToInt(fac->elements[0].get(), ctx.env, ctx.sumIndices));
                            int b = static_cast<int>(evalToInt(fac->elements[1].get(), ctx.env, ctx.sumIndices));
                            Frac e = Frac(static_cast<int64_t>(evalToInt(fac->elements[2].get(), ctx.env, ctx.sumIndices)));
                            jac.push_back({a, b, e});
                        }
                    }
                    jacid.push_back({c, jac});
                }
                jacids.push_back(std::move(jacid));
            }
            int N = static_cast<int>(ctx.evi(1));
            auto results = provemodfuncidBATCH(jacids, N);
            int all_proven = 1;
            for (size_t i = 0; i < results.size(); ++i) {
                std::cout << "id[" << (i+1) << "] sturm_bound=" << results[i].sturm_bound
                          << " proven=" << results[i].proven << " " << results[i].message << std::endl;
                if (results[i].proven != 1) all_proven = 0;
            }
            return static_cast<int64_t>(all_proven);
        });
        regBuiltin("partition", H2("partition(n)", "partition number p(n)", {}, {}), [](DispatchContext& ctx) {
            if (ctx.args.size() != 1) throw std::runtime_error(runtimeErr(ctx.name, "expects 1 argument"));
            Frac p = partition_number(static_cast<int>(ctx.evi(0)));
            std::cout << p.str() << std::endl;
            return DisplayOnly{};
        });
        regBuiltin("partitions", H2("partitions(n)", "list all partitions of n", {}, {}), [](DispatchContext& ctx) {
            if (ctx.args.size() != 1) throw std::runtime_error(runtimeErr(ctx.name, "expects 1 argument"));
            int n = static_cast<int>(ctx.evi(0));
            if (n > 100) throw std::runtime_error(runtimeErr(ctx.name, "n too large (limit 100)"));
            auto raw = enumerate_partitions(n);
            std::vector<Partition> result;
            result.reserve(raw.size());
            for (auto& v : raw) {
                Partition ptn;
                ptn.parts = std::move(v);
                result.push_back(std::move(ptn));
            }
            return result;
        });
        regBuiltin("conjpart", H2("conjpart(ptn)", "conjugate partition", {}, {}), [](DispatchContext& ctx) {
            if (ctx.args.size() != 1) throw std::runtime_error(runtimeErr(ctx.name, "expects 1 argument"));
            Partition ptn = ctx.evalToPartition(ctx.args[0].get());
            Partition conj;
            conj.parts = conjugate_partition(ptn.parts);
            return conj;
        });
        regBuiltin("lp", H2("lp(ptn)", "largest part", {}, {}), [](DispatchContext& ctx) {
            if (ctx.args.size() != 1) throw std::runtime_error(runtimeErr(ctx.name, "expects 1 argument"));
            Partition ptn = ctx.evalToPartition(ctx.args[0].get());
            if (ptn.parts.empty()) return static_cast<int64_t>(0);
            return *std::max_element(ptn.parts.begin(), ptn.parts.end());
        });
        regBuiltin("np_parts", H2("np_parts(ptn)", "number of parts", {}, {}), [](DispatchContext& ctx) {
            if (ctx.args.size() != 1) throw std::runtime_error(runtimeErr(ctx.name, "expects 1 argument"));
            Partition ptn = ctx.evalToPartition(ctx.args[0].get());
            return static_cast<int64_t>(ptn.parts.size());
        });
        regBuiltin("nep", H2("nep(ptn)", "number of even parts", {}, {}), [](DispatchContext& ctx) {
            if (ctx.args.size() != 1) throw std::runtime_error(runtimeErr(ctx.name, "expects 1 argument"));
            Partition ptn = ctx.evalToPartition(ctx.args[0].get());
            int64_t count = 0;
            for (int64_t p : ptn.parts)
                if (p % 2 == 0) ++count;
            return count;
        });
        regBuiltin("ptnnorm", H2("ptnnorm(ptn)", "sum of parts", {}, {}), [](DispatchContext& ctx) {
            if (ctx.args.size() != 1) throw std::runtime_error(runtimeErr(ctx.name, "expects 1 argument"));
            Partition ptn = ctx.evalToPartition(ctx.args[0].get());
            int64_t sum = 0;
            for (int64_t p : ptn.parts) sum += p;
            return sum;
        });
        regBuiltin("drank", H2("drank(ptn)", "Dyson rank", {}, {}), [](DispatchContext& ctx) {
            if (ctx.args.size() != 1) throw std::runtime_error(runtimeErr(ctx.name, "expects 1 argument"));
            Partition ptn = ctx.evalToPartition(ctx.args[0].get());
            return ptn_drank(ptn.parts);
        });
        regBuiltin("agcrank", H2("agcrank(ptn)", "Andrews-Garvan crank", {}, {}), [](DispatchContext& ctx) {
            if (ctx.args.size() != 1) throw std::runtime_error(runtimeErr(ctx.name, "expects 1 argument"));
            Partition ptn = ctx.evalToPartition(ctx.args[0].get());
            return ptn_agcrank(ptn.parts);
        });
        regBuiltin("kapPD", H2("kapPD(ptn)", "kappa for distinct parts", {}, {}), [](DispatchContext& ctx) {
            if (ctx.args.size() != 1) throw std::runtime_error(runtimeErr(ctx.name, "expects 1 argument"));
            Partition ptn = ctx.evalToPartition(ctx.args[0].get());
            return ptn_kapPD(ptn.parts);
        });
        regBuiltin("lamPD", H2("lamPD(ptn)", "lambda for distinct parts", {}, {}), [](DispatchContext& ctx) {
            if (ctx.args.size() != 1) throw std::runtime_error(runtimeErr(ctx.name, "expects 1 argument"));
            Partition ptn = ctx.evalToPartition(ctx.args[0].get());
            return ptn_lamPD(ptn.parts);
        });
        regBuiltin("numLE", H2("numLE(ptn)", "number of parts <= something", {}, {}), [](DispatchContext& ctx) {
            if (ctx.args.size() != 1) throw std::runtime_error(runtimeErr(ctx.name, "expects 1 argument"));
            Partition ptn = ctx.evalToPartition(ctx.args[0].get());
            return ptn_numLE(ptn.parts);
        });
        regBuiltin("ptnDP", H2("ptnDP(ptn)", "is distinct parts", {}, {}), [](DispatchContext& ctx) {
            if (ctx.args.size() != 1) throw std::runtime_error(runtimeErr(ctx.name, "expects 1 argument"));
            Partition ptn = ctx.evalToPartition(ctx.args[0].get());
            return static_cast<int64_t>(ptn_is_distinct(ptn.parts) ? 1 : 0);
        });
        regBuiltin("ptnOP", H2("ptnOP(ptn)", "is odd parts", {}, {}), [](DispatchContext& ctx) {
            if (ctx.args.size() != 1) throw std::runtime_error(runtimeErr(ctx.name, "expects 1 argument"));
            Partition ptn = ctx.evalToPartition(ctx.args[0].get());
            return static_cast<int64_t>(ptn_is_odd_parts(ptn.parts) ? 1 : 0);
        });
        regBuiltin("ptnRR", H2("ptnRR(ptn)", "is Rogers-Ramanujan", {}, {}), [](DispatchContext& ctx) {
            if (ctx.args.size() != 1) throw std::runtime_error(runtimeErr(ctx.name, "expects 1 argument"));
            Partition ptn = ctx.evalToPartition(ctx.args[0].get());
            return static_cast<int64_t>(ptn_is_RR(ptn.parts) ? 1 : 0);
        });
        regBuiltin("ptnCC", H2("ptnCC(ptn)", "is Capparelli", {}, {}), [](DispatchContext& ctx) {
            if (ctx.args.size() != 1) throw std::runtime_error(runtimeErr(ctx.name, "expects 1 argument"));
            Partition ptn = ctx.evalToPartition(ctx.args[0].get());
            return static_cast<int64_t>(ptn_is_CC(ptn.parts) ? 1 : 0);
        });
        regBuiltin("ptnSCHUR", H2("ptnSCHUR(ptn)", "is Schur", {}, {}), [](DispatchContext& ctx) {
            if (ctx.args.size() != 1) throw std::runtime_error(runtimeErr(ctx.name, "expects 1 argument"));
            Partition ptn = ctx.evalToPartition(ctx.args[0].get());
            return static_cast<int64_t>(ptn_is_SCHUR(ptn.parts) ? 1 : 0);
        });
        regBuiltin("ptnOE", H2("ptnOE(ptn)", "is odd-equal", {}, {}), [](DispatchContext& ctx) {
            if (ctx.args.size() != 1) throw std::runtime_error(runtimeErr(ctx.name, "expects 1 argument"));
            Partition ptn = ctx.evalToPartition(ctx.args[0].get());
            return static_cast<int64_t>(ptn_is_OE(ptn.parts) ? 1 : 0);
        });
        regBuiltin("PDP", H2("PDP(n)", "count distinct parts partitions", {}, {}), [](DispatchContext& ctx) {
            if (ctx.args.size() != 1) throw std::runtime_error(runtimeErr(ctx.name, "expects 1 argument"));
            return count_PDP(static_cast<int>(ctx.evi(0)));
        });
        regBuiltin("POE", H2("POE(n)", "count odd parts partitions", {}, {}), [](DispatchContext& ctx) {
            if (ctx.args.size() != 1) throw std::runtime_error(runtimeErr(ctx.name, "expects 1 argument"));
            return count_POE(static_cast<int>(ctx.evi(0)));
        });
        regBuiltin("PRR", H2("PRR(n)", "count RR partitions", {}, {}), [](DispatchContext& ctx) {
            if (ctx.args.size() != 1) throw std::runtime_error(runtimeErr(ctx.name, "expects 1 argument"));
            return count_PRR(static_cast<int>(ctx.evi(0)));
        });
        regBuiltin("PSCHUR", H2("PSCHUR(n)", "count Schur partitions", {}, {}), [](DispatchContext& ctx) {
            if (ctx.args.size() != 1) throw std::runtime_error(runtimeErr(ctx.name, "expects 1 argument"));
            return count_PSCHUR(static_cast<int>(ctx.evi(0)));
        });
        regBuiltin("overptns", H2("overptns(n)", "list overpartitions", {}, {}), [](DispatchContext& ctx) {
            if (ctx.args.size() != 1) throw std::runtime_error(runtimeErr(ctx.name, "expects 1 argument"));
            int n = static_cast<int>(ctx.evi(0));
            auto ops = enumerate_overpartitions(n);
            for (const auto& [dp, p] : ops) {
                std::cout << "[" << formatPartition(Partition{dp}) << ", " << formatPartition(Partition{p}) << "]" << std::endl;
            }
            std::cout << "Total: " << ops.size() << " overpartitions" << std::endl;
            return DisplayOnly{};
        });
        regBuiltin("overptnrank", H2("overptnrank(dptn,ptn)", "overpartition rank", {}, {}), [](DispatchContext& ctx) {
            if (ctx.args.size() != 2) throw std::runtime_error(runtimeErr(ctx.name, "expects 2 arguments"));
            Partition dptn = ctx.evalToPartition(ctx.args[0].get());
            Partition ptn = ctx.evalToPartition(ctx.args[1].get());
            return overptn_rank(dptn.parts, ptn.parts);
        });
        regBuiltin("overptncrank", H2("overptncrank(dptn,ptn)", "overpartition crank", {}, {}), [](DispatchContext& ctx) {
            if (ctx.args.size() != 2) throw std::runtime_error(runtimeErr(ctx.name, "expects 2 arguments"));
            Partition dptn = ctx.evalToPartition(ctx.args[0].get());
            Partition ptn = ctx.evalToPartition(ctx.args[1].get());
            return overptn_crank(dptn.parts, ptn.parts);
        });
        regBuiltin("rvec", H2("rvec(ptn,t,k)", "t-core r-vector component", {}, {}), [](DispatchContext& ctx) {
            if (ctx.args.size() != 3) throw std::runtime_error(runtimeErr(ctx.name, "expects 3 arguments: rvec(ptn, t, k)"));
            Partition ptn = ctx.evalToPartition(ctx.args[0].get());
            int t = static_cast<int>(ctx.evi(1));
            int k = static_cast<int>(ctx.evi(2));
            if (t <= 0) throw std::runtime_error(runtimeErr(ctx.name, "t must be positive"));
            return tcore_rvec(ptn.parts, t, k);
        });
        regBuiltin("istcore", H2("istcore(ptn,t)", "is t-core", {}, {}), [](DispatchContext& ctx) {
            if (ctx.args.size() != 2) throw std::runtime_error(runtimeErr(ctx.name, "expects 2 arguments: istcore(ptn, t)"));
            Partition ptn = ctx.evalToPartition(ctx.args[0].get());
            int t = static_cast<int>(ctx.evi(1));
            if (t <= 0) throw std::runtime_error(runtimeErr(ctx.name, "t must be positive"));
            return static_cast<int64_t>(tcore_istcore(ptn.parts, t) ? 1 : 0);
        });
        regBuiltin("tcoreofptn", H2("tcoreofptn(ptn,t)", "t-core of partition", {}, {}), [](DispatchContext& ctx) {
            if (ctx.args.size() != 2) throw std::runtime_error(runtimeErr(ctx.name, "expects 2 arguments: tcoreofptn(ptn, t)"));
            Partition ptn = ctx.evalToPartition(ctx.args[0].get());
            int t = static_cast<int>(ctx.evi(1));
            if (t <= 0) throw std::runtime_error(runtimeErr(ctx.name, "t must be positive"));
            Partition core;
            core.parts = tcore_tcoreofptn(ptn.parts, t);
            return core;
        });
        regBuiltin("tcores", H2("tcores(t,n)", "list t-cores of n", {}, {}), [](DispatchContext& ctx) {
            if (ctx.args.size() != 2) throw std::runtime_error(runtimeErr(ctx.name, "expects 2 arguments: tcores(t, n)"));
            int t = static_cast<int>(ctx.evi(0));
            int n = static_cast<int>(ctx.evi(1));
            if (t <= 0) throw std::runtime_error(runtimeErr(ctx.name, "t must be positive"));
            if (n > 100) throw std::runtime_error(runtimeErr(ctx.name, "n too large (limit 100)"));
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
        });
        regBuiltin("ptn2nvec", H2("ptn2nvec(ptn,t)", "partition to n-vector", {}, {}), [](DispatchContext& ctx) {
            if (ctx.args.size() != 2) throw std::runtime_error(runtimeErr(ctx.name, "expects 2 arguments: ptn2nvec(ptn, t)"));
            Partition ptn = ctx.evalToPartition(ctx.args[0].get());
            int t = static_cast<int>(ctx.evi(1));
            if (t <= 0) throw std::runtime_error(runtimeErr(ctx.name, "t must be positive"));
            Partition nvec;
            nvec.parts = tcore_ptn2nvec(ptn.parts, t);
            return nvec;
        });
        regBuiltin("ptn2rvec", H2("ptn2rvec(ptn,t)", "partition to r-vector", {}, {}), [](DispatchContext& ctx) {
            if (ctx.args.size() != 2) throw std::runtime_error(runtimeErr(ctx.name, "expects 2 arguments: ptn2rvec(ptn, t)"));
            Partition ptn = ctx.evalToPartition(ctx.args[0].get());
            int t = static_cast<int>(ctx.evi(1));
            if (t <= 0) throw std::runtime_error(runtimeErr(ctx.name, "t must be positive"));
            Partition rvec;
            rvec.parts = tcore_ptn2rvec(ptn.parts, t);
            return rvec;
        });
        regBuiltin("nvec2ptn", H2("nvec2ptn(nvec)", "n-vector to partition", {}, {}), [](DispatchContext& ctx) {
            if (ctx.args.size() != 1) throw std::runtime_error(runtimeErr(ctx.name, "expects 1 argument: nvec2ptn(nvec)"));
            Partition nvec = ctx.evalToPartition(ctx.args[0].get());
            Partition result;
            result.parts = tcore_nvec2ptn(nvec.parts);
            return result;
        });
        regBuiltin("nvec2alphavec", H2("nvec2alphavec(nvec)", "n-vector to alpha (t=5,7)", {}, {}), [](DispatchContext& ctx) {
            if (ctx.args.size() != 1) throw std::runtime_error(runtimeErr(ctx.name, "expects 1 argument: nvec2alphavec(nvec)"));
            Partition nvec = ctx.evalToPartition(ctx.args[0].get());
            int t = static_cast<int>(nvec.parts.size());
            if (t == 5) {
                int64_t n0 = nvec.parts[0], n1 = nvec.parts[1], n2 = nvec.parts[2], n3 = nvec.parts[3];
                Frac a3 = Frac(1,5)*Frac(n0) + Frac(1,5) + Frac(3,5)*Frac(n2) + Frac(2,5)*Frac(n1) + Frac(4,5)*Frac(n3);
                Frac a0 = Frac(-1,5)*Frac(n3) + Frac(1,5)*Frac(n0) + Frac(1,5) - Frac(2,5)*Frac(n2) - Frac(3,5)*Frac(n1);
                Frac a2 = Frac(-1,5)*Frac(n1) - Frac(2,5)*Frac(n3) - Frac(3,5)*Frac(n0) + Frac(2,5) + Frac(1,5)*Frac(n2);
                Frac a1 = Frac(-4,5)*Frac(n2) - Frac(1,5)*Frac(n1) - Frac(2,5)*Frac(n3) - Frac(3,5)*Frac(n0) + Frac(2,5);
                Frac a4 = Frac(1) - a0 - a1 - a2 - a3;
                std::cout << "[" << a0.str() << ", " << a1.str() << ", " << a2.str() << ", " << a3.str() << ", " << a4.str() << "]" << std::endl;
                return DisplayOnly{};
            }
            if (t == 7) {
                int64_t n0 = nvec.parts[0], n1 = nvec.parts[1], n2 = nvec.parts[2], n3 = nvec.parts[3], n4 = nvec.parts[4], n5 = nvec.parts[5];
                Frac a3 = Frac(6,7)*Frac(n1) + Frac(5,7)*Frac(n3) + Frac(1,7)*Frac(n4) + Frac(3,7)*Frac(n0) - Frac(1,7) + Frac(2,7)*Frac(n2) + Frac(4,7)*Frac(n5);
                Frac a4 = Frac(-1,7)*Frac(n4) - Frac(3,7)*Frac(n0) + Frac(1,7) - Frac(2,7)*Frac(n2) - Frac(4,7)*Frac(n5) + Frac(2,7)*Frac(n3) + Frac(1,7)*Frac(n1);
                Frac a0 = Frac(-1,7)*Frac(n5) + Frac(1,7)*Frac(n0) - Frac(2,7)*Frac(n4) + Frac(2,7) - Frac(4,7)*Frac(n2) - Frac(3,7)*Frac(n3) - Frac(5,7)*Frac(n1);
                Frac a2 = Frac(3,7)*Frac(n2) + Frac(6,7)*Frac(n5) + Frac(1,7)*Frac(n0) + Frac(5,7)*Frac(n4) + Frac(2,7) + Frac(4,7)*Frac(n3) + Frac(2,7)*Frac(n1);
                Frac a1 = Frac(-2,7)*Frac(n0) + Frac(3,7) + Frac(1,7)*Frac(n2) + Frac(2,7)*Frac(n5) - Frac(3,7)*Frac(n4) - Frac(1,7)*Frac(n3) - Frac(4,7)*Frac(n1);
                Frac a5 = Frac(-5,7)*Frac(n3) - Frac(1,7)*Frac(n4) - Frac(3,7)*Frac(n0) + Frac(1,7) - Frac(2,7)*Frac(n2) - Frac(4,7)*Frac(n5) + Frac(1,7)*Frac(n1);
                Frac a6 = Frac(1) - a0 - a1 - a2 - a3 - a4 - a5;
                std::cout << "[" << a0.str() << ", " << a1.str() << ", " << a2.str() << ", " << a3.str() << ", " << a4.str() << ", " << a5.str() << ", " << a6.str() << "]" << std::endl;
                return DisplayOnly{};
            }
            throw std::runtime_error(runtimeErr(ctx.name, "only implemented for t=5 and t=7"));
        });
        regBuiltin("tquot", H2("tquot(ptn,t)", "t-quotient", {}, {}), [](DispatchContext& ctx) {
            if (ctx.args.size() != 2) throw std::runtime_error(runtimeErr(ctx.name, "expects 2 arguments: tquot(ptn, t)"));
            Partition ptn = ctx.evalToPartition(ctx.args[0].get());
            int t = static_cast<int>(ctx.evi(1));
            if (t <= 0) throw std::runtime_error(runtimeErr(ctx.name, "t must be positive"));
            auto raw = tcore_tquot(ptn.parts, t);
            std::vector<Partition> result;
            for (auto& v : raw) {
                Partition p;
                p.parts = std::move(v);
                result.push_back(std::move(p));
            }
            return result;
        });
        regBuiltin("phi1", H2("phi1(ptn,t)", "GSK bijection: core + quotient", {}, {}), [](DispatchContext& ctx) {
            if (ctx.args.size() != 2) throw std::runtime_error(runtimeErr(ctx.name, "expects 2 arguments: phi1(ptn, t)"));
            Partition ptn = ctx.evalToPartition(ctx.args[0].get());
            int t = static_cast<int>(ctx.evi(1));
            if (t <= 0) throw std::runtime_error(runtimeErr(ctx.name, "t must be positive"));
            Phi1Result res;
            res.core.parts = tcore_tcoreofptn(ptn.parts, t);
            auto raw_quot = tcore_tquot(ptn.parts, t);
            for (auto& v : raw_quot) {
                Partition p;
                p.parts = std::move(v);
                res.quotient.push_back(std::move(p));
            }
            return res;
        });
        regBuiltin("invphi1", H2("invphi1(phi1result,t)", "inverse GSK bijection", {}, {}), [](DispatchContext& ctx) {
            if (ctx.args.size() != 2) throw std::runtime_error(runtimeErr(ctx.name, "expects 2 arguments: invphi1(phi1result, t)"));
            int t = static_cast<int>(ctx.evi(1));
            if (t <= 0) throw std::runtime_error(runtimeErr(ctx.name, "t must be positive"));
            EvalResult r = eval(ctx.args[0].get(), ctx.env, ctx.sumIndices);
            if (!std::holds_alternative<Phi1Result>(r))
                throw std::runtime_error(runtimeErr(ctx.name, "first argument must be a phi1 result"));
            const auto& phi1r = std::get<Phi1Result>(r);
            std::vector<std::vector<int64_t>> quot_raw;
            for (const auto& p : phi1r.quotient)
                quot_raw.push_back(p.parts);
            Partition result;
            result.parts = tcore_invphi1(phi1r.core.parts, quot_raw, t);
            return result;
        });
        regBuiltin("tcrank", H2("tcrank(ptn,t)", "t-crank (t odd prime)", {}, {}), [](DispatchContext& ctx) {
            if (ctx.args.size() != 2) throw std::runtime_error(runtimeErr(ctx.name, "expects 2 arguments: tcrank(ptn, t)"));
            Partition ptn = ctx.evalToPartition(ctx.args[0].get());
            int64_t t = ctx.evi(1);
            if (t < 3 || t % 2 == 0) throw std::runtime_error(runtimeErr(ctx.name, "t must be an odd prime >= 3"));
            int64_t val = tcore_tcrank(ptn.parts, t);
            std::cout << val << std::endl;
            return DisplayOnly{};
        });
        regBuiltin("tresdiag", H2("tresdiag(ptn,t)", "t-residue diagram", {}, {}), [](DispatchContext& ctx) {
            if (ctx.args.size() != 2) throw std::runtime_error(runtimeErr(ctx.name, "expects 2 arguments: tresdiag(ptn, t)"));
            Partition ptn = ctx.evalToPartition(ctx.args[0].get());
            int t = static_cast<int>(ctx.evi(1));
            if (t <= 0) throw std::runtime_error(runtimeErr(ctx.name, "t must be positive"));
            for (size_t row = 0; row < ptn.parts.size(); ++row) {
                for (int64_t col = 1; col <= ptn.parts[row]; ++col) {
                    if (col > 1) std::cout << " ";
                    std::cout << safe_mod(col - static_cast<int64_t>(row) - 1, t);
                }
                std::cout << "\n";
            }
            return DisplayOnly{};
        });
        regBuiltin("makebiw", H2("makebiw(ptn,t,m)", "bivalent word from partition", {}, {}), [](DispatchContext& ctx) {
            if (ctx.args.size() != 3) throw std::runtime_error(runtimeErr(ctx.name, "expects 3 arguments: makebiw(ptn, t, m)"));
            Partition ptn = ctx.evalToPartition(ctx.args[0].get());
            int t = static_cast<int>(ctx.evi(1));
            int mj = static_cast<int>(ctx.evi(2));
            if (t <= 0) throw std::runtime_error(runtimeErr(ctx.name, "t must be positive"));
            if (mj <= 0) throw std::runtime_error(runtimeErr(ctx.name, "m must be positive"));
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
            std::cout << "   ";
            for (int r = -mj; r <= mj; ++r) {
                if (r < 0) std::cout << r;
                else std::cout << " " << r;
            }
            std::cout << "\n";
            for (int i = 0; i < t; ++i) {
                std::cout << "W" << i << " ";
                for (int r = -mj; r <= mj; ++r) {
                    std::cout << " " << reg[{r, i}];
                }
                std::cout << "\n";
            }
            return DisplayOnly{};
        });

        regBuiltin("help", H2("help or help(func)", "list built-ins or show help for func", {}, {}), [](DispatchContext& ctx) {
            const auto& reg = getBuiltinRegistry();
            if (ctx.args.size() == 0) {
                std::cout << "q-series REPL. Commands: expr, var := expr, help, help(func).\nBuilt-ins: ";
                bool first = true;
                for (const auto& [n, e] : reg) {
                    if (!first) std::cout << ", ";
                    std::cout << ansi::gold() << n << ansi::reset();
                    first = false;
                }
                std::cout << std::endl;
                return DisplayOnly{};
            }
            if (ctx.args.size() == 1 && ctx.args[0]->tag == Expr::Tag::Var) {
                const std::string& fn = ctx.args[0]->varName;
                auto it = reg.find(fn);
                if (it != reg.end()) formatHelpEntry(fn, it->second.help);
                else std::cout << "unknown function: " << fn << std::endl;
                return DisplayOnly{};
            }
            throw std::runtime_error(runtimeErr(ctx.name, "expects 0 or 1 argument"));
        });

#undef H2
        return m;
    }();
    return reg;
}

// Helper: get Series from EnvValue (for variable lookup in arithmetic)
inline Series getSeriesFromEnv(const EnvValue& v) {
    if (std::holds_alternative<Series>(v))
        return std::get<Series>(v);
    if (std::holds_alternative<Partition>(v))
        throw std::runtime_error("variable holds partition, not series");
    if (std::holds_alternative<Phi1Result>(v))
        throw std::runtime_error("variable holds phi1 result, not series");
    if (std::holds_alternative<RelationKernelResult>(v))
        throw std::runtime_error("variable holds findhom/findnonhom result, not series");
    if (std::holds_alternative<Omega3>(v))
        throw std::runtime_error("variable holds Omega3, not series");
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

// isOmegaLike: true if expr is Var "omega" (for omega^exp, sum(omega^n))
inline bool isOmegaLike(const Expr* e) {
    return e && e->tag == Expr::Tag::Var && e->varName == "omega";
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
inline std::string formatEtamake(const std::vector<std::pair<int, Frac>>& eta);
inline std::string formatUndefinedVariableMsg(const Environment& env, const std::string& name);

inline EvalResult dispatchBuiltin(const std::string& name,
    const std::vector<ExprPtr>& args, Environment& env,
    const std::map<std::string, int64_t>& sumIndices) {
    if (env.env.find("q") == env.env.end())
        throw std::runtime_error("q not set");
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

    DispatchContext ctx{name, args, env, sumIndices, q, T, ev, evi, evalListToSeries, evalListToInt, evalToPartition};

    auto it = getBuiltinRegistry().find(name);
    if (it != getBuiltinRegistry().end())
        return it->second.handler(ctx);

    {
        const auto& reg = getBuiltinRegistry();
        std::vector<std::pair<int, std::string>> suggestions;
        for (const auto& [key, _] : reg) {
            int d = levenshteinDistance(name, key);
            if (d <= kLevenshteinSuggestionThreshold)
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
            if (ev == env.env.end()) {
                if (e->varName == "omega")
                    return Omega3::omega();
                throw std::runtime_error(formatUndefinedVariableMsg(env, e->varName));
            }
            if (std::holds_alternative<Omega3>(ev->second))
                return std::get<Omega3>(ev->second);
            if (std::holds_alternative<SeriesOmega>(ev->second))
                return std::get<SeriesOmega>(ev->second);
            if (std::holds_alternative<Partition>(ev->second))
                return std::get<Partition>(ev->second);
            if (std::holds_alternative<Phi1Result>(ev->second))
                return std::get<Phi1Result>(ev->second);
            if (std::holds_alternative<RelationKernelResult>(ev->second))
                return std::get<RelationKernelResult>(ev->second);
            return getSeriesFromEnv(ev->second);
        }
        case Expr::Tag::BinOp: {
            // omega^int (before q^exp)
            if (e->binOp == BinOp::Pow && isOmegaLike(e->left.get())) {
                EvalResult leftRes = eval(e->left.get(), env, sumIndices);
                Omega3 base = std::holds_alternative<Omega3>(leftRes) ? std::get<Omega3>(leftRes) : Omega3::omega();
                int k = static_cast<int>(evalToInt(e->right.get(), env, sumIndices));
                return Omega3::pow(base, k);
            }
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
            EvalResult leftRes = eval(e->left.get(), env, sumIndices);
            EvalResult rightRes = eval(e->right.get(), env, sumIndices);
            bool lOmega = std::holds_alternative<Omega3>(leftRes);
            bool rOmega = std::holds_alternative<Omega3>(rightRes);
            if (lOmega || rOmega) {
                if (e->binOp == BinOp::Add && lOmega && rOmega)
                    return std::get<Omega3>(leftRes) + std::get<Omega3>(rightRes);
                if (e->binOp == BinOp::Mul) {
                    if (lOmega && std::holds_alternative<Series>(rightRes))
                        return std::get<Omega3>(leftRes) * std::get<Series>(rightRes);
                    if (std::holds_alternative<Series>(leftRes) && rOmega)
                        return std::get<Series>(leftRes) * std::get<Omega3>(rightRes);
                    if (lOmega && rOmega)
                        return std::get<Omega3>(leftRes) * std::get<Omega3>(rightRes);
                    if (lOmega && std::holds_alternative<int64_t>(rightRes))
                        return std::get<Omega3>(leftRes) * Omega3::fromRational(Frac(std::get<int64_t>(rightRes)));
                    if (std::holds_alternative<int64_t>(leftRes) && rOmega)
                        return Omega3::fromRational(Frac(std::get<int64_t>(leftRes))) * std::get<Omega3>(rightRes);
                }
                throw std::runtime_error("omega + series not supported");
            }
            Series l = toSeries(leftRes, "binary op", env.T);
            Series r = toSeries(rightRes, "binary op", env.T);
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
                        if (expVal > kPowExponentLimit || expVal < -kPowExponentLimit)
                            throw std::runtime_error("pow: exponent magnitude too large (limit " + std::to_string(kPowExponentLimit) + ")");
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
            auto idx = sumIndices;
            idx[e->sumVar] = lo;
            EvalResult first = eval(e->body.get(), env, idx);
            if (std::holds_alternative<Omega3>(first)) {
                Omega3 acc = std::get<Omega3>(first);
                for (int64_t n = lo + 1; n <= hi; ++n) {
                    idx[e->sumVar] = n;
                    EvalResult term = eval(e->body.get(), env, idx);
                    if (!std::holds_alternative<Omega3>(term))
                        throw std::runtime_error("sum: mixed Omega3 and non-Omega3 terms not supported");
                    acc = acc + std::get<Omega3>(term);
                }
                return acc;
            }
            if (std::holds_alternative<SeriesOmega>(first)) {
                SeriesOmega acc = std::get<SeriesOmega>(first);
                for (int64_t n = lo + 1; n <= hi; ++n) {
                    idx[e->sumVar] = n;
                    EvalResult term = eval(e->body.get(), env, idx);
                    if (!std::holds_alternative<SeriesOmega>(term))
                        throw std::runtime_error("sum: mixed SeriesOmega and non-SeriesOmega terms not supported");
                    acc = (acc + std::get<SeriesOmega>(term)).truncTo(env.T);
                }
                return acc;
            }
            Series acc = toSeries(first, "sum body", env.T);
            for (int64_t n = lo + 1; n <= hi; ++n) {
                idx[e->sumVar] = n;
                Series term = toSeries(eval(e->body.get(), env, idx), "sum body", env.T);
                acc = (acc + term).truncTo(env.T);
            }
            return acc;
        }
        case Expr::Tag::Subscript: {
            EvalResult baseRes = eval(e->left.get(), env, sumIndices);
            if (!std::holds_alternative<RelationKernelResult>(baseRes))
                throw std::runtime_error("subscript only supported for findhom/findnonhom results");
            const auto& arg = std::get<RelationKernelResult>(baseRes);
            int64_t i = evalToInt(e->right.get(), env, sumIndices);
            if (i < 1 || i > static_cast<int64_t>(arg.basis.size()))
                throw std::runtime_error("index out of range");
            RelationKernelResult single;
            single.basis = {arg.basis[static_cast<size_t>(i - 1)]};
            single.monomialExponents = arg.monomialExponents;
            return single;
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

inline std::string formatFactor(const FactorResult& fr) {
    static auto sub = [](int n) -> std::string {
        static const char* d[] = {"\xe2\x82\x80","\xe2\x82\x81","\xe2\x82\x82","\xe2\x82\x83","\xe2\x82\x84","\xe2\x82\x85","\xe2\x82\x86","\xe2\x82\x87","\xe2\x82\x88","\xe2\x82\x89"};
        if (n == 0) return d[0];
        std::string s;
        bool neg = (n < 0);
        if (neg) n = -n;
        while (n > 0) { s = std::string(d[n % 10]) + s; n /= 10; }
        return neg ? "\xe2\x81\xbb" + s : s;  // ₋ for minus
    };
    std::string out;
    if (fr.q_power != 0)
        out += "q" + Series::expToUnicode(fr.q_power);
    for (const auto& [d, a] : fr.cyclotomic) {
        if (!out.empty()) out += "\xC2\xB7";  // middle dot
        out += "\xCE\xA6";  // Φ
        out += sub(d);
        if (a != 1 && a != -1)
            out += Series::expToUnicode(a);
        else if (a == -1)
            out += Series::expToUnicode(-1);
    }
    return out.empty() ? "1" : out;
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

inline std::string formatFindlincombomodp(const FindlincombomodpResult& r) {
    if (!r.coeffs) return "no linear combination mod " + std::to_string(r.p);
    std::string out;
    for (size_t i = 0; i < r.coeffs->size(); ++i) {
        if (i) out += " + ";
        out += std::to_string((*r.coeffs)[i]) + " L" + std::to_string(i + 1);
    }
    return out + " (mod " + std::to_string(r.p) + ")";
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

inline bool strEqualCaseInsensitive(const std::string& a, const std::string& b) {
    if (a.size() != b.size()) return false;
    for (size_t i = 0; i < a.size(); ++i)
        if (std::tolower(static_cast<unsigned char>(a[i])) != std::tolower(static_cast<unsigned char>(b[i])))
            return false;
    return true;
}

inline std::string formatUndefinedVariableMsg(const Environment& env, const std::string& name) {
    auto candidates = getCompletionCandidates(env);
    // (distance, case_insensitive_match?0:1, -length, key) — prefer case-only typos (RR→rr)
    std::vector<std::tuple<int, int, int, std::string>> suggestions;
    for (const auto& key : candidates) {
        int d = levenshteinDistance(name, key);
        if (d <= kLevenshteinSuggestionThreshold) {
            int caseMatch = strEqualCaseInsensitive(name, key) ? 0 : 1;
            suggestions.push_back({d, caseMatch, -static_cast<int>(key.size()), key});
        }
    }
    std::sort(suggestions.begin(), suggestions.end());
    std::string msg = "undefined variable: " + name;
    if (!suggestions.empty()) {
        msg += ". Did you mean:";
        for (size_t i = 0; i < std::min(suggestions.size(), size_t(2)); ++i)
            msg += " " + std::get<3>(suggestions[i]);
        msg += "?";
    }
    return msg;
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
        if (isFunc) {
            auto it = getHelpTable().find(match);
            if (it != getHelpTable().end() && !it->second.sig.empty())
                std::cout << "\n  " << it->second.sig << "\n";
            match += "(";
        }
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

/* Single (line, pos) buffer: line and pos form the one editing buffer shared by
   tab completion and history navigation. No second buffer. handleTabCompletion
   and up/down arrows modify line/pos in place. savedLine and histIdx are
   auxiliary for history recall only. */
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
        } else if constexpr (std::is_same_v<T, FactorResult>) {
            std::cout << formatFactor(arg) << std::endl;
        } else if constexpr (std::is_same_v<T, CheckprodResult>) {
            std::cout << formatCheckprod(arg) << std::endl;
        } else if constexpr (std::is_same_v<T, CheckmultResult>) {
            std::cout << formatCheckmult(arg) << std::endl;
        } else if constexpr (std::is_same_v<T, FindmaxindResult>) {
            std::cout << formatFindmaxind(arg) << std::endl;
        } else if constexpr (std::is_same_v<T, FindlincombomodpResult>) {
            std::cout << formatFindlincombomodp(arg) << std::endl;
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
        } else if constexpr (std::is_same_v<T, Omega3>) {
            std::cout << arg.str() << std::endl;
        } else if constexpr (std::is_same_v<T, SeriesOmega>) {
            std::cout << arg.str(30) << std::endl;
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
        if (std::holds_alternative<RelationKernelResult>(res)) {
            env.env[s->assignName] = std::get<RelationKernelResult>(res);
            return res;
        }
        if (std::holds_alternative<Omega3>(res)) {
            env.env[s->assignName] = std::get<Omega3>(res);
            return res;
        }
        if (std::holds_alternative<SeriesOmega>(res)) {
            env.env[s->assignName] = std::get<SeriesOmega>(res);
            return res;
        }
        throw std::runtime_error("assignment requires Series, Jacobi product, Partition, Omega3, SeriesOmega, or findhom/findnonhom result");
    }
    return eval(s->expr.get(), env, {});
}

#ifndef __EMSCRIPTEN__
inline bool bracketsUnclosed(const std::string& s) {
    int depth_paren = 0, depth_brack = 0;
    for (unsigned char c : s) {
        if (c == '(') ++depth_paren;
        else if (c == ')') { if (--depth_paren < 0) return true; }
        else if (c == '[') ++depth_brack;
        else if (c == ']') { if (--depth_brack < 0) return true; }
    }
    return depth_paren > 0 || depth_brack > 0;
}

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
    const size_t maxHistory = kMaxHistory;
    size_t inputLineNum = 0;

    if (stdin_is_tty()) {
#if defined(__CYGWIN__) || !defined(_WIN32)
        repl_install_sigint_handler();
#endif
        loadHistory(history, maxHistory);
    }

    constexpr size_t maxContinuations = kMaxContinuations;

    auto readLineFn = [&](const std::string& prompt) -> std::optional<std::string> {
        if (stdin_is_tty()) {
            std::cout << ansi::gold() << prompt << ansi::reset() << std::flush;
            auto opt = readLineRaw(env, history);
            std::cout << std::endl;
            return opt;
        } else {
            std::string line;
            if (!std::getline(std::cin, line)) return std::nullopt;
            std::cout << prompt << line << std::endl;
            return line;
        }
    };

    for (;;) {
        std::string line;
        auto opt = readLineFn("qseries> ");
        if (!opt) break;
        line = *opt;

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
            auto nextOpt = readLineFn("  > ");
            if (!nextOpt) break;
            line += " " + *nextOpt;
            ++contCount;
        }

        // Bracket continuation: while ( ) [ ] unbalanced, read more lines
        size_t bracketContCount = 0;
        constexpr size_t maxBracketContinuations = kMaxBracketContinuations;
        while (bracketsUnclosed(line) && bracketContCount < maxBracketContinuations) {
            auto nextOpt = readLineFn("  > ");
            if (!nextOpt) break;
            if (trim(*nextOpt).empty()) break;
            line += " " + *nextOpt;
            ++bracketContCount;
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
            std::string prefix;
            if (!stdin_is_tty()) {
                prefix = (inputLineNum > 0) ? ("line " + std::to_string(inputLineNum) + ": ") : "line ?: ";
            }
            std::string text = e.what();
            bool isParse = (text.size() >= 8 && text.compare(0, 8, "parser: ") == 0);
            std::string msg = isParse ? formatParseErrorWithSnippet(trimmed, text) : text;
            std::cerr << ansi::red() << prefix << msg << ansi::reset() << std::endl;
        }
    }

    if (stdin_is_tty())
        saveHistory(history);
}
#endif // __EMSCRIPTEN__

#endif
