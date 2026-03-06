#ifndef MODFORMS_H
#define MODFORMS_H

#include "series.h"
#include "qfuncs.h"
#include <vector>
#include <cmath>

// DELTA12(T) = q * eta(q)^24 = q * etaq(1,T)^24 (discriminant cusp form of weight 12)
inline Series DELTA12(int T) {
    Series q = Series::q(T + 30);
    Series eta1 = etaq(q, 1, T + 30);
    Series d = (q * eta1.pow(24)).truncTo(T + 5);
    d.trunc = T;
    d.clean();
    return d;
}

// makebasisM(k, T): basis of M_k(SL_2(Z)) using E4 and E6
// Basis elements: E4^a * E6^b where 4*a + 6*b = k, a>=0, b>=0
inline std::vector<Series> makebasisM(int k, int T) {
    if (k < 0 || k % 2 != 0)
        throw std::runtime_error("makebasisM: k must be nonnegative even integer");
    std::vector<Series> FL;
    Series E4 = EISENq(4, T + 5);
    Series E6 = EISENq(6, T + 5);
    for (int a = 0; a <= k / 4; ++a) {
        int b6 = k - 4 * a;
        if (b6 % 6 != 0) continue;
        int b = b6 / 6;
        if (b < 0) continue;
        Series f = (E4.pow(a) * E6.pow(b)).truncTo(T + 5);
        f.trunc = T;
        f.clean();
        FL.push_back(std::move(f));
    }
    return FL;
}

// makeALTbasisM(k, T): alternative basis of M_k(SL_2(Z)) using DELTA12 and E6
// Basis: Es * E6^(2r-2i) * DELTA12^i for i=0..r, where k=12r+s, s=k mod 12
inline std::vector<Series> makeALTbasisM(int k, int T) {
    if (k < 0 || k % 2 != 0)
        throw std::runtime_error("makeALTbasisM: k must be nonnegative even integer");
    std::vector<Series> FL;
    int s = k % 12;
    int r = (s == 2) ? (k - 14) / 12 : (k - s) / 12;
    Series Es;
    if (s == 0) Es = Series::one(T + 5);
    else if (s == 2) Es = EISENq(14, T + 5);
    else if (s == 4) Es = EISENq(4, T + 5);
    else if (s == 6) Es = EISENq(6, T + 5);
    else if (s == 8) Es = EISENq(8, T + 5);
    else Es = EISENq(10, T + 5);  // s == 10
    Series E6 = EISENq(6, T + 5);
    Series D = DELTA12(T + 5);
    for (int i = 0; i <= r; ++i) {
        Series f = (Es * E6.pow(2 * r - 2 * i) * D.pow(i)).truncTo(T + 5);
        f.trunc = T;
        f.clean();
        FL.push_back(std::move(f));
    }
    return FL;
}

// makebasisPX(k, T): basis for P*X[k] using partition function P = 1/eta(q) and Phiq(1,3,5)
// Basis elements: P * Phiq(1)^a * Phiq(3)^b * Phiq(5)^c where 2a+4b+6c <= k
inline std::vector<Series> makebasisPX(int k, int T) {
    if (k < 0 || k % 2 != 0)
        throw std::runtime_error("makebasisPX: k must be nonnegative even integer");
    std::vector<Series> FL;
    Series q = Series::q(T + 10);
    Series eta1 = etaq(q, 1, T + 10);
    Series P = eta1.inverse().truncTo(T + 5);
    P.trunc = T + 5;
    Series Phi1 = Phiq(1, T + 5);
    Series Phi3 = Phiq(3, T + 5);
    Series Phi5 = Phiq(5, T + 5);
    for (int a = 0; a <= k / 2; ++a) {
        for (int b = 0; b <= (k - 2 * a) / 4; ++b) {
            for (int c = 0; c <= (k - 2 * a - 4 * b) / 6; ++c) {
                Series f = (P * Phi1.pow(a) * Phi3.pow(b) * Phi5.pow(c)).truncTo(T + 5);
                f.trunc = T;
                f.clean();
                FL.push_back(std::move(f));
            }
        }
    }
    return FL;
}

#endif
