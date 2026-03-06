#ifndef TCORE_H
#define TCORE_H

#include <vector>
#include <cstdint>
#include <algorithm>
#include <stdexcept>

// Safe positive modulo: always returns value in [0, p)
inline int64_t safe_mod(int64_t x, int64_t p) {
    return ((x % p) + p) % p;
}

// Count nodes colored k in the p-residue diagram of partition ptn.
// ptn can be in any order; internally sorts descending.
inline int64_t tcore_rvec(const std::vector<int64_t>& ptn, int p, int k) {
    std::vector<int64_t> desc(ptn);
    std::sort(desc.begin(), desc.end(), std::greater<int64_t>());
    int64_t x = 0;
    int64_t N = static_cast<int64_t>(desc.size());
    for (int64_t m = 1; m <= N; ++m) {
        int64_t part = desc[m - 1];
        int64_t offset = safe_mod(p - m - k, p);
        x += (part + offset) / p;
    }
    return x;
}

// Test whether partition ptn is a p-core.
// A partition is a t-core iff no hook length is divisible by t.
inline bool tcore_istcore(const std::vector<int64_t>& ptn, int p) {
    std::vector<int64_t> R(p);
    for (int i = 0; i < p; ++i)
        R[i] = tcore_rvec(ptn, p, i);

    int64_t x = 0;
    for (int i = 0; i < p; ++i) {
        int i1 = (i + 1) % p;
        x += R[i] * R[i] - R[i] * R[i1];
    }
    return (x == R[0]);
}

// Compute the n-vector of partition ptn with respect to modulus p.
// nvec[k] = rvec(ptn,p,k) - rvec(ptn,p,(k+1) mod p) for k=0..p-1
inline std::vector<int64_t> tcore_ptn2nvec(const std::vector<int64_t>& ptn, int p) {
    std::vector<int64_t> R(p);
    for (int i = 0; i < p; ++i)
        R[i] = tcore_rvec(ptn, p, i);
    std::vector<int64_t> nvec(p);
    for (int k = 0; k < p; ++k)
        nvec[k] = R[k] - R[(k + 1) % p];
    return nvec;
}

// Compute the r-vector (residue vector) of ptn mod p.
// rvec_full[k] = rvec(ptn,p,k) for k=0..p-1
inline std::vector<int64_t> tcore_ptn2rvec(const std::vector<int64_t>& ptn, int p) {
    std::vector<int64_t> R(p);
    for (int i = 0; i < p; ++i)
        R[i] = tcore_rvec(ptn, p, i);
    return R;
}

// Reconstruct a t-core partition from its n-vector using Frobenius coordinates.
// Returns partition in non-increasing (descending) order.
inline std::vector<int64_t> tcore_nvec2ptn(const std::vector<int64_t>& nvec) {
    int t = static_cast<int>(nvec.size());
    if (t == 0) return {};

    // X = sum of positive entries (Durfee square size)
    int64_t X = 0;
    for (int j = 0; j < t; ++j) {
        if (nvec[j] > 0) X += nvec[j];
    }
    if (X == 0) return {};

    // Build Frobenius arm pre-parts (ppartsA)
    std::vector<int64_t> ppartsA;
    for (int j = 0; j < t; ++j) {
        if (nvec[j] > 0) {
            for (int64_t k = 1; k <= nvec[j]; ++k)
                ppartsA.push_back(static_cast<int64_t>(t) * (k - 1) + j);
        }
    }
    std::sort(ppartsA.begin(), ppartsA.end());

    // Build conjugate n-vector: cnvec[j] = -nvec[t-1-j]
    std::vector<int64_t> cnvec(t);
    for (int j = 0; j < t; ++j)
        cnvec[j] = -nvec[t - 1 - j];

    // Build Frobenius leg pre-parts (ppartsB)
    std::vector<int64_t> ppartsB;
    for (int j = 0; j < t; ++j) {
        if (cnvec[j] > 0) {
            for (int64_t k = 1; k <= cnvec[j]; ++k)
                ppartsB.push_back(static_cast<int64_t>(t) * (k - 1) + j);
        }
    }
    std::sort(ppartsB.begin(), ppartsB.end());

    // partsA: Frobenius arm lengths → row lengths through Durfee square
    // partsA[j] = ppartsA[j] + X - j (0-indexed; Maple: ppartsA[j] + X - j + 1 with 1-indexing)
    std::vector<int64_t> partsA;
    for (int64_t j = 0; j < X; ++j)
        partsA.push_back(ppartsA[j] + X - j);

    // pp2a: Frobenius leg lengths → column lengths below diagonal
    // pp2a[j] = ppartsB[j] - j (0-indexed; Maple: ppartsB[j] - j + 1 with 1-indexing)
    std::vector<int64_t> pp2a;
    for (int64_t j = 0; j < X; ++j) {
        int64_t val = ppartsB[j] - j;
        if (val > 0) pp2a.push_back(val);
    }

    // Conjugate the leg partition to get the lower part of the partition
    std::vector<int64_t> partsB;
    if (!pp2a.empty()) {
        int64_t largest = *std::max_element(pp2a.begin(), pp2a.end());
        for (int64_t i = 1; i <= largest; ++i) {
            int64_t count = 0;
            for (int64_t p : pp2a)
                if (p >= i) ++count;
            partsB.push_back(count);
        }
    }

    // Combine: result = partsB ++ partsA
    std::vector<int64_t> result;
    result.insert(result.end(), partsB.begin(), partsB.end());
    result.insert(result.end(), partsA.begin(), partsA.end());

    // Sort descending for non-increasing output convention
    std::sort(result.begin(), result.end(), std::greater<int64_t>());
    return result;
}

// Compute the t-core of partition ptn.
// Returns partition in non-increasing (descending) order.
inline std::vector<int64_t> tcore_tcoreofptn(const std::vector<int64_t>& ptn, int p) {
    std::vector<int64_t> nvec = tcore_ptn2nvec(ptn, p);
    return tcore_nvec2ptn(nvec);
}

// List all p-cores of n (brute force: enumerate partitions, filter by istcore).
// Requires enumerate_partitions from qfuncs.h.
// Returns list of partitions, each in non-increasing order.
inline std::vector<std::vector<int64_t>> tcore_tcores(int p, [[maybe_unused]] int n,
    const std::vector<std::vector<int64_t>>& all_partitions) {
    std::vector<std::vector<int64_t>> result;
    for (const auto& ptn : all_partitions) {
        if (tcore_istcore(ptn, p))
            result.push_back(ptn);
    }
    return result;
}

// Floor division: rounds toward negative infinity (unlike C++ / which truncates)
inline int64_t floor_div(int64_t a, int64_t b) {
    int64_t d = a / b;
    int64_t r = a % b;
    if (r != 0 && ((a ^ b) < 0)) d -= 1;
    return d;
}

// Compute the t-quotient of partition ptn.
// Returns a list of t partitions (each in non-increasing order).
inline std::vector<std::vector<int64_t>> tcore_tquot(const std::vector<int64_t>& ptn, int t) {
    std::vector<int64_t> desc(ptn);
    std::sort(desc.begin(), desc.end(), std::greater<int64_t>());

    int nps = static_cast<int>(desc.size());

    // ptnz = descending partition + t zeros
    std::vector<int64_t> ptnz(desc);
    for (int i = 0; i < t; ++i) ptnz.push_back(0);

    int total = nps + t;

    // Beta-set: lambdabar[j] = ptnz[j] - (j+1) (0-indexed; Maple uses 1-indexed)
    std::vector<int64_t> lambdabar(total);
    for (int j = 0; j < total; ++j)
        lambdabar[j] = ptnz[j] - (j + 1);

    // For each residue class i, extract sub-beta-set and convert to partition
    std::vector<std::vector<int64_t>> result;
    for (int i = 0; i < t; ++i) {
        std::vector<int64_t> muibar;
        for (int j = 0; j < total; ++j) {
            if (safe_mod(lambdabar[j], t) == i)
                muibar.push_back(lambdabar[j]);
        }

        std::vector<int64_t> lamhati;
        int ki = static_cast<int>(muibar.size());
        for (int k = 0; k < ki - 1; ++k) {
            int64_t nik = floor_div(muibar[k], t) - floor_div(muibar[k + 1], t) - 1;
            for (int64_t jj = 0; jj < nik; ++jj)
                lamhati.push_back(k + 1);
        }

        std::sort(lamhati.begin(), lamhati.end(), std::greater<int64_t>());
        result.push_back(lamhati);
    }
    return result;
}

// Find the p-th "E" position in bi-infinite word W_w.
// Partition is in descending order; returns 0-indexed position.
inline int tcore_findhookinpos(const std::vector<int64_t>& ptn, int t, int w, int p) {
    std::vector<int64_t> padded(ptn);
    for (int i = 0; i < p * t; ++i)
        padded.push_back(0);

    int find = 0;
    int fj = 0;
    for (size_t j = 0; j < padded.size(); ++j) {
        if (safe_mod(padded[j] - static_cast<int64_t>(j + 1), t) == w) {
            fj = static_cast<int>(j);
            find++;
            if (find == p) return fj;
        }
    }
    return fj;
}

// Add a rim hook of length L*t cells starting at position j (0-indexed).
// Partition is in descending order. Returns modified partition.
inline std::vector<int64_t> tcore_addrimthook(std::vector<int64_t> ptn, int j, int L, int t) {
    while (j >= static_cast<int>(ptn.size()))
        ptn.push_back(0);

    int cj = j;
    for (int len = 0; len < L * t; ++len) {
        if (len == 0) {
            ptn[cj]++;
        } else if (cj == 0) {
            ptn[cj]++;
        } else {
            int nj = cj - 1;
            if (ptn[nj] < ptn[cj]) {
                ptn[nj]++;
                cj = nj;
            } else {
                ptn[cj]++;
            }
        }
    }
    return ptn;
}

// Inverse GSK phi1 bijection: reconstruct partition from [core, quotient].
// Core and each quotient component are in descending order.
// Returns partition in descending order.
inline std::vector<int64_t> tcore_invphi1(
    const std::vector<int64_t>& core,
    const std::vector<std::vector<int64_t>>& quotient, int t) {

    std::vector<int64_t> ptn(core);
    std::sort(ptn.begin(), ptn.end(), std::greater<int64_t>());

    // Process quotient components from j=t-1 downto 0
    for (int j = t - 1; j >= 0; --j) {
        const auto& tqj = quotient[j];
        int r = j;

        if (!tqj.empty()) {
            // Sort descending to process largest parts first
            std::vector<int64_t> sorted_tqj(tqj);
            std::sort(sorted_tqj.begin(), sorted_tqj.end(), std::greater<int64_t>());

            for (size_t k = 0; k < sorted_tqj.size(); ++k) {
                int p_val = static_cast<int>(sorted_tqj[k]);
                int posi = tcore_findhookinpos(ptn, t, r, p_val);
                ptn = tcore_addrimthook(ptn, posi, p_val, t);
            }
        }
    }

    // Remove trailing zeros, sort descending
    while (!ptn.empty() && ptn.back() == 0) ptn.pop_back();
    std::sort(ptn.begin(), ptn.end(), std::greater<int64_t>());
    return ptn;
}

// Modular exponentiation: base^exp mod mod, non-negative result
inline int64_t mod_pow(int64_t base, int64_t exp, int64_t mod) {
    base = safe_mod(base, mod);
    int64_t result = 1;
    while (exp > 0) {
        if (exp & 1) result = safe_mod(result * base, mod);
        base = safe_mod(base * base, mod);
        exp >>= 1;
    }
    return result;
}

// GKS t-core crank statistic. Returns value in {0, 1, ..., p-1}.
// Uses h(t) = (t - (p-1)/2)^(p-3) mod p (inverse-square via Fermat).
// Partition parts are in non-increasing (descending) order.
inline int64_t tcore_tcrank(const std::vector<int64_t>& ptn, int64_t p) {
    if (p < 3 || p % 2 == 0)
        throw std::runtime_error("tcrank: p must be an odd prime >= 3");
    int64_t half_p = (p - 1) / 2;
    int64_t x = 0;
    int64_t m = static_cast<int64_t>(ptn.size());
    for (int64_t i = 1; i <= m; ++i) {
        int64_t lam = ptn[i - 1];
        int64_t h_lam_i = mod_pow(lam - i - half_p, p - 3, p);
        int64_t h_i_1 = mod_pow((i - 1) - half_p, p - 3, p);
        x = safe_mod(x + h_lam_i - h_i_1, p);
    }
    return safe_mod(x, p);
}

#endif
