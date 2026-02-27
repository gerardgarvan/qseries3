# Phase 14: Relations demo — Research

**Phase goal:** Relation-finding examples reproduce qseriesdoc §4  
**Success criteria:**
1. findhom yields Gauss AGM relations (X₁²+X₂²-2X₃², -X₁X₂+X₄²) in demo
2. findhomcombo example runs and produces expected polynomial
3. findnonhomcombo example runs and produces expected output

**Requirements:** DEMO-04

---

## Source References

### qseriesdoc §4 — The Search for Relations
- §4.1 findhom — Gauss AGM (Output 17–18)
- §4.2 findhomcombo — Eisenstein U_{5,6} (Output 19)
- §4.4 findnonhomcombo — Watson modular equation (Output 24)

### SPEC acceptance tests
- **Test 5 (Gauss AGM):** findhom with theta3, theta4, theta3(q²), theta4(q²)
- **Test 8 (Watson):** findnonhomcombo(T², [T, ξ], [1, 7], 0)

---

## Exact REPL Commands

### 1. findhom — Gauss AGM (§4.1, Test 5)

**Maple (qseriesdoc):**
```maple
findhom([theta3(q,100), theta4(q,100), theta3(q^2,100), theta4(q^2,100)], q, 2, 0)
```

**REPL (exact):**
```
findhom([theta3(q,100), theta4(q,100), theta3(subs_q(q,2),100), theta4(subs_q(q,2),100)], 2, 0)
```

**Notes:**
- REPL `theta3`/`theta4` accept `(q, T)` or `(T)` using default q.
- `theta3(q^2)` = `theta3(q,T).subs_q(2)` → `theta3(subs_q(q,2), 100)`.
- findhom(L, n, topshift): 3 args; no `q` arg.

**Expected output:** Two relations:
- `X₁² + X₂² - 2·X₃²`
- `-X₁·X₂ + X₄²`

Variables: X₁=θ₃(q), X₂=θ₄(q), X₃=θ₃(q²), X₄=θ₄(q²).

---

### 2. findhomcombo — Eisenstein U_{5,6} (§4.2)

**Maple (qseriesdoc):**
```maple
f := UE(q, 6, 5, 50)
B1 := etaq(q,1,50)^5/etaq(q,5,50)
B2 := q*etaq(q,5,50)^5/etaq(q,1,50)
findhomcombo(f, [B1, B2], q, 3, 0, yes)
```

**UE definition (Eisenstein series):**
```
U_{p,k}(q) = Σ_{m=1} Σ_{n=1} χ(m) n^(k-1) q^(mn)
```
where χ(m) = Legendre symbol (m/p). For p=5, k=6: χ(m)*n^5*q^(m*n).

**REPL (exact):**
```
set_trunc(50)
f := sum(sum(legendre(m,5)*n^5*q^(m*n), n, 1, 50), m, 1, 50)
B1 := etaq(1,50)^5/etaq(5,50)
B2 := q*etaq(5,50)^5/etaq(1,50)
findhomcombo(f, [B1, B2], 3, 0)
```

**Notes:**
- etaq(1,50) = etaq(q,1,50) when q is default.
- Double sum: inner n, outer m. Truncation 50; terms with m*n ≥ 50 are dropped.
- etaoption: REPL passes false; output is polynomial in X₁,X₂.

**Expected output:** Polynomial
- `X₁²·X₂ + 40·X₁·X₂² + 335·X₂³`
- (X₁=B1, X₂=B2)

---

### 3. findnonhomcombo — Watson modular equation (§4.4, Test 8)

**Maple (qseriesdoc):**
```maple
xi := q^2*etaq(q,49,100)/etaq(q,1,100)
T := q*(etaq(q,7,100)/etaq(q,1,100))^4
findnonhomcombo(T^2, [T, xi], q, [1, 7], 0, no)
```

**REPL (exact):**
```
set_trunc(100)
xi := q^2*etaq(49,100)/etaq(1,100)
T := q*(etaq(7,100)/etaq(1,100))^4
findnonhomcombo(T^2, [T, xi], [1, 7], 0)
```

**Notes:**
- n_list = [1, 7]: max degree 1 in T (X₁), max degree 7 in ξ (X₂).

**Expected output:** Watson's equation
- `T² = (49ξ³ + 35ξ² + 7ξ)·T + 343ξ⁷ + 343ξ⁶ + 147ξ⁵ + 49ξ⁴ + 21ξ³ + 7ξ² + ξ`
- In X₁,X₂ form: `(49·X₂³ + 35·X₂² + 7·X₂)·X₁ + 343·X₂⁷ + 343·X₂⁶ + 147·X₂⁵ + 49·X₂⁴ + 21·X₂³ + 7·X₂² + X₂`

---

## Run Block Structure for garvan-demo.sh

Use the same pattern as other sections: `echo` separator, then `run "cmd1" "cmd2" ...`.

### Block 1: findhom (Gauss AGM)
```bash
echo ""
echo "--- findhom: Gauss AGM (qseriesdoc §4.1) ---"
run "set_trunc(100)" "findhom([theta3(q,100), theta4(q,100), theta3(subs_q(q,2),100), theta4(subs_q(q,2),100)], 2, 0)"
```

### Block 2: findhomcombo (Eisenstein)
```bash
echo ""
echo "--- findhomcombo: U_{5,6} (qseriesdoc §4.2) ---"
run "set_trunc(50)" "f := sum(sum(legendre(m,5)*n^5*q^(m*n), n, 1, 50), m, 1, 50)" "B1 := etaq(1,50)^5/etaq(5,50)" "B2 := q*etaq(5,50)^5/etaq(1,50)" "findhomcombo(f, [B1, B2], 3, 0)"
```

### Block 3: findnonhomcombo (Watson)
```bash
echo ""
echo "--- findnonhomcombo: Watson modular equation (qseriesdoc §4.4) ---"
run "set_trunc(100)" "xi := q^2*etaq(49,100)/etaq(1,100)" "T := q*(etaq(7,100)/etaq(1,100))^4" "findnonhomcombo(T^2, [T, xi], [1, 7], 0)"
```

---

## Implementation Notes

| Built-in | Signature | Used in phase |
|----------|-----------|---------------|
| findhom | (L, n, topshift) | Gauss AGM |
| findhomcombo | (f, L, n, topshift [, etaopt]) | Eisenstein |
| findnonhomcombo | (f, L, n_list, topshift [, etaopt]) | Watson |
| subs_q | (f, k) | θ(q²) = theta(subs_q(q,2), T) |
| legendre | (a, p) | Eisenstein χ(m) |
| etaq | (k, T) or (q, k, T) | All blocks |

All built-ins exist (Phase 8/10). Focus is exact commands and run-block layout.

---

## Verification

- **findhom:** Output contains "X" and both relations (grep for "X₁" or "X1").
- **findhomcombo:** Output contains polynomial with coefficients 1, 40, 335.
- **findnonhomcombo:** Output contains "343" and "49" (Watson coefficients).
