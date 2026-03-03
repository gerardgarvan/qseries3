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

#endif
