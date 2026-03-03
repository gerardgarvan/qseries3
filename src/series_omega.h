#ifndef SERIES_OMEGA_H
#define SERIES_OMEGA_H

#include "omega3.h"
#include "series.h"
#include <map>
#include <algorithm>
#include <vector>

struct SeriesOmega {
    std::map<int, Omega3> c;
    int trunc;

    static SeriesOmega zero(int T) {
        SeriesOmega s;
        s.trunc = T;
        return s;
    }

    static SeriesOmega one(int T) {
        SeriesOmega s;
        s.c[0] = Omega3(Frac(1), Frac(0));
        s.trunc = T;
        return s;
    }

    static SeriesOmega constant(Omega3 z, int T) {
        SeriesOmega s;
        if (!z.isZero())
            s.c[0] = z;
        s.trunc = T;
        return s;
    }

    static SeriesOmega q(int T) {
        SeriesOmega s;
        s.c[1] = Omega3(Frac(1), Frac(0));
        s.trunc = T;
        return s;
    }

    Omega3 coeff(int n) const {
        auto it = c.find(n);
        if (it == c.end())
            return Omega3(Frac(0), Frac(0));
        return it->second;
    }

    void setCoeff(int n, Omega3 v) {
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

    void clean() {
        std::vector<int> toErase;
        for (const auto& [e, v] : c) {
            if (e >= trunc || v.isZero())
                toErase.push_back(e);
        }
        for (int e : toErase)
            c.erase(e);
    }

    SeriesOmega truncTo(int T) const {
        SeriesOmega s;
        s.trunc = std::min(trunc, T);
        for (const auto& [e, v] : c) {
            if (e < s.trunc && !v.isZero())
                s.c[e] = v;
        }
        return s;
    }

    SeriesOmega operator+(const SeriesOmega& o) const {
        SeriesOmega s;
        s.trunc = std::min(trunc, o.trunc);
        for (const auto& [e, v] : c) {
            if (e < s.trunc)
                s.c[e] = v;
        }
        for (const auto& [e, v] : o.c) {
            if (e < s.trunc) {
                Omega3 sum = s.coeff(e) + v;
                s.setCoeff(e, sum);
            }
        }
        s.clean();
        return s;
    }

    SeriesOmega operator*(const SeriesOmega& o) const {
        SeriesOmega s;
        int t = std::min(trunc, o.trunc);
        s.trunc = t;
        for (const auto& [e1, c1] : c) {
            for (const auto& [e2, c2] : o.c) {
                int exp = e1 + e2;
                if (exp >= t)
                    continue;
                Omega3 prod = c1 * c2;
                if (!prod.isZero()) {
                    Omega3 old = s.coeff(exp);
                    s.setCoeff(exp, old + prod);
                }
            }
        }
        s.clean();
        return s;
    }

    SeriesOmega operator*(const Omega3& z) const {
        if (z.isZero())
            return zero(trunc);
        SeriesOmega s;
        s.trunc = trunc;
        for (const auto& [e, v] : c)
            s.c[e] = z * v;
        return s;
    }

    std::string str(int maxTerms = 30) const {
        std::string oTerm = " + O(q" + Series::expToUnicode(trunc) + ")";
        if (c.empty())
            return "0" + oTerm;
        std::string out;
        int count = 0;
        for (const auto& [exp, coeff] : c) {
            if (count >= maxTerms) break;
            if (!coeff.isZero()) {
                if (count > 0) out += " + ";
                out += coeff.str();
                if (exp != 0)
                    out += (exp == 1) ? "*q" : ("*q" + Series::expToUnicode(exp));
                ++count;
            }
        }
        return out + oTerm;
    }
};

// Omega3 * Series -> SeriesOmega  (and Series * Omega3, commutative)
inline SeriesOmega operator*(const Omega3& z, const Series& s) {
    SeriesOmega so;
    so.trunc = s.trunc;
    for (const auto& [e, v] : s.c) {
        if (e < so.trunc && !v.isZero()) {
            Omega3 ov = Omega3::fromRational(v);
            Omega3 prod = z * ov;
            if (!prod.isZero())
                so.c[e] = prod;
        }
    }
    return so;
}
inline SeriesOmega operator*(const Series& s, const Omega3& z) { return z * s; }

#endif
