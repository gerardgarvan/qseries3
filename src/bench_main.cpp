#include "bigint.h"
#include "frac.h"
#include "series.h"
#include "qfuncs.h"
#include "convert.h"
#include <iostream>
#include <iomanip>
#include <chrono>
#include <vector>
#include <algorithm>
#include <string>

template <typename T>
inline void doNotOptimize(const T& val) {
    asm volatile("" : : "r,m"(val) : "memory");
}

struct BenchResult {
    std::string name;
    double median_us;
    int iterations;
};

template <typename F>
BenchResult bench(const std::string& name, int iters, F&& fn) {
    std::vector<double> times;
    times.reserve(iters);
    for (int i = 0; i < iters; ++i) {
        auto t0 = std::chrono::steady_clock::now();
        fn();
        auto t1 = std::chrono::steady_clock::now();
        double us = std::chrono::duration<double, std::micro>(t1 - t0).count();
        times.push_back(us);
    }
    std::sort(times.begin(), times.end());
    double median = times[times.size() / 2];
    return {name, median, iters};
}

int main() {
    std::cout << std::fixed << std::setprecision(1);
    std::cout << "q-series benchmark suite\n";
    std::cout << "========================\n\n";

    std::vector<BenchResult> results;

    // --- BigInt multiply (small: 4 limbs ~ 36 digits) ---
    {
        BigInt a("123456789012345678901234567890123456");
        BigInt b("987654321098765432109876543210987654");
        auto r = bench("BigInt mul 36-digit", 5000, [&]() {
            auto c = a * b;
            doNotOptimize(c);
        });
        results.push_back(r);
    }

    // --- BigInt multiply (large: ~100 limbs ~ 900 digits) ---
    {
        std::string sa(900, '1');
        std::string sb(900, '2');
        BigInt a(sa), b(sb);
        auto r = bench("BigInt mul 900-digit", 500, [&]() {
            auto c = a * b;
            doNotOptimize(c);
        });
        results.push_back(r);
    }

    // --- Series multiply (T=50) ---
    {
        Series q = Series::q(50);
        Series f = (q + q * q + q * q * q).truncTo(50);
        Series g = (Series::one(50) + q + q * q).truncTo(50);
        for (int e = 3; e < 50; ++e) {
            f.setCoeff(e, Frac(e % 7 + 1));
            g.setCoeff(e, Frac(e % 5 + 1));
        }
        auto r = bench("Series mul T=50", 1000, [&]() {
            auto h = f * g;
            doNotOptimize(h);
        });
        results.push_back(r);
    }

    // --- Series multiply (T=200) ---
    {
        Series f;
        f.trunc = 200;
        Series g;
        g.trunc = 200;
        for (int e = 0; e < 200; ++e) {
            f.setCoeff(e, Frac(e % 11 + 1));
            g.setCoeff(e, Frac(e % 7 + 1));
        }
        auto r = bench("Series mul T=200", 50, [&]() {
            auto h = f * g;
            doNotOptimize(h);
        });
        results.push_back(r);
    }

    // --- etaq(1, 100) ---
    {
        auto r = bench("etaq(1, 100)", 100, [&]() {
            clear_etaq_cache();
            Series q = Series::q(100);
            auto e = etaq(q, 1, 100);
            doNotOptimize(e);
        });
        results.push_back(r);
    }

    // --- etaq(1, 500) ---
    {
        auto r = bench("etaq(1, 500)", 10, [&]() {
            clear_etaq_cache();
            Series q = Series::q(500);
            auto e = etaq(q, 1, 500);
            doNotOptimize(e);
        });
        results.push_back(r);
    }

    // --- prodmake (Rogers-Ramanujan, T=50) ---
    {
        Series q = Series::q(50);
        Series rr = Series::zero(50);
        rr.setCoeff(0, Frac(1));
        for (int n = 1; n <= 8; ++n) {
            Series num = Series::qpow(n * n, 50);
            Series den = aqprod(q, q, n, 50);
            rr = rr + num / den;
        }
        rr = rr.truncTo(50);
        auto r = bench("prodmake RR T=50", 100, [&]() {
            auto pm = prodmake(rr, 50);
            doNotOptimize(pm);
        });
        results.push_back(r);
    }

    // --- Print results ---
    std::cout << std::left << std::setw(28) << "Benchmark"
              << std::right << std::setw(12) << "Median (us)"
              << std::setw(8) << "Iters" << "\n";
    std::cout << std::string(48, '-') << "\n";
    for (const auto& r : results) {
        std::cout << std::left << std::setw(28) << r.name
                  << std::right << std::setw(12) << r.median_us
                  << std::setw(8) << r.iterations << "\n";
    }
    std::cout << "\n";
    return 0;
}
