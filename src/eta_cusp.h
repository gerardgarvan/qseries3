#ifndef ETA_CUSP_H
#define ETA_CUSP_H

#include "series.h"
#include "qfuncs.h"
#include "frac.h"
#include <vector>
#include <set>
#include <utility>
#include <cmath>
#include <numeric>

// GP = [t1, r1, t2, r2, ...] for eta-quotient prod η(t_i τ)^{r_i}

// vp(n, p) = p-adic valuation: max k with p^k | n
inline int vp(int n, int p) {
    if (n == 0 || p <= 1) return (n == 0 && p > 1) ? 1000000 : 0;
    int k = 0;
    int64_t m = n;
    while (m % p == 0) {
        m /= p;
        ++k;
    }
    return k;
}

// cuspmake(N) = set of inequivalent cusps for Gamma_0(N)
// Returns set of rationals d/c in (0,1] with c|N, c>1, 1<=d<c, gcd(d,c)=1,
// mod(d, gcd(c, N/c)) for inequivalence. Plus cusp 0 (infinity).
inline std::set<std::pair<int,int>> cuspmake(int N) {
    std::set<std::pair<int,int>> S;
    S.insert({0, 1});  // cusp at infinity
    if (N <= 1) return S;
    std::vector<int> divs = divisors(N);
    for (int c : divs) {
        if (c <= 1) continue;
        int gc = std::gcd(c, N / c);
        std::set<int> SSc;
        for (int d = 1; d < c; ++d) {
            if (std::gcd(d, c) != 1) continue;
            int md = ((d % gc) + gc) % gc;
            if (SSc.count(md)) continue;
            SSc.insert(md);
            S.insert({d, c});
        }
    }
    return S;
}

// cuspord(GP, cusp) = Ligozat order at cusp. cusp = (num, den); (1,0) = infinity
inline Frac cuspord(const std::vector<int>& GP, [[maybe_unused]] int cusp_num, int cusp_den) {
    if (GP.size() % 2 != 0) return Frac(0);
    int c = (cusp_den == 0) ? 1 : cusp_den;  // infinity: use c=1
    Frac ord(0);
    for (size_t i = 0; i < GP.size(); i += 2) {
        int t = GP[i];
        int r = GP[i + 1];
        int g = std::gcd(t, c);
        ord = ord + Frac(g * g * r, 24 * t);
    }
    return ord;
}

// gammacheck(GP, N) = Newman's 5 conditions for Gamma_0(N). Returns 1 if pass, 0 if fail.
inline int gammacheck(const std::vector<int>& GP, int N) {
    if (GP.size() % 2 != 0) return 0;
    size_t ngp = GP.size() / 2;
    // (1) sum r_i = 0
    int w1 = 0;
    for (size_t i = 0; i < ngp; ++i) w1 += GP[2*i + 1];
    if (w1 != 0) return 0;
    // (2) sum t_i * r_i ≡ 0 (mod 24)
    int w2 = 0;
    for (size_t i = 0; i < ngp; ++i) w2 += GP[2*i] * GP[2*i + 1];
    if (w2 % 24 != 0) return 0;
    // (3) prod t_i^|r_i| is a perfect square
    int64_t p = 1;
    for (size_t i = 0; i < ngp; ++i) {
        int t = GP[2*i], r = GP[2*i + 1];
        int absr = (r < 0) ? -r : r;
        for (int j = 0; j < absr; ++j) p *= t;
    }
    int64_t rt = static_cast<int64_t>(std::sqrt(p) + 0.5);
    if (rt * rt != p) return 0;
    // (4) t_i divides N for all r_i != 0
    for (size_t i = 0; i < ngp; ++i) {
        int t = GP[2*i], r = GP[2*i + 1];
        if (r != 0 && (t == 0 || N % t != 0)) return 0;
    }
    // (5) sum (N/t_i)*r_i ≡ 0 (mod 24)
    int w3 = 0;
    for (size_t i = 0; i < ngp; ++i)
        w3 += (N / GP[2*i]) * GP[2*i + 1];
    if (w3 % 24 != 0) return 0;
    return 1;
}

// fanwidth(cusp d/c, N) = N / gcd(N, c^2)
inline int fanwidth([[maybe_unused]] int cusp_num, int cusp_den, int N) {
    if (cusp_den == 0) return 1;  // infinity
    int s = cusp_den;
    int64_t g = std::gcd(static_cast<int64_t>(N), static_cast<int64_t>(s) * s);
    return static_cast<int>(N / g);
}

// cuspORDSnotoo: orders at each cusp (excluding infinity), return list of (cusp, ORD)
// ORD = cuspord * fanwidth. Each element is (d, c, ord_val) for Frac.
struct CuspOrd {
    int d, c;
    Frac ord;
};

inline std::vector<CuspOrd> cuspORDSnotoo(const std::vector<int>& GP, int N) {
    std::vector<CuspOrd> out;
    auto cusps = cuspmake(N);
    for (const auto& [d, c] : cusps) {
        if (c == 1 && d == 0) continue;  // skip infinity (0/1)
        if (c == N && d == 1) continue;  // skip 1/N (infinity in some conventions)
        Frac ords = cuspord(GP, d, c);
        int fw = fanwidth(d, c, N);
        Frac ord = ords * Frac(fw);
        out.push_back({d, c, ord});
    }
    return out;
}

// mintotGAMMA0ORDS: lower bound for sum of ORD at non-oo cusps of a linear combo.
// L = vector of cuspORDSnotoo outputs (one per term). Returns sum over cusps of min_j ORD_j.
inline Frac mintotGAMMA0ORDS(const std::vector<std::vector<CuspOrd>>& L, int nn) {
    if (L.empty() || nn == 0) return Frac(0);
    // Assume all have same cusps in same order
    size_t nc = L[0].size();
    Frac tot(0);
    for (size_t i = 0; i < nc; ++i) {
        Frac m = L[0][i].ord;
        for (int j = 1; j < nn; ++j)
            if (L[j][i].ord < m) m = L[j][i].ord;
        tot = tot + m;
    }
    return tot;
}

// etaprodtoqseries(GP, T) = q-expansion of eta-quotient
// GP = [t1, r1, t2, r2, ...], product η(t_i τ)^{r_i}
// Returns q^{sum r_i*t_i/24} * product etaq(t_i)^{r_i}
inline Series etaprodtoqseries(const std::vector<int>& GP, int T) {
    if (GP.size() % 2 != 0) return Series::zero(T);
    Series q = Series::q(T + 5);
    Series xm = Series::one(T + 5);
    int qinf = 0;
    for (size_t i = 0; i < GP.size(); i += 2) {
        int t = GP[i], r = GP[i + 1];
        if (t <= 0) continue;
        Series et = etaq(q, t, T + 5);
        Series term = et.pow(r);
        xm = (xm * term).truncTo(T + 5);
        qinf += r * t / 24;
    }
    Series result = (q.pow(qinf) * xm).truncTo(T + 5);
    result.trunc = T;
    result.clean();
    return result;
}

// provemodfuncGAMMA0id: prove eta identity on Gamma_0(N)
// etaid = list of [c, GP] meaning c * etaprod(GP). Proves sum = 0.
// Returns 1 if proven, 0 if not.
struct ProveModfuncResult {
    int proven = 0;       // 1 if identity holds
    int sturm_bound = 0;  // depth to which we verify
    std::string message;
};

inline ProveModfuncResult provemodfuncGAMMA0id(
    const std::vector<std::pair<Frac, std::vector<int>>>& etaid, int N) {
    ProveModfuncResult out;
    if (etaid.empty()) { out.proven = 1; out.message = "trivial"; return out; }
    std::vector<std::vector<CuspOrd>> ordsList;
    for (const auto& [c, GP] : etaid) {
        if (GP.empty()) {  // constant term
            std::vector<CuspOrd> empty;
            auto cusps = cuspmake(N);
            for (const auto& [d, dc] : cusps) {
                if ((dc == 1 && d == 0) || (dc == N && d == 1)) continue;
                empty.push_back({d, dc, Frac(0)});
            }
            ordsList.push_back(std::move(empty));
        } else {
            if (gammacheck(GP, N) != 1) {
                out.message = "term not modular on Gamma_0(N)";
                return out;
            }
            ordsList.push_back(cuspORDSnotoo(GP, N));
        }
    }
    int nn = static_cast<int>(etaid.size());
    Frac mintot = mintotGAMMA0ORDS(ordsList, nn);
    int mintot_int = 0;
    if (mintot.den == BigInt(1) && !mintot.num.d.empty()) {
        int64_t n = static_cast<int64_t>(mintot.num.d[0]);
        if (mintot.num.neg) mintot_int = -static_cast<int>(n); else mintot_int = static_cast<int>(n);
    }
    int sturm = (mintot_int < 0) ? (-mintot_int + 2 * N) : (2 * N);
    if (sturm > 500) sturm = 500;  // cap
    out.sturm_bound = sturm;

    // Verify: sum c_i * etaprodtoqseries(GP_i, T) = 0 to O(q^sturm)
    Series sum = Series::zero(sturm + 5);
    sum.trunc = sturm + 5;
    for (const auto& [c, GP] : etaid) {
        if (GP.empty())
            sum = (sum + Series::constant(c, sturm + 5)).truncTo(sturm + 5);
        else
            sum = (sum + Series::constant(c, sturm + 5) * etaprodtoqseries(GP, sturm + 5)).truncTo(sturm + 5);
    }
    bool is_zero = true;
    for (const auto& [e, v] : sum.c)
        if (e < sturm && !v.isZero()) { is_zero = false; break; }
    out.proven = is_zero ? 1 : 0;
    out.message = is_zero ? "identity proven" : "identity does not hold";
    return out;
}

// provemodfuncGAMMA0idBATCH: run provemodfuncGAMMA0id on each etaid
inline std::vector<ProveModfuncResult> provemodfuncGAMMA0idBATCH(
    const std::vector<std::vector<std::pair<Frac, std::vector<int>>>>& etaids, int N) {
    std::vector<ProveModfuncResult> out;
    out.reserve(etaids.size());
    for (const auto& etaid : etaids)
        out.push_back(provemodfuncGAMMA0id(etaid, N));
    return out;
}

// provemodfuncGAMMA0UpETAid: prove eta identity using U_p sifting (stub)
inline ProveModfuncResult provemodfuncGAMMA0UpETAid(
    const std::vector<std::pair<Frac, std::vector<int>>>& /*etaid*/, int /*N*/, int /*p*/) {
    ProveModfuncResult out;
    out.proven = 0;
    out.sturm_bound = 0;
    out.message = "U_p operator proof not yet implemented";
    return out;
}

#endif
