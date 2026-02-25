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
};

#endif
