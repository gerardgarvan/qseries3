#ifndef SERIES_H
#define SERIES_H

#include "frac.h"
#include <map>
#include <string>
#include <vector>
#include <stdexcept>
#include <iostream>
#include <algorithm>

struct Series {
    std::map<int, Frac> c;
    int trunc;

    // Constructors / factories
    static Series zero(int T) {
        Series s;
        s.trunc = T;
        return s;
    }

    static Series one(int T) {
        Series s;
        s.c[0] = Frac(1);
        s.trunc = T;
        return s;
    }

    static Series q(int T) {
        Series s;
        s.c[1] = Frac(1);
        s.trunc = T;
        return s;
    }

    static Series qpow(int e, int T) {
        Series s;
        if (e >= 0 && e < T) {
            s.c[e] = Frac(1);
        }
        s.trunc = T;
        return s;
    }

    static Series constant(Frac f, int T) {
        Series s;
        if (!f.isZero()) {
            s.c[0] = f;
        }
        s.trunc = T;
        return s;
    }

    // Coefficient access
    Frac coeff(int n) const {
        auto it = c.find(n);
        if (it == c.end())
            return Frac(0);
        return it->second;
    }

    void setCoeff(int n, Frac v) {
        if (v.isZero())
            c.erase(n);
        else
            c[n] = v;
    }

    int minExp() const {
        if (c.empty())
            return 0;
        return c.begin()->first;
    }

    int maxExp() const {
        if (c.empty())
            return -1;
        return c.rbegin()->first;
    }

    // Truncation
    void clean() {
        std::vector<int> toErase;
        for (const auto& [e, v] : c) {
            if (e >= trunc || v.isZero())
                toErase.push_back(e);
        }
        for (int e : toErase)
            c.erase(e);
    }

    Series truncTo(int T) const {
        Series s;
        s.trunc = std::min(trunc, T);
        for (const auto& [e, v] : c) {
            if (e < s.trunc && !v.isZero())
                s.c[e] = v;
        }
        return s;
    }

    // Arithmetic
    Series operator-() const {
        Series s;
        s.trunc = trunc;
        for (const auto& [e, v] : c)
            s.c[e] = -v;
        return s;
    }

    Series operator+(const Series& o) const {
        Series s;
        s.trunc = std::min(trunc, o.trunc);
        for (const auto& [e, v] : c) {
            if (e < s.trunc)
                s.c[e] = v;
        }
        for (const auto& [e, v] : o.c) {
            if (e < s.trunc) {
                Frac sum = s.coeff(e) + v;
                s.setCoeff(e, sum);
            }
        }
        s.clean();
        return s;
    }

    Series operator-(const Series& o) const {
        return *this + (-o);
    }

    Series operator*(const Series& o) const {
        Series s;
        int t = std::min(trunc, o.trunc);
        s.trunc = t;
        for (const auto& [e1, c1] : c) {
            for (const auto& [e2, c2] : o.c) {
                int exp = e1 + e2;
                if (exp >= t)  // skip early (PITFALLS: mult trunc skip)
                    continue;
                Frac prod = c1 * c2;
                if (!prod.isZero()) {
                    Frac old = s.coeff(exp);
                    s.setCoeff(exp, old + prod);
                }
            }
        }
        s.clean();
        return s;
    }

    Series operator*(const Frac& f) const {
        if (f.isZero())
            return zero(trunc);
        Series s;
        s.trunc = trunc;
        for (const auto& [e, v] : c)
            s.c[e] = v * f;
        return s;
    }

    Series inverse() const {
        int m = minExp();
        if (m > 0) {
            // f = q^m * h, h[0]!=0. 1/f = q^{-m} * (1/h)
            Series h;
            h.trunc = trunc - m;
            for (const auto& [e, v] : c) {
                int newExp = e - m;
                if (newExp >= 0 && newExp < h.trunc)
                    h.c[newExp] = v;
            }
            Series invH = h.inverse();
            Series result;
            result.trunc = trunc;
            for (const auto& [e, v] : invH.c) {
                int newExp = e - m;
                if (newExp >= -invH.trunc)  // include negative exponents
                    result.c[newExp] = v;
            }
            return result;
        }
        Frac c0 = coeff(0);
        if (c0.isZero())
            throw std::invalid_argument("Series::inverse: constant term zero");
        Series g;
        g.trunc = trunc;
        g.c[0] = Frac(1) / c0;
        for (int n = 1; n < trunc; ++n) {
            Frac sum(0);
            // j MUST start at 1 — j=0 would include c₀*g[n] (circular)
            for (int j = 1; j <= n; ++j) {
                sum = sum + coeff(j) * g.coeff(n - j);
            }
            g.c[n] = -(Frac(1) / c0) * sum;
        }
        return g;
    }

    Series operator/(const Series& o) const {
        return *this * o.inverse();
    }

    Series pow(int n) const {
        if (n == 0)
            return one(trunc);
        if (n < 0)
            return inverse().pow(-n);
        Series base = *this;
        Series result = one(trunc);
        while (n > 0) {
            if (n % 2 == 1)
                result = result * base;
            base = base * base;
            n /= 2;
        }
        return result;
    }

    // Composition: f(q) -> f(q^k)
    Series subs_q(int k) const {
        if (k == 0) {
            Frac sum(0);
            for (const auto& [e, v] : c)
                sum = sum + v;
            return constant(sum, trunc);
        }
        int absK = (k < 0) ? -k : k;
        Series s;
        s.trunc = trunc * absK;
        for (const auto& [e, v] : c) {
            int newExp = e * k;
            if (newExp < s.trunc)
                s.c[newExp] = v;
        }
        s.clean();
        return s;
    }

    static std::string expToUnicode(int n) {
        static const char* digits[] = {"⁰","¹","²","³","⁴","⁵","⁶","⁷","⁸","⁹"};
        if (n == 0) return "⁰";
        std::string s;
        bool neg = (n < 0);
        if (neg) n = -n;
        while (n > 0) {
            s = std::string(digits[n % 10]) + s;
            n /= 10;
        }
        return neg ? "⁻" + s : s;
    }

    std::string str(int maxTerms = 30) const {
        if (c.empty())
            return "0 + O(q^" + std::to_string(trunc) + ")";
        std::string out;
        int count = 0;
        for (const auto& [e, v] : c) {
            if (count >= maxTerms) break;
            if (count > 0) {
                if (v < Frac(0))
                    out += " - ";
                else
                    out += " + ";
            }
            Frac av = v.abs();
            if (e == 0) {
                out += (v < Frac(0) ? "-" : "") + (av.isOne() ? "1" : av.str());
            } else {
                if (!av.isOne())
                    out += (v < Frac(0) ? "-" : "") + av.str();
                if (e == 1)
                    out += "q";
                else
                    out += "q" + expToUnicode(e);
            }
            ++count;
        }
        out += " + O(q^" + std::to_string(trunc) + ")";
        return out;
    }

    void display(int maxTerms = 40) const {
        std::cout << str(maxTerms) << std::endl;
    }

    std::vector<Frac> coeffList(int from, int to) const {
        std::vector<Frac> list;
        for (int n = from; n <= to; ++n)
            list.push_back(coeff(n));
        return list;
    }
};

#endif
