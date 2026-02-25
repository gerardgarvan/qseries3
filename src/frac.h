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
};

#endif
