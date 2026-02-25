#ifndef QFUNCS_H
#define QFUNCS_H

#include "series.h"
#include <vector>
#include <cmath>
#include <cstdint>

// ============ Number theory helpers ============

inline std::vector<int> divisors(int n) {
    if (n <= 0) return {};
    if (n == 1) return {1};
    std::vector<int> d;
    int i = 1;
    for (; i * i < n; ++i) {
        if (n % i == 0) {
            d.push_back(i);
            d.push_back(n / i);
        }
    }
    if (i * i == n) d.push_back(i);
    std::sort(d.begin(), d.end());
    return d;
}

inline int mobius(int n) {
    if (n <= 0) return 0;
    if (n == 1) return 1;
    int count = 0;
    for (int p = 2; p * p <= n; ++p) {
        if (n % p == 0) {
            int exp = 0;
            while (n % p == 0) { n /= p; ++exp; }
            if (exp > 1) return 0;
            ++count;
        }
    }
    if (n > 1) ++count;
    return (count % 2 == 0) ? 1 : -1;
}

inline int legendre(int64_t a, int64_t p) {
    if (p <= 0 || (p > 2 && p % 2 == 0)) return 0;
    a = a % p;
    if (a < 0) a += p;
    if (a == 0) return 0;
    int64_t r = 1;
    int64_t exp = (p - 1) / 2;
    int64_t base = a;
    while (exp > 0) {
        if (exp % 2 == 1) r = (r * base) % p;
        base = (base * base) % p;
        exp /= 2;
    }
    return static_cast<int>(r == 1 ? 1 : -1);
}

inline int64_t sigma(int n, int k = 1) {
    if (n <= 0) return 0;
    if (n == 1) return 1;
    int64_t sum = 0;
    for (int d : divisors(n)) {
        int64_t term = 1;
        for (int i = 0; i < k; ++i) term *= static_cast<int64_t>(d);
        sum += term;
    }
    return sum;
}

inline int64_t euler_phi(int n) {
    if (n <= 0) return 0;
    if (n == 1) return 1;
    int64_t result = static_cast<int64_t>(n);
    int x = n;
    for (int p = 2; p * p <= x; ++p) {
        if (x % p == 0) {
            while (x % p == 0) x /= p;
            result = result / p * (p - 1);
        }
    }
    if (x > 1) result = result / x * (x - 1);
    return result;
}

// ============ q-series functions ============

inline Series etaq(const Series& q, int k, int T) {
    Series result = Series::one(T);
    for (int n = 1; k * n < T; ++n) {
        Series qkn = q.truncTo(T).pow(k * n);
        Series factor = Series::one(T) - qkn;
        result = (result * factor).truncTo(T);
    }
    result.trunc = T;
    return result;
}

inline Series aqprod(const Series& a, const Series& q, int n, int T) {
    if (n <= 0) return Series::one(T);
    Series result = Series::one(T);
    Series qt = q.truncTo(T);
    Series at = a.truncTo(T);
    for (int k = 0; k < n; ++k) {
        Series qk = qt.pow(k);
        Series factor = Series::one(T) - (at * qk);
        result = (result * factor).truncTo(T);
    }
    result.trunc = T;
    return result;
}

inline Series qbin(const Series& q, int m, int n, int T) {
    if (m < 0 || m > n) return Series::zero(T);
    if (m == 0 || m == n) return Series::one(T);
    Series result = Series::one(T);
    Series qt = q.truncTo(T);
    for (int i = 1; i <= m; ++i) {
        Series num = Series::one(T) - qt.pow(n - m + i);
        Series den = Series::one(T) - qt.pow(i);
        result = (result * num / den).truncTo(T);
    }
    result.trunc = T;
    return result;
}

// ============ Theta functions ============

inline Series theta2(const Series& q, int T) {
    Series result = Series::zero(T);
    result.trunc = T;
    for (int n = 0; n * (n + 1) < T; ++n) {
        int exp = n * (n + 1);
        result.setCoeff(exp, Frac(2));
    }
    return result;
}

inline Series theta3(const Series& q, int T) {
    Series result = Series::one(T);
    result.trunc = T;
    for (int n = 1; n * n < T; ++n) {
        int exp = n * n;
        result.setCoeff(exp, result.coeff(exp) + Frac(2));
    }
    return result;
}

inline Series theta4(const Series& q, int T) {
    Series result = Series::one(T);
    result.trunc = T;
    for (int n = 1; n * n < T; ++n) {
        int exp = n * n;
        int sign = (n % 2 == 0) ? 2 : -2;
        result.setCoeff(exp, result.coeff(exp) + Frac(sign));
    }
    return result;
}

inline Series theta(const Series& z, const Series& q, int T) {
    Series result = Series::zero(T);
    result.trunc = T;
    Series zt = z.truncTo(T);
    Series qt = q.truncTo(T);
    for (int i = -T; i <= T; ++i) {
        Series zi = (i >= 0) ? zt.pow(i) : zt.inverse().pow(-i);
        Series qi2 = qt.pow(i * i);
        Series term = (zi * qi2).truncTo(T);
        for (const auto& [e, v] : term.c) {
            if (e < T) result.setCoeff(e, result.coeff(e) + v);
        }
    }
    result.clean();
    return result;
}

#endif
