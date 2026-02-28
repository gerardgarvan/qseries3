/*
 * Phase 1 BigInt + Phase 2 Frac + Phase 3 Series test driver.
 * Exercises all SPEC and .cursorrules edge cases.
 * Exits 0 on success, non-zero on failure.
 */
#include "bigint.h"
#include "frac.h"
#include "series.h"
#include "qfuncs.h"
#include "convert.h"
#include "linalg.h"
#include "relations.h"
#include "parser.h"
#include "repl.h"
#include <iostream>
#include <string>

static int fail_count = 0;

#define CHECK(cond) do { \
    if (!(cond)) { std::cerr << "FAIL: " << #cond << '\n'; ++fail_count; } \
    else { std::cout << "PASS: " << #cond << '\n'; } \
} while(0)

int runUnitTests() {
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
        CHECK(legendre(2, 7) == 1);
        CHECK(legendre(3, 7) == -1);
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
        CHECK(et.coeff(15) == Frac(-1));
    }
    {
        // TEST-02 Partition: p = 1/etaq(q,1,50); coeffs 1,1,2,3,5,7,11,15,22,30,42
        auto q = Series::q(55);
        auto eta = etaq(q, 1, 55);
        auto p = eta.inverse();
        CHECK(p.coeff(0) == Frac(1));
        CHECK(p.coeff(1) == Frac(1));
        CHECK(p.coeff(2) == Frac(2));
        CHECK(p.coeff(3) == Frac(3));
        CHECK(p.coeff(4) == Frac(5));
        CHECK(p.coeff(5) == Frac(7));
        CHECK(p.coeff(6) == Frac(11));
        CHECK(p.coeff(7) == Frac(15));
        CHECK(p.coeff(8) == Frac(22));
        CHECK(p.coeff(9) == Frac(30));
        CHECK(p.coeff(10) == Frac(42));
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
    {
        auto q = Series::q(25);
        auto et2 = etaq(q, 2, 25);
        CHECK(et2.coeff(0) == Frac(1));
        CHECK(et2.coeff(2) == Frac(-1));
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
    std::cout << "--- qfuncs tripleprod, quinprod, winquist (04-03) ---\n";
    {
        auto q = Series::q(30);
        auto z = Series::q(30);
        auto q3 = Series::q(30).subs_q(3);
        auto tp = tripleprod(z, q3, 15);
        CHECK(tp.coeff(0) == Frac(1));
    }
    {
        auto q = Series::q(25);
        auto z = Series::q(25);
        auto q5 = q.subs_q(5);
        auto qp = quinprod(z, q5, 15);
        CHECK(qp.coeff(0) == Frac(1));
    }
    {
        auto q = Series::q(50);
        auto a = Series::constant(Frac(1), 50);
        auto b = Series::constant(Frac(1), 50);
        auto w = winquist(a, b, q, 30);
        CHECK(w.trunc == 30);
    }

    std::cout << "--- Phase 6: sift and etamake (06-01) ---\n";
    {
        auto q = Series::q(50);
        auto et = etaq(q, 1, 50);
        auto s = sift(et, 5, 0, 49);
        CHECK(s.coeff(0) == et.coeff(0));
        CHECK(s.trunc == 10);  // floor((49-0)/5)+1 = 10
    }
    std::cout << "--- Phase 6: TEST-03 etamake theta3/theta4 ---\n";
    {
        auto q = Series::q(100);
        auto t3 = theta3(q, 100);
        auto e3 = etamake(t3, 100);
        CHECK(!e3.empty());
        std::map<int, Frac> m3;
        for (const auto& p : e3) m3[p.first] = p.second;
        // theta3 → η(2τ)^5/(η(4τ)^2 η(τ)^2): (1,-2), (2,5), (4,-2)
        CHECK(m3[1] == Frac(-2));
        CHECK(m3[2] == Frac(5));
        CHECK(m3[4] == Frac(-2));
        auto t4 = theta4(q, 100);
        auto e4 = etamake(t4, 100);
        CHECK(!e4.empty());
        std::map<int, Frac> m4;
        for (const auto& p : e4) m4[p.first] = p.second;
        // theta4 → η(τ)^2/η(2τ): (1,2), (2,-1)
        CHECK(m4[1] == Frac(2));
        CHECK(m4[2] == Frac(-1));
    }
    std::cout << "--- Phase 6: TEST-06 Rødseth ---\n";
    {
        auto q = Series::q(200);
        auto eta1 = etaq(q, 1, 200);
        auto eta2 = etaq(q, 2, 200);
        auto PD = (eta2 / eta1).truncTo(200);
        auto PD1 = sift(PD, 5, 1, 199);
        auto e6 = etamake(PD1, 38);
        CHECK(!e6.empty());
        std::map<int, Frac> m6;
        for (const auto& p : e6) m6[p.first] = p.second;
        // Rødseth: η(5τ)³·η(2τ)²/(η(10τ)·η(τ)⁴) → (1,-4), (2,2), (5,3), (10,-1)
        CHECK(m6[1] == Frac(-4));
        CHECK(m6[2] == Frac(2));
        CHECK(m6[5] == Frac(3));
        CHECK(m6[10] == Frac(-1));
    }
    std::cout << "--- Phase 6: etamake failure (non-eta) ---\n";
    {
        auto q = Series::q(25);
        auto noneta = (Series::one(25) + (q * Frac(1, 2))).truncTo(25);  // 1+q/2 has non-integer coeff → fail
        auto ef = etamake(noneta, 20);
        CHECK(ef.empty());
    }

    std::cout << "--- Phase 6: TEST-07 qfactor T(8,8) ---\n";
    {
        auto t8 = T_rn(8, 8, 64);
        CHECK(t8.coeff(6) != Frac(0));
        CHECK(t8.maxExp() == 42);
        auto qf = qfactor(t8, 50);
        CHECK(qf.q_power == 6);
        CHECK(qf.num_exponents[9] > Frac(0));
        CHECK(qf.num_exponents[10] > Frac(0));
        CHECK(qf.num_exponents[11] > Frac(0));
        CHECK(qf.num_exponents[16] > Frac(0));
        CHECK(qf.den_exponents[1] > Frac(0));
        CHECK(qf.den_exponents[2] > Frac(0));
        CHECK(qf.den_exponents[3] > Frac(0));
        CHECK(qf.den_exponents[4] > Frac(0));
    }

    std::cout << "--- Phase 6: TEST-04 jacprodmake Rogers-Ramanujan ---\n";
    {
        auto q_var = Series::q(50);
        Series rr = Series::zero(50);
        for (int n = 0; n <= 8; ++n) {
            rr = (rr + q_var.pow(n * n) / aqprod(q_var, q_var, n, 50)).truncTo(50);
        }
        rr.trunc = 50;
        auto jp = jacprodmake(rr, 40);
        CHECK(!jp.empty());
        std::map<std::pair<int,int>, Frac> m;
        for (const auto& [a, b, e] : jp) m[{a,b}] = e;
        CHECK((m[{0,5}] == Frac(1)));
        CHECK((m[{1,5}] == Frac(-1)));
        std::string prod = jac2prod(jp);
        CHECK(prod.find("(q,q^5)") != std::string::npos);
        CHECK(prod.find("(q^4,q^5)") != std::string::npos);
    }
    std::cout << "--- Phase 6: TEST-09 jacprodmake Euler pentagonal ---\n";
    {
        auto q = Series::q(500);
        auto EULER = etaq(q, 1, 500);
        auto E0 = sift(EULER, 5, 0, 499);
        auto jp = jacprodmake(E0, 50);
        CHECK(!jp.empty());
        std::map<std::pair<int,int>, Frac> m;
        for (const auto& [a, b, e] : jp) m[{a,b}] = e;
        CHECK((m[{2,5}] == Frac(1)));
        CHECK((m[{0,5}] == Frac(1)));
        CHECK((m[{1,5}] == Frac(-1)));
    }

    std::cout << "--- Phase 8: TEST-05 findhom (Gauss AGM) ---\n";
    {
        auto q = Series::q(100);
        auto t3 = theta3(q, 100);
        auto t4 = theta4(q, 100);
        auto t3q2 = theta3(q, 100).subs_q(2).truncTo(100);
        auto t4q2 = theta4(q, 100).subs_q(2).truncTo(100);
        std::vector<Series> L = {t3, t4, t3q2, t4q2};
        auto rels = findhom(L, 2, 0);
        CHECK(rels.size() >= 2u);
        // Monomial order (reverse lex): indices 0..9 = X3²,X3X4,X4²,X2X3,X2X4,X2²,X1X3,X1X4,X1X2,X1²
        // Expected (Garvan): X1²+X2²-2X3² and -X1*X2+X4². Kernel basis may differ; verify we span correct relations.
        // Check: (1) X1²+X2² proportional to X3² or X4² (Gauss AGM), (2) X1*X2 proportional to X3² or X4²
        bool has_sum_sq = false;   // relation involving X1²+X2² and X3² or X4²
        bool has_product = false;  // relation involving X1*X2 and X3² or X4²
        for (const auto& r : rels) {
            if (r.size() != 10u) continue;
            // X1² index 9, X2² index 5, X3² index 0, X4² index 2, X1*X2 index 8
            if (!r[9].isZero() && !r[5].isZero() && r[9] == r[5] && (r[0] != Frac(0) || r[2] != Frac(0)))
                has_sum_sq = true;  // X1²+X2²-2X3² or X1²+X2²-2X4²
            if (!r[8].isZero() && (r[0] != Frac(0) || r[2] != Frac(0)) && (r[8] == -r[0] || r[8] == -r[2]))
                has_product = true;  // X1*X2 = X3² or X1*X2 = X4²
        }
        CHECK(has_sum_sq);
        CHECK(has_product);
    }

    std::cout << "--- Phase 8: solve (linalg) ---\n";
    {
        std::vector<std::vector<Frac>> M2 = {{Frac(1), Frac(0)}, {Frac(0), Frac(1)}};
        std::vector<Frac> b2 = {Frac(1), Frac(2)};
        auto x2 = solve(M2, b2);
        CHECK(x2 && x2->size() == 2u && (*x2)[0] == Frac(1) && (*x2)[1] == Frac(2));
        std::vector<std::vector<Frac>> Mbad = {{Frac(1), Frac(0)}, {Frac(1), Frac(0)}};
        std::vector<Frac> bbad = {Frac(1), Frac(2)};  // inconsistent: x1=1 and x1=2
        auto xbad = solve(Mbad, bbad);
        CHECK(!xbad);
    }

    std::cout << "--- Phase 8: findnonhom and findpoly ---\n";
    {
        auto q = Series::q(30);
        auto f1 = Series::one(30) + q;
        auto f2 = Series::one(30) - q;
        std::vector<Series> L2 = {f1, f2};
        auto rels_nh = findnonhom(L2, 1, 0);
        for (const auto& r : rels_nh) CHECK(r.size() == 3u);  // 3 monomials for k=2,n=1
        auto x = Series::one(20) + Series::q(20);
        auto y = Series::one(20) - Series::q(20);
        auto rels_poly = findpoly(x, y, 2, 2);
        for (const auto& r : rels_poly) CHECK(r.size() == 9u);  // (deg1+1)*(deg2+1)=9
    }

    std::cout << "--- Phase 8: TEST-08 findnonhomcombo (Watson) ---\n";
    {
        auto q = Series::q(100);
        auto eta1 = etaq(q, 1, 100);
        auto eta7 = etaq(q, 7, 100);
        auto eta49 = etaq(q, 49, 100);
        auto xi = (q * q * (eta49 / eta1)).truncTo(100);
        auto T = (q * (eta7 / eta1).pow(4)).truncTo(100);
        auto T2 = (T * T).truncTo(100);
        std::vector<Series> L = {T, xi};
        std::vector<int> n_list = {1, 7};
        auto sol = findnonhomcombo(T2, L, n_list, 0, false);
        CHECK(sol);
        // Monomial order: (0,0),(1,0),(0,1),(1,1),(0,2),(1,2),...,(0,7),(1,7) — 16 monomials
        // Expected: T² = (49ξ³+35ξ²+7ξ)T + 343ξ⁷+343ξ⁶+147ξ⁵+49ξ⁴+21ξ³+7ξ²+ξ
        // Index map: (0,0)→0, (1,0)→1, (0,1)→2, (1,1)→3, (0,2)→4, (1,2)→5, ..., (0,7)→14, (1,7)→15
        // (1,1)→3: coeff 7, (1,2)→5: coeff 35, (1,3)→7: coeff 49
        // (0,1)→2: coeff 1, (0,2)→4: coeff 7, (0,3)→6: coeff 21, (0,4)→8: coeff 49, (0,5)→10: coeff 147, (0,6)→12: coeff 343, (0,7)→14: coeff 343
        auto& c = *sol;
        CHECK(c.size() == 16u);
        // Watson: T² = (49ξ³+35ξ²+7ξ)T + 343ξ⁷+...+ξ. Monomial order: 1,ξ,ξ²,..,ξ⁷,T,Tξ,..,Tξ⁷
        CHECK(c[1] == Frac(1));   // ξ
        CHECK(c[2] == Frac(7));   // ξ²
        CHECK(c[3] == Frac(21));  // ξ³
        CHECK(c[4] == Frac(49));  // ξ⁴
        CHECK(c[9] == Frac(7));   // T·ξ
        CHECK(c[10] == Frac(35)); // T·ξ²
        CHECK(c[11] == Frac(49)); // T·ξ³
    }

    std::cout << "--- Phase 7: linalg kernel ---\n";
    {
        auto Mv_is_zero = [](const std::vector<std::vector<Frac>>& M, const std::vector<Frac>& v) {
            if (M.empty() || v.size() != M[0].size()) return false;
            for (const auto& row : M) {
                Frac sum(0);
                for (size_t j = 0; j < row.size(); ++j) sum = sum + row[j] * v[j];
                if (!sum.isZero()) return false;
            }
            return true;
        };

        std::vector<std::vector<Frac>> M23 = {{Frac(1), Frac(1), Frac(0)}, {Frac(2), Frac(2), Frac(0)}};
        auto k23 = kernel(M23);
        CHECK(k23.size() == 2u);
        for (const auto& v : k23) CHECK(Mv_is_zero(M23, v));

        std::vector<std::vector<Frac>> M2id = {{Frac(1), Frac(0)}, {Frac(0), Frac(1)}};
        auto k2id = kernel(M2id);
        CHECK(k2id.empty());

        std::vector<std::vector<Frac>> Mz = {{Frac(0), Frac(0), Frac(0)}, {Frac(0), Frac(0), Frac(0)}};
        auto kz = kernel(Mz);
        CHECK(kz.size() == 3u);
        for (const auto& v : kz) CHECK(Mv_is_zero(Mz, v));

        std::vector<std::vector<Frac>> M1 = {{Frac(1), Frac(2), Frac(3)}};
        auto k1 = kernel(M1);
        CHECK(k1.size() == 2u);
        for (const auto& v : k1) CHECK(Mv_is_zero(M1, v));
    }

    std::cout << "--- Phase 5: Rogers-Ramanujan (TEST-01) ---\n";
    {
        auto q_var = Series::q(50);
        Series rr = Series::zero(50);
        for (int n = 0; n <= 8; ++n) {
            Series term = q_var.pow(n * n) / aqprod(q_var, q_var, n, 50);
            rr = (rr + term).truncTo(50);
        }
        rr.trunc = 50;
        auto a = prodmake(rr, 50);
        for (int n = 1; n <= 49; ++n) {
            Frac expected = (n % 5 == 1 || n % 5 == 4) ? Frac(1) : Frac(0);
            Frac got = Frac(0);
            auto it = a.find(n);
            if (it != a.end()) got = it->second;
            CHECK(got == expected);
        }
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

    std::cout << "--- Phase 9: parser ---\n";
    {
        Tokenizer tok("x := 42 + q # comment");
        auto t1 = tok.next(); CHECK(t1.kind == Token::Kind::IDENT && t1.text == "x");
        auto t2 = tok.next(); CHECK(t2.kind == Token::Kind::ASSIGN);
        auto t3 = tok.next(); CHECK(t3.kind == Token::Kind::INT && t3.text == "42");
        auto t4 = tok.next(); CHECK(t4.kind == Token::Kind::PLUS);
        auto t5 = tok.next(); CHECK(t5.kind == Token::Kind::Q);
        auto t6 = tok.next(); CHECK(t6.kind == Token::Kind::END);
    }
    {
        auto st = parse("1 + 2 * 3");
        CHECK(st && st->tag == Stmt::Tag::Expr && st->expr);
        CHECK(st->expr->tag == Expr::Tag::BinOp && st->expr->binOp == BinOp::Add);
        CHECK(st->expr->left->tag == Expr::Tag::IntLit && st->expr->left->intVal == 1);
        CHECK(st->expr->right->tag == Expr::Tag::BinOp && st->expr->right->binOp == BinOp::Mul);
    }
    {
        auto st = parse("a := etaq(q, 1, 50)");
        CHECK(st && st->tag == Stmt::Tag::Assign && st->assignName == "a");
        CHECK(st->assignRhs && st->assignRhs->tag == Expr::Tag::Call && st->assignRhs->callName == "etaq");
    }
    {
        auto st = parse("-q");
        CHECK(st && st->expr && st->expr->tag == Expr::Tag::UnOp && st->expr->operand->tag == Expr::Tag::Q);
    }
    {
        auto st = parse("a ^ b ^ c");
        CHECK(st && st->expr->tag == Expr::Tag::BinOp && st->expr->binOp == BinOp::Pow);
        CHECK(st->expr->right->tag == Expr::Tag::BinOp && st->expr->right->binOp == BinOp::Pow);
    }
    {
        auto st = parse("sum(q^n, n, 0, 10)");
        CHECK(st && st->expr->tag == Expr::Tag::Sum && st->expr->sumVar == "n");
        CHECK(st->expr->lo->intVal == 0 && st->expr->hi->intVal == 10);
    }
    {
        auto st = parse("add(1, k, 1, 5)");
        CHECK(st && st->expr->tag == Expr::Tag::Sum && st->expr->sumVar == "k");
    }
    {
        auto st = parse("[theta3(q,100), theta4(q,100)]");
        CHECK(st && st->expr->tag == Expr::Tag::List && st->expr->elements.size() == 2u);
        CHECK(st->expr->elements[0]->tag == Expr::Tag::Call && st->expr->elements[0]->callName == "theta3");
        CHECK(st->expr->elements[1]->tag == Expr::Tag::Call && st->expr->elements[1]->callName == "theta4");
    }
    {
        auto st = parse("# comment\nx := 1");
        CHECK(st && st->tag == Stmt::Tag::Assign && st->assignName == "x" && st->assignRhs->intVal == 1);
    }
    {
        parse("sum(q^(n^2)/aqprod(q,q,n,50), n, 0, 8)");  // Rogers-Ramanujan pattern — must not throw
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

int main(int argc, char** argv) {
    if (argc > 1) {
        std::string arg = argv[1];
        if (arg == "--version") {
            std::cout << "qseries 2.0\n";
            return 0;
        }
        if (arg == "--test")
            return runUnitTests();
    }
    runRepl();
    return 0;
}
