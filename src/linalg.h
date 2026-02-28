#ifndef LINALG_H
#define LINALG_H

#include "frac.h"
#include <vector>
#include <algorithm>
#include <optional>

// Reduce A to RREF in-place; return pivot column indices.
// If maxCols >= 0, only pivot on columns 0..maxCols-1 (for augmented [M|b] use maxCols=M.cols).
inline std::vector<int> gauss_to_rref(std::vector<std::vector<Frac>>& A, int maxCols = -1) {
    std::vector<int> pivot_cols;
    if (A.empty() || A[0].empty()) return pivot_cols;
    int rows = static_cast<int>(A.size());
    int cols = static_cast<int>(A[0].size());
    int pivotLimit = (maxCols >= 0 && maxCols < cols) ? maxCols : cols;
    int pivot_row = 0;
    for (int c = 0; c < pivotLimit; ++c) {
        int r = pivot_row;
        while (r < rows && A[r][c].isZero()) ++r;
        if (r >= rows) continue;  // free column
        if (r != pivot_row)
            std::swap(A[r], A[pivot_row]);
        Frac pivot_val = A[pivot_row][c];
        for (int j = 0; j < cols; ++j)
            A[pivot_row][j] = A[pivot_row][j] / pivot_val;
        for (int i = 0; i < rows; ++i) {
            if (i == pivot_row) continue;
            if (!A[i][c].isZero()) {
                Frac scale = A[i][c];
                for (int j = 0; j < cols; ++j)
                    A[i][j] = A[i][j] - scale * A[pivot_row][j];
            }
        }
        pivot_cols.push_back(c);
        ++pivot_row;
    }
    return pivot_cols;
}

// Compute kernel (null space) of M. Returns basis for {v : M*v = 0}.
// Does not modify M. Each returned vector has length = ncols.
inline std::vector<std::vector<Frac>> kernel(std::vector<std::vector<Frac>>& M) {
    if (M.empty() || M[0].empty()) return {};
    std::vector<std::vector<Frac>> A = M;
    std::vector<int> pivot_cols = gauss_to_rref(A);
    int ncols = static_cast<int>(A[0].size());
    std::vector<int> free_cols;
    for (int j = 0; j < ncols; ++j) {
        if (std::find(pivot_cols.begin(), pivot_cols.end(), j) == pivot_cols.end())
            free_cols.push_back(j);
    }
    std::vector<std::vector<Frac>> basis;
    for (int j : free_cols) {
        std::vector<Frac> v(ncols, Frac(0));
        v[j] = Frac(1);
        for (size_t pi = 0; pi < pivot_cols.size(); ++pi) {
            int pc = pivot_cols[pi];
            Frac sum(0);
            for (int fc : free_cols)
                sum = sum + A[pi][fc] * v[fc];
            v[pc] = Frac(-1) * sum;
        }
        basis.push_back(std::move(v));
    }
    return basis;
}

// Solve M·x = b. M is rows×cols, b has length rows.
// Returns solution x (length cols) if consistent; std::nullopt if inconsistent.
inline std::optional<std::vector<Frac>> solve(
    const std::vector<std::vector<Frac>>& M, const std::vector<Frac>& b) {
    if (M.empty() || M[0].empty() || static_cast<int>(b.size()) != static_cast<int>(M.size()))
        return std::nullopt;
    int rows = static_cast<int>(M.size());
    int cols = static_cast<int>(M[0].size());
    std::vector<std::vector<Frac>> A(rows);
    for (int i = 0; i < rows; ++i) {
        A[i].reserve(cols + 1);
        for (int j = 0; j < cols; ++j) A[i].push_back(M[i][j]);
        A[i].push_back(b[i]);
    }
    std::vector<int> pivot_cols = gauss_to_rref(A, cols);  // Only pivot on M columns, not b
    // Check consistency: no row has zeros in cols 0..cols-1 and nonzero in col cols
    for (int i = 0; i < rows; ++i) {
        bool all_zero = true;
        for (int j = 0; j < cols; ++j) {
            if (!A[i][j].isZero()) { all_zero = false; break; }
        }
        if (all_zero && !A[i][cols].isZero()) return std::nullopt;
    }
    // Back-substitute: free vars = 0; x[pc] = A[pi][cols] for pivot cols
    std::vector<Frac> x(cols, Frac(0));
    std::vector<int> free_cols;
    for (int j = 0; j < cols; ++j) {
        if (std::find(pivot_cols.begin(), pivot_cols.end(), j) == pivot_cols.end())
            free_cols.push_back(j);
    }
    int pi = 0;
    for (int pc : pivot_cols) {
        if (pi >= rows) break;
        Frac sum(0);
        for (int fc : free_cols) sum = sum + A[pi][fc] * x[fc];
        x[pc] = A[pi][cols] - sum;
        ++pi;
    }
    return x;
}

#endif
