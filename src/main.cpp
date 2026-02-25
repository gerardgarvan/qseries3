/*
 * Phase 1 BigInt + Phase 2 Frac test driver.
 * Exercises all SPEC and .cursorrules edge cases.
 * Exits 0 on success, non-zero on failure.
 */
#include "bigint.h"
#include "frac.h"
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
