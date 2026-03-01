# q-Series Document: Maple Code & Exercises Checklist

---

## Part 1: All Maple Input Blocks

### 3.1 prodmake

**Block 1** (lines 206–210)
```maple
> with(qseries):
> x:=add(q^(n^2)/aqprod(q,q,n),n=0..8):
> series(x,q,50);
```
*Expected output: see line 212*
- [x] Verified

**Block 2** (lines 222–224)
```maple
> prodmake(x,q,40);
```
*Expected output: see line 226*
- [x] Verified

### 3.2 qfactor

**Block 3** (lines 264–280)
```maple
> with(qseries):
>   T:=proc(r,j)
>       option remember;
>       local x,k;
>       x:=0;
>       if j=0 or j=1 then
>           RETURN((j-1)^2):
>       else
>           for k from 1 to floor(j/2) do
>               x:=x-qbin(q,k,r+2*k)*T(r+2*k,j-2*k);
>           od:
>           RETURN(expand(x));
>       fi:
>   end:
>   t8:=T(8,8);
```
*Expected output: see line 282*
- [x] Verified

**Block 4** (lines 284–286)
```maple
> factor(t8);
```
*Expected output: see line 288*
- N/A — Maple `factor()` not available; qfactor subsumes this

**Block 5** (lines 290–292)
```maple
> qfactor(t8,20);
```
*Expected output: see line 294*
- [x] Verified

**Block 6** (lines 323–335)
```maple
> dixson:=proc(a,b,c,q)
>     local x,k,y;
>     x:=0: y:=min(a,b,c):
>     for k from -y to y do
>         x:=x+(-1)^(k)*q^(k*(3*k+1)/2)*
>         qbin(q,c+k,b+c)*qbin(q,a+k,c+a)*qbin(q,b+k,a+b);
>     od:
>     RETURN(x):
> end:
> dx := expand(dixson(5,5,5,q)):
> qfactor(dx,20);
```
*Expected output: see line 337*
- [x] Verified

### 3.3 etamake

**Block 7** (lines 367–370)
```maple
> t2:=theta2(q,100)/q^(1/4);
> etamake(t2,q,100);
```
*Expected output: see line 372*
- [x] Verified

**Block 8** (lines 379–382)
```maple
> t3:=theta3(q,100);
> etamake(t3,q,100);
```
*Expected output: see line 384*
- [x] Verified

**Block 9** (lines 391–394)
```maple
> t4:=theta4(q,100);
> etamake(t4,q,100);
```
*Expected output: see line 396*
- [x] Verified

**Block 10** (lines 448–450)
```maple
> omega:=RootOf(z^2+z+1=0);
```
- N/A — `RootOf` algebraic extension not supported

### 3.4 jacprodmake

**Block 11** (lines 482–490)
```maple
> with(qseries):
>   x:=1:
>   for n from 1 to 8 do
>       x:=x+q^(n*n)/aqprod(q,q,n):
>   od:
>   x:=series(x,q,50):
>   y:=jacprodmake(x,q,40);
```
*Expected output: see line 492*
- [x] Verified

**Block 12** (lines 499–501)
```maple
>   z:=jac2prod(y);
```
*Expected output: see line 503*
- [x] Verified

**Block 13** (lines 516–524)
```maple
> with(qseries):
> x:=0:
> for n from 0 to 10 do
>   x := x + q^(n*(n+1)/2)*aqprod(-q,q,n)/aqprod(q,q,2*n+1):
> od:
> x:=series(x,q,50):
> jp:=jacprodmake(x,q,50);
```
*Expected output: see line 526*
- [ ] FAIL — jacprodmake returns trivial product; expected half-integer Jacobi exponents not supported

**Block 14** (lines 532–534)
```maple
> jac2series(jp,500);
```
*Expected output: see line 536*
- [ ] FAIL — depends on Block 13 (jacprodmake fractional exponents)

### 4.1 findhom

**Block 15** (lines 574–577)
```maple
> with(qseries):
> findhom([theta3(q,100),theta4(q,100),theta3(q^2,100),theta4(q^2,100)],q,1,0);
```
*Expected output: see line 579*
- [x] Verified — correctly finds no degree-1 relation

**Block 16** (lines 584–586)
```maple
> findhom([theta3(q,100),theta4(q,100),theta3(q^2,100), theta4(q^2,100)],q,2,0);
```
*Expected output: see line 588*
- [x] Verified — finds Gauss AGM relations

### 4.2 findhomcombo

**Block 17** (lines 662–673)
```maple
> with(numtheory):
> UE:=proc(q,k,p,trunk)
>     local x,m,n:
>     x:=0:
>     for m from 1 to trunk do
>         for n from 1 to trunk/m do
>             x:=x + legendre(m,p)*n^(k-1)*q^(m*n):
>         od:
>     od:
> end:
```
- [x] Verified — computed via inline `sum(sum(legendre(m,5)*n^5*q^(m*n), ...))`

**Block 18** (lines 679–685)
```maple
> with(qseries):
> f := UE(q,6,5,50):
> B1 := etaq(q,1,50)^5/etaq(q,5,50):
> B2 := q*etaq(q,5,50)^5/etaq(q,1,50):
> findhomcombo(f,[B1,B2],q,3,0,yes);
```
*Expected output: see line 687*
- [x] Verified — outputs 335X₂³+40X₁X₂²+X₁²X₂

### 4.3 findnonhom

**Block 19** (lines 721–725)
```maple
> with(qseries):
> F := q -> theta3(q,500)/theta3(q^5,100):
> U := 2*q*theta(q^10,q^25,5)/theta3(q^25,20);
```
*Expected output: see line 727*
- [x] Verified — uses theta(q^10,q^25,100) for proper truncation

**Block 20** (lines 729–731)
```maple
> EQNS := findnonhom([F(q),F(q^5),U],q,3,20);
```
*Expected output: see line 733*
- [x] Verified — finds relation (note: findnonhom omits constant term -1)

**Block 21** (lines 738–740)
```maple
> ANS:=EQNS[1];
```
*Expected output: see line 742*
- N/A — result indexing (EQNS[1]) not applicable in REPL

**Block 22** (lines 747–750)
```maple
> CHECK := subs({X[1]=F(q),X[2]=F(q^5),X[3]=U},ANS):
> series(CHECK,q,500);
```
*Expected output: see line 752*
- [x] Verified — substituting known relation gives 0 + O(q⁵⁰)

### 4.4 findnonhomcombo

**Block 23** (lines 805–810)
```maple
> with(qseries):
> xi:=series(q^2*etaq(q,49,100)/etaq(q,1,100),q,101):
> T:=series(q*(etaq(q,7,100)/etaq(q,1,100))^4,q,101):
> findnonhomcombo(T^2,[T,xi],q,[1,7],0,no);
```
*Expected output: see line 812*
- [x] Verified — outputs Watson modular equation with 343X₂⁷ terms

**Block 24** (lines 817–819)
```maple
> collect(%[1],[X[1]]);
```
*Expected output: see line 821*
- N/A — `collect()` is a formatting function

### 4.5 findpoly

**Block 25** (lines 916–926)
```maple
> with(qseries):
> x1 := radsimp(theta2(q,100)^2/theta2(q^3,40)^2):
> x2 := theta3(q,100)^2/theta3(q^3,40)^2:
> x := x1+x2:
> c := q*etaq(q,3,100)^9/etaq(q,1,100)^3:
> a := radsimp(theta3(q,100)*theta3(q^3,40)+theta2(q,100)*theta2(q^3,40)):
> c := 3*q^(1/3)*etaq(q,3,100)^3/etaq(q,1,100):
> y := radsimp(c^3/a^3):
> P1:=findpoly(x,y,q,3,1,60);
```
*Expected output: see line 928*
- [ ] FAIL — cannot add series with different q-shifts (theta2/theta3 quotients)

### 5. Sifting Coefficients

**Block 26** (lines 1017–1021)
```maple
> with(qseries):
> PD:=series(etaq(q,2,200)/etaq(q,1,200),q,200):
> PD1:=sift(PD,q,5,1,199);
```
*Expected output: see line 1023*
- [x] Verified

**Block 27** (lines 1025–1027)
```maple
> etamake(PD1,q,38);
```
*Expected output: see line 1029*
- [x] Verified — outputs η(2τ)² η(5τ)³ / (q^(5/24) η(τ)⁴ η(10τ))

### 6.1 The Triple Product Identity

**Block 28** (lines 1070–1072)
```maple
> tripleprod(z,q,10);
```
*Expected output: see line 1074*
- [ ] Verified

**Block 29** (lines 1076–1078)
```maple
> tripleprod(q,q^3,10);
```
*Expected output: see line 1080*
- [ ] Verified

### 6.2 The Quintuple Product Identity

**Block 30** (lines 1112–1115)
```maple
> with(qseries):
> quinprod(z,q,prodid);
```
*Expected output: see line 1117*
- [ ] Verified

**Block 31** (lines 1119–1121)
```maple
> quinprod(z,q,seriesid);
```
*Expected output: see line 1123*
- [ ] Verified

**Block 32** (lines 1125–1127)
```maple
> quinprod(z,q,3);
```
*Expected output: see line 1129*
- [ ] Verified

**Block 33** (lines 1141–1145)
```maple
> with(qseries):
> EULER:=etaq(q,1,500):
> E0:=sift(EULER,q,5,0,499);
```
*Expected output: see line 1147*
- [ ] Verified

**Block 34** (lines 1149–1151)
```maple
> jp:=jacprodmake(E0,q,50);
```
*Expected output: see line 1153*
- [ ] Verified

**Block 35** (lines 1160–1162)
```maple
> jac2prod(jp);
```
*Expected output: see line 1164*
- [ ] Verified

**Block 36** (lines 1171–1174)
```maple
> qp:=quinprod(q,q^5,20):
> series(qp,q,100);
```
*Expected output: see line 1176*
- [ ] Verified

### 6.3 Winquist's Identity

**Block 37** (lines 1254–1262)
```maple
> with(qseries):
> Q:=n->tripleprod(q^n,q^33,10):
> A0:=Q(15):       A3:=Q(12):       A7:=Q(6):
> A8:=Q(3):        A9:=Q(9):
> B2:=Q(13)-q^3*Q(2):    B4:=Q(7)+q*Q(4):
> IDG:=series( ( A0*B2-q^2*A9*B4),q,200):
> series(IDG,q,10);
```
*Expected output: see line 1264*
- [ ] Verified

**Block 38** (lines 1269–1271)
```maple
> jp:=jacprodmake(IDG,q,50);
```
*Expected output: see line 1273*
- [ ] Verified

**Block 39** (lines 1280–1282)
```maple
> jac2prod(jp);
```
*Expected output: see line 1284*
- [ ] Verified

**Block 40** (lines 1289–1291)
```maple
> series(winquist(q^5,q^3,q^11,20),q,20);
```
*Expected output: see line 1293*
- [ ] Verified

**Block 41** (lines 1298–1300)
```maple
> series(IDG-winquist(q^5,q^3,q^11,20),q,60);
```
*Expected output: see line 1302*
- [ ] Verified

---

## Part 2: All Exercises

### Exercise 1 (line 236, from ### 3.1 prodmake)

**Exercise 1.** Find (and prove) a product form for the q-series:

```
  ∞     q^(n²)
  Σ  ──────────── .
 n=0  (q, q)_(2n)
```

The identity you find is originally due to Rogers [34, p.330]. See also Andrews [2, pp.38--39] for a list of some related papers.


---

### Exercise 2 (line 311, from ### 3.2 qfactor)

**EXERCISE 2.** Use `qfactor` to factorize T(r,n) for different values of r and n. Then write T(r,n) (defined above) as a q-product for general r and n.


---

### Exercise 3 (line 349, from ### 3.2 qfactor)

**EXERCISE 3.** Write the sum:

```
  ∞              k(3k+1)         3
  Σ    (-1)^k  q^(────────) [2a]
k=-∞                2       [a+k]
```

as a q-product for general integral a. The identity you obtain is a special case of [4, Eq.(4.24), p.38].


---

### Exercise 4 (line 429, from ### 3.3 etamake)

**EXERCISE 4.** Define the q-series:

```
             ∞       ∞
a(q) :=     Σ       Σ    q^(n² + nm + m²),
          n=-∞    m=-∞

             ∞       ∞
b(q) :=     Σ       Σ    ω^(n-m) q^(n² + nm + m²),
          n=-∞    m=-∞

             ∞       ∞       ((n+1/3)² + (n+1/3)(m+1/3) + (m+1/3)²)
c(q) :=     Σ       Σ    q^
          n=-∞    m=-∞
```

where `ω = exp(2πi/3)`. Two of the three functions above can be written as eta products. Can you find them?

*Hint:* It would be wise to define:
```maple
> omega:=RootOf(z^2+z+1=0);
```

See [12] for the answer and much more.


---

### Exercise 5 (line 550, from ### 3.4 jacprodmake)

**EXERCISE 5.** Use `jacprodmake` and `jac2series` to compute the q-series expansion of:

```
  ∞     q^(n(3n+1)/2)  (-q; q)_n
  Σ   ────────────────────────────
 n=0        (q, q)_(2n+1)
```

up to `q^1000`, assuming it is Jacobi-type product. Can you identify the infinite product? This function occurs in Slater's list [36, Eq.(46), p.156].


---

### Exercise 6 (line 612, from ### 4.1 findhom)

**EXERCISE 6.** Define `a(q)`, `b(q)`, `c(q)` as in Exercise 2. Find homogeneous relations between the functions `a(q)`, `b(q)`, `c(q)`, `a(q³)`, `b(q³)`, `c(q³)`.

In particular, try to get `a(q³)`, `b(q³)` and in terms of `a(q)`, `b(q)`. See [12] for more details. These results lead to a cubic analog of the AGM due to Jon and Peter Borwein [10], [11].


---

### Exercise 7 (line 703, from ### 4.2 findhomcombo)

**EXERCISE 7.** Define the following eta products:

```
       η(7τ)⁷                                      η(τ)⁷
C₁ := ──────── ,   C₂ := η(τ)³ η(7τ)³ ,   C₃ := ──────── .
        η(τ)                                        η(7τ)
```

What is the weight of these modular forms? Write `U_{7,3}` in terms of C₁, C₂, C₃. The identity that you should find was originally due to Ramanujan. Also see Fine [15, p. 159] and [19, Eq. (5.4)]. If you are ambitious find `U_{7,9}` in terms of C₁, C₂, C₃.


---

### Exercise 8 (line 836, from ### 4.4 findnonhomcombo)

**EXERCISE 8.** Define:

```
        η(25τ)
ξ := ──────────
         η(τ)
```

and

```
         ⎛ η(5τ) ⎞⁶
T :=     ⎜────── ⎟  .
         ⎝  η(τ) ⎠
```

Use `findnonhomcombo` to express T as a polynomial in ξ of degree 5. The modular equation you find was used by Watson to prove Ramanujan's partition congruences for powers of 5. See [23] for an elementary treatment.


---

### Exercise 9 (line 856, from ### 4.4 findnonhomcombo)

**EXERCISE 9.** Define `a(q)` and `c(q)` as in Exercise 2. Define:

```
          c(q)³
x(q) := ──────── ,
          a(q)³
```

and the classical Eisenstein series (usually called E₆; see [35, p. 93]):

```
                   ∞     n⁵ qⁿ
N(q) := 1 - 504   Σ   ──────── .
                  n=1   1 - qⁿ
```

Use `findnonhomcombo` to express N(q) in terms of a(q) and x(q).

*HINT:* N(q) is a modular form of weight 6 and a(q) and c(q) are modular forms of weight 1. See [8] for this result and many others.


---

### Exercise 10 (line 961, from ### 4.5 findpoly)

**EXERCISE 10.** Define:

```
        ⎛ θ_3(q)  ⎞²
m :=    ⎜──────── ⎟  .
        ⎝ θ_3(q³) ⎠
```

Use `polyfind` to find `y = c³/a³` as a rational function in m. The answer is Eq.(12.8) in [8].


---

### Exercise 11 (line 1048, from ## 5. Sifting Coefficients)

**EXERCISE 11.** Rodseth also found the generating functions for `pd(5n + r)` for r = 0, 1, 2, 3, and 4. For each r use `sift` and `jacprodmake` to identify these generating functions as infinite products.


---

### Exercise 12 (line 1193, from ### 6.2 The Quintuple Product Identity)

**EXERCISE 12.** (i) Use the quintuple product identity (6.3) and Euler's pentagonal number theorem to prove (6.4).
(ii) Use MAPLE to identify and prove product expressions for E₁ and E₂.
(iii) This time see if you can repeat (i), (ii) but split the exponent mod 7.
(iv) Can you generalize these results to arbitrary modulus? Atkin and Swinnerton-Dyer found a generalization. See Lemma 6 in [6].


---

### Exercise 13 (line 1320, from ### 6.3 Winquist's Identity)

**EXERCISE 13.**
(i) Prove (6.6) by using the triple product identity (6.1) to write the right side of Winquist's identity (6.5) as a sum of two products.
(ii) In a similar manner find and prove a product form for `A_0 B_0 - q³ A_7 B_4`.


---

