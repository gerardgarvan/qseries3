#ifndef QFUNCS_H
#define QFUNCS_H

#include "series.h"
#include <vector>
#include <cmath>
#include <cstdint>
#include <utility>

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

inline int jacobi(int64_t a, int64_t n) {
    if (n <= 0 || n % 2 == 0) return 0;
    if (n == 1) return 1;
    a = a % n;
    if (a < 0) a += n;
    if (a == 0) return 0;
    int result = 1;
    while (a != 0) {
        while (a % 2 == 0) {
            a /= 2;
            if (n % 8 == 3 || n % 8 == 5) result = -result;
        }
        std::swap(a, n);
        if (a % 4 == 3 && n % 4 == 3) result = -result;
        a = a % n;
    }
    return (n == 1) ? result : 0;
}

inline int kronecker(int64_t a, int64_t n) {
    if (n == 0) return (a == 1 || a == -1) ? 1 : 0;
    int result = 1;
    if (n < 0) {
        n = -n;
        if (a < 0) result = -result;
    }
    int twos = 0;
    while (n % 2 == 0) { n /= 2; ++twos; }
    if (twos > 0) {
        if (a % 2 == 0) return 0;
        int64_t a8 = ((a % 8) + 8) % 8;
        int kr2 = (a8 == 1 || a8 == 7) ? 1 : -1;
        for (int i = 0; i < twos; ++i) result *= kr2;
    }
    if (n == 1) return result;
    return result * jacobi(a, n);
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

// ============ Eisenstein series ============

inline Frac bernoulli_even(int n) {
    static const std::pair<int64_t, int64_t> table[] = {
        {1, 6},           // B_2
        {-1, 30},         // B_4
        {1, 42},          // B_6
        {-1, 30},         // B_8
        {5, 66},          // B_10
        {-691, 2730},     // B_12
        {7, 6},           // B_14
        {-3617, 510},     // B_16
        {43867, 798},     // B_18
        {-174611, 330},   // B_20
    };
    int idx = n / 2 - 1;
    if (idx < 0 || idx >= 10)
        throw std::runtime_error("eisenstein: k too large (max k=10)");
    return Frac(table[idx].first, table[idx].second);
}

inline Series eisenstein(int k, int T) {
    if (k < 1) throw std::runtime_error("eisenstein: k must be >= 1");
    Frac B = bernoulli_even(2 * k);
    Frac multiplier = Frac(-4 * k) / B;
    Series result;
    result.trunc = T;
    result.setCoeff(0, Frac(1));
    for (int n = 1; n < T; ++n) {
        Frac s(0);
        for (int d : divisors(n)) {
            Frac df(d);
            Frac term(1);
            for (int i = 0; i < 2 * k - 1; ++i) term = term * df;
            s = s + term;
        }
        result.setCoeff(n, multiplier * s);
    }
    return result;
}

// ============ q-series functions ============

inline std::map<std::pair<int,int>, Series>& etaq_cache() {
    static std::map<std::pair<int,int>, Series> cache;
    return cache;
}

inline void clear_etaq_cache() {
    etaq_cache().clear();
}

inline Series etaq(const Series& q, int k, int T) {
    bool standardQ = (q.c.size() == 1
                      && q.c.begin()->first == 1
                      && q.c.begin()->second == Frac(1));

    if (standardQ) {
        auto it = etaq_cache().find({k, T});
        if (it != etaq_cache().end()) return it->second;
    }

    Series result;
    if (k == 1 && standardQ) {
        result = Series::zero(T);
        result.trunc = T;
        for (int j = 0; ; ++j) {
            int p1 = j * (3 * j - 1) / 2;
            int p2 = j * (3 * j + 1) / 2;
            if (p1 >= T && p2 >= T) break;
            Frac sign(j % 2 == 0 ? 1 : -1);
            if (p1 < T) result.setCoeff(p1, sign);
            if (p2 < T && p2 != p1) result.setCoeff(p2, sign);
        }
    } else {
        result = Series::one(T);
        for (int n = 1; k * n < T; ++n) {
            Series qkn = q.truncTo(T).pow(k * n);
            Series factor = Series::one(T) - qkn;
            result = (result * factor).truncTo(T);
        }
        result.trunc = T;
    }

    if (standardQ) {
        etaq_cache()[{k, T}] = result;
    }
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

inline Series theta2(const Series& /*q*/, int T) {
    Series result = Series::zero(T);
    result.trunc = T;
    for (int n = 0; n * (n + 1) < T; ++n) {
        int exp = n * (n + 1);
        result.setCoeff(exp, Frac(2));
    }
    return result;
}

inline Series theta3(const Series& /*q*/, int T) {
    Series result = Series::one(T);
    result.trunc = T;
    for (int n = 1; n * n < T; ++n) {
        int exp = n * n;
        result.setCoeff(exp, result.coeff(exp) + Frac(2));
    }
    return result;
}

inline Series theta4(const Series& /*q*/, int T) {
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

// ============ Triple and quintuple products ============

inline Series tripleprod(const Series& z, const Series& q, int T) {
    Series result = Series::one(T);
    Series zt = z.truncTo(T);
    Series qt = q.truncTo(T);
    Series zinv = zt.inverse();
    for (int n = 1; n < T; ++n) {
        Series f1 = Series::one(T) - (zt * qt.pow(n - 1));
        Series f2 = Series::one(T) - (zinv * qt.pow(n));
        Series f3 = Series::one(T) - qt.pow(n);
        result = (result * f1 * f2 * f3).truncTo(T);
    }
    result.trunc = T;
    return result;
}

inline Series quinprod(const Series& z, const Series& q, int T) {
    Series result = Series::one(T);
    Series zt = z.truncTo(T);
    Series qt = q.truncTo(T);
    Series zinv = zt.inverse();
    for (int n = 1; n < T; ++n) {
        Series f1 = Series::one(T) + (zt * qt.pow(n - 1));
        Series f2 = Series::one(T) + (qt.pow(n) * zinv);
        Series f3 = Series::one(T) - qt.pow(n);
        result = (result * f1 * f2 * f3).truncTo(T);
    }
    for (int m = 1; 2 * m - 1 < T; ++m) {
        int e = 2 * m - 1;
        Series f4 = Series::one(T) - (zt.pow(2) * qt.pow(e));
        Series f5 = Series::one(T) - (qt.pow(e) * zinv.pow(2));
        result = (result * f4 * f5).truncTo(T);
    }
    result.trunc = T;
    return result;
}

inline Series winquist(const Series& a, const Series& b, const Series& q, int T) {
    Series result = Series::zero(T);
    result.trunc = T;
    Series at = a.truncTo(T);
    Series bt = b.truncTo(T);
    Series qt = q.truncTo(T);
    Series ainv = at.inverse();
    Series binv = bt.inverse();
    for (int n = 0; 3 * n * (n + 1) / 2 < T; ++n) {
        for (int m = -T; m <= T; ++m) {
            int exp = 3 * n * (n + 1) / 2 + m * (3 * m + 1) / 2;
            if (exp < 0 || exp >= T) continue;
            int sign = ((n + m) % 2 == 0) ? 1 : -1;
            Series t1a = (n == 0) ? Series::one(T) : ainv.pow(3 * n);
            Series t1b = at.pow(3 * n + 3);
            Series t1 = t1a - t1b;
            Series t2a = (m == 0) ? Series::one(T) : binv.pow(3 * m);
            Series t2b = bt.pow(3 * m + 1);
            Series t2 = t2a - t2b;
            Series t3a = at.pow(1 - 3 * m);
            Series t3b = at.pow(3 * m + 2);
            Series t3 = t3a - t3b;
            Series t4a = bt.pow(3 * n + 2);
            Series t4b = binv.pow(3 * n + 1);
            Series t4 = t4a - t4b;
            Series term = ((t1 * t2) + (t3 * t4)).truncTo(T);
            if (sign < 0) term = -term;
            Series qpow = qt.pow(exp);
            result = (result + (term * qpow).truncTo(T)).truncTo(T);
        }
    }
    result.clean();
    return result;
}

// ============ Partition & q-derivative ============

inline Frac partition_number(int n) {
    if (n < 0) return Frac(0);
    if (n == 0) return Frac(1);
    int T = n + 1;
    Series q = Series::q(T);
    Series eta = etaq(q, 1, T);
    Series gen = eta.inverse();
    return gen.coeff(n);
}

inline Series qdiff(const Series& f) {
    Series result;
    result.trunc = f.trunc;
    for (const auto& [e, v] : f.c) {
        if (e != 0 && e < f.trunc)
            result.setCoeff(e, v * Frac(e));
    }
    return result;
}

#endif
