#ifndef BIGINT_H
#define BIGINT_H

#include <cstdint>
#include <climits>
#include <string>
#include <vector>
#include <stdexcept>
#include <cctype>

struct BigInt {
    static constexpr uint32_t BASE = 1000000000;

    bool neg = false;
    std::vector<uint32_t> d;  // digits base 10^9, d[0] = least significant

    BigInt() : neg(false), d(1, 0) {}

    BigInt(int64_t v) : neg(v < 0) {
        uint64_t u;
        if (v == INT64_MIN) {
            u = static_cast<uint64_t>(INT64_MAX) + 1;
        } else {
            u = static_cast<uint64_t>(v < 0 ? -v : v);
        }
        if (u == 0) {
            d = {0};
            neg = false;
            return;
        }
        d.clear();
        while (u > 0) {
            d.push_back(static_cast<uint32_t>(u % BASE));
            u /= BASE;
        }
    }

    BigInt(const std::string& s) {
        size_t i = 0;
        while (i < s.size() && std::isspace(static_cast<unsigned char>(s[i])))
            ++i;
        if (i >= s.size())
            throw std::invalid_argument("BigInt: empty or whitespace-only string");
        neg = false;
        if (s[i] == '-') {
            neg = true;
            ++i;
        } else if (s[i] == '+') {
            ++i;
        }
        if (i >= s.size())
            throw std::invalid_argument("BigInt: string ends after sign");
        for (; i < s.size() && s[i] == '0'; ++i) {}
        if (i >= s.size()) {
            d = {0};
            neg = false;
            return;
        }
        for (size_t j = i; j < s.size(); ++j) {
            if (!std::isdigit(static_cast<unsigned char>(s[j])))
                throw std::invalid_argument("BigInt: invalid character in decimal string");
        }
        d.clear();
        size_t end = s.size();
        while (end > i) {
            size_t start = (end >= 9 && end - 9 >= i) ? end - 9 : i;
            uint32_t val = 0;
            for (size_t k = start; k < end; ++k)
                val = val * 10 + static_cast<uint32_t>(s[k] - '0');
            d.push_back(val);
            end = start;
        }
        if (d.empty())
            d = {0};
        normalize();
        if (isZero())
            neg = false;
    }

    bool isZero() const {
        return d.size() == 1 && d[0] == 0;
    }

    BigInt abs() const {
        BigInt r = *this;
        r.neg = false;
        return r;
    }

    void normalize() {
        while (d.size() > 1 && d.back() == 0)
            d.pop_back();
        if (d.size() == 1 && d[0] == 0)
            neg = false;
    }

    static int cmpAbs(const BigInt& a, const BigInt& b) {
        if (a.d.size() != b.d.size())
            return a.d.size() < b.d.size() ? -1 : 1;
        for (int i = static_cast<int>(a.d.size()) - 1; i >= 0; --i) {
            if (a.d[i] != b.d[i])
                return a.d[i] < b.d[i] ? -1 : 1;
        }
        return 0;
    }

    bool operator==(const BigInt& o) const {
        if (neg != o.neg) return false;
        if (d.size() != o.d.size()) return false;
        for (size_t i = 0; i < d.size(); ++i)
            if (d[i] != o.d[i]) return false;
        return true;
    }

    bool operator!=(const BigInt& o) const { return !(*this == o); }

    bool operator<(const BigInt& o) const {
        if (neg != o.neg) return neg;
        int c = cmpAbs(*this, o);
        if (c == 0) return false;
        if (neg) return c > 0;
        return c < 0;
    }

    bool operator>(const BigInt& o) const { return o < *this; }

    bool operator<=(const BigInt& o) const { return !(o < *this); }

    bool operator>=(const BigInt& o) const { return !(*this < o); }

    std::string str() const {
        if (d.empty()) return "0";
        std::string s;
        s += std::to_string(d.back());
        for (int i = static_cast<int>(d.size()) - 2; i >= 0; --i) {
            std::string t = std::to_string(d[i]);
            s += std::string(9 - static_cast<int>(t.size()), '0') + t;
        }
        if (neg) s = "-" + s;
        return s;
    }

    static BigInt addAbs(const BigInt& a, const BigInt& b) {
        BigInt r;
        r.neg = false;
        r.d.clear();
        uint64_t carry = 0;
        size_t n = (a.d.size() > b.d.size()) ? a.d.size() : b.d.size();
        for (size_t i = 0; i < n || carry; ++i) {
            uint64_t cur = carry;
            if (i < a.d.size()) cur += a.d[i];
            if (i < b.d.size()) cur += b.d[i];
            r.d.push_back(static_cast<uint32_t>(cur % BASE));
            carry = cur / BASE;
        }
        r.normalize();
        return r;
    }

    static BigInt subAbs(const BigInt& a, const BigInt& b) {
        BigInt r;
        r.neg = false;
        r.d = a.d;
        uint64_t borrow = 0;
        for (size_t i = 0; i < r.d.size(); ++i) {
            int64_t cur = static_cast<int64_t>(r.d[i]) - borrow;
            if (i < b.d.size())
                cur -= static_cast<int64_t>(b.d[i]);
            borrow = 0;
            if (cur < 0) {
                cur += static_cast<int64_t>(BASE);
                borrow = 1;
            }
            r.d[i] = static_cast<uint32_t>(cur);
        }
        r.normalize();
        return r;
    }

    BigInt operator-() const {
        BigInt r = *this;
        if (!isZero()) r.neg = !r.neg;
        return r;
    }

    BigInt operator+(const BigInt& o) const {
        if (isZero()) return o;
        if (o.isZero()) return *this;
        if (neg == o.neg)
            return addAbs(*this, o).withSign(neg);
        int c = cmpAbs(*this, o);
        if (c == 0) return BigInt();
        if (c > 0)
            return subAbs(*this, o).withSign(neg);
        return subAbs(o, *this).withSign(o.neg);
    }

    BigInt operator-(const BigInt& o) const {
        return *this + (-o);
    }

    BigInt operator*(const BigInt& o) const {
        if (isZero() || o.isZero()) return BigInt();
        BigInt r;
        r.neg = (neg != o.neg);
        r.d.assign(d.size() + o.d.size(), 0);
        for (size_t i = 0; i < d.size(); ++i) {
            uint64_t carry = 0;
            for (size_t j = 0; j < o.d.size() || carry; ++j) {
                uint64_t cur = r.d[i + j] + carry;
                if (j < o.d.size())
                    cur += d[i] * 1ULL * o.d[j];
                r.d[i + j] = static_cast<uint32_t>(cur % BASE);
                carry = cur / BASE;
            }
        }
        r.normalize();
        return r;
    }

private:
    BigInt withSign(bool n) const {
        BigInt r = *this;
        r.neg = n;
        return r;
    }
};

#endif
