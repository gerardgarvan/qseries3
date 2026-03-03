#ifndef CONVERT_H
#define CONVERT_H

#include "series.h"
#include "qfuncs.h"
#include <map>
#include <set>
#include <vector>
#include <string>
#include <iostream>
#include <climits>

// sift(f, n, k, T): extract coefficients a_{n*i+k}, return Series Σ_i a_{n*i+k} q^i
inline Series sift(const Series& f, int n, int k, int T) {
    if (n <= 0)
        throw std::runtime_error("sift: modulus n must be positive, got " + std::to_string(n));
    Series result;
    int i = 0;
    while (n * i + k < T) {
        result.setCoeff(i, f.coeff(n * i + k));
        ++i;
    }
    result.trunc = i;
    return result;
}

// dilly(f, d): substitute q → q^d (q-dilation)
inline Series dilly(const Series& f, int d) {
    if (d <= 0) throw std::runtime_error("dilly: d must be positive");
    Series result;
    result.trunc = (f.trunc + d - 1) / d;
    for (const auto& [e, v] : f.c) {
        if (e % d == 0)
            result.setCoeff(e / d, v);
    }
    return result;
}

// sieveqcheck(f, p): true iff all nonzero exponents are ≡ same residue mod p
inline bool sieveqcheck(const Series& f, int p) {
    if (p <= 0) return false;
    std::set<int> residues;
    for (const auto& [e, v] : f.c) {
        if (!v.isZero())
            residues.insert(((e % p) + p) % p);
    }
    return residues.size() <= 1;
}

// siftfindrange(f, p, T): [min_count, min_res, avg, max_count] for residue classes mod p
struct SiftFindResult {
    int min_count = 0;
    int min_res = 0;
    double avg = 0;
    int max_count = 0;
};

inline SiftFindResult siftfindrange(const Series& f, int p, int T) {
    SiftFindResult out;
    if (p <= 0) return out;
    int min_count = INT_MAX, max_count = 0;
    int total = 0;
    for (int j = 0; j < p; ++j) {
        Series s = sift(f, p, j, T);
        int cnt = 0;
        for (const auto& [e, v] : s.c)
            if (!v.isZero()) ++cnt;
        if (cnt < min_count) { min_count = cnt; out.min_res = j; }
        if (cnt > max_count) max_count = cnt;
        total += cnt;
    }
    out.min_count = min_count;
    out.max_count = max_count;
    out.avg = (p > 0) ? static_cast<double>(total) / p : 0;
    return out;
}

// polyfind(n0, n1, n2, T): quadratic p(x)=ax^2+bx+c with p(T)=n0, p(T+1)=n1, p(T+2)=n2
struct PolyfindResult {
    Frac a, b, c;
    bool ok = false;
};

inline PolyfindResult polyfind(int64_t n0, int64_t n1, int64_t n2, int64_t T) {
    PolyfindResult out;
    // a*(T+2)^2 + b*(T+2) + c = n2
    // a*(T+1)^2 + b*(T+1) + c = n1
    // a*T^2 + b*T + c = n0
    // Subtracting: a*(2T+3) + b = n2-n1, a*(2T+1) + b = n1-n0
    // So 2a = (n2-n1) - (n1-n0) = n2 - 2*n1 + n0
    int64_t two_a = n2 - 2*n1 + n0;
    if (two_a % 2 != 0) return out;
    int64_t a_val = two_a / 2;
    int64_t b_val = (n1 - n0) - a_val * (2*T + 1);
    int64_t c_val = n0 - a_val*T*T - b_val*T;
    out.a = Frac(static_cast<int64_t>(a_val));
    out.b = Frac(static_cast<int64_t>(b_val));
    out.c = Frac(static_cast<int64_t>(c_val));
    out.ok = true;
    return out;
}

// etamake(f, T): identify f as eta-product; return list of (k, e_k) for η(kτ)^{e_k}
// k=0 entry stores the scalar leading coefficient (omitted when 1)
inline std::vector<std::pair<int, Frac>> etamake(const Series& f, int T) {
    std::vector<std::pair<int, Frac>> result;
    if (!(f.q_shift == Frac(0)))
        result.push_back({-1, f.q_shift});
    Series g = f.truncTo(T);
    int me = g.minExp();
    Frac lead = g.coeff(me);
    if (lead.isZero()) {
        std::cerr << "etamake: zero series\n";
        return result;
    }
    g = (g / Series::constant(lead, T)).truncTo(T);
    if (!(lead == Frac(1)))
        result.push_back({0, lead});

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

// newprodmake(f, T): Andrews' algorithm with arbitrary leading term.
// Extracts lead = c*q^k, normalizes f/lead to constant 1, runs prodmake.
// Returns (lead_exp, lead_coeff, a_map). Empty a_map on failure.
struct NewProdmakeResult {
    int lead_exp = 0;
    Frac lead_coeff = Frac(1);
    std::map<int, Frac> a;
};

inline NewProdmakeResult newprodmake(const Series& f, int T) {
    NewProdmakeResult out;
    Series g = f.truncTo(T);
    int me = g.minExp();
    Frac lead = g.coeff(me);
    if (lead.isZero()) {
        std::cerr << "newprodmake: zero series\n";
        return out;
    }
    out.lead_exp = me;
    out.lead_coeff = lead;
    int Teff = std::min(T, g.trunc) - me;
    if (Teff <= 1) return out;

    std::vector<Frac> b(Teff);
    Frac scale = Frac(1) / lead;
    for (int i = 0; i < Teff; ++i)
        b[i] = g.coeff(me + i) * scale;
    if (b[0] != Frac(1)) {
        std::cerr << "newprodmake: normalization failed\n";
        return out;
    }

    std::vector<Frac> c(Teff);
    for (int n = 1; n < Teff; ++n) {
        Frac sum = Frac(0);
        for (int j = 1; j < n; ++j)
            sum = sum + b[n - j] * c[j];
        c[n] = Frac(n) * b[n] - sum;
    }

    for (int n = 1; n < Teff; ++n) {
        Frac div_sum = Frac(0);
        for (int d : divisors(n)) {
            if (d < n)
                div_sum = div_sum + Frac(d) * out.a[d];
        }
        Frac a_n = (c[n] - div_sum) / Frac(n);
        out.a[n] = a_n;
    }
    return out;
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

// Gaussian binomial via addition recurrence (no division):
// [n;m]_q = [n-1;m-1]_q + q^m * [n-1;m]_q
inline const Series& qbin_fast(int m, int n, int T,
                                std::map<int64_t, Series>& qbc) {
    if (m == 0 || m == n) {
        int64_t key = (int64_t(m) << 20) | n;
        auto [it, inserted] = qbc.try_emplace(key, Series::one(T));
        return it->second;
    }
    int64_t key = (int64_t(m) << 20) | n;
    auto it = qbc.find(key);
    if (it != qbc.end()) return it->second;
    const Series& a = qbin_fast(m - 1, n - 1, T, qbc);
    const Series& b = qbin_fast(m, n - 1, T, qbc);
    Series result = a;
    for (auto& [e, coeff] : b.c)
        if (e + m < T) {
            Frac& rc = result.c[e + m];
            rc = rc + coeff;
            if (rc.isZero()) result.c.erase(e + m);
        }
    result.trunc = T;
    qbc[key] = std::move(result);
    return qbc[key];
}

// T(r, n): polynomial from qseriesdoc §3.2 recurrence (memoized)
inline const Series& T_rn_impl(int r, int n, int T_trunc,
                                std::map<int64_t, Series>& cache,
                                std::map<int64_t, Series>& qbin_cache) {
    if (n == 0) {
        int64_t key = (int64_t(r) << 20) | n;
        auto [it, inserted] = cache.try_emplace(key, Series::one(T_trunc));
        return it->second;
    }
    if (n == 1) {
        int64_t key = (int64_t(r) << 20) | n;
        auto [it, inserted] = cache.try_emplace(key, Series::zero(T_trunc));
        return it->second;
    }
    int64_t key = (int64_t(r) << 20) | n;
    auto it = cache.find(key);
    if (it != cache.end()) return it->second;
    Series sum = Series::zero(T_trunc);
    for (int k = 1; k <= n / 2; ++k) {
        const Series& qb = qbin_fast(k, r + 2 * k, T_trunc, qbin_cache);
        const Series& t_sub = T_rn_impl(r + 2 * k, n - 2 * k, T_trunc, cache, qbin_cache);
        sum = sum - qb * t_sub;
    }
    sum.trunc = T_trunc;
    cache[key] = std::move(sum);
    return cache[key];
}

inline Series T_rn(int r, int n, int T_trunc) {
    std::map<int64_t, Series> cache;
    std::map<int64_t, Series> qbin_cache;
    return T_rn_impl(r, n, T_trunc, cache, qbin_cache);
}

// qfactor(f, T): write f as q^e · num/den
struct QFactorResult {
    int q_power = 0;
    std::map<int, Frac> num_exponents;
    std::map<int, Frac> den_exponents;
};

// factor(f, T): cyclotomic expansion q^e · ∏ Φ_d^a_d (for maple-checklist Block 4)
struct FactorResult {
    Frac content = Frac(1);
    int q_power = 0;
    std::map<int, int> cyclotomic;  // n → exponent of Φ_n(q)
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

inline FactorResult factor(const Series& f, int T) {
    FactorResult result;
    QFactorResult qf = qfactor(f, T);
    int nonzero = 0;
    for (const auto& [e, v] : f.c)
        if (!v.isZero()) ++nonzero;
    if (qf.num_exponents.empty() && qf.den_exponents.empty() && nonzero > 1)
        throw std::runtime_error("factor: cannot factor (not a q-product)");
    result.q_power = qf.q_power;
    auto add_cyclo = [&](int n, const Frac& e, int sign) {
        if (e.den != BigInt(1))
            throw std::runtime_error("factor: non-integer exponent in q-product");
        int ex = 0;
        if (e.num.d.size() == 1 && e.num.d[0] <= static_cast<uint32_t>(INT_MAX))
            ex = e.num.neg ? -static_cast<int>(e.num.d[0]) : static_cast<int>(e.num.d[0]);
        else if (!e.num.isZero())
            throw std::runtime_error("factor: non-integer exponent in q-product");
        for (int d : divisors(n))
            result.cyclotomic[d] += sign * ex;
    };
    for (const auto& [n, e] : qf.num_exponents)
        add_cyclo(n, e, 1);
    for (const auto& [n, e] : qf.den_exponents)
        add_cyclo(n, e, -1);
    for (auto it = result.cyclotomic.begin(); it != result.cyclotomic.end(); )
        if (it->second == 0)
            it = result.cyclotomic.erase(it);
        else
            ++it;
    return result;
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
        if (exp.isZero()) continue;
        Series fac;
        if (a == 0) {
            fac = jac_factor_series(b, b, T);
        } else {
            fac = (jac_factor_series(a, b, T) * jac_factor_series(b - a, b, T)
                   * jac_factor_series(b, b, T)).truncTo(T);
        }
        if (exp.den == BigInt(1)) {
            int ex = 0;
            if (exp.num.d.size() == 1 && exp.num.d[0] <= 1000)
                ex = exp.num.neg ? -static_cast<int>(exp.num.d[0]) : static_cast<int>(exp.num.d[0]);
            prod = (prod * fac.pow(ex)).truncTo(T);
        } else {
            prod = (prod * fac.powFrac(exp)).truncTo(T);
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

        // Decompose e[1..b] into JAC(a,b) exponents.
        // JAC(a,b,∞) for a>0 covers residues a, b-a, and b (mod b).
        // x[a] = e[a] with symmetry check e[a] == e[b-a].
        // x[0] = e[b] - Σ x[a] for a=1..floor((b-1)/2) [- x[b/2] if even].
        std::map<int, Frac> x;
        Frac sum_x(0);
        bool sym_ok = true;
        for (int a = 1; a <= (b - 1) / 2; ++a) {
            Frac ea = e.count(a) ? e[a] : Frac(0);
            Frac eba = e.count(b - a) ? e[b - a] : Frac(0);
            if (!(ea == eba)) { sym_ok = false; break; }
            x[a] = ea;
            sum_x = sum_x + ea;
        }
        if (!sym_ok) continue;
        if (b % 2 == 0) {
            int half = b / 2;
            Frac eh = e.count(half) ? e[half] : Frac(0);
            x[half] = eh / Frac(2);
            sum_x = sum_x + x[half];
        }
        Frac eb_val = e.count(b) ? e[b] : Frac(0);
        x[0] = eb_val - sum_x;

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
        if (exp.isZero()) continue;
        std::string part;
        auto qa = (a == 1) ? "q" : ("q^" + std::to_string(a));
        auto qba = (b - a == 1) ? "q" : ("q^" + std::to_string(b - a));
        if (a == 0) {
            part = "(q^" + std::to_string(b) + ",q^" + std::to_string(b) + ")_∞";
        } else {
            part = "(" + qa + ",q^" + std::to_string(b) + ")_∞ (" + qba + ",q^" + std::to_string(b) + ")_∞ (q^" + std::to_string(b) + ",q^" + std::to_string(b) + ")_∞";
        }
        Frac absexp = (exp > Frac(0)) ? exp : Frac(-1) * exp;
        if (!(absexp == Frac(1))) {
            if (absexp.den == BigInt(1)) {
                int ae = absexp.num.d.empty() ? 0 : static_cast<int>(absexp.num.d[0]);
                part += "^" + std::to_string(ae);
            } else {
                part += "^(" + absexp.str() + ")";
            }
        }
        if (exp > Frac(0))
            num_parts.push_back(part);
        else
            den_parts.push_back(part);
    }
    std::string num_str, den_str;
    for (const auto& s : num_parts) num_str += (num_str.empty() ? "" : " ") + s;
    for (const auto& s : den_parts) den_str += (den_str.empty() ? "" : " ") + s;
    if (num_str.empty()) num_str = "1";
    if (den_str.empty()) return num_str;
    return num_str + " / (" + den_str + ")";
}

#endif
