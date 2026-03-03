#ifndef OMEGA3_H
#define OMEGA3_H

#include "frac.h"
#include <string>
#include <stdexcept>

// Omega3: a + b·ω where ω² = -ω-1 (ω = exp(2πi/3))
struct Omega3 {
    Frac a, b;

    Omega3() : a(0), b(0) {}

    Omega3(Frac a_, Frac b_) : a(std::move(a_)), b(std::move(b_)) {}

    static Omega3 omega() { return Omega3(Frac(0), Frac(1)); }
    static Omega3 omega2() { return Omega3(Frac(-1), Frac(-1)); }  // ω² = -ω-1
    static Omega3 fromRational(Frac r) { return Omega3(std::move(r), Frac(0)); }

    bool isZero() const { return a.isZero() && b.isZero(); }

    Frac norm() const {
        // N(a+bω) = a² - ab + b²
        return a * a - a * b + b * b;
    }

    Omega3 conjugate() const {
        // conjugate of a+bω is (a-b) - bω (ω̅ = ω² = -ω-1)
        return Omega3(a - b, -b);
    }

    Omega3 operator+(const Omega3& o) const {
        return Omega3(a + o.a, b + o.b);
    }

    Omega3 operator-(const Omega3& o) const {
        return Omega3(a - o.a, b - o.b);
    }

    Omega3 operator-() const {
        return Omega3(-a, -b);
    }

    Omega3 operator*(const Omega3& o) const {
        // (a+bω)(c+dω) = ac + (ad+bc)ω + bdω² = ac - bd + (ad+bc-bd)ω
        Frac ac = a * o.a;
        Frac bd = b * o.b;
        Frac ad = a * o.b;
        Frac bc = b * o.a;
        return Omega3(ac - bd, ad + bc - bd);
    }

    Omega3 operator/(const Omega3& o) const {
        if (o.isZero())
            throw std::invalid_argument("Omega3: division by zero");
        Frac N = o.norm();
        Omega3 num = *this * o.conjugate();
        Frac ra = num.a / N;
        Frac rb = num.b / N;
        return Omega3(std::move(ra), std::move(rb));
    }

    bool operator==(const Omega3& o) const {
        return a == o.a && b == o.b;
    }

    bool operator!=(const Omega3& o) const { return !(*this == o); }

    static Omega3 pow(Omega3 z, int k) {
        if (z == omega() || z == omega2()) {
            k = ((k % 3) + 3) % 3;
            if (k == 0) return Omega3(Frac(1), Frac(0));
            if (k == 1) return omega();
            return omega2();
        }
        if (k == 0) return Omega3(Frac(1), Frac(0));
        if (k < 0) return Omega3(Frac(1), Frac(0)) / pow(z, -k);
        Omega3 r(Frac(1), Frac(0));
        Omega3 base = z;
        int p = k;
        while (p) {
            if (p & 1) r = r * base;
            base = base * base;
            p >>= 1;
        }
        return r;
    }

    std::string str() const {
        if (isZero()) return "0";
        if (a == Frac(1) && b == Frac(0)) return "1";
        if (a == Frac(0) && b == Frac(1)) return "omega";
        if (a == Frac(-1) && b == Frac(-1)) return "omega^2";
        if (b.isZero()) return a.str();
        if (a.isZero()) {
            if (b == Frac(1)) return "omega";
            if (b == Frac(-1)) return "-omega";
            if (b > Frac(0)) return b.str() + "*omega";
            return "-" + b.abs().str() + "*omega";
        }
        // a nonzero, b nonzero
        if (b == Frac(1)) return a.str() + " + omega";
        if (b == Frac(-1)) return a.str() + " - omega";
        if (b > Frac(0)) return a.str() + " + " + b.str() + "*omega";
        return a.str() + " - " + b.abs().str() + "*omega";
    }
};

#endif
