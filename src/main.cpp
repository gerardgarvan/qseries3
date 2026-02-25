/*
 * Phase 1 BigInt + Phase 2 Frac + Phase 3 Series test driver.
 * Exercises all SPEC and .cursorrules edge cases.
 * Exits 0 on success, non-zero on failure.
 */
#include "bigint.h"
#include "frac.h"
#include "series.h"
#include "qfuncs.h"
#include <iostream>
#include <string>

static int fail_count = 0;

#define CHECK(cond) do { \
    if (!(cond)) { std::cerr << "FAIL: " << #cond << '\n'; ++fail_count; } \
    else { std::cout << "PASS: " << #cond << '\n'; } \
} while(0)

int main() {
    std::cout << "=== BigInt Phase 1 test driver ===\n\n";

    // --- Group 1: 0 * anything = 0 ---
    std::cout << "--- 0 * anything = 0 ---\n";
    CHECK(BigInt(0) * BigInt(0) == BigInt(0));
    CHECK(BigInt(0) * BigInt(1) == BigInt(0));
    CHECK(BigInt(0) * BigInt(-1) == BigInt(0));
    CHECK(BigInt(0) * BigInt("123456789012345") == BigInt(0));
    CHECK(BigInt("123456789012345") * BigInt(0) == BigInt(0));

    // --- Group 2: (-a) * (-b) = a*b ---
    std::cout << "\n--- (-a) * (-b) = a*b ---\n";
    CHECK((-BigInt(7)) * (-BigInt(3)) == BigInt(21));
    CHECK((-BigInt("999999999")) * (-BigInt("999999999")) == BigInt("999999998000000001"));

    // --- Group 3: Division cases ---
    std::cout << "\n--- Division: 1000000000/1, 999999999/1000000000, 123456789012345/123 ---\n";
    auto [q1, r1] = BigInt::divmod(BigInt("1000000000"), BigInt("1"));
    CHECK(q1 == BigInt("1000000000") && r1 == BigInt(0));
    auto [q2, r2] = BigInt::divmod(BigInt("999999999"), BigInt("1000000000"));
    CHECK(q2 == BigInt(0) && r2 == BigInt("999999999"));
    auto [q3, r3] = BigInt::divmod(BigInt("123456789012345"), BigInt("123"));
    CHECK(q3 * BigInt("123") + r3 == BigInt("123456789012345"));
    CHECK(BigInt(0) <= r3 && r3 < BigInt("123"));

    // --- Group 4: Negatives — (-7)/3, 7/(-3), remainder sign = dividend sign ---
    std::cout << "\n--- Negatives: (-7)/3, 7/(-3), remainder sign = dividend ---\n";
    auto [qd4, rd4] = BigInt::divmod(BigInt(-7), BigInt(3));
    CHECK(qd4 == BigInt(-2) && rd4 == BigInt(-1));  // -7 = -2*3 + (-1)
    auto [qd5, rd5] = BigInt::divmod(BigInt(7), BigInt(-3));
    CHECK(qd5 == BigInt(-2) && rd5 == BigInt(1));   // 7 = -2*(-3) + 1
    CHECK((BigInt(-7) / BigInt(3)) == BigInt(-2));
    CHECK((BigInt(7) / BigInt(-3)) == BigInt(-2));
    CHECK((BigInt(-7) % BigInt(3)) == BigInt(-1));
    CHECK((BigInt(7) % BigInt(-3)) == BigInt(1));

    // --- Group 5: GCD ---
    std::cout << "\n--- GCD: gcd(48,18)==6, gcd(0,x)=|x|, gcd with negatives ---\n";
    CHECK(bigGcd(BigInt(48), BigInt(18)) == BigInt(6));
    CHECK(bigGcd(BigInt(0), BigInt(7)) == BigInt(7));
    CHECK(bigGcd(BigInt(0), BigInt(-7)) == BigInt(7));
    CHECK(bigGcd(BigInt(-12), BigInt(8)) == BigInt(4));

    // --- Group 6: Division by zero must throw ---
    std::cout << "\n--- Division by zero: must throw ---\n";
    {
        bool threw = false;
        try { BigInt(1) / BigInt(0); } catch (const std::invalid_argument&) { threw = true; }
        CHECK(threw);
    }
    {
        bool threw = false;
        try { BigInt::divmod(BigInt(1), BigInt(0)); } catch (const std::invalid_argument&) { threw = true; }
        CHECK(threw);
    }

    // --- Group 7: Invalid string must throw ---
    std::cout << "\n--- Invalid string: BigInt(\"abc\"), BigInt(\"12x3\") must throw ---\n";
    {
        bool threw = false;
        try { (void)BigInt("abc"); } catch (const std::invalid_argument&) { threw = true; }
        CHECK(threw);
    }
    {
        bool threw = false;
        try { (void)BigInt("12x3"); } catch (const std::invalid_argument&) { threw = true; }
        CHECK(threw);
    }

    // --- Basic sanity (from plans 01-01, 01-02) ---
    std::cout << "\n--- Basic sanity ---\n";
    CHECK(BigInt("0").str() == "0");
    CHECK(BigInt(-42).str() == "-42");
    CHECK(BigInt(48) / BigInt(18) == BigInt(2));
    CHECK(BigInt(48) % BigInt(18) == BigInt(12));

    std::cout << "\n=== Frac Phase 2 test driver ===\n\n";

    // --- Frac: SPEC reduction ---
    std::cout << "--- Frac SPEC reduction: 6/4→3/2, 0/5→0/1 ---\n";
    CHECK(Frac(6, 4).str() == "3/2");
    CHECK(Frac(0, 5).str() == "0");
    CHECK(Frac(0, 5).num == BigInt(0) && Frac(0, 5).den == BigInt(1));

    // --- Frac: Zero denominator throws ---
    std::cout << "\n--- Frac: zero denominator must throw ---\n";
    {
        bool threw = false;
        try { (void)Frac(1, 0); } catch (const std::invalid_argument&) { threw = true; }
        CHECK(threw);
    }
    {
        bool threw = false;
        try { (void)(Frac(1, 2) / Frac(0)); } catch (const std::invalid_argument&) { threw = true; }
        CHECK(threw);
    }

    // --- Frac: add/sub/mul/div ---
    std::cout << "\n--- Frac arithmetic: add/sub/mul/div ---\n";
    CHECK((Frac(1, 2) + Frac(1, 3)).str() == "5/6");
    CHECK((Frac(1, 2) - Frac(1, 3)).str() == "1/6");
    CHECK((Frac(1, 2) * Frac(2, 3)).str() == "1/3");
    CHECK((Frac(1, 2) / Frac(2, 3)).str() == "3/4");
    CHECK((Frac(-1, 2) + Frac(1, 2)) == Frac(0));
    CHECK((Frac(-1, 2) * Frac(2, 1)) == Frac(-1));

    // --- Frac: Sign handling ---
    std::cout << "\n--- Frac sign handling ---\n";
    CHECK(Frac(-3, 4).str() == "-3/4");
    CHECK(Frac(3, -4).str() == "-3/4");
    CHECK(Frac(-1, 2) < Frac(1, 2));
    CHECK(Frac(1, 3) < Frac(1, 2));

    // --- Frac: str() integer form ---
    std::cout << "\n--- Frac str() integer form ---\n";
    CHECK(Frac(7).str() == "7");
    CHECK(Frac(0).str() == "0");

    std::cout << "\n=== Series Phase 3 test driver ===\n\n";

    // --- Task 1: SPEC inverse, minExp>0 inverse, invalid inverse throws ---
    std::cout << "--- Series SPEC: (1-q)*(1/(1-q))=1 ---\n";
    {
        auto one_q = Series::one(20) - Series::q(20);
        auto inv = one_q.inverse();
        auto prod = one_q * inv;
        CHECK(prod.coeff(0) == Frac(1));
        for (int i = 1; i < 20; i++)
            CHECK(prod.coeff(i) == Frac(0));
    }
    std::cout << "--- Series: inverse of q*(1-q) (minExp>0) ---\n";
    {
        auto f = Series::q(20) * (Series::one(20) - Series::q(20));
        auto g = f.inverse();
        // 1/(q*(1-q)) = q^{-1} * 1/(1-q) = q^{-1} * (1+q+q²+...), so coeff at -1 is 1, at 0 is 1, at 1 is 1, etc.
        CHECK(g.coeff(-1) == Frac(1));
        CHECK(g.coeff(0) == Frac(1));
        CHECK(g.coeff(1) == Frac(1));
    }
    std::cout << "--- Series: invalid inverse throws ---\n";
    {
        bool threw = false;
        try { (void)Series::zero(10).inverse(); } catch (const std::invalid_argument&) { threw = true; }
        CHECK(threw);
    }
    // Series::q(10).inverse() succeeds (minExp>0 handled via shift: 1/q = q^{-1})

    // --- Task 2: add/sub/mul/div, truncation ---
    std::cout << "\n--- Series add/sub ---\n";
    {
        auto a = Series::one(10) + Series::q(10);
        auto b = Series::one(10) - Series::q(10);
        auto sum = a + b;
        CHECK(sum.coeff(0) == Frac(2));
        CHECK(sum.coeff(1) == Frac(0));
    }
    {
        auto f = Series::one(10) + Series::q(10) + Series::qpow(2, 10);
        auto g = f - Series::one(10);
        CHECK(g.coeff(0) == Frac(0));
        CHECK(g.coeff(1) == Frac(1));
        CHECK(g.coeff(2) == Frac(1));
    }
    std::cout << "--- Series truncation propagation ---\n";
    {
        auto a = Series::one(5) + Series::q(5);
        auto b = Series::one(3) + Series::q(3);
        auto sum = a + b;
        CHECK(sum.trunc == 3);
    }
    std::cout << "--- Series mul ---\n";
    {
        auto a = Series::one(10) - Series::q(10);
        auto b = Series::one(10) + Series::q(10);
        auto prod = a * b;
        CHECK(prod.coeff(0) == Frac(1));
        CHECK(prod.coeff(1) == Frac(0));
        CHECK(prod.coeff(2) == Frac(-1));
    }
    {
        auto f = Series::one(10) + Series::q(10);
        auto sq = f * f;
        CHECK(sq.coeff(0) == Frac(1));
        CHECK(sq.coeff(1) == Frac(2));
        CHECK(sq.coeff(2) == Frac(1));
    }
    std::cout << "--- Series div ---\n";
    {
        auto num = Series::one(10) - Series::qpow(2, 10);
        auto den = Series::one(10) - Series::q(10);
        auto quot = num / den;
        CHECK(quot.coeff(0) == Frac(1));
        CHECK(quot.coeff(1) == Frac(1));
    }
    std::cout << "--- Series: division by non-invertible throws ---\n";
    {
        bool threw = false;
        try { (void)(Series::one(10) / Series::zero(10)); } catch (const std::invalid_argument&) { threw = true; }
        CHECK(threw);
    }

    // --- Task 3: subs_q, pow, str ---
    std::cout << "\n--- Series subs_q ---\n";
    {
        auto f = Series::one(10) + Series::q(10) + Series::qpow(2, 10);
        auto g = f.subs_q(2);
        CHECK(g.coeff(0) == Frac(1));
        CHECK(g.coeff(2) == Frac(1));
        CHECK(g.coeff(4) == Frac(1));
        CHECK(g.trunc == 20);
    }
    {
        auto f = Series::one(10) + Series::q(10) + Series::qpow(2, 10);
        auto g = f.subs_q(0);
        CHECK(g.coeff(0) == Frac(3));
        CHECK(g.c.size() <= 1u);
    }
    std::cout << "--- Series pow ---\n";
    {
        auto f = Series::one(10) + Series::q(10);
        auto sq = f.pow(2);
        CHECK(sq.coeff(0) == Frac(1));
        CHECK(sq.coeff(1) == Frac(2));
        CHECK(sq.coeff(2) == Frac(1));
    }
    {
        auto f = Series::one(10) + Series::q(10);
        CHECK(f.pow(0).coeff(0) == Frac(1));
    }
    {
        auto one_q = Series::one(20) - Series::q(20);
        auto inv_one_q = one_q.inverse();
        auto back = inv_one_q.pow(-1);
        CHECK(back.coeff(0) == one_q.coeff(0));
        CHECK(back.coeff(1) == one_q.coeff(1));
    }
    std::cout << "--- qfuncs Phase 4 (04-01 verification) ---\n";
    {
        auto d = divisors(12);
        CHECK(d.size() == 6u && d[0] == 1 && d[1] == 2 && d[2] == 3 && d[3] == 4 && d[4] == 6 && d[5] == 12);
        CHECK(mobius(6) == 1);
        CHECK(sigma(6) == 12);
        CHECK(euler_phi(6) == 2);
    }
    {
        auto q = Series::q(25);
        auto et = etaq(q, 1, 25);
        // Euler pentagonal: coeff ±1 at 0,1,2,5,7,12,15
        CHECK(et.coeff(0) == Frac(1));
        CHECK(et.coeff(1) == Frac(-1));
        CHECK(et.coeff(2) == Frac(-1));
        CHECK(et.coeff(5) == Frac(1));
        CHECK(et.coeff(7) == Frac(1));
        CHECK(et.coeff(12) == Frac(-1));
    }
    {
        auto q = Series::q(25);
        auto ap = aqprod(q, q, 5, 25);
        // (1-q)(1-q²)(1-q³)(1-q⁴)(1-q⁵): coeff 0..5
        CHECK(ap.coeff(0) == Frac(1));
        CHECK(ap.coeff(1) == Frac(-1));
        CHECK(ap.coeff(2) == Frac(-1));
    }
    {
        auto q = Series::q(25);
        CHECK(aqprod(q, q, 0, 25).coeff(0) == Frac(1));
        CHECK(qbin(q, 0, 5, 25).coeff(0) == Frac(1));
        CHECK(qbin(q, 5, 5, 25).coeff(0) == Frac(1));
        auto qb = qbin(q, 1, 3, 25);
        CHECK(qb.coeff(0) == Frac(1) && qb.coeff(1) == Frac(1) && qb.coeff(2) == Frac(1));
    }
    std::cout << "--- qfuncs theta (04-02 verification) ---\n";
    {
        auto q = Series::q(50);
        auto t2 = theta2(q, 50);
        CHECK(t2.coeff(0) == Frac(2));
        CHECK(t2.coeff(2) == Frac(2));
        auto t3 = theta3(q, 50);
        CHECK(t3.coeff(0) == Frac(1));
        CHECK(t3.coeff(1) == Frac(2));
        CHECK(t3.coeff(4) == Frac(2));
        CHECK(t3.coeff(9) == Frac(2));
        auto t4 = theta4(q, 50);
        CHECK(t4.coeff(0) == Frac(1));
        CHECK(t4.coeff(1) == Frac(-2));
        CHECK(t4.coeff(4) == Frac(2));
    }
    {
        auto q = Series::q(25);
        auto z = Series::constant(Frac(-1), 25);
        auto th = theta(z, q, 25);
        CHECK(th.coeff(0) == Frac(1));
        CHECK(th.coeff(1) == Frac(-2));
        CHECK(th.coeff(4) == Frac(2));
    }

    std::cout << "--- Series str ---\n";
    {
        std::string s = Series::one(5).str();
        CHECK(s.find("1") != std::string::npos);
        CHECK(s.find("O(q^") != std::string::npos);
    }
    {
        std::string s = Series::q(5).str();
        CHECK(s.find("q") != std::string::npos);
    }

    // --- Frac: Long-chain growth test (no exponential BigInt growth) ---
    std::cout << "\n--- Frac long-chain: add chain 1+50*(1/2)=26 ---\n";
    {
        Frac f(1, 1);
        for (int i = 0; i < 50; i++) f = f + Frac(1, 2);
        CHECK(f.den == BigInt(1) && f.num == BigInt(26));
    }
    std::cout << "\n--- Frac long-chain: mul chain (3/2)^20 reduced ---\n";
    {
        Frac f(1, 1);
        for (int i = 0; i < 20; i++) f = f * Frac(3, 2);
        // Reduced form: (3/2)^20 = 3^20/2^20, gcd(3^20,2^20)=1 so den=2^20
        BigInt g = bigGcd(f.num.abs(), f.den);
        CHECK(g == BigInt(1));
        CHECK(f.den > BigInt(0));
    }

    std::cout << "\n=== " << (fail_count == 0 ? "All PASS" : "FAILURES") << " (fail_count=" << fail_count << ") ===\n";
    return fail_count;
}
