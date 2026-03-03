#ifndef RR_IDS_H
#define RR_IDS_H

#include "series.h"
#include "qfuncs.h"
#include "frac.h"
#include "convert.h"
#include "theta_ids.h"
#include <vector>
#include <tuple>
#include <optional>
#include <numeric>
#include <algorithm>
#include <map>

// Add two series with different q_shifts by collecting terms by exponent
inline Series addSeriesAligned(const Series& s1, const Series& s2, int c1_sign, int T) {
    std::map<Frac, Frac> terms;
    for (const auto& [n, v] : s1.c) {
        if (!v.isZero())
            terms[s1.q_shift + Frac(n)] = terms[s1.q_shift + Frac(n)] + v;
    }
    for (const auto& [n, v] : s2.c) {
        if (!v.isZero()) {
            Frac add = v * Frac(c1_sign);
            terms[s2.q_shift + Frac(n)] = terms[s2.q_shift + Frac(n)] + add;
        }
    }
    Frac minExp;
    bool first = true;
    for (const auto& [e, c] : terms) {
        if (!c.isZero()) {
            if (first) { minExp = e; first = false; }
            else if (e < minExp) minExp = e;
        }
    }
    if (first) return Series::zero(T);
    Series out;
    out.trunc = T;
    out.q_shift = minExp;
    int idx = 0;
    for (const auto& [e, c] : terms) {
        if (c.isZero()) continue;
        Frac expOffset = e - minExp;
        if (expOffset.den != BigInt(1)) continue;
        int n = 0;
        if (expOffset.num.d.size() == 1 && expOffset.num.d[0] <= 10000)
            n = expOffset.num.neg ? -static_cast<int>(expOffset.num.d[0]) : static_cast<int>(expOffset.num.d[0]);
        if (n >= 0 && n < T)
            out.c[n] = c;
        ++idx;
    }
    out.clean();
    return out;
}

// Rogers-Ramanujan G(1) = Σ q^(n²)/(q;q)_n = 1/((q;q⁵)_∞(q⁴;q⁵)_∞)
inline Series RRG1(int T) {
    Series sum = Series::one(T);
    Series q = Series::q(T);
    for (int n = 1; n * n < T; ++n) {
        Series term = q.pow(n * n) / aqprod(q, q, n, T);
        sum = (sum + term).truncTo(T);
    }
    return sum;
}

// Rogers-Ramanujan H(1) = Σ q^(n²+n)/(q;q)_n = 1/((q²;q⁵)_∞(q³;q⁵)_∞)
inline Series RRH1(int T) {
    Series sum = Series::one(T);
    Series q = Series::q(T);
    for (int n = 1; n * n + n < T; ++n) {
        Series term = q.pow(n * n + n) / aqprod(q, q, n, T);
        sum = (sum + term).truncTo(T);
    }
    return sum;
}

// Geta(g,d,n) = q^(n·QP2(g/d)·d/2) · JAC(n·g, n·d, ∞) / JAC(0, n·d, ∞)
inline Series geta(int g, int d, int n, int T) {
    Frac gOverD(g, d);
    Frac qp2 = QP2(gOverD);
    Frac getaExp = Frac(n) * qp2 * Frac(d, 2);

    std::vector<JacFactor> jac;
    jac.push_back({n * g, n * d, Frac(1)});
    jac.push_back({0, n * d, Frac(-1)});

    Series jacSer = jac2series(jac, T);
    Series q_var = Series::q(T);
    Series result = (q_var.powFrac(getaExp) * jacSer).truncTo(T);
    return result;
}

inline Series RRG(int n, int T) {
    if (n == 1) return RRG1(T);
    return geta(1, 5, n, T);
}

inline Series RRH(int n, int T) {
    if (n == 1) return RRH1(T);
    return geta(2, 5, n, T);
}

// Göllnitz-Gordon G*(1) = Σ q^(n²)*(-q;q²)_n/(q²;q²)_n
inline Series RRGstar1(int T) {
    Series sum = Series::one(T);
    Series q = Series::q(T);
    Series neg_q = Series::constant(Frac(-1), T) * q;
    Series q2 = q.pow(2);
    for (int n = 1; n * n < T; ++n) {
        Series num = q.pow(n * n) * aqprod(neg_q, q2, n, T);
        Series den = aqprod(q2, q2, n, T);
        Series term = num / den;
        sum = (sum + term).truncTo(T);
    }
    return sum;
}

// Göllnitz-Gordon H*(1) = Σ q^(n(n+2))*(-q;q²)_n/(q²;q²)_n
inline Series RRHstar1(int T) {
    Series sum = Series::one(T);
    Series q = Series::q(T);
    Series neg_q = Series::constant(Frac(-1), T) * q;
    Series q2 = q.pow(2);
    for (int n = 1; n * (n + 2) < T; ++n) {
        Series num = q.pow(n * (n + 2)) * aqprod(neg_q, q2, n, T);
        Series den = aqprod(q2, q2, n, T);
        Series term = num / den;
        sum = (sum + term).truncTo(T);
    }
    return sum;
}

inline Series RRGstar(int n, int T) {
    if (n == 1) return RRGstar1(T);
    (void)n;
    return RRGstar1(T);  // n>1 deferred per plan
}

inline Series RRHstar(int n, int T) {
    if (n == 1) return RRHstar1(T);
    (void)n;
    return RRHstar1(T);  // n>1 deferred per plan
}

// GE(a), HE(a) for Rogers-Ramanujan modulus 5
inline Frac GE(int a) {
    return Frac(5 * a * a - 5 * a + 2, 10);
}
inline Frac HE(int a) {
    return Frac(5 * a * a + 5 * a + 2, 10);
}

inline bool fracIsInteger(const Frac& x) {
    return x.den == BigInt(1);
}

// ABCOND type 1: GE(p)+HE(a)-(GE(a)+HE(p)) integer
inline bool abcond_type1(int p, int a) {
    Frac t = GE(p) + HE(a) - GE(a) - HE(p);
    return fracIsInteger(t);
}

// ABCOND type 2: GE(a)+GE(p)-(HE(a)+HE(p)) integer
inline bool abcond_type2(int a, int p) {
    Frac t = GE(a) + GE(p) - HE(a) - HE(p);
    return fracIsInteger(t);
}

// checkid: CHECKRAMIDF - normalize, prodmake, check max|a_n|<acc, etamake
struct CheckidResult {
    bool ok = false;
    int ldq = 0;
    std::vector<std::pair<int, Frac>> eta;
};

inline CheckidResult checkid(const Series& f, int T, int acc = 10) {
    CheckidResult out;
    int ldq = f.minExp();
    if (ldq < 0) return out;
    Series g = f.truncTo(T);
    Frac c0 = g.coeff(ldq);
    if (c0.isZero()) return out;
    Series q_var = Series::q(T);
    Series h = (g / (Series::constant(c0, T) * q_var.pow(ldq))).truncTo(T);
    if (h.coeff(0) != Frac(1) || h.minExp() > 0) return out;
    auto a_map = prodmake(h, T);
    if (a_map.empty()) return out;
    int plx = 0;
    for (const auto& [n, an] : a_map) {
        if (an.den != BigInt(1)) return out;
        int v = 0;
        if (an.num.d.size() == 1 && an.num.d[0] <= static_cast<uint32_t>(10000))
            v = an.num.neg ? -static_cast<int>(an.num.d[0]) : static_cast<int>(an.num.d[0]);
        if (v < 0) v = -v;
        if (v > plx) plx = v;
    }
    if (plx >= acc) return out;
    auto eta_list = etamake(h, T);
    if (eta_list.empty()) return out;
    out.ok = true;
    out.ldq = ldq;
    out.eta = std::move(eta_list);
    return out;
}

// findids_type1: G(p)H(a) ± G(a)H(p). Uses addSeriesAligned for different q_shifts.
using FindidTuple = std::tuple<int, int, int>;  // (p,a,c1) or (a,p,c1)
inline std::vector<FindidTuple> findids_type1(int T, int acc = 10) {
    std::vector<FindidTuple> out;
    for (int n = 2; n <= T; ++n) {
        for (int p : divisors(n)) {
            int a = n / p;
            if (a == p || a > p || std::gcd(a, p) != 1) continue;
            for (int c1 : {-1, 1}) {
                if (!abcond_type1(p, a)) continue;
                Series t1 = (RRG(p, T) * RRH(a, T)).truncTo(T);
                Series t2 = (RRG(a, T) * RRH(p, T)).truncTo(T);
                Series symf;
                try {
                    symf = addSeriesAligned(t1, t2, c1, T);
                } catch (...) { continue; }
                if (checkid(symf, T, acc).ok)
                    out.push_back({p, a, c1});
            }
        }
    }
    return out;
}

// findids_type2: G(a)G(p) ± H(a)H(p)
inline std::vector<FindidTuple> findids_type2(int T, int acc = 10) {
    std::vector<FindidTuple> out;
    for (int n = 1; n <= T; ++n) {
        for (int p : divisors(n)) {
            int a = n / p;
            if (a > p || std::gcd(a, p) != 1) continue;
            for (int c1 : {-1, 1}) {
                if (!abcond_type2(a, p)) continue;
                Series t1 = (RRG(a, T) * RRG(p, T)).truncTo(T);
                Series t2 = (RRH(a, T) * RRH(p, T)).truncTo(T);
                Series symf;
                try {
                    symf = addSeriesAligned(t1, t2, c1, T);
                } catch (...) { continue; }
                if (checkid(symf, T, acc).ok)
                    out.push_back({a, p, c1});
            }
        }
    }
    return out;
}

inline std::vector<FindidTuple> findids(int type, int T, int acc = 10) {
    if (type == 1) return findids_type1(T, acc);
    if (type == 2) return findids_type2(T, acc);
    return {};
}

#endif
