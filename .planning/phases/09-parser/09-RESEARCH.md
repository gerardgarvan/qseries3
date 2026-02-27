# Phase 9: parser — Research

**Researched:** 2025-02-25  
**Domain:** Tokenizer + recursive-descent expression parser (Maple-like syntax, zero external deps)  
**Confidence:** HIGH

---

## User Constraints

- **Zero external dependencies** — no lex/yacc, no GMP, Boost, etc. All parser code hand-written in C++20.
- **Maple-like syntax** — `:=` assignment, `sum(expr, var, lo, hi)`, `add` alias, `[a,b,c]` list literals, `#` comments.
- **Success criteria (REPL-01):** Tokenizer handles identifiers, numbers, operators, `:=`; recursive-descent produces AST; sum/add parse correctly.
- **Output:** `.planning/phases/09-parser/09-RESEARCH.md` for plan-phase consumer.

---

## Standard Stack

For zero-dependency C++: **hand-rolled tokenizer + recursive-descent expression parser**. No lex/yacc, no parser generators.

| Component | Approach |
|-----------|----------|
| Lexer | Single-pass tokenizer over `std::string` with `char*` or index; `getNextToken()` returns next token |
| Parser | Recursive descent with **precedence climbing** (Pratt-style) for operators |
| AST | `std::variant` or tagged union of node structs; no virtuals (per .cursorrules) |
| Error handling | Throw `std::runtime_error` or custom `ParseError` with position/location |

**Why precedence climbing:** Classic recursive-descent (one function per precedence level) works but scales poorly with many levels. Precedence climbing uses a single parameterized `parseExpr(minPrec)` — cleaner, fewer functions, and correctly handles right-associative `^`.

---

## Architecture Patterns

### Token stream → recursive descent by precedence level → AST

1. **Tokenizer:** Turn `std::string` input into `std::vector<Token>` (or iterator yielding tokens). Skip `#` comments (discard to end of line).
2. **Parser:** Consume tokens; each parsing function returns an AST node.
3. **Precedence table:** Assign numeric precedence to each operator; use `minPrec` to know when to stop consuming.
4. **Primary expressions:** Atoms and function calls — the base case before any binary operator.

```
input string → [Tokenizer] → token stream → [Parser] → AST
```

### Data flow

```
TokenStream { tokens[], pos }
  → parseStmt()     → Assign | Expr
  → parseExpr(p)    → binary/unary chains by precedence
  → parsePrimary()  → Int | Q | Id | Call | List | Paren
```

---

## Token Kinds (Enumerated)

| Token | Kind | Lexeme / Notes |
|-------|------|----------------|
| `IDENT` | Identifier | `[a-zA-Z_][a-zA-Z0-9_]*` — e.g. `etaq`, `theta3`, `n` |
| `INT` | Integer | `[0-9]+` — nonnegative decimal |
| `Q` | Formal variable | Literal `q` (reserved) |
| `ASSIGN` | `:=` | Two-character operator |
| `PLUS` | `+` | |
| `MINUS` | `-` | Binary and unary; context determines |
| `STAR` | `*` | |
| `SLASH` | `/` | |
| `CARET` | `^` | Exponentiation |
| `LPAREN` | `(` | |
| `RPAREN` | `)` | |
| `LBRACK` | `[` | List literal start |
| `RBRACK` | `]` | List literal end |
| `COMMA` | `,` | Argument/element separator |
| `END` | End of input | After last token |

**Operator boundary handling:** The tokenizer must correctly split `:=` (not `:` then `=`), `q` (as Q, not IDENT). For `-`, the parser distinguishes unary vs binary by context (unary after start, `(`, `,`, or another operator).

---

## Precedence Table

| Precedence | Operators | Associativity | Notes |
|------------|-----------|---------------|-------|
| 0 | `:=` | Right | Assignment only; handled in parseStmt(), not parseExpr(); LHS must be identifier |
| 1 | `+`, `-` | Left | Binary additive |
| 2 | `*`, `/` | Left | Multiplicative |
| 3 | unary `-` | Prefix | Only at primary/operand position |
| 4 | `^` | **Right** | Exponentiation |
| 5+ | primary | — | Atoms, calls, lists, parens |

**Right-associative `^`:** For `a ^ b ^ c`, parse as `a ^ (b ^ c)`. In precedence climbing: when consuming `^`, use `q = prec(^)` (not `prec+1`) so the next `^` is consumed by the recursive call, not the loop.

---

## AST Node Types

| Node | Fields | Example |
|------|--------|---------|
| `IntLit` | `int64_t val` | `42` |
| `Q` | (none) | `q` |
| `Var` | `std::string name` | `x`, `etaq` (when not a call) |
| `BinOp` | `Op op, Expr left, Expr right` | `a + b`, `f * g` |
| `UnOp` | `Op op, Expr operand` | `-x` |
| `Call` | `std::string name, std::vector<Expr> args` | `etaq(q, 1, 50)` |
| `Sum` | `Expr body, std::string var, Expr lo, Expr hi` | `sum(q^(n^2)/aqprod(q,q,n,50), n, 0, 8)` |
| `List` | `std::vector<Expr> elements` | `[theta3(q,100), theta4(q,100)]` |
| `Paren` | `Expr inner` | `(1 + q)` — or flatten to inner; often omitted in AST |
| `Assign` | `std::string name, Expr rhs` | `x := etaq(q, 1, 50)` |

Use `std::variant` or a tagged struct; avoid virtual dispatch per project style.

```cpp
struct Expr;
using ExprPtr = std::unique_ptr<Expr>;

struct Expr {
    enum class Tag { IntLit, Q, Var, BinOp, UnOp, Call, Sum, List };
    Tag tag;
    // ... payload via variant or union
};
```

---

## sum/add Grammar

**Form:** `sum(expr, var, lo, hi)` or `add(expr, var, lo, hi)`.

- `expr` — expression (may use `var` as a free variable; evaluated for each `var = lo..hi`).
- `var` — identifier (summation index).
- `lo`, `hi` — expressions (typically integers; eval at parse or eval time).

**Parsing:** Treat `sum` and `add` as special-case in primary/call handling: when name is `sum` or `add` and `(` follows, parse four comma-separated arguments, then `)`.

**Production:**
```
sum_call := ("sum" | "add") "(" expr "," id "," expr "," expr ")"
```

**Variable scoping:** The summation variable `var` is bound in `expr`. Phase 9 only builds the AST; Phase 10 (REPL/eval) will implement the semantics (bind `var` to lo..hi, evaluate `expr` for each, accumulate Series).

---

## List Literal Grammar

**Form:** `[expr₁, expr₂, ..., exprₖ]`.

**Production:**
```
list := "[" [ expr ( "," expr )* ] "]"
```

**Parsing:** At primary level, if token is `[`, consume `[`, parse comma-separated expressions, expect `]`, return `List` node.

**No ambiguity with function calls:** Function calls use `name(args)` with parentheses. Lists use brackets. `findhom([theta3(q,100), ...], 2, 0)` — first arg is a `List` node; no conflict.

---

## Common Pitfalls

### 1. Right-associative `^`

**What goes wrong:** Parsing `a ^ b ^ c` as `(a ^ b) ^ c` instead of `a ^ (b ^ c)`.

**Fix:** In precedence climbing, for right-associative operators use `q = prec(op)` (same as current) so the loop exits on the next `^`; the recursive `parseExpr(q)` consumes the right-associative chain.

### 2. Unary `-` vs binary `-`

**What goes wrong:** `-5` parsed as binary minus with missing left operand; or `a - -b` fails.

**Fix:** Parse unary `-` only at the start of primary (in `parsePrimary`). Binary `-` is consumed in the `parseExpr` loop when we see `MINUS` after already having a left operand. Since `parsePrimary` is only entered when we need an operand (start, after `(`, `,`, or binary op), unary `-` is correctly recognized there; the loop sees binary `-` between two operands.

### 3. List vs call ambiguity

**What goes wrong:** `[x]` confused with something else; or `[` after identifier parsed as subscript.

**Fix:** No subscript in our language. `[` always starts a list. `name(` starts a call. No ambiguity.

### 4. sum/add var scoping

**What goes wrong:** Eval phase uses global `n` instead of summation index in `sum(q^(n^2)/..., n, 0, 8)`.

**Fix:** Parser produces `Sum(body, "n", lo, hi)`. Evaluator must bind `var` in a local scope when evaluating `body`; do not look up `var` in the global environment during the sum.

### 5. `:=` tokenization

**What goes wrong:** `x:=1` tokenized as `x`, `:`, `=`, `1` and then `=` causes confusion.

**Fix:** When seeing `:`, peek next char; if `=`, consume both and emit `ASSIGN`. Otherwise error or treat as invalid.

### 6. Comment handling

**What goes wrong:** `#` inside a string (we don't have strings) or `q#x` — we don't have strings; `q#x` would be `q` then `#` starting comment. So `#` always starts a comment to end of line. Skip from `#` to newline in tokenizer before emitting next token.

### 7. Primary after binary operator

**What goes wrong:** `a +` or `a *` with no right operand — parser should fail cleanly.

**Fix:** After consuming a binary operator, `parsePrimary()` (or `parseExpr` at appropriate prec) must succeed or throw. Don't allow EOF after `+`/`*`.

---

## Code Examples

### Tokenizer pattern

```cpp
struct Token {
    enum class Kind { IDENT, INT, Q, ASSIGN, PLUS, MINUS, STAR, SLASH, CARET,
                      LPAREN, RPAREN, LBRACK, RBRACK, COMMA, END };
    Kind kind;
    std::string text;  // for IDENT, INT
};

class Tokenizer {
    std::string input;
    size_t pos = 0;
public:
    explicit Tokenizer(std::string s) : input(std::move(s)) {}
    Token next() {
        while (pos < input.size()) {
            if (input[pos] == '#') { while (pos < input.size() && input[pos] != '\n') ++pos; continue; }
            if (std::isspace(static_cast<unsigned char>(input[pos]))) { ++pos; continue; }
            if (input[pos] == ':') {
                if (pos+1 < input.size() && input[pos+1] == '=') { pos += 2; return {Token::ASSIGN, ""}; }
                throw parse_error("expected :=");
            }
            if (std::isdigit(input[pos])) { /* scan integer */ return {Token::INT, numstr}; }
            if (std::isalpha(input[pos]) || input[pos] == '_') {
                std::string id = scanIdent();
                if (id == "q") return {Token::Q, ""};
                return {Token::IDENT, id};
            }
            // single-char: + - * / ^ ( ) [ ] ,
            // ...
        }
        return {Token::END, ""};
    }
};
```

### Precedence climbing expression parser

```cpp
ExprPtr parseExpr(int minPrec) {
    ExprPtr left = parsePrimary();
    for (;;) {
        Token op = peek();
        if (op.kind == Token::END || op.kind == Token::RPAREN || op.kind == Token::COMMA || op.kind == Token::RBRACK || op.kind == Token::ASSIGN)
            break;  // ASSIGN only in parseStmt
        int prec = precedence(op);
        if (prec < minPrec) break;
        // ASSIGN handled in parseStmt only; never in parseExpr
        bool rightAssoc = (op.kind == Token::CARET);
        consume();
        int nextMin = rightAssoc ? prec : prec + 1;
        ExprPtr right = parseExpr(nextMin);
        left = std::make_unique<BinOp>(opToEnum(op), std::move(left), std::move(right));
    }
    return left;
}

ExprPtr parsePrimary() {
    // Unary - can start any primary (after start, '(', ',', or binary op)
    if (peek().kind == Token::MINUS) { consume(); return makeUnOp(parsePrimary()); }
    if (peek().kind == Token::INT) { /* ... */ }
    if (peek().kind == Token::Q) { consume(); return makeQ(); }
    if (peek().kind == Token::IDENT) {
        std::string name = consumeIdent();
        if (peek().kind == Token::LPAREN) {
            consume();  // (
            if (name == "sum" || name == "add") return parseSumCall(name);
            auto args = parseCommaSeparatedExprs();
            expect(Token::RPAREN);
            return makeCall(name, std::move(args));
        }
        return makeVar(name);
    }
    if (peek().kind == Token::LPAREN) { consume(); auto e = parseExpr(0); expect(Token::RPAREN); return e; }
    if (peek().kind == Token::LBRACK) { consume(); auto elems = parseCommaSeparatedExprs(); expect(Token::RBRACK); return makeList(elems); }
    throw parse_error("expected primary");
}
```

### sum/add parsing

```cpp
ExprPtr parseSumCall(const std::string& name) {
    ExprPtr body = parseExpr(0);
    expect(Token::COMMA);
    Token id = expect(Token::IDENT);
    expect(Token::COMMA);
    ExprPtr lo = parseExpr(0);
    expect(Token::COMMA);
    ExprPtr hi = parseExpr(0);
    expect(Token::RPAREN);
    return std::make_unique<Sum>(std::move(body), id.text, std::move(lo), std::move(hi));
}
```

### Top-level statement

```cpp
Stmt parseStmt() {
    ExprPtr e = parseExpr(0);
    if (e->tag == Expr::Var && peek().kind == Token::ASSIGN) {
        std::string name = e->var.name;
        consume();  // :=
        ExprPtr rhs = parseExpr(0);
        return Assign{name, std::move(rhs)};
    }
    return ExprStmt{std::move(e)};
}
```

---

## Don't Hand-Roll

**N/A** — Project requires zero external dependencies. We must hand-roll the tokenizer and parser. Avoid:

- Adding flex/bison or ANTLR
- Using regex (`std::regex` is available but overkill; simple character-by-character scan is sufficient and faster)
- Over-engineering: keep the parser small and focused on the SPEC expression language

---

## Quality Gate Checklist

- [x] **Token kinds enumerated:** IDENT, INT, Q, ASSIGN, PLUS, MINUS, STAR, SLASH, CARET, LPAREN, RPAREN, LBRACK, RBRACK, COMMA, END
- [x] **Precedence table:** :=(0,right), +- (1,left), */ (2,left), unary- (3), ^ (4,right), primary (5+)
- [x] **AST node types:** IntLit, Q, Var, BinOp, UnOp, Call, Sum, List, Paren, Assign
- [x] **sum/add grammar:** sum(expr, var, lo, hi) and add(...) as alias; four args
- [x] **List literal grammar:** [ expr ( , expr )* ]

---

## Dependencies

| Dependency | Layer | Purpose |
|------------|-------|---------|
| bigint.h | Phase 1 | IntLit may hold BigInt for large integers; or use int64_t for parser, convert at eval |
| frac.h | Phase 2 | Not needed in parser (parser produces AST only) |
| series.h | Phase 3 | Not needed in parser |
| (none for eval) | Phase 10 | Parser output consumed by REPL evaluator |

Phase 9 parser is **standalone** — it produces an AST. Phase 10 REPL will interpret the AST against Series, qfuncs, etc.

---

## RESEARCH COMPLETE
