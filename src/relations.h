#ifndef RELATIONS_H
#define RELATIONS_H

#include "series.h"
#include "linalg.h"
#include "frac.h"
#include <vector>
#include <optional>
#include <string>

// Enumerate exponent vectors (a_0,...,a_{k-1}) with a_i >= 0 and sum = n.
// Appends each to 'out' in lex order.
inline void enumerate_hom_exponents(int k, int n, std::vector<int> prefix,
    std::vector<std::vector<int>>& out) {
    if (k == 1) {
        prefix.push_back(n);
        out.push_back(prefix);
        return;
    }
    for (int a = 0; a <= n; ++a) {
        std::vector<int> p = prefix;
        p.push_back(a);
        enumerate_hom_exponents(k - 1, n - a, p, out);
    }
}

// Enumerate exponent vectors for findnonhom: degree 0..n, all monomials.
inline void enumerate_nonhom_exponents(int k, int n, std::vector<std::vector<int>>& out) {
    out.clear();
    for (int d = 0; d <= n; ++d) {
        std::vector<std::vector<int>> sub;
        enumerate_hom_exponents(k, d, {}, sub);
        for (auto& v : sub)
            out.push_back(std::move(v));
    }
}

// Build matrix M: rows = monomials, cols = q-powers [0..numCols-1].
// Returns M (not transposed).
inline std::vector<std::vector<Frac>> build_matrix(
    const std::vector<Series>& monomials, int numCols) {
    std::vector<std::vector<Frac>> M;
    for (const auto& m : monomials) {
        M.push_back(m.coeffList(0, numCols - 1));
    }
    return M;
}

// Transpose matrix
inline std::vector<std::vector<Frac>> transpose(const std::vector<std::vector<Frac>>& M) {
    if (M.empty() || M[0].empty()) return {};
    int rows = static_cast<int>(M.size());
    int cols = static_cast<int>(M[0].size());
    std::vector<std::vector<Frac>> MT(cols, std::vector<Frac>(rows));
    for (int i = 0; i < rows; ++i)
        for (int j = 0; j < cols; ++j)
            MT[j][i] = M[i][j];
    return MT;
}

struct FindmaxindResult {
    std::vector<Series> subset;
    std::vector<int> indices;
};

// findmaxind(L, topshift): maximal linearly independent subset of q-series in L.
// Returns subset and 1-based indices (Maple compatible).
inline FindmaxindResult findmaxind(const std::vector<Series>& L, int topshift) {
    if (L.empty()) return {{}, {}};
    int T = L[0].trunc;
    for (const auto& s : L) T = std::min(T, s.trunc);
    int numCols = T + topshift;
    if (numCols <= 0) return {{}, {}};

    auto M = build_matrix(L, numCols);
    auto MT = transpose(M);
    auto MT_copy = MT;
    auto pivot_cols = gauss_to_rref(MT_copy);

    std::vector<Series> subset;
    std::vector<int> indices;
    for (int i : pivot_cols) {
        subset.push_back(L[i]);
        indices.push_back(i + 1);
    }
    return {subset, indices};
}

// findhom(L, n, topshift): homogeneous relations of degree n among L.
// Returns kernel basis: each vector is coeffs for monomials in lex order.
inline std::vector<std::vector<Frac>> findhom(
    const std::vector<Series>& L, int n, int topshift) {
    if (L.empty() || n < 0) return {};
    int T = L[0].trunc;
    for (const auto& s : L) T = std::min(T, s.trunc);
    int numCols = T + topshift;
    if (numCols <= 0) return {};

    std::vector<std::vector<int>> exps;
    enumerate_hom_exponents(static_cast<int>(L.size()), n, {}, exps);

    std::vector<Series> monomials;
    for (const auto& e : exps) {
        Series prod = Series::one(T);
        for (size_t i = 0; i < L.size(); ++i) {
            prod = (prod * L[i].truncTo(T).pow(e[i])).truncTo(T);
        }
        monomials.push_back(std::move(prod));
    }

    auto M = build_matrix(monomials, numCols);
    auto MT = transpose(M);
    return kernel(MT);
}

// findnonhom(L, n, topshift): nonhomogeneous relations degree <= n.
inline std::vector<std::vector<Frac>> findnonhom(
    const std::vector<Series>& L, int n, int topshift) {
    if (L.empty() || n < 0) return {};
    int T = L[0].trunc;
    for (const auto& s : L) T = std::min(T, s.trunc);
    int numCols = T + topshift;
    if (numCols <= 0) return {};

    std::vector<std::vector<int>> exps;
    enumerate_nonhom_exponents(static_cast<int>(L.size()), n, exps);

    std::vector<Series> monomials;
    for (const auto& e : exps) {
        Series prod = Series::one(T);
        for (size_t i = 0; i < L.size(); ++i) {
            prod = (prod * L[i].truncTo(T).pow(e[i])).truncTo(T);
        }
        monomials.push_back(std::move(prod));
    }

    auto M = build_matrix(monomials, numCols);
    auto MT = transpose(M);
    return kernel(MT);
}

// findpoly(x, y, deg1, deg2, check): polynomial relations P(X,Y)=0.
// Monomials X^i Y^j, i=0..deg1, j=0..deg2, column-major order (i varies first).
inline std::vector<std::vector<Frac>> findpoly(
    const Series& x, const Series& y, int deg1, int deg2, std::optional<int> check) {
    int T = std::min(x.trunc, y.trunc);
    int topshift = 0;
    if (check) topshift = std::max(0, *check - T);
    int numCols = T + topshift;
    if (numCols <= 0 || deg1 < 0 || deg2 < 0) return {};

    std::vector<Series> monomials;
    auto xt = x.truncTo(T);
    auto yt = y.truncTo(T);
    for (int j = 0; j <= deg2; ++j) {
        for (int i = 0; i <= deg1; ++i) {
            Series m = (xt.pow(i) * yt.pow(j)).truncTo(T);
            monomials.push_back(std::move(m));
        }
    }

    auto M = build_matrix(monomials, numCols);
    auto MT = transpose(M);
    return kernel(MT);
}

// Overload without check
inline std::vector<std::vector<Frac>> findpoly(
    const Series& x, const Series& y, int deg1, int deg2) {
    return findpoly(x, y, deg1, deg2, std::nullopt);
}

// Enumerate exponent vectors for findnonhomcombo: 0 <= d_i <= n_list[i], row-major.
inline void enumerate_nlist_exponents(const std::vector<int>& n_list,
    std::vector<int> prefix, size_t idx, std::vector<std::vector<int>>& out) {
    if (idx == n_list.size()) {
        out.push_back(prefix);
        return;
    }
    for (int d = 0; d <= n_list[idx]; ++d) {
        std::vector<int> p = prefix;
        p.push_back(d);
        enumerate_nlist_exponents(n_list, p, idx + 1, out);
    }
}

// findhomcombo(f, L, n, topshift, etaoption): express f as homogeneous poly in L of degree n.
inline std::optional<std::vector<Frac>> findhomcombo(
    const Series& f, const std::vector<Series>& L, int n, int topshift, bool /*etaoption*/) {
    if (L.empty() || n < 0) return std::nullopt;
    int T = f.trunc;
    for (const auto& s : L) T = std::min(T, s.trunc);
    int numCols = T + topshift;
    if (numCols <= 0) return std::nullopt;

    std::vector<std::vector<int>> exps;
    enumerate_hom_exponents(static_cast<int>(L.size()), n, {}, exps);

    std::vector<Series> monomials;
    for (const auto& e : exps) {
        Series prod = Series::one(T);
        for (size_t i = 0; i < L.size(); ++i) {
            prod = (prod * L[i].truncTo(T).pow(e[i])).truncTo(T);
        }
        monomials.push_back(std::move(prod));
    }

    auto M = build_matrix(monomials, numCols);
    auto MT = transpose(M);
    std::vector<Frac> f_coeffs = f.truncTo(T).coeffList(0, numCols - 1);
    if (static_cast<int>(f_coeffs.size()) < numCols) f_coeffs.resize(numCols, Frac(0));
    return solve(MT, f_coeffs);
}

// findnonhomcombo(f, L, n_list, topshift, etaoption): express f as poly in L with per-var bounds.
inline std::optional<std::vector<Frac>> findnonhomcombo(
    const Series& f, const std::vector<Series>& L,
    const std::vector<int>& n_list, int topshift, bool /*etaoption*/) {
    if (L.empty() || n_list.size() != L.size()) return std::nullopt;
    int T = f.trunc;
    for (const auto& s : L) T = std::min(T, s.trunc);
    int numCols = T + topshift;
    if (numCols <= 0) return std::nullopt;

    std::vector<std::vector<int>> exps;
    enumerate_nlist_exponents(n_list, {}, 0, exps);

    std::vector<Series> monomials;
    for (const auto& e : exps) {
        Series prod = Series::one(T);
        for (size_t i = 0; i < L.size(); ++i) {
            prod = (prod * L[i].truncTo(T).pow(e[i])).truncTo(T);
        }
        monomials.push_back(std::move(prod));
    }

    auto M = build_matrix(monomials, numCols);
    auto MT = transpose(M);
    std::vector<Frac> f_coeffs = f.truncTo(T).coeffList(0, numCols - 1);
    if (static_cast<int>(f_coeffs.size()) < numCols) f_coeffs.resize(numCols, Frac(0));
    return solve(MT, f_coeffs);
}

#endif
