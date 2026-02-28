# Updated q-Product Tutorial for a q-Series Maple Package

**Author:** Frank Garvan

The original version of this tutorial was written Tuesday, December 15, 1998 and revised Friday, February 12, 1999 using Maple V (quite an old version of MAPLE).

**Recent updates:** 05.12.20, 06.10.20
**Current version of MAPLE:** MAPLE 2020.

> **Note:** The research for the original version was supported by the NSF under grant number DMS-9870052 and was *Dedicated to George E. Andrews on the occasion of his 60th Birthday.*

---

## 1. Introduction

In the study of q-series one is quite often interested in identifying generating functions as infinite products. The classic example is the Rogers-Ramanujan identity:

```
  ∞       n²        ∞              1
  Σ   ─────────  =  Π  ───────────────────────────
 n=0  (q, q)_n    n=1  (1 - q^(5n-1))(1 - q^(5n-4))
```

Here we have used the notation in (2.2). It can be shown that the left-side of this identity is the generating function for partitions whose parts differ by at least two. The identity is equivalent to saying the number of such partitions of n is equinumerous with partitions of n into parts congruent to ±1 (mod 5).

The main goals of the package are to provide facility for handling the following problems:

1. Conversion of a given q-series into an *infinite* product.
2. Factorization of a given rational function into a finite q-product if one exists.
3. Find algebraic relations (if they exist) among the q-series in a given list.

A q-product has the form:

```
  N
  Π  (1 - q^j)^(b_j)         (1.1)
 n=1
```

where `b_j` are integers.

In [4, section 10.7], George Andrews also considered Problems 1 and 2, and asked for an easily accessible implementation. We provide implementations as well as considering factorisations into theta-products and eta-products. The package provides some basic functions for computing q-series expansions of eta functions, theta functions, Gaussian polynomials and q-products. It also has a function for sifting out coefficients of a q-series. It also has the basic infinite product identities: the triple product identity, the quintuple product identity and Winquist's identity.

### 1.1 Installation instructions

The **qseries** package can be downloaded via the WWW. First use your favorite browser to access the URL:

<https://qseries.org/fgarvan/qmaple/qseries/index.html>

Then you can find directions for installing the package.

---

## 2. Basic Functions

We describe the basic functions in the package which are used to build q-series.

### 2.1. Finite q-products

#### 2.1.1. Rising q-factorial

`aqprod(a,q,n)` returns the product:

```
(a; q)_n = (1 - a)(1 - aq)(1 - aq²)···(1 - aq^(n-1))      (1.2)
```

We also use the notation:

```
              ∞
(a; q)_∞ =   Π  (1 - aq^(n-1)).
             n=1
```

#### 2.1.2 Gaussian polynomials

When `0 ≤ m ≤ n`, `qbin(q,m,n)` returns the Gaussian polynomial (or q-binomial coefficient):

```
 [n]       (q)_n
 [m]  = ───────────
        (q)_m (q)_(n-m)
```

otherwise it returns 0.

### 2.2 Infinite products

#### 2.2.1 Dedekind eta products

Suppose `Im(τ) > 0`, and `q = exp(2πiτ)`. The Dedekind eta function [27, p.121] is defined by:

```
                   πiτ     ∞
η(τ) = exp( ──── )  Π  (1 - exp(2πinτ))  =  q^(1/24)  Π  (1 - q^n).
                12      n=1                              n=1
```

`etaq(q,k,T)` returns the q-series expansion (up to `q^T`) of the eta product:

```
  ∞
  Π  (1 - q^(kn)).
 n=1
```

This corresponds to the eta function `η(kτ)` except for a power of q. Eta products occur frequently in the study of q-series. For example, the generating function for `p(n)`, the number of partitions of n, can be written as:

```
  ∞                  ∞        1
  Σ  p(n) q^n  =    Π  ─────────── .
 n=0                n=1  (1 - q^n)
```

See [1, pp. 3-4]. The generating function for the number of partitions of n that are p-cores [19], `a_p(n)`, can be written:

```
  ∞                  ∞    (1 - q^(pn))^p
  Σ  a_p(n) q^n  =  Π   ─────────────── .
 n=0                n=1    (1 - q^n)
```

Granville and Ono [21] proved a long-standing conjecture in group representation theory using elementary and function-theoretic properties of the eta product above.

#### 2.2.2. Theta functions

Jacobi [24, Vol I, pp. 497--538] defined four theta functions `θ_i(z, q)`, i=1,2,3,4. See also [41, Ch. XXI]. Each theta function can be written in terms of the others using a simple change of variables. For this reason, it is common to define:

```
                ∞
θ(z, q) :=     Σ   z^n  q^(n²).
              n=-∞
```

`theta(z,q,T)` returns the truncated theta-series:

```
   T
   Σ   z^n  q^(n²).
  n=-T
```

The case z = 1 of Jacobi's theta functions occurs quite frequently. We define:

```
              ∞
θ_2(q) :=    Σ    q^((n + 1/2)²),
            n=-∞

              ∞
θ_3(q) :=    Σ    q^(n²),
            n=-∞

              ∞
θ_4(q) :=    Σ    (-1)^n  q^(n²).
            n=-∞
```

`theta2(q,T)`, `theta3(q,T)`, `theta4(q,T)` (resp.) returns the q-series expansion to order `O(q^T)` of `θ_2(q)`, `θ_3(q)`, `θ_4(q)`, respectively.

Let a, and b be positive integers and suppose |q|<1. Infinite products of the form:

```
(q^a; q^b)_∞  (q^(b-a); q^b)_∞
```

occur quite frequently in the theory of partitions and q-series. For example the right side of the Rogers-Ramanujan identity is the reciprocal of the product with `(a, b) = (1, 5)`. In (3.4) we will see how the function `jacprodmake` can be used to identify such products.

---

## 3. Product Conversion

In [1, p. 233], [3, section 10.7] there is a very nice and useful algorithm for converting a q-series into an infinite product. Any given q-series may be written formally as an infinite product:

```
            ∞                ∞            -a_n
1  +   Σ  b_n  q^n   =     Π   (1 - q^n)      .
          n=1              n=1
```

Here we assume the `b_n` are integers. By taking the logarithmic derivative of both sides we can obtain the recurrence:

```
              n
n * b_n  =   Σ   b_(n-j)  Σ   d * a_d .
             j=1           d|n
```

Letting `a_n = 1` we obtain the well-known special case:

```
                 n
n * p(n)  =     Σ   p(n - j)  σ(j).
                j=1
```

We can also easily construct a recurrence for the `a_n` from the recurrence above.

The function `prodmake` is an implementation of Andrews' algorithm. Other related functions are `etamake` and `jacprodmake`.

### 3.1 prodmake

`prodmake(f,q,T)` converts the q-series f into an infinite product that agrees with f to `O(q^T)`. Let's take a look at the left side of the Rogers-Ramanujan identity.

```maple
> with(qseries):
> x:=add(q^(n^2)/aqprod(q,q,n),n=0..8):
> series(x,q,50);
```

Output (1):
```
1 + q + q² + q³ + 2q⁴ + 2q⁵ + 3q⁶ + 3q⁷ + 4q⁸ + 5q⁹ + 6q¹⁰ + 7q¹¹ + 9q¹² + 10q¹³
  + 12q¹⁴ + 14q¹⁵ + 17q¹⁶ + 19q¹⁷ + 23q¹⁸ + 26q¹⁹ + 31q²⁰ + 35q²¹ + 41q²²
  + 46q²³ + 54q²⁴ + 61q²⁵ + 70q²⁶ + 79q²⁷ + 91q²⁸ + 102q²⁹ + 117q³⁰ + 131q³¹
  + 149q³² + 167q³³ + 189q³⁴ + 211q³⁵ + 239q³⁶ + 266q³⁷ + 299q³⁸ + 333q³⁹
  + 374q⁴⁰ + 415q⁴¹ + 465q⁴² + 515q⁴³ + 575q⁴⁴ + 637q⁴⁵ + 709q⁴⁶ + 783q⁴⁷
  + 871q⁴⁸ + 961q⁴⁹ + O(q⁵⁰)
```

```maple
> prodmake(x,q,40);
```

Output (2):
```
1 / ((1-q)(-q⁴+1)(-q⁶+1)(-q⁹+1)(-q¹¹+1)(-q¹⁴+1)(-q¹⁶+1)(-q¹⁹+1)
     (-q²¹+1)(-q²⁴+1)(-q²⁶+1)(-q²⁹+1)(-q³¹+1)(-q³⁴+1)(-q³⁶+1)(-q³⁹+1))
```

We have rediscovered the right side of the Rogers-Ramanujan identity!

---

**Exercise 1.** Find (and prove) a product form for the q-series:

```
  ∞     q^(n²)
  Σ  ──────────── .
 n=0  (q, q)_(2n)
```

The identity you find is originally due to Rogers [34, p.330]. See also Andrews [2, pp.38--39] for a list of some related papers.

---

### 3.2 qfactor

The function `qfactor` is a version of `prodmake`.

`qfactor(f,T)` attempts to write a rational function f in q as a q-product, i.e., as a product of terms of the form `(1 - q^i)`. The second argument `T` is optional. It specifies an upper bound for the exponents of q that can occur in the product. If `T` is not specified it is given a default value of `4d+3` where d is the maximum of the degree in q of the numerator and denominator. The algorithm is quite simple. First the function is factored as usual, and then it uses `prodmake` to do further factorisation into q-products. Thus even if only part of the function can be written as a q-product `qfactor` is able to find it.

As an example we consider some rational functions `T(r, h)` introduced by Andrews [4, p.14] to explain Rogers's [34] first proof of the Rogers-Ramanujan identities. The `T(r, n)` are defined recursively as follows:

```
(3.3)  T(r, 0) = 1,
(3.4)  T(r, 1) = 0,

(3.5)  T(r, N) = -  Σ        [r + 2j]  T(r + 2j, N - 2j).
                  1≤2j≤N    [  j   ]
```

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

Output (3): *(polynomial in q of degree 42)*

```maple
> factor(t8);
```

Output (4): *(factored into cyclotomic polynomials)*

```maple
> qfactor(t8,20);
```

Output (5):
```
q⁶(-q¹⁰+1)(-q¹¹+1)(-q⁹+1)(-q¹⁶+1)
─────────────────────────────────────
(1-q)(-q²+1)(-q⁴+1)(-q³+1)
```

Observe how we used `factor` to factor `t8` into cyclotomic polynomials. However, `qfactor` was able to factor `t8` as a q-product. We see that:

```
            (q⁹; q)_3  (1 - q¹⁶) q⁶
T(8, 8) = ─────────────────────────── .
                  (q; q)_4
```

---

**EXERCISE 2.** Use `qfactor` to factorize T(r,n) for different values of r and n. Then write T(r,n) (defined above) as a q-product for general r and n.

---

For our next example we examine the sum:

```
  ∞              k(3k+1)
  Σ    (-1)^k  q^(────────)  [b+c]  [c+a]  [a+b]
k=-∞                2        [c+k]  [a+k]  [b+k]
```

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

Output (6): *(q-product factorization)*

We find that:

```
  ∞              k(3k+1)         3          (q⁶; q)_10
  Σ    (-1)^k  q^(────────) [10]    =  ──────────── .
k=-∞                2       [5+k]        (q; q)_5²
```

---

**EXERCISE 3.** Write the sum:

```
  ∞              k(3k+1)         3
  Σ    (-1)^k  q^(────────) [2a]
k=-∞                2       [a+k]
```

as a q-product for general integral a. The identity you obtain is a special case of [4, Eq.(4.24), p.38].

---

### 3.3 etamake

Recall from (2.2.1) that `etaq` is the function to use for computing q-expansions of eta products. If one wants to apply the theory of modular forms to q-series it is quite useful to determine whether a given q-series is a product of eta functions. The function in the package for doing this conversion is `etamake`.

`etamake(f,q,T)` will write the given q-series f as a product of eta functions which agrees with f up to `q^T`. As an example, let's see how we can write the theta functions as eta products.

```maple
> t2:=theta2(q,100)/q^(1/4);
> etamake(t2,q,100);
```

Output (8):
```
  2 η(4τ)²
  ──────────
  q^(1/4) η(2τ)
```

```maple
> t3:=theta3(q,100);
> etamake(t3,q,100);
```

Output (10):
```
    η(2τ)⁵
  ─────────────
  η(4τ)² η(τ)²
```

```maple
> t4:=theta4(q,100);
> etamake(t4,q,100);
```

Output (12):
```
  η(τ)²
  ──────
  η(2τ)
```

We are led to the well-known identities:

```
              2 η(4τ)²
θ_2(q) :=  ────────── ,
               η(2τ)

               η(2τ)⁵
θ_3(q) :=  ───────────── ,
            η(4τ)² η(τ)²

              η(τ)²
θ_4(q) :=  ──────── .
              η(2τ)
```

The idea of the algorithm is quite simple. Given a q-series f (say with leading coefficient 1) one just keeps recursively multiplying by powers of the right eta function until the desired terms agree. For example, suppose we are given a q-series:

```
1 + c*q^k + ···
```

Then the next step is to multiply by `etaq(q,k,T)^(-c)`.

---

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

### 3.4 jacprodmake

In (2.2.2) we observed that the right side of the Rogers-Ramanujan identity could be written in terms of a Jacobi product. The function `jacprodmake` converts a q-series into a Jacobi-type product if one exists. Given a q-series f, `jacprodmake(f,q,T)` attempts to convert f into a product of theta functions that agrees with f to order `O(q^T)`. Each theta-function has the form `JAC(a,b,infinity)`, where a, b are integers and `0 ≤ a < b`. If `0 < a`, then `JAC(a,b,infinity)` corresponds to the theta-product:

```
(q^a; q^b)_∞  (q^(b-a); q^b)_∞  (q^b; q^b)_∞ .
```

We call this a theta product because it is `θ(-q^((b-2a)/2), q^(b/2))`.

The `jacprodmake` function is really a variant of `prodmake`. It involves using `prodmake` to compute the sequence of exponents and then searching for periodicity.

If `a = 0`, then `JAC(0,b,infinity)` corresponds to the eta-product:

```
(q^b; q^b)_∞ .
```

We note that this product can also be thought of as a theta-product since `(q^b; q^b)_∞` can be written:

```
(q^b; q^b)_∞ = (q^b; q^(3b))_∞  (q^(2b); q^(3b))_∞  (q^(3b); q^(3b))_∞ .
```

Let's re-examine the Rogers-Ramanujan identity.

```maple
> with(qseries):
>   x:=1:
>   for n from 1 to 8 do
>       x:=x+q^(n*n)/aqprod(q,q,n):
>   od:
>   x:=series(x,q,50):
>   y:=jacprodmake(x,q,40);
```

Output (13):
```
       JAC(0, 5, ∞)
y := ──────────────
       JAC(1, 5, ∞)
```

```maple
>   z:=jac2prod(y);
```

Output (14):
```
            1
z := ─────────────────────
     (q, q⁵)_∞  (q⁴, q⁵)_∞
```

Note that we were able to observe that the left side of the Rogers-Ramanujan identity (at least up through `q^40`) can be written as a quotient of theta functions. We used the function `jac2prod`, to simplify the result and get it into a more recognizable form. The function `jac2prod(jacexpr)` converts a product of theta functions into q-product form; i.e., as a product of functions of the form `(q^a; q^b)_∞`.

Here `jacexpr` is a product (or quotient) of terms `JAC(i,j,infinity)`, where i, j are integers and `0 ≤ i < j`.

A related function is `jac2series`. This converts a Jacobi-type product into a form better for computing its q-series. It simply replaces each Jacobi-type product with its corresponding theta-series.

```maple
> with(qseries):
> x:=0:
> for n from 0 to 10 do
>   x := x + q^(n*(n+1)/2)*aqprod(-q,q,n)/aqprod(q,q,2*n+1):
> od:
> x:=series(x,q,50):
> jp:=jacprodmake(x,q,50);
```

Output (15):
```
jp := JAC(0,14,∞)^(13|2) / (JAC(1,14,∞)² JAC(3,14,∞) JAC(4,14,∞)
      JAC(5,14,∞) JAC(6,14,∞) √JAC(7,14,∞))
```

```maple
> jac2series(jp,500);
```

Output (16): *(large expression involving products of theta-series)*

It seems that the q-series:

```
  ∞     q^(n(n+1)/2) (-q; q)_n
  Σ   ──────────────────────────
 n=0       (q, q)_(2n+1)
```

can be written as Jacobi-type product. Assuming that this is the case we used `jac2series` to write this q-series in terms of theta-series at least up to `q^500`. This should provide an efficient method for computing the q-series expansion and also for computing the function at particular values of q.

---

**EXERCISE 5.** Use `jacprodmake` and `jac2series` to compute the q-series expansion of:

```
  ∞     q^(n(3n+1)/2)  (-q; q)_n
  Σ   ────────────────────────────
 n=0        (q, q)_(2n+1)
```

up to `q^1000`, assuming it is Jacobi-type product. Can you identify the infinite product? This function occurs in Slater's list [36, Eq.(46), p.156].

---

## 4. The Search for Relations

The functions for finding relations between q-series are `findhom`, `findhomcombo`, `findnonhom`, `findnonhomcombo`, and `findpoly`.

### 4.1 findhom

If the q-series one is concerned with are modular forms of a particular weight, then theoretically these functions will satisfy homogeneous polynomial relations. See [18, p. 263], for more details and examples.

The function `findhom(L,q,n,topshift)` returns a set of potential homogeneous relations of order n among the q-series in the list L. The value of `topshift` is usually taken to be zero. However if it appears that spurious relations are being generated then a higher value of `topshift` should be taken.

The idea is to convert this into a linear algebra problem. This program generates a list of monomials of degree n of the functions in the given list of q-series L. The q-expansion (up to a certain point) of each monomial is found and converted into a row vector of a matrix. The set of relations is then found by computing the kernel of the transpose of this matrix. As an example, we now consider relations between the theta functions `θ_3(q)`, `θ_4(q)`, `θ_3(q²)`, and `θ_4(q²)`.

```maple
> with(qseries):
> findhom([theta3(q,100),theta4(q,100),theta3(q^2,100),theta4(q^2,100)],q,1,0);
```

Output (17):
```
{∅}
```

```maple
> findhom([theta3(q,100),theta4(q,100),theta3(q^2,100), theta4(q^2,100)],q,2,0);
```

Output (18):
```
{-X₁X₂ + X₄², X₁² + X₂² - 2X₃²}
```

From the session above we see that there is no linear relation between the functions `θ_3(q)`, `θ_4(q)`, `θ_3(q²)`, and `θ_4(q²)`. However, it appears that there are two quadratic relations:

```
                    ┌──────────────────
                   /  θ_3(q)² + θ_4(q)²
θ_3(q²) =        / ────────────────────
                \/          2
```

and

```
θ_4(q²) = √(θ_3(q)² θ_4(q)²) .
```

This is Gauss' parametrization of the arithmetic-geometric mean iteration. See [13, Ch 2] for details.

---

**EXERCISE 6.** Define `a(q)`, `b(q)`, `c(q)` as in Exercise 2. Find homogeneous relations between the functions `a(q)`, `b(q)`, `c(q)`, `a(q³)`, `b(q³)`, `c(q³)`.

In particular, try to get `a(q³)`, `b(q³)` and in terms of `a(q)`, `b(q)`. See [12] for more details. These results lead to a cubic analog of the AGM due to Jon and Peter Borwein [10], [11].

---

### 4.2 findhomcombo

The function `findhomcombo` is a variant of `findhom`.

Suppose f is a q-series and L is a list of q-series.

`findhomcombo(f,L,q,n,topshift,etaoption)` tries to express f as a homogeneous polynomial in the members of L. If `etaoption=yes` then each monomial in the combination is *converted* into an eta-product using `etamake`.

We illustrate this function with certain Eisenstein series.

For p an odd prime define:

```
χ(m) = (m/p)    (the Legendre symbol).
```

Suppose k is an integer, `k ≥ 2`, and `(p-1)/2 ≡ k (mod 2)`.

Define the Eisenstein series:

```
                ∞     ∞
U_{p,k}(q) :=  Σ     Σ   χ(m) n^(k-1) q^(mn).
               m=1   n=1
```

Then `U_{p,k}(q)` is a modular form of weight k and character χ for the congruence subgroup `Γ_0(p)`. See [28], [20] for more details. The classical result is the following identity found by Ramanujan [32, Eq. (1.52), p. 354]:

```
              η(5τ)⁵
U_{5,2} = ────────── .
              η(τ)
```

Kolberg [28] has found many relations between such Eisenstein series and certain eta products. The eta function `η(τ)` is a modular form of weight 1/2 [27, p.121]. Hence the modular forms:

```
       η(5τ)⁵              η(τ)⁵
B₁ := ──────── ,   B₂ := ──────── 
        η(τ)               η(5τ)
```

are modular forms of weight 2. In fact, it can be shown that they are modular forms on `Γ_0(p)` with character `(·/5)`. We might therefore expect that `U_{5,6}` can be written as a homogeneous cubic polynomial in B₁ and B₂. We write a short maple program to compute the Eisenstein series.

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

The function `UE(q,k,p,trunk)` returns the q-expansion of `U_{p,k}` up through `q^trunk`. We note that `legendre(m,p)` returns the Legendre symbol `(m/p)`.

We are now ready to study `U_{5,6}`.

```maple
> with(qseries):
> f := UE(q,6,5,50):
> B1 := etaq(q,1,50)^5/etaq(q,5,50):
> B2 := q*etaq(q,5,50)^5/etaq(q,1,50):
> findhomcombo(f,[B1,B2],q,3,0,yes);
```

Output (19):
```
{η(5τ)³ η(τ)⁹ + 40 η(5τ)⁹ η(τ)³ + 335 η(5τ)¹⁵ / η(τ)³}
{X₁² X₂ + 40 X₁ X₂² + 335 X₂³}
```

It would appear that:

```
U_{5,6} = η(5τ)³ η(τ)⁹ + 40 η(5τ)⁹ η(τ)³ + 335 η(5τ)¹⁵ / η(τ)³ .
```

The proof is a straightforward exercise using the theory of modular forms.

---

**EXERCISE 7.** Define the following eta products:

```
       η(7τ)⁷                                      η(τ)⁷
C₁ := ──────── ,   C₂ := η(τ)³ η(7τ)³ ,   C₃ := ──────── .
        η(τ)                                        η(7τ)
```

What is the weight of these modular forms? Write `U_{7,3}` in terms of C₁, C₂, C₃. The identity that you should find was originally due to Ramanujan. Also see Fine [15, p. 159] and [19, Eq. (5.4)]. If you are ambitious find `U_{7,9}` in terms of C₁, C₂, C₃.

---

### 4.3 findnonhom

In section 4.1 we introduced the function `findhom` to find homogeneous relations between q-series. The nonhomogeneous analog is `findnonhom`. The syntax of `findnonhom` is the same as `findhom`.

Typically (but not necessarily) `findhom` is used to find relations between modular forms of a certain weight. To find relations between modular functions we would use `findnonhom`. We consider an example involving theta functions.

```maple
> with(qseries):
> F := q -> theta3(q,500)/theta3(q^5,100):
> U := 2*q*theta(q^10,q^25,5)/theta3(q^25,20);
```

Output (20): *(expression for U)*

```maple
> EQNS := findnonhom([F(q),F(q^5),U],q,3,20);
```

Output (21):
```
EQNS := {-X₁ X₂ X₃ + X₂² + X₃² + X₃ - 1}
```

```maple
> ANS:=EQNS[1];
```

Output (22):
```
ANS := -X₁ X₂ X₃ + X₂² + X₃² + X₃ - 1
```

```maple
> CHECK := subs({X[1]=F(q),X[2]=F(q^5),X[3]=U},ANS):
> series(CHECK,q,500);
```

Output (23):
```
O(q⁵⁰⁰)
```

We define:

```
            θ_3(q)
F(q) := ────────────
          θ_3(q⁵)
```

and

```
            ∞
            Σ     q^(25n² + 10n + 1)
           n=-∞
U(q) := ────────────────────────────── .
               θ_3(q²⁵)
```

We note that `U(q)` and `F(q)` are modular functions since they are ratios of theta series. From the session above we see that it appears that:

```
1 + F(q) F(q⁵) U(q) = F(q⁵)² + U(q)² + U(q).
```

Observe how we were able to verify this equation to high order.

When `findnonhom` returns a set of relations the variable X has been declared *global*. This is so we can manipulate the relations. It this way we were able to assign `ANS` to the relation found and then use `subs` and `series` to check it to order `O(q⁵⁰⁰)`.

### 4.4 findnonhomcombo

The syntax of `findnonhomcombo` is the same as `findhomcombo`. We consider an example involving eta functions. First we define:

```
        η(49τ)
ξ := ──────────
         η(τ)
```

and

```
         ⎛ η(7τ) ⎞⁴
T :=     ⎜────── ⎟  .
         ⎝  η(τ) ⎠
```

Using the theory of modular functions it can be shown that one must be able to write `T²` in terms of T and powers of ξ. We now use `findnonhomcombo` to get `T²` in terms of T and ξ.

```maple
> with(qseries):
> xi:=series(q^2*etaq(q,49,100)/etaq(q,1,100),q,101):
> T:=series(q*(etaq(q,7,100)/etaq(q,1,100))^4,q,101):
> findnonhomcombo(T^2,[T,xi],q,[1,7],0,no);
```

Output (24):
```
{343 X₂⁷ + 343 X₂⁶ + 147 X₂⁵ + 49 X₁ X₂³ + 49 X₂⁴ + 35 X₁ X₂² + 21 X₂³ + 7 X₁ X₂ + 7 X₂² + X₂}
```

```maple
> collect(%[1],[X[1]]);
```

Output (25):
```
(49 X₂³ + 35 X₂² + 7 X₂) X₁ + 343 X₂⁷ + 343 X₂⁶ + 147 X₂⁵ + 49 X₂⁴ + 21 X₂³ + 7 X₂² + X₂
```

Then it seems that:

```
T² = (49ξ³ + 35ξ² + 7ξ) T + 343ξ⁷ + 343ξ⁶ + 147ξ⁵ + 49ξ⁴ + 21ξ³ + 7ξ² + ξ .
```

This is the modular equation used by Watson [41] to prove Ramanujan's partition congruences for powers of 7. Also see [5] and [26], and see [16] for an elementary treatment.

---

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

### 4.5 findpoly

The function `findpoly` is used to find a polynomial relation between two given q-series with degrees specified.

`findpoly(x,y,q,deg1,deg2,check)` returns a possible polynomial in X, Y (with corresponding degrees deg1, deg2) which is satisfied by the q-series x and y. If `check` is assigned then the relation is checked to `O(q^check)`.

We illustrate this function with an example involving theta functions and the function `a(q)` and `c(q)` encountered in Exercises 2 and 7. It can be shown that:

```
a(q) = θ_3(q) θ_3(q³) + θ_2(q) θ_2(q³) .
```

See [12] for details. This equation provides a better way of computing the q-series expansion of `a(q)` than the definition. In Exercise 2 you would have found that:

```
              η(3τ)³
c(q) = 3  ────────── .
               η(τ)
```

See [12] for a proof. Define:

```
       c³
y := ────
       a³
```

and

```
       ⎛ θ_2(q)  ⎞²     ⎛ θ_3(q)  ⎞²
x :=   ⎜──────── ⎟   +  ⎜──────── ⎟  .
       ⎝ θ_2(q³) ⎠      ⎝ θ_3(q³) ⎠
```

We use `findpoly` to find a polynomial relation between x and y.

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

Output:
```
`WARNING: X,Y are global.`
dims, 8, 18
The polynomial is
(X + 6)³ Y - 27 (X + 2)²
Checking to order, 60
O(q⁵⁹)
```

Output (26):
```
P1 := (X + 6)³ Y - 27 (X + 2)²
```

It seems that x and y satisfy the equation:

```
p(x, y) = (x + 6)³ y - 27·(x + 2)² = 0.
```

Therefore it would seem that:

```
  c³      27·(x + 2)²
─── = ──────────────── .
  a³       (x + 6)³
```

See [8, pp. 4237-4240] for more details.

---

**EXERCISE 10.** Define:

```
        ⎛ θ_3(q)  ⎞²
m :=    ⎜──────── ⎟  .
        ⎝ θ_3(q³) ⎠
```

Use `polyfind` to find `y = c³/a³` as a rational function in m. The answer is Eq.(12.8) in [8].

---

## 5. Sifting Coefficients

Suppose we are given a q-series:

```
            ∞
A(q)  =    Σ   a_n  q^n .
           n=0
```

Occasionally it will turn out the generating function:

```
  ∞
  Σ   a_{mn+r}  q^n
 n=0
```

will have a very nice form. A famous example for `p(n)` is due to Ramanujan:

```
  ∞                       ∞    (1 - q^(5n))⁶
  Σ   p(5n + 4) q^n  = 5 Π   ─────────────── .
 n=0                     n=1    (1 - q^n)⁵
```

See [1, Cor. 10.6]. In fact, G.H. Hardy and Major MacMahon [31, p. xxxv] both agreed that this is Ramanujan's most beautiful identity.

Suppose s is the q-series `Σ a_i q^i + O(q^T)`, then `sift(s,q,n,k,T)` returns the q-series:

```
Σ  a_{ni+k}  q^i  +  O(q^(T/n))
```

We illustrate this function with another example from the theory of partitions. Let `pd(n)` denote the number of partitions of n into distinct parts. Then it is well known that:

```
  ∞                 ∞                    ∞    (1 - q^(2n))
  Σ  pd(n) q^n  =  Π  (1 + q^n)  =    Π   ─────────────── .
 n=0               n=1                 n=1    (1 - q^n)
```

We now examine the generating function of `pd(5n + 1)` in MAPLE.

```maple
> with(qseries):
> PD:=series(etaq(q,2,200)/etaq(q,1,200),q,200):
> PD1:=sift(PD,q,5,1,199);
```

Output (27): *(polynomial in q)*

```maple
> etamake(PD1,q,38);
```

Output (28):
```
  η(5τ)³  η(2τ)²
──────────────────────
 q^(5/24) η(10τ) η(τ)⁴
```

So it would seem that:

```
  ∞                        ∞    (1 - q^(5n))³ (1 - q^(2n))²
  Σ  pd(5n + 1) q^n   =  Π   ──────────────────────────────── .
 n=0                      n=1    (1 - q^(10n))(1 - q^n)⁴
```

This result was found originally by Rodseth [33].

---

**EXERCISE 11.** Rodseth also found the generating functions for `pd(5n + r)` for r = 0, 1, 2, 3, and 4. For each r use `sift` and `jacprodmake` to identify these generating functions as infinite products.

---

## 6. Product Identities

At present, the package contains the Triple Product identity, the Quintuple Product identity and Winquist's identity. These are the most commonly used of the Macdonald identities [30], [37], [38]. The Macdonald identities are the analogs of the Weyl denominator for affine roots systems. Hopefully, a later version of this package will include these more general identities.

### 6.1 The Triple Product Identity

The triple product identity is:

```
  ∞                n(n-1)       ∞
  Σ   (-1)^n z^n q^(─────)  =  Π  (1 - zq^(n-1))(1 - z⁻¹q^n)(1 - q^n).   (6.1)
n=-∞                 2         n=1
```

where `z ≠ 0` and `|q| < 1`. The Triple Product Identity is originally due to Jacobi [24, Vol I]. The first combinatorial proof of the triple product identity is due to Sylvester [39]. Recently, Andrews [3] and Lewis [29] have found nice combinatorial proofs. The triple product occurs frequently in the theory of partitions. For instance, most proofs of the Rogers-Ramanujan identity crucially depend on the triple product identity.

`tripleprod(z,q,T)` returns the q-series expansion to order `O(q^T)` of Jacobi's triple product (6.1). This expansion is found by simply truncating the right side of (6.1).

```maple
> tripleprod(z,q,10);
```

Output (29): *(Laurent series in z with q coefficients)*

```maple
> tripleprod(q,q^3,10);
```

Output (30):
```
q⁵⁷ + q⁵¹ - q⁴⁰ - q³⁵ + q²⁶ + q²² - q¹⁵ - q¹² + q⁷ + q⁵ - q² - q + 1
```

The last calculation is an illustration of Euler's Pentagonal Number Theorem [1, Cor. 1.7 p.11]:

```
  ∞                ∞                                          ∞              n(3n-1)
  Π  (1 - q^n) =  Π  (1 - q^(3n-1))(1 - q^(3n-2))(1-q^3n) = Σ   (-1)^n  q^(─────────)   (6.2)
 n=1              n=1                                        n=-∞               2
```

### 6.2 The Quintuple Product Identity

The following identity is the Quintuple Product Identity:

```
                                                           ∞                          n(3n+1)
(-z; q)_∞ (-q/z; q)_∞ (z²q; q²)_∞ (q/z²; q²)_∞ (q;q)_∞ = Σ  ((-z)^(-3n) - (-z)^(3n+1)) q^(────────)
                                                          n=-∞                            2
```
(6.3)

Here `|q| < 1` and `z ≠ 0`. This identity is the BC₁ case of the Macdonald identities [30]. The quintuple product identity is usually attributed to Watson [40]. However it can be found in Ramanujan's lost notebook [32, p. 207]. Also see [7] for more history and some proofs.

The function `quinprod(z,q,T)` returns the quintuple product identity in different forms:

- (i) If T is a positive integer it returns the q-expansion of the right side of (6.3) to order `O(q^T)`.
- (ii) If `T = prodid` then `quinprod(z,q,prodid)` returns the quintuple product identity in product form.
- (iii) If `T = seriesid` then `quinprod(z,q,seriesid)` returns the quintuple product identity in series form.

```maple
> with(qseries):
> quinprod(z,q,prodid);
```

Output (31): *(product identity form)*

```maple
> quinprod(z,q,seriesid);
```

Output (32): *(series identity form)*

```maple
> quinprod(z,q,3);
```

Output (33): *(truncated q-expansion)*

Let's examine a more interesting application. Euler's infinite product may be dissected according to the residue of the exponent of q mod 5:

```
  ∞
  Π (1 - q^n) = E_0(q⁵) + q E_1(q⁵) + q² E_2(q⁵) + q³ E_3(q⁵) + q⁴ E_4(q⁵)
 n=1
```

By (6.2) we see that `E_3 = E_4 = 0` since `n(3n-1)/2 ≢ 3 or 4 (mod 5)`. Let's see if we can identify E_0.

```maple
> with(qseries):
> EULER:=etaq(q,1,500):
> E0:=sift(EULER,q,5,0,499);
```

Output (34): *(polynomial in q)*

```maple
> jp:=jacprodmake(E0,q,50);
```

Output (35):
```
       JAC(2, 5, ∞)  JAC(0, 5, ∞)
jp := ─────────────────────────────
             JAC(1, 5, ∞)
```

```maple
> jac2prod(jp);
```

Output (36):
```
  (q⁵; q⁵)_∞  (q²; q⁵)_∞  (q³; q⁵)_∞
  ─────────────────────────────────────
       (q; q⁵)_∞  (q⁴; q⁵)_∞
```

```maple
> qp:=quinprod(q,q^5,20):
> series(qp,q,100);
```

Output (37):
```
1 + q - q³ - q⁷ - q⁸ - q¹⁴ + q²⁰ + q²⁹ + q³¹ + q⁴² - q⁵² - q⁶⁶ - q⁶⁹ - q⁸⁵ + O(q⁹⁹)
```

From our maple session it appears that:

```
          (q⁵; q⁵)_∞ (q²; q⁵)_∞ (q³; q⁵)_∞
E_0  =  ─────────────────────────────────────     (6.4)
              (q; q⁵)_∞ (q⁴; q⁵)_∞
```

and that this product can be gotten by replacing q by `q⁵` and z by q in the product side of the quintuple product identity (6.3).

---

**EXERCISE 12.** (i) Use the quintuple product identity (6.3) and Euler's pentagonal number theorem to prove (6.4).
(ii) Use MAPLE to identify and prove product expressions for E₁ and E₂.
(iii) This time see if you can repeat (i), (ii) but split the exponent mod 7.
(iv) Can you generalize these results to arbitrary modulus? Atkin and Swinnerton-Dyer found a generalization. See Lemma 6 in [6].

---

### 6.3 Winquist's Identity

Back in 1969, Lasse Winquist [43] discovered a remarkable identity:

```
(a;q)_∞ (a⁻¹q;q)_∞ (b;q)_∞ (b⁻¹q;q)_∞ (ab;q)_∞ ((ab)⁻¹q;q)_∞ (ab⁻¹;q)_∞ (a⁻¹bq;q)_∞ (q;q)²_∞

      ∞       ∞
 =    Σ       Σ   (-1)^(n+m) ((a^(-3n) - a^(3n+3))(b^(-3m) - b^(3m+1))
    n=0    m=-∞
                                            3n(n+1)     m(3m+1)
     + (a^(-3m+1) - a^(3m+2))(b^(3n+2) - b^(-3n-1))) q^(──────── + ────────)
                                                  2           2
```
(6.5)

By dividing both sides by `(1-a)(1-b)` and letting `a, b → 1` he was able to express the product:

```
  ∞
  Π  (1 - q^n)^10
 n=1
```

as a double series and prove Ramanujan's partition congruence:

```
p(11n + 6) ≡ 0 (mod 11).
```

This was probably the first truly elementary proof of Ramanujan's congruence modulo 11. The interested reader should see Dyson's article [14] for some fascinating history on identities for powers of the Dedekind eta function and how they led to the Macdonald identities. A new proof of Winquist's identity has been found recently by S.-Y. Kang [25]. Mike Hirschhorn [22] has found a four-parameter generalization of Winquist's identity.

The function `winquist(a,b,q,T)` returns the q-expansion of the right side of (6.5) to order `O(q^T)`.

We close with an example. For `1 < k < 33` define:

```
Q(k) := (q^k; q³³)_∞ (q^(33-k); q³³)_∞ (q³³; q³³)_∞ .
```

Now define the following functions:

```
A_0 = Q(15),   A_3 = Q(12),   A_7 = Q(6),   A_8 = Q(3),   A_9 = Q(9).

B_0 = Q(16) - q² Q(5),
B_1 = Q(14) - q Q(8),
B_2 = Q(13) - q³ Q(2),
B_4 = Q(7) + q Q(4),
B_7 = Q(10) + q³ Q(1).
```

These functions occur in Theorem 6.7 of [17] as well as the function `A_0 B_2 - q² A_9 B_4`.

```maple
> with(qseries):
> Q:=n->tripleprod(q^n,q^33,10):
> A0:=Q(15):       A3:=Q(12):       A7:=Q(6):
> A8:=Q(3):        A9:=Q(9):
> B2:=Q(13)-q^3*Q(2):    B4:=Q(7)+q*Q(4):
> IDG:=series( ( A0*B2-q^2*A9*B4),q,200):
> series(IDG,q,10);
```

Output (38):
```
1 - q² - 2q³ + q⁵ + q⁷ + q⁹ + O(q¹¹)
```

```maple
> jp:=jacprodmake(IDG,q,50);
```

Output (39):
```
       JAC(2, 11, ∞)  JAC(3, 11, ∞)²  JAC(5, 11, ∞)
jp := ──────────────────────────────────────────────────
                    JAC(0, 11, ∞)²
```

```maple
> jac2prod(jp);
```

Output (40):
```
(q²; q¹¹)_∞ (q⁹; q¹¹)_∞ (q¹¹; q¹¹)²_∞ (q³; q¹¹)²_∞ (q⁸; q¹¹)²_∞ (q⁵; q¹¹)_∞ (q⁶; q¹¹)_∞
```

```maple
> series(winquist(q^5,q^3,q^11,20),q,20);
```

Output (41):
```
1 - q² - 2q³ + q⁵ + q⁷ + q⁹ + q¹¹ + q¹² - q¹³ - q¹⁵ - q¹⁶ - q¹⁸ + O(q²⁰)
```

```maple
> series(IDG-winquist(q^5,q^3,q^11,20),q,60);
```

Output (42):
```
O(q⁴⁹)
```

From our maple session it seems that:

```
A_0 B_2 - q² A_9 B_4

= (q²;q¹¹)_∞ (q⁹;q¹¹)_∞ (q¹¹;q¹¹)²_∞ (q³;q¹¹)²_∞ (q⁸;q¹¹)²_∞ (q⁵;q¹¹)_∞ (q⁶;q¹¹)_∞ .
```
(6.6)

and that this product appears in Winquist's identity on replacing q by `q¹¹` and letting `a = q⁵` and `b = q³`.

---

**EXERCISE 13.**
(i) Prove (6.6) by using the triple product identity (6.1) to write the right side of Winquist's identity (6.5) as a sum of two products.
(ii) In a similar manner find and prove a product form for `A_0 B_0 - q³ A_7 B_4`.

---

## References

1. George E. Andrews. *The theory of partitions.* Addison-Wesley Publishing Co., Reading, Mass.-London-Amsterdam, 1976. Encyclopedia of Mathematics and its Applications, Vol. 2.

2. George E. Andrews. *Partitions: yesterday and today,* With a foreword by J. C. Turner. New Zealand Math. Soc., Wellington, 1979.

3. George E. Andrews. Generalized Frobenius partitions. *Mem. Amer. Math. Soc.*, 49(301):iv+44, 1984.

4. George E. Andrews. *q-series: their development and application in analysis, number theory, combinatorics, physics, and computer algebra.* Published for the Conference Board of the Mathematical Sciences, Washington, D.C., 1986.

5. A. O. L. Atkin. Ramanujan congruences for p_k(n). *Canad. J. Math.* 20 (1968), 67-78; corrigendum, ibid., 21:256, 1968.

6. A. O. L. Atkin and H. P. F. Swinnerton-Dyer. Some properties of partitions. *Proc. London Math. Soc.*, 4:84-106, 1954.

7. Bruce C. Berndt. Ramanujan's theory of theta-functions. In *Theta functions: from the classical to the modern*, volume 1 of CRM Proc. Lecture Notes, pages 1-63. Amer. Math. Soc., Providence, RI, 1993.

8. Bruce C. Berndt, S. Bhargava, and Frank G. Garvan. Ramanujan's theories of elliptic functions to alternative bases. *Trans. Amer. Math. Soc.*, 347(11):4163-4244, 1995.

9. J. Borwein, P. Borwein, and F. Garvan. Hypergeometric analogues of the arithmetic-geometric mean iteration. *Constr. Approx.*, 9(4):509-523, 1993.

10. J. M. Borwein and P. B. Borwein. A remarkable cubic mean iteration. In *Computational methods and function theory (Valparaiso, 1989)*, volume 1435 of Lecture Notes in Math., pages 27-31. Springer, Berlin, 1990.

11. J. M. Borwein and P. B. Borwein. A cubic counterpart of Jacobi's identity and the AGM. *Trans. Amer. Math. Soc.*, 323(2):691-701, 1991.

12. J. M. Borwein, P. B. Borwein, and F. G. Garvan. Some cubic modular identities of Ramanujan. *Trans. Amer. Math. Soc.*, 343(1):35-47, 1994.

13. Jonathan M. Borwein and Peter B. Borwein. *Pi and the AGM.* Canadian Mathematical Society Series of Monographs and Advanced Texts. John Wiley & Sons Inc., New York, 1987.

14. Freeman J. Dyson. Missed opportunities. *Bull. Amer. Math. Soc.*, 78:635-652, 1972.

15. N. J. Fine. On a system of modular functions connected with the Ramanujan identities. *Tohoku Math. J. (2)*, 8:149-164, 1956.

16. F. G. Garvan. A simple proof of Watson's partition congruences for powers of 7. *J. Austral. Math. Soc. Ser. A*, 36(3):316-334, 1984.

17. F. G. Garvan. New combinatorial interpretations of Ramanujan's partition congruences mod 5, 7 and 11. *Trans. Amer. Math. Soc.*, 305(1):47-77, 1988.

18. Frank Garvan. Cubic modular identities of Ramanujan, hypergeometric functions and analogues of the arithmetic-geometric mean iteration. In *The Rademacher legacy to mathematics (University Park, PA, 1992)*, volume 166 of Contemp. Math., pages 245-264. Amer. Math. Soc., Providence, RI, 1994.

19. Frank Garvan, Dongsu Kim, and Dennis Stanton. Cranks and t-cores. *Invent. Math.*, 101(1):1-17, 1990.

20. Frank G. Garvan. Some congruences for partitions that are p-cores. *Proc. London Math. Soc. (3)*, 66(3):449-478, 1993.

21. Andrew Granville and Ken Ono. Defect zero p-blocks for finite simple groups. *Trans. Amer. Math. Soc.*, 348(1):331-347, 1996.

22. Michael D. Hirschhorn. A generalisation of Winquist's identity and a conjecture of Ramanujan. *J. Indian Math. Soc. (N.S.)*, 51:49-55 (1988), 1987.

23. Michael D. Hirschhorn and David C. Hunt. A simple proof of the Ramanujan conjecture for powers of 5. *J. Reine Angew. Math.*, 326:1-17, 1981.

24. C. G. J. Jacobi. *Gesammelte Werke.* Bände I-VIII. Chelsea Publishing Co., New York, 1969.

25. Soon-Yi Kang. A new proof of Winquist's identity. *J. Combin. Theory Ser. A*, 78(2):313-318, 1997.

26. Marvin I. Knopp. *Modular functions in analytic number theory.* Markham Publishing Co., Chicago, Ill., 1970.

27. Neal Koblitz. *Introduction to elliptic curves and modular forms*, volume 97 of Graduate Texts in Mathematics. Springer-Verlag, New York, second edition, 1993.

28. O. Kolberg. Note on the Eisenstein series of Γ₀(p). *Årbok Univ. Bergen Mat.-Natur. Ser.*, 1968(6):20 pp. (1969), 1968.

29. R. P. Lewis. A combinatorial proof of the triple product identity. *Amer. Math. Monthly*, 91(7):420-423, 1984.

30. I. G. Macdonald. Affine root systems and Dedekind's η-function. *Invent. Math.*, 15:91-143, 1972.

31. S. Ramanujan. *Collected papers of Srinivasa Ramanujan.* Chelsea Publishing Co., New York, 1962. Edited with notes by G. H. Hardy, P. V. Sesu Aiyar and B. M. Wilson.

32. Srinivasa Ramanujan. *The lost notebook and other unpublished papers.* Springer-Verlag, Berlin, 1988. With an introduction by George E. Andrews.

33. Øystein Rødseth. Dissections of the generating functions of q(n) and q₀(n). *Årbok Univ. Bergen Mat.-Natur. Ser.*, (12):12 pp. (1970), 1969.

34. L. J. Rogers. Second memoir on the expansion of certain infinite products. *Proc. London Math. Soc.*, 25:318-343, 1894.

35. J.-P. Serre. *A course in arithmetic.* Springer-Verlag, New York, 1973. Translated from the French, Graduate Texts in Mathematics, No. 7.

36. L. J. Slater. Further identities of the Rogers-Ramanujan type. *Proc. London Math. Soc.*, 54:147-167, 1952.

37. Dennis Stanton. Sign variations of the Macdonald identities. *SIAM J. Math. Anal.*, 17(6):1454-1460, 1986.

38. Dennis Stanton. An elementary approach to the Macdonald identities. In *q-series and partitions (Minneapolis, MN, 1988)*, volume 18 of IMA Vol. Math. Appl., pages 139-149. Springer, New York, 1989.

39. J. J. Sylvester. A constructive theory of partitions, arranged in three acts, an interact and an exodion. *Amer. J. Math.*, 5:251-331, 1882.

40. G. N. Watson. Theorems stated by Ramanujan (vii): Theorems on continued fractions. *J. London Math. Soc.*, 4:39-48, 1929.

41. G. N. Watson. Ramanujans Vermutung über Zerfällungsanzahlen. *J. Reine Angew. Math.*, 179:97-128, 1938.

42. E. T. Whittaker and G. N. Watson. *A course of modern analysis.* Cambridge Mathematical Library. Cambridge University Press, Cambridge, 1996.

43. Lasse Winquist. An elementary proof of p(11m + 6) ≡ 0 (mod 11). *J. Combinatorial Theory*, 6:56-59, 1969.
