#ifndef CONVERT_H
#define CONVERT_H

#include "series.h"
#include "qfuncs.h"
#include <map>
#include <set>
#include <vector>
#include <string>
#include <iostream>

// sift(f, n, k, T): extract coefficients a_{n*i+k}, return Series Σ_i a_{n*i+k} q^i
inline Series sift(const Series& f, int n, int k, int T) {
    Series result;
    int i = 0;
    while (n * i + k < T) {
        result.setCoeff(i, f.coeff(n * i + k));
        ++i;
    }
    result.trunc = i;  // floor((T-1-k)/n)+1
    return result;
}

// etamake(f, T): identify f as eta-product; return list of (k, e_k) for η(kτ)^{e_k}
inline std::vector<std::pair<int, Frac>> etamake(const Series& f, int T) {
    std::vector<std::pair<int, Frac>> result;
    Series g = f.truncTo(T);
    int me = g.minExp();
    Frac lead = g.coeff(me);
    if (lead.isZero()) {
        std::cerr << "etamake: zero series\n";
        return result;
    }
    g = (g / Series::constant(lead, T)).truncTo(T);


    Series q_var = Series::q(T);
    const int max_iter = 2 * T;
    int iter = 0;

    while (iter < max_iter) {
        // Check if g is constant 1
        if (g.coeff(0) == Frac(1)) {
            bool only_const = true;
            for (int n = 1; n < T; ++n)
                if (g.coeff(n) != Frac(0)) { only_const = false; break; }
            if (only_const) return result;
        }

        // Find smallest k > 0 with g.coeff(k) nonzero
        int k = 1;
        while (k < T && g.coeff(k) == Frac(0)) ++k;
        if (k >= T) return result;  // g is constant

        Frac c = g.coeff(k);
        if (c.den != BigInt(1)) {
            std::cerr << "etamake: not an eta product (non-integer coefficient at q^" << k << ")\n";
            return {};
        }
        int c_int = 0;
        if (c.num.d.size() == 1 && c.num.d[0] <= static_cast<uint32_t>(INT_MAX))
            c_int = c.num.neg ? -static_cast<int>(c.num.d[0]) : static_cast<int>(c.num.d[0]);
        else if (!c.num.isZero()) {
            std::cerr << "etamake: coefficient too large at q^" << k << "\n";
            return {};
        }
        if (c_int == 0) continue;

        Series et = etaq(q_var, k, T);
        g = (g * et.pow(c_int)).truncTo(T);
        result.push_back({k, Frac(-c_int)});
        ++iter;
    }

    std::cerr << "etamake: not an eta product (max iterations reached)\n";
    return {};
}

// Andrews' algorithm: convert q-series to infinite product form.
// f = (leading) * Π_{n=1}^{T-1} (1 - q^n)^{-a[n]}
// Returns a[n] for n=1..T-1. Warns on edge cases; returns best-effort.
inline std::map<int, Frac> prodmake(const Series& f, int T) {
    std::map<int, Frac> result;
    Series g = f.truncTo(T);
    int Teff = std::min(T, g.trunc);
    if (Teff <= 1) return result;

    Frac b0 = g.coeff(0);
    if (b0.isZero() || g.minExp() > 0) {
        std::cerr << "prodmake: b[0]=0 or no constant term\n";
        return result;
    }
    // Normalize: divide by |b0| per CONTEXT
    Frac scale = (b0.num < BigInt(0)) ? Frac(-1) / b0 : Frac(1) / b0;

    std::vector<Frac> b(Teff);
    for (int n = 0; n < Teff; ++n)
        b[n] = g.coeff(n) * scale;
    // b[0] is now 1

    std::vector<Frac> c(Teff);
    for (int n = 1; n < Teff; ++n) {
        Frac sum = Frac(0);
        for (int j = 1; j < n; ++j)  // j=1..n-1, NOT j=n (PITFALLS)
            sum = sum + b[n - j] * c[j];
        c[n] = Frac(n) * b[n] - sum;
    }

    bool warned_nonint = false;
    for (int n = 1; n < Teff; ++n) {
        Frac div_sum = Frac(0);
        for (int d : divisors(n)) {
            if (d < n)  // CRITICAL: exclude d=n (PITFALLS)
                div_sum = div_sum + Frac(d) * result[d];
        }
        Frac a_n = (c[n] - div_sum) / Frac(n);
        result[n] = a_n;
        if (!warned_nonint && a_n.den != BigInt(1)) {
            std::cerr << "prodmake: non-integer a[n] at n=" << n << "\n";
            warned_nonint = true;
        }
    }
    return result;
}

// mprodmake(f, T): convert q-series to product (1+q^n1)(1+q^n2)... (m-product form)
// Returns sorted vector of exponents; empty on failure or constant 1.
inline std::vector<int> mprodmake(const Series& f, int T) {
    std::vector<int> result;
    Series g = f.truncTo(T);
    if (g.coeff(0).isZero() || g.minExp() > 0) {
        std::cerr << "mprodmake: b[0]=0 or no constant term\n";
        return result;
    }
    auto a = prodmake(g, T);
    if (a.empty()) return result;

    for (const auto& [n, an] : a) {
        if (an.den != BigInt(1)) {
            std::cerr << "mprodmake: not an m-product (non-integer a[n] at n=" << n << ")\n";
            return {};
        }
        int v = 0;
        if (an.num.d.size() == 1 && an.num.d[0] <= static_cast<uint32_t>(INT_MAX))
            v = an.num.neg ? -static_cast<int>(an.num.d[0]) : static_cast<int>(an.num.d[0]);
        else if (!an.num.isZero()) {
            std::cerr << "mprodmake: not an m-product (coefficient too large at n=" << n << ")\n";
            return {};
        }
        if (v < -1 || v > 1) {
            std::cerr << "mprodmake: not an m-product\n";
            return {};
        }
    }

    std::set<int> S;
    for (const auto& [n, an] : a) {
        int v = 0;
        if (an.num.d.size() == 1 && an.num.d[0] <= static_cast<uint32_t>(INT_MAX))
            v = an.num.neg ? -static_cast<int>(an.num.d[0]) : static_cast<int>(an.num.d[0]);
        if (v == 1) S.insert(n);
        if (v == -1 && n % 2 == 0) S.insert(n / 2);  // a[2n]=-1 => n in S
    }
    result.assign(S.begin(), S.end());
    return result;
}

// checkprod result: minExp, nice (all |a[n]| < M), M
struct CheckprodResult { int minExp; bool nice; int M; };

// checkprod(f, M, T) — explicit M (define first so overload is visible)
inline CheckprodResult checkprod(const Series& f, int M, int T) {
    Series g = f.truncTo(T);
    int minExp = g.minExp();
    if (g.coeff(0).isZero() || minExp > 0) {
        return {minExp, false, M};
    }
    auto a = prodmake(g, T);
    if (a.empty()) return {minExp, false, M};

    for (int n = 1; n < T; ++n) {
        auto it = a.find(n);
        if (it == a.end()) continue;
        const Frac& an = it->second;
        if (an.den != BigInt(1)) return {minExp, false, M};
        int v = 0;
        if (an.num.d.size() == 1 && an.num.d[0] <= static_cast<uint32_t>(INT_MAX))
            v = an.num.neg ? -static_cast<int>(an.num.d[0]) : static_cast<int>(an.num.d[0]);
        else if (!an.num.isZero()) return {minExp, false, M};
        if (v < 0) v = -v;
        if (v >= M) return {minExp, false, M};
    }
    return {minExp, true, M};
}

// checkprod(f, T) — M=2 (nice = exponents in {-1,0,1})
inline CheckprodResult checkprod(const Series& f, int T) {
    return checkprod(f, 2, T);
}

// checkmult result: multiplicative flag and failure pairs
struct CheckmultResult {
    bool multiplicative;
    std::vector<std::pair<int,int>> failures;
};

inline CheckmultResult checkmult(const Series& f, int T, bool verbose = false) {
    Series g = f.truncTo(T);
    CheckmultResult out{true, {}};
    auto gcd_int = [](int a, int b) {
        a = (a < 0) ? -a : a;
        b = (b < 0) ? -b : b;
        while (b) { int t = b; b = a % b; a = t; }
        return a;
    };
    for (int m = 2; m < T; ++m) {
        for (int n = 2; n < T; ++n) {
            if (m * n >= T) continue;
            if (gcd_int(m, n) != 1) continue;
            Frac a_mn = g.coeff(m * n);
            Frac a_m = g.coeff(m);
            Frac a_n = g.coeff(n);
            if (a_mn != a_m * a_n) {
                out.multiplicative = false;
                out.failures.push_back({m, n});
                if (!verbose) return out;
            }
        }
    }
    return out;
}

// T(r, n): polynomial from qseriesdoc §3.2 recurrence
inline Series T_rn(int r, int n, int T_trunc) {
    if (n == 0) return Series::one(T_trunc);
    if (n == 1) return Series::zero(T_trunc);
    Series q_var = Series::q(T_trunc);
    Series sum = Series::zero(T_trunc);
    for (int k = 1; k <= n / 2; ++k) {
        auto qb = qbin(q_var, k, r + 2 * k, T_trunc);
        auto t_sub = T_rn(r + 2 * k, n - 2 * k, T_trunc);
        sum = (sum - (qb * t_sub).truncTo(T_trunc)).truncTo(T_trunc);
    }
    sum.trunc = T_trunc;
    return sum;
}

// qfactor(f, T): write f as q^e · num/den
struct QFactorResult {
    int q_power = 0;
    std::map<int, Frac> num_exponents;
    std::map<int, Frac> den_exponents;
};
inline QFactorResult qfactor(const Series& f, int T) {
    QFactorResult out;
    Series g = f.truncTo(T);
    int me = g.minExp();
    Frac lead = g.coeff(me);
    if (lead.isZero()) return out;
    Series h;
    for (int n = 0; n + me < T; ++n)
        if (g.coeff(n + me) != Frac(0))
            h.setCoeff(n, g.coeff(n + me));
    h.trunc = (T - me > 1) ? T - me : 1;
    out.q_power = me;
    if (h.coeff(0).isZero() || h.minExp() > 0) return out;
    auto a = prodmake(h, h.trunc);
    for (const auto& [n, an] : a) {
        if (an < Frac(0))
            out.num_exponents[n] = Frac(-1) * an;
        else if (an > Frac(0))
            out.den_exponents[n] = an;
    }
    return out;
}

// JAC factor: (a, b, exponent) for JAC(a,b,∞)^exponent
using JacFactor = std::tuple<int, int, Frac>;

// (q^a;q^b)_∞ truncated to T
inline Series jac_factor_series(int a, int b, int T) {
    Series r = Series::one(T);
    Series q = Series::q(T);
    for (int j = 0; a + j * b < T; ++j) {
        int exp = a + j * b;
        r = (r * (Series::one(T) - q.pow(exp))).truncTo(T);
    }
    r.trunc = T;
    return r;
}

inline Series jac2series_impl(const std::vector<JacFactor>& jac, int T) {
    Series q = Series::q(T);
    Series prod = Series::one(T);
    for (const auto& [a, b, exp] : jac) {
        Series fac;
        if (a == 0) {
            fac = jac_factor_series(b, b, T);
        } else {
            fac = (jac_factor_series(a, b, T) * jac_factor_series(b - a, b, T) * jac_factor_series(b, b, T)).truncTo(T);
        }
        int ex = 0;
        if (exp.den == BigInt(1) && exp.num.d.size() == 1 && exp.num.d[0] <= 1000)
            ex = exp.num.neg ? -static_cast<int>(exp.num.d[0]) : static_cast<int>(exp.num.d[0]);
        if (ex > 0) {
            for (int i = 0; i < ex; ++i) prod = (prod * fac).truncTo(T);
        } else if (ex < 0) {
            auto inv = fac.inverse();
            for (int i = 0; i < -ex; ++i) prod = (prod * inv).truncTo(T);
        }
    }
    prod.trunc = T;
    return prod;
}

inline std::vector<JacFactor> jacprodmake(const Series& f, int T) {
    std::vector<JacFactor> result;
    auto a_map = prodmake(f, T);
    if (a_map.empty()) return result;
    std::map<int, Frac> e;
    for (const auto& [n, an] : a_map) e[n] = Frac(-1) * an;

    int Tmid = T / 2;
    for (int b = 2; b <= Tmid; ++b) {
        int range = T - 1 - b;
        if (range <= 0) continue;
        int match = 0;
        for (int n = 1; n <= range; ++n) {
            auto it_n = e.find(n);
            auto it_nb = e.find(n + b);
            Frac en = (it_n != e.end()) ? it_n->second : Frac(0);
            Frac enb = (it_nb != e.end()) ? it_nb->second : Frac(0);
            if (en == enb) ++match;
        }
        if (match * 100 < range * 80) continue;

        // Decompose e[1..b] into JAC: x1=e[1], x2=e[2], x0=e[5]-e[1]-e[2] for b=5
        std::map<int, Frac> x;
        for (int i = 0; i < b; ++i) {
            Frac ei = (e.count(i + 1)) ? e[i + 1] : Frac(0);
            if (i == 0) x[0] = ei;
            else if (i < (b + 1) / 2) x[i] = ei;
        }
        if (b >= 2) {
            Frac e1 = (e.count(1)) ? e[1] : Frac(0);
            Frac e2 = (e.count(2)) ? e[2] : Frac(0);
            Frac eb = (e.count(b)) ? e[b] : Frac(0);
            x[0] = eb - e1 - e2;
        }
        if (b == 5) {
            Frac e1 = (e.count(1)) ? e[1] : Frac(0);
            Frac e2 = (e.count(2)) ? e[2] : Frac(0);
            Frac e5 = (e.count(5)) ? e[5] : Frac(0);
            x[0] = e5 - e1 - e2;
            x[1] = e1;
            x[2] = e2;
        }

        result.clear();
        if (x[0] != Frac(0)) result.push_back({0, b, x[0]});
        for (int a = 1; a < b; ++a) {
            if (a > b - a) continue;
            Frac xa = (x.count(a)) ? x[a] : Frac(0);
            if (xa != Frac(0)) result.push_back({a, b, xa});
        }

        Series recon = jac2series_impl(result, T);
        bool ok = true;
        for (int n = 0; n < T && ok; ++n) {
            if (recon.coeff(n) != f.coeff(n)) ok = false;
        }
        if (ok) return result;
    }
    return {};
}

inline Series jac2series(const std::vector<JacFactor>& jac, int T) {
    return jac2series_impl(jac, T);
}

inline std::string jac2prod(const std::vector<JacFactor>& jac) {
    std::vector<std::string> num_parts, den_parts;
    for (const auto& [a, b, exp] : jac) {
        int ex = 0;
        if (exp.den == BigInt(1) && exp.num.d.size() == 1 && exp.num.d[0] <= 100)
            ex = exp.num.neg ? -static_cast<int>(exp.num.d[0]) : static_cast<int>(exp.num.d[0]);
        if (ex == 0) continue;
        std::string part;
        auto qa = (a == 1) ? "q" : ("q^" + std::to_string(a));
        auto qba = (b - a == 1) ? "q" : ("q^" + std::to_string(b - a));
        if (a == 0) {
            part = "(q^" + std::to_string(b) + ",q^" + std::to_string(b) + ")_∞";
        } else {
            part = "(" + qa + ",q^" + std::to_string(b) + ")_∞ (" + qba + ",q^" + std::to_string(b) + ")_∞ (q^" + std::to_string(b) + ",q^" + std::to_string(b) + ")_∞";
        }
        if (ex > 1) part += "^" + std::to_string(ex);
        if (ex > 0) num_parts.push_back(part);
        else for (int i = 0; i < -ex; ++i) den_parts.push_back(part);
    }
    std::string num_str, den_str;
    for (const auto& s : num_parts) num_str += (num_str.empty() ? "" : " ") + s;
    for (const auto& s : den_parts) den_str += (den_str.empty() ? "" : " ") + s;
    if (den_str.empty()) return num_str.empty() ? "1" : num_str;
    return "1 / ((" + den_str + "))";
}

#endif
