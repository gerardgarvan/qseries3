---
title: REPL & Commands
description: "Interactive features, settings, and utility functions"
---

This page documents REPL features, settings, and utility functions that are not
part of the core q-series mathematics.

---

## Assignment

Store a result in a variable for later use. Variables persist for the duration
of the session.

```text
var := expression
```

**Example**

```text
qseries> rr := sum(q^(n^2)/aqprod(q, q, n, 50), n, 0, 8)
qseries> prodmake(rr, 40)
```

Variable names follow standard identifier rules (letters, digits, underscores;
must start with a letter).

---

## set_trunc

Set the default truncation order. Affects `q` and all subsequent computations
that don't specify an explicit truncation.

**Syntax**

```text
set_trunc(N: int)
```

**Parameters**

| Parameter | Type | Description |
|-----------|------|-------------|
| `N` | int | New default truncation order |

The initial default is 50. Increase to 100 or 200 for identity verification or
relation finding; decrease for faster exploration.

**Example**

```text
qseries> set_trunc(100)
Truncation set to 100

qseries> q
q + O(q^100)
```

---

## help

Built-in help system. With no argument, lists all available functions. With a
function name, shows its signature and description.

**Syntax**

```text
help
help(func)
```

**Example**

```text
qseries> help
Available commands: aqprod, coeffs, etaq, etamake, ...

qseries> help(prodmake)
prodmake(f, T?) - Convert series to infinite product form (Andrews' algorithm)
  f: Series to convert
  T: Number of product terms (optional, default from truncation)
```

---

## version

Print the package version.

**Syntax**

```text
version
```

**Example**

```text
qseries> version
qseries 2.0
```

---

## Number Theory Utilities

### legendre

Legendre symbol $\left(\frac{a}{p}\right)$. Returns $-1$, $0$, or $1$.

**Syntax**

```text
legendre(a: int, p: int) → int
```

**Parameters**

| Parameter | Type | Description |
|-----------|------|-------------|
| `a` | int | Integer to test |
| `p` | int | Odd prime |

**Example**

```text
qseries> legendre(2, 7)
1

qseries> legendre(3, 7)
-1
```

### sigma

Divisor sum function $\sigma_k(n) = \sum_{d \mid n} d^k$. Default $k=1$.

**Syntax**

```text
sigma(n: int, k?: int) → int
```

**Parameters**

| Parameter | Type | Description |
|-----------|------|-------------|
| `n` | int | Positive integer |
| `k` | int (optional) | Power to raise divisors to (default 1) |

**Example**

```text
qseries> sigma(12)
28

qseries> sigma(12, 2)
210
```

---

## Interactive Features

### Comments

Anything after `#` to end of line is ignored:

```text
qseries> p := 1/etaq(1)  # partition generating function
```

### Multi-line input

End a line with `\` to continue on the next line. The continuation prompt is
`  > `:

```text
qseries> rr := sum(q^(n^2)/aqprod(q,q,n,50), \
  > n, 0, 8)
```

Maximum 100 continuation lines.

### Output suppression

End a command with `:` (colon) to suppress output display:

```text
qseries> p := 1/etaq(1, 200):
```

The assignment still happens — only the display is suppressed. Useful for
intermediate computations with large output.

### Script mode

Run commands from a file by piping to stdin:

```bash
qseries < script.qs
```

No banner is displayed. Only command output is printed. Errors include the
script line number for debugging.

### Tab completion

In interactive mode, press **Tab** to autocomplete built-in function names and
user-defined variables:

- **Unique match** — completes the identifier
- **Multiple matches** — lists all matching options
- **No match** — no change

### Command history

The last 100 commands are kept in history. Use the **Up** and **Down** arrow keys
to navigate through previous commands.

### Line editing

Use **Left** and **Right** arrow keys to move the cursor within a line for
editing.

### Timing

After each command completes, the REPL displays the elapsed time:

```text
qseries> prodmake(rr, 40)
(1-q)^(-1) * (1-q^4)^(-1) * ...
0.042s
```
