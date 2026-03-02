#ifndef FRAC_H
#define FRAC_H

#include "bigint.h"
#include <string>

struct Frac {
    BigInt num, den;

    Frac() : num(0), den(1) {}

    Frac(int64_t n) : num(n), den(1) {
        reduce();
    }

    Frac(int64_t n, int64_t d) : num(n), den(d) {
        if (d == 0)
            throw std::invalid_argument("Frac: denominator zero");
        reduce();
    }

    Frac(BigInt n, BigInt d) : num(std::move(n)), den(std::move(d)) {
        if (den.isZero())
            throw std::invalid_argument("Frac: denominator zero");
        reduce();
    }

    void reduce() {
        if (den.isZero())
            throw std::invalid_argument("Frac: denominator zero");
        if (den < BigInt(0)) {
            num = -num;
            den = -den;
        }
        if (num.isZero()) {
            den = BigInt(1);
            return;
        }
        BigInt g = bigGcd(num.abs(), den);
        num = num / g;
        den = den / g;
    }

    bool isZero() const { return num.isZero(); }
    bool isOne() const { return num == den && !num.isZero(); }
    Frac abs() const {
        Frac r;
        r.num = num.abs();
        r.den = den;
        return r;
    }

    Frac operator-() const {
        Frac r;
        r.num = -num;
        r.den = den;
        return r;
    }

    Frac operator+(const Frac& o) const {
        BigInt n = num * o.den + o.num * den;
        BigInt d = den * o.den;
        return Frac(std::move(n), std::move(d));
    }

    Frac operator-(const Frac& o) const {
        BigInt n = num * o.den - o.num * den;
        BigInt d = den * o.den;
        return Frac(std::move(n), std::move(d));
    }

    Frac operator*(const Frac& o) const {
        BigInt n = num * o.num;
        BigInt d = den * o.den;
        return Frac(std::move(n), std::move(d));
    }

    Frac operator/(const Frac& o) const {
        if (o.isZero())
            throw std::invalid_argument("Frac: division by zero");
        BigInt n = num * o.den;
        BigInt d = den * o.num;
        return Frac(std::move(n), std::move(d));
    }

    bool operator==(const Frac& o) const {
        return num == o.num && den == o.den;
    }

    bool operator!=(const Frac& o) const {
        return !(*this == o);
    }

    bool operator<(const Frac& o) const {
        return (num * o.den) < (o.num * den);
    }

    bool operator<=(const Frac& o) const {
        return !(o < *this);
    }

    bool operator>(const Frac& o) const {
        return o < *this;
    }

    bool operator>=(const Frac& o) const {
        return !(*this < o);
    }

    Frac rational_pow(Frac alpha) const {
        if (isZero()) {
            if (alpha > Frac(0)) return Frac(0);
            throw std::invalid_argument("rational_pow: 0 to non-positive power");
        }

        // Integer exponent: simple repeated multiply
        if (alpha.den == BigInt(1)) {
            int64_t p64 = 0;
            if (!alpha.num.d.empty()) p64 = static_cast<int64_t>(alpha.num.d[0]);
            if (alpha.num.neg) p64 = -p64;
            bool negp = (p64 < 0);
            int pabs = static_cast<int>(negp ? -p64 : p64);
            BigInt rn = bigpow(num.abs(), pabs);
            BigInt rd = bigpow(den, pabs);
            if (negp) std::swap(rn, rd);
            bool rneg = num.neg && (pabs % 2 == 1);
            if (rneg) rn = -rn;
            return Frac(std::move(rn), std::move(rd));
        }

        // General case: alpha = p/q
        int64_t p64 = 0;
        if (!alpha.num.d.empty()) p64 = static_cast<int64_t>(alpha.num.d[0]);
        if (alpha.num.neg) p64 = -p64;
        int64_t q64 = 0;
        if (!alpha.den.d.empty()) q64 = static_cast<int64_t>(alpha.den.d[0]);
        int qint = static_cast<int>(q64);

        if (num.neg && (qint % 2 == 0))
            throw std::invalid_argument("rational_pow: even root of negative");

        bool negp = (p64 < 0);
        int pabs = static_cast<int>(negp ? -p64 : p64);

        BigInt num_p = bigpow(num.abs(), pabs);
        BigInt den_p = bigpow(den, pabs);
        BigInt num_root = iroot(num_p, qint);
        BigInt den_root = iroot(den_p, qint);

        if (negp) std::swap(num_root, den_root);
        bool rneg = num.neg && (pabs % 2 == 1);
        if (rneg) num_root = -num_root;
        return Frac(std::move(num_root), std::move(den_root));
    }

    std::string str() const {
        if (den == BigInt(1))
            return num.str();
        return num.str() + "/" + den.str();
    }
};

#endif
