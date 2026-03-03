#ifndef SERIES_H
#define SERIES_H

#include "frac.h"
#include <map>
#include <string>
#include <utility>
#include <vector>
#include <stdexcept>
#include <iostream>
#include <algorithm>

struct Series {
    std::map<int, Frac> c;
    int trunc;
    Frac q_shift;  // rational q-power offset: series represents q^{q_shift} * Σ c[n] q^n

    // Constructors / factories
    static Series zero(int T) {
        Series s;
        s.trunc = T;
        s.q_shift = Frac(0);
        return s;
    }

    static Series one(int T) {
        Series s;
        s.c[0] = Frac(1);
        s.trunc = T;
        s.q_shift = Frac(0);
        return s;
    }

    static Series q(int T) {
        Series s;
        s.c[1] = Frac(1);
        s.trunc = T;
        s.q_shift = Frac(0);
        return s;
    }

    static Series qpow(int e, int T) {
        Series s;
        if (e >= 0 && e < T) {
            s.c[e] = Frac(1);
        }
        s.trunc = T;
        s.q_shift = Frac(0);
        return s;
    }

    static Series constant(Frac f, int T) {
        Series s;
        if (!f.isZero()) {
            s.c[0] = f;
        }
        s.trunc = T;
        s.q_shift = Frac(0);
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

    // Absorb integer part of q_shift into coefficient indices, keeping q_shift in [0, 1)
    void normalize_q_shift() {
        if (q_shift.isZero()) return;

        auto [quot, rem] = BigInt::divmod(q_shift.num, q_shift.den);
        if (!rem.isZero() && rem.neg)
            quot = quot - BigInt(1);

        if (quot.isZero()) return;

        int F = quot.d.empty() ? 0 : static_cast<int>(quot.d[0]);
        if (quot.neg) F = -F;

        std::map<int, Frac> new_c;
        for (const auto& [e, v] : c)
            new_c[e + F] = v;
        c = std::move(new_c);

        trunc += F;
        q_shift = q_shift - Frac(BigInt(quot), BigInt(1));

        if (trunc <= 0) {
            c.clear();
            trunc = 0;
        }

        clean();
    }

    Series truncTo(int T) const {
        Series s;
        s.trunc = std::min(trunc, T);
        s.q_shift = q_shift;
        for (const auto& [e, v] : c) {
            if (e < s.trunc && !v.isZero())
                s.c[e] = v;
        }
        return s;
    }

    Series withTrunc(int T) const {
        Series s;
        s.trunc = T;
        s.q_shift = q_shift;
        for (const auto& [e, v] : c) {
            if (e < T && !v.isZero())
                s.c[e] = v;
        }
        return s;
    }

    // Arithmetic
    Series operator-() const {
        Series s;
        s.trunc = trunc;
        s.q_shift = q_shift;
        for (const auto& [e, v] : c)
            s.c[e] = -v;
        return s;
    }

    Series operator+(const Series& o) const {
        if (!(q_shift == o.q_shift) && !c.empty() && !o.c.empty())
            throw std::runtime_error("cannot add series with different q-shifts");
        Series s;
        s.trunc = std::min(trunc, o.trunc);
        s.q_shift = c.empty() ? o.q_shift : q_shift;
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
        s.q_shift = q_shift + o.q_shift;
        for (const auto& [e1, c1] : c) {
            for (const auto& [e2, c2] : o.c) {
                int exp = e1 + e2;
                if (exp >= t)
                    break;
                Frac prod = c1 * c2;
                if (!prod.isZero()) {
                    Frac old = s.coeff(exp);
                    s.setCoeff(exp, old + prod);
                }
            }
        }
        s.clean();
        s.normalize_q_shift();
        return s;
    }

    Series operator*(const Frac& f) const {
        if (f.isZero())
            return zero(trunc);
        Series s;
        s.trunc = trunc;
        s.q_shift = q_shift;
        for (const auto& [e, v] : c)
            s.c[e] = v * f;
        return s;
    }

    Series inverse() const {
        int m = minExp();
        if (m != 0) {
            Series h;
            h.trunc = trunc - m;
            h.q_shift = Frac(0);
            if (h.trunc <= 0) h.trunc = 1;
            for (const auto& [e, v] : c) {
                int newExp = e - m;
                if (newExp >= 0 && newExp < h.trunc)
                    h.c[newExp] = v;
            }
            if (h.c.empty() || h.coeff(0).isZero())
                throw std::invalid_argument("Series::inverse: constant term zero");
            Series invH = h.inverse();
            Series result;
            result.trunc = trunc;
            result.q_shift = Frac(0) - q_shift;
            for (const auto& [e, v] : invH.c) {
                int newExp = e - m;
                if (newExp >= -invH.trunc)
                    result.c[newExp] = v;
            }
            result.normalize_q_shift();
            return result;
        }
        Frac c0 = coeff(0);
        if (c0.isZero())
            throw std::invalid_argument("Series::inverse: constant term zero");
        Series g;
        g.trunc = trunc;
        g.q_shift = Frac(0) - q_shift;
        g.c[0] = Frac(1) / c0;
        for (int n = 1; n < trunc; ++n) {
            Frac sum(0);
            // j MUST start at 1 — j=0 would include c₀*g[n] (circular)
            for (int j = 1; j <= n; ++j) {
                sum = sum + coeff(j) * g.coeff(n - j);
            }
            g.c[n] = -(Frac(1) / c0) * sum;
        }
        g.normalize_q_shift();
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

    Series powFrac(Frac alpha) const {
        // Integer shortcut
        if (alpha.den == BigInt(1)) {
            int64_t p64 = 0;
            if (!alpha.num.d.empty()) p64 = static_cast<int64_t>(alpha.num.d[0]);
            if (alpha.num.neg) p64 = -p64;
            return pow(static_cast<int>(p64));
        }

        // Empty series: 0^alpha
        if (c.empty()) {
            if (alpha > Frac(0))
                return zero(trunc);
            throw std::runtime_error("powFrac: 0 to non-positive power");
        }

        // Extract leading q-power
        int k = minExp();
        Series f;
        f.trunc = trunc - k;
        f.q_shift = Frac(0);
        for (const auto& [e, v] : c) {
            int ne = e - k;
            if (ne >= 0 && ne < f.trunc)
                f.c[ne] = v;
        }

        Frac c0 = f.coeff(0);
        if (c0.isZero())
            throw std::runtime_error("powFrac: zero constant term after shift");

        // scale = c0^alpha (must be exact rational)
        Frac scale = c0.rational_pow(alpha);

        // Build normalized g: g[j] = f[j]/c0 for j >= 1
        int T = f.trunc;
        std::vector<Frac> g(T);
        for (int j = 1; j < T; ++j)
            g[j] = f.coeff(j) / c0;

        // Coefficient recurrence: h = (1 + g)^alpha
        std::vector<Frac> h(T);
        h[0] = Frac(1);
        for (int m = 1; m < T; ++m) {
            Frac sum(0);
            for (int j = 1; j < m; ++j) {
                Frac coef = (alpha + Frac(1)) * Frac(j) - Frac(m);
                sum = sum + coef * g[j] * h[m - j];
            }
            h[m] = alpha * g[m] + sum / Frac(m);
        }

        // Build result with scale and q_shift
        Series result;
        result.trunc = T;
        for (int m = 0; m < T; ++m) {
            Frac val = scale * h[m];
            if (!val.isZero())
                result.c[m] = val;
        }
        result.q_shift = (q_shift + Frac(k)) * alpha;
        result.normalize_q_shift();
        return result;
    }

    Series modp(int p) const {
        if (p <= 0) throw std::runtime_error("modp: prime must be positive");
        Series s;
        s.trunc = trunc;
        s.q_shift = q_shift;
        for (const auto& [e, v] : c) {
            if (v.den != BigInt(1))
                throw std::runtime_error("modp: coefficient at q^" + std::to_string(e) + " is not an integer (" + v.str() + ")");
            auto [q_bi, r] = BigInt::divmod(v.num, BigInt(p));
            int64_t rem = r.d.empty() ? 0 : static_cast<int64_t>(r.d[0]);
            if (r.neg) rem = -rem;
            rem = ((rem % p) + p) % p;
            if (rem != 0)
                s.c[e] = Frac(static_cast<int64_t>(rem));
        }
        return s;
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
        s.q_shift = q_shift * Frac(k);
        for (const auto& [e, v] : c) {
            int newExp = e * k;
            if (newExp < s.trunc)
                s.c[newExp] = v;
        }
        s.clean();
        s.normalize_q_shift();
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

    static std::string fracExpStr(const Frac& f) {
        if (f.den == BigInt(1) && f.num.d.size() <= 1) {
            int v = f.num.d.empty() ? 0 : static_cast<int>(f.num.d[0]);
            if (f.num.neg) v = -v;
            return expToUnicode(v);
        }
        return "^(" + f.str() + ")";
    }

    std::string str(int maxTerms = 30) const {
        std::string oTerm = " + O(q" + expToUnicode(trunc) + ")";

        if (q_shift.isZero()) {
            if (c.empty())
                return "0" + oTerm;
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
                bool showNeg = (count == 0 && v < Frac(0));
                if (e == 0) {
                    if (showNeg) out += "-";
                    out += av.isOne() ? "1" : av.str();
                } else {
                    if (av.isOne()) {
                        if (showNeg) out += "-";
                    } else {
                        if (showNeg) out += "-";
                        out += av.str();
                    }
                    out += (e == 1) ? "q" : ("q" + expToUnicode(e));
                }
                ++count;
            }
            out += oTerm;
            return out;
        }

        // Fractional q_shift: expanded term display
        if (c.empty())
            return "0" + oTerm;
        std::string out;
        int count = 0;
        for (const auto& [e, v] : c) {
            if (count >= maxTerms) break;
            Frac actual_exp = Frac(e) + q_shift;
            if (count > 0) {
                out += (v < Frac(0)) ? " - " : " + ";
            }
            Frac av = v.abs();
            bool showNeg = (count == 0 && v < Frac(0));
            if (actual_exp.isZero()) {
                if (showNeg) out += "-";
                out += av.isOne() ? "1" : av.str();
            } else {
                if (av.isOne()) {
                    if (showNeg) out += "-";
                } else {
                    if (showNeg) out += "-";
                    out += av.str();
                }
                if (actual_exp.den == BigInt(1)) {
                    int iexp = actual_exp.num.d.empty() ? 0 : static_cast<int>(actual_exp.num.d[0]);
                    if (actual_exp.num.neg) iexp = -iexp;
                    out += (iexp == 1) ? "q" : ("q" + expToUnicode(iexp));
                } else {
                    out += "q^(" + actual_exp.str() + ")";
                }
            }
            ++count;
        }
        out += oTerm;
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

// BivariateSeries: map (z_exp, q_exp) -> Frac for Laurent in z, power series in q
struct BivariateSeries {
    std::map<std::pair<int,int>, Frac> c;  // (z_exp, q_exp) -> coefficient
    int trunc;  // q truncation
};

#endif
