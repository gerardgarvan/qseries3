#include "bigint.h"
#include <iostream>
#include <cassert>

int main() {
    // Task 1 verification
    assert(BigInt("0").str() == "0");
    assert(BigInt("-42").str() == "-42");
    assert(BigInt(123456789).str() == "123456789");
    assert(BigInt(0).str() == "0");
    assert(BigInt(-1).str() == "-1");

    BigInt a = BigInt("0");
    BigInt b = BigInt(-42);
    assert(a == BigInt(0));
    assert(b < BigInt(0));
    assert(BigInt(5) > BigInt(3));
    assert(BigInt(-5) < BigInt(-3));

    // Task 2 verification
    BigInt p = BigInt("999999999") * BigInt("999999999");
    assert(p.str() == "999999998000000001");
    assert((-BigInt(7)) * (-BigInt(3)) == BigInt(21));
    assert(BigInt(0) * BigInt("123456789012345") == BigInt(0));
    assert(BigInt(100) + BigInt(-30) == BigInt(70));
    assert(BigInt(100) - BigInt(30) == BigInt(70));

    // Plan 01-02: divmod verification
    auto [q1, r1] = BigInt::divmod(BigInt("1000000000"), BigInt("1"));
    assert(q1 == BigInt("1000000000") && r1 == BigInt(0));
    auto [q2, r2] = BigInt::divmod(BigInt("999999999"), BigInt("1000000000"));
    assert(q2 == BigInt(0) && r2 == BigInt("999999999"));
    auto [q3, r3] = BigInt::divmod(BigInt("123456789012345"), BigInt("123"));
    assert(q3 * BigInt("123") + r3 == BigInt("123456789012345"));
    assert(BigInt(0) <= r3 && r3 < BigInt("123"));

    // Plan 01-02 Task 2: operator/, operator%, bigGcd
    assert(BigInt(48) / BigInt(18) == BigInt(2));
    assert(BigInt(48) % BigInt(18) == BigInt(12));
    assert(bigGcd(BigInt(48), BigInt(18)) == BigInt(6));
    assert(bigGcd(BigInt(0), BigInt(7)) == BigInt(7));
    assert(bigGcd(BigInt(-12), BigInt(8)) == BigInt(4));

    bool threw = false;
    try { BigInt(1) / BigInt(0); } catch (const std::invalid_argument&) { threw = true; }
    assert(threw);

    std::cout << "Task 1 & 2 OK: BigInt +, -, * with correct signed arithmetic\n";
    std::cout << "Plan 01-02: divmod, /, %, bigGcd OK\n";
    return 0;
}
