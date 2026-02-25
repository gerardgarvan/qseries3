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

    std::cout << "Task 1 & 2 OK: BigInt +, -, * with correct signed arithmetic\n";
    return 0;
}
