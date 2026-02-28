#ifndef PARSER_H
#define PARSER_H

#include <string>
#include <vector>
#include <memory>
#include <cctype>
#include <climits>
#include <stdexcept>

// --- Token ---
struct Token {
    enum class Kind {
        IDENT, INT, Q, ASSIGN,
        PLUS, MINUS, STAR, SLASH, CARET,
        LPAREN, RPAREN, LBRACK, RBRACK, COMMA,
        END
    };
    Kind kind;
    std::string text;
    size_t offset = 0;

    Token() : kind(Kind::END), text() {}
    Token(Kind k, std::string t = "", size_t off = 0) : kind(k), text(std::move(t)), offset(off) {}
};

inline void offsetToLineCol(const std::string& s, size_t off, int& line, int& col) {
    line = 1; col = 1;
    for (size_t i = 0; i < off && i < s.size(); ++i) {
        if (s[i] == '\n') { ++line; col = 1; } else ++col;
    }
}

inline std::string kindToExpected(Token::Kind k) {
    switch (k) {
        case Token::Kind::IDENT: return "'identifier'";
        case Token::Kind::INT: return "'integer'";
        case Token::Kind::Q: return "'q'";
        case Token::Kind::ASSIGN: return "':='";
        case Token::Kind::LPAREN: return "'('";
        case Token::Kind::RPAREN: return "')'";
        case Token::Kind::LBRACK: return "'['";
        case Token::Kind::RBRACK: return "']'";
        case Token::Kind::COMMA: return "','";
        default: return "'token'";
    }
}

// --- Tokenizer ---
class Tokenizer {
    std::string input;
    size_t pos = 0;

    void skipWhitespace() {
        while (pos < input.size() && std::isspace(static_cast<unsigned char>(input[pos])))
            ++pos;
    }

    void skipComment() {
        if (pos < input.size() && input[pos] == '#') {
            while (pos < input.size() && input[pos] != '\n')
                ++pos;
        }
    }

public:
    explicit Tokenizer(std::string s) : input(std::move(s)) {}

    Token next() {
        for (;;) {
            skipWhitespace();
            skipComment();
            if (pos >= input.size()) return Token(Token::Kind::END, "", pos);

            char c = input[pos];

            if (c == '#') continue;
            if (std::isspace(static_cast<unsigned char>(c))) continue;

            if (c == ':') {
                if (pos + 1 < input.size() && input[pos + 1] == '=') {
                    size_t start = pos;
                    pos += 2;
                    return Token(Token::Kind::ASSIGN, "", start);
                }
                int line = 1, col = 1;
                offsetToLineCol(input, pos, line, col);
                throw std::runtime_error("parser: line " + std::to_string(line) + ", col " + std::to_string(col) + ": expected := ");
            }

            if (std::isdigit(static_cast<unsigned char>(c))) {
                size_t start = pos;
                while (pos < input.size() && std::isdigit(static_cast<unsigned char>(input[pos])))
                    ++pos;
                return Token(Token::Kind::INT, input.substr(start, pos - start), start);
            }

            if (std::isalpha(static_cast<unsigned char>(c)) || c == '_') {
                size_t start = pos;
                while (pos < input.size() &&
                       (std::isalnum(static_cast<unsigned char>(input[pos])) || input[pos] == '_'))
                    ++pos;
                std::string id = input.substr(start, pos - start);
                if (id == "q") return Token(Token::Kind::Q, "", start);
                return Token(Token::Kind::IDENT, id, start);
            }

            if (c == '+') { size_t start = pos; ++pos; return Token(Token::Kind::PLUS, "", start); }
            if (c == '-') { size_t start = pos; ++pos; return Token(Token::Kind::MINUS, "", start); }
            if (c == '*') { size_t start = pos; ++pos; return Token(Token::Kind::STAR, "", start); }
            if (c == '/') { size_t start = pos; ++pos; return Token(Token::Kind::SLASH, "", start); }
            if (c == '^') { size_t start = pos; ++pos; return Token(Token::Kind::CARET, "", start); }
            if (c == '(') { size_t start = pos; ++pos; return Token(Token::Kind::LPAREN, "", start); }
            if (c == ')') { size_t start = pos; ++pos; return Token(Token::Kind::RPAREN, "", start); }
            if (c == '[') { size_t start = pos; ++pos; return Token(Token::Kind::LBRACK, "", start); }
            if (c == ']') { size_t start = pos; ++pos; return Token(Token::Kind::RBRACK, "", start); }
            if (c == ',') { size_t start = pos; ++pos; return Token(Token::Kind::COMMA, "", start); }

            int line = 1, col = 1;
            offsetToLineCol(input, pos, line, col);
            std::string msg = "parser: line " + std::to_string(line) + ", col " + std::to_string(col) + ": unexpected character";
            if (c >= 32 && c < 127)
                msg += " '" + std::string(1, c) + "'";
            throw std::runtime_error(msg);
        }
    }
};

// --- AST ---
struct Expr;
struct Stmt;
using ExprPtr = std::unique_ptr<Expr>;
using StmtPtr = std::unique_ptr<Stmt>;

enum class BinOp { Add, Sub, Mul, Div, Pow };

struct Expr {
    enum class Tag { IntLit, Q, Var, BinOp, UnOp, Call, List, Sum };
    Tag tag = Tag::IntLit;

    int64_t intVal = 0;
    std::string varName;
    BinOp binOp = BinOp::Add;
    ExprPtr left;
    ExprPtr right;
    ExprPtr operand;
    std::string callName;
    std::vector<ExprPtr> args;
    std::vector<ExprPtr> elements;
    ExprPtr body;
    std::string sumVar;
    ExprPtr lo;
    ExprPtr hi;

    static ExprPtr makeInt(int64_t v) {
        auto e = std::make_unique<Expr>();
        e->tag = Tag::IntLit;
        e->intVal = v;
        return e;
    }
    static ExprPtr makeQ() {
        auto e = std::make_unique<Expr>();
        e->tag = Tag::Q;
        return e;
    }
    static ExprPtr makeVar(std::string name) {
        auto e = std::make_unique<Expr>();
        e->tag = Tag::Var;
        e->varName = std::move(name);
        return e;
    }
    static ExprPtr makeBinOp(BinOp op, ExprPtr l, ExprPtr r) {
        auto e = std::make_unique<Expr>();
        e->tag = Tag::BinOp;
        e->binOp = op;
        e->left = std::move(l);
        e->right = std::move(r);
        return e;
    }
    static ExprPtr makeUnOp(ExprPtr o) {
        auto e = std::make_unique<Expr>();
        e->tag = Tag::UnOp;
        e->operand = std::move(o);
        return e;
    }
    static ExprPtr makeCall(std::string name, std::vector<ExprPtr> a) {
        auto e = std::make_unique<Expr>();
        e->tag = Tag::Call;
        e->callName = std::move(name);
        e->args = std::move(a);
        return e;
    }
    static ExprPtr makeList(std::vector<ExprPtr> elems) {
        auto e = std::make_unique<Expr>();
        e->tag = Tag::List;
        e->elements = std::move(elems);
        return e;
    }
    static ExprPtr makeSum(ExprPtr b, std::string var, ExprPtr l, ExprPtr h) {
        auto e = std::make_unique<Expr>();
        e->tag = Tag::Sum;
        e->body = std::move(b);
        e->sumVar = std::move(var);
        e->lo = std::move(l);
        e->hi = std::move(h);
        return e;
    }
};

struct Stmt {
    enum class Tag { Expr, Assign };
    Tag tag = Tag::Expr;
    ExprPtr expr;
    std::string assignName;
    ExprPtr assignRhs;
};

// --- Parser ---
class Parser {
    std::vector<Token> tokens;
    size_t pos = 0;
    std::string inputStr;
    int depth = 0;

    const Token& peek() const {
        if (pos >= tokens.size()) return tokens.back();
        return tokens[pos];
    }

    Token consume() {
        if (pos >= tokens.size()) return Token(Token::Kind::END, "");
        return tokens[pos++];
    }

    void expect(Token::Kind k) {
        if (peek().kind != k) {
            const Token& t = peek();
            int line = 1, col = 1;
            offsetToLineCol(inputStr, t.offset, line, col);
            throw std::runtime_error("parser: line " + std::to_string(line) + ", col " + std::to_string(col) + ": expected " + kindToExpected(k));
        }
        consume();
    }

    static int precedence(const Token& t) {
        switch (t.kind) {
            case Token::Kind::PLUS:
            case Token::Kind::MINUS: return 1;
            case Token::Kind::STAR:
            case Token::Kind::SLASH: return 2;
            case Token::Kind::CARET: return 4;
            default: return -1;
        }
    }

    static BinOp toBinOp(Token::Kind k) {
        switch (k) {
            case Token::Kind::PLUS: return BinOp::Add;
            case Token::Kind::MINUS: return BinOp::Sub;
            case Token::Kind::STAR: return BinOp::Mul;
            case Token::Kind::SLASH: return BinOp::Div;
            case Token::Kind::CARET: return BinOp::Pow;
            default: return BinOp::Add;
        }
    }

    std::vector<ExprPtr> parseCommaSeparatedExprs() {
        std::vector<ExprPtr> args;
        args.push_back(parseExpr(0));
        while (peek().kind == Token::Kind::COMMA) {
            consume();
            args.push_back(parseExpr(0));
        }
        return args;
    }

    ExprPtr parseSumCall(const std::string&) {
        expect(Token::Kind::LPAREN);
        ExprPtr body = parseExpr(0);
        expect(Token::Kind::COMMA);
        Token idTok = consume();
        if (idTok.kind != Token::Kind::IDENT) {
            int line = 1, col = 1;
            offsetToLineCol(inputStr, idTok.offset, line, col);
            throw std::runtime_error("parser: line " + std::to_string(line) + ", col " + std::to_string(col) + ": sum/add expects identifier");
        }
        expect(Token::Kind::COMMA);
        ExprPtr lo = parseExpr(0);
        expect(Token::Kind::COMMA);
        ExprPtr hi = parseExpr(0);
        expect(Token::Kind::RPAREN);
        return Expr::makeSum(std::move(body), idTok.text, std::move(lo), std::move(hi));
    }

    ExprPtr parsePrimary() {
        if (peek().kind == Token::Kind::MINUS) {
            consume();
            return Expr::makeUnOp(parsePrimary());
        }
        if (peek().kind == Token::Kind::INT) {
            Token t = consume();
            int64_t v = 0;
            for (char c : t.text) {
                int digit = c - '0';
                if (v > (INT64_MAX - digit) / 10)
                    throw std::runtime_error("parser: integer literal too large for int64");
                v = v * 10 + digit;
            }
            return Expr::makeInt(v);
        }
        if (peek().kind == Token::Kind::Q) {
            consume();
            return Expr::makeQ();
        }
        if (peek().kind == Token::Kind::IDENT) {
            std::string name = consume().text;
            if (peek().kind == Token::Kind::LPAREN) {
                if (name == "sum" || name == "add")
                    return parseSumCall(name);
                consume();
                std::vector<ExprPtr> args;
                if (peek().kind != Token::Kind::RPAREN)
                    args = parseCommaSeparatedExprs();
                expect(Token::Kind::RPAREN);
                return Expr::makeCall(name, std::move(args));
            }
            return Expr::makeVar(name);
        }
        if (peek().kind == Token::Kind::LPAREN) {
            consume();
            if (++depth > 256)
                throw std::runtime_error("parser: expression too deeply nested (limit 256)");
            ExprPtr e = parseExpr(0);
            --depth;
            expect(Token::Kind::RPAREN);
            return e;
        }
        if (peek().kind == Token::Kind::LBRACK) {
            consume();
            auto elems = parseCommaSeparatedExprs();
            expect(Token::Kind::RBRACK);
            return Expr::makeList(std::move(elems));
        }
        const Token& t = peek();
        int line = 1, col = 1;
        offsetToLineCol(inputStr, t.offset, line, col);
        throw std::runtime_error("parser: line " + std::to_string(line) + ", col " + std::to_string(col) + ": expected primary expression");
    }

    ExprPtr parseExpr(int minPrec) {
        ExprPtr left = parsePrimary();
        for (;;) {
            const Token& op = peek();
            if (op.kind == Token::Kind::END || op.kind == Token::Kind::RPAREN ||
                op.kind == Token::Kind::COMMA || op.kind == Token::Kind::RBRACK ||
                op.kind == Token::Kind::ASSIGN)
                break;
            int prec = precedence(op);
            if (prec < 0 || prec < minPrec) break;
            bool rightAssoc = (op.kind == Token::Kind::CARET);
            consume();
            int nextMin = rightAssoc ? prec : prec + 1;
            ExprPtr right = parseExpr(nextMin);
            left = Expr::makeBinOp(toBinOp(op.kind), std::move(left), std::move(right));
        }
        return left;
    }

public:
    explicit Parser(std::string input) : inputStr(input) {
        Tokenizer tok(std::move(input));
        for (;;) {
            Token t = tok.next();
            tokens.push_back(t);
            if (t.kind == Token::Kind::END) break;
        }
    }

    StmtPtr parseStmt() {
        ExprPtr e = parseExpr(0);
        if (e->tag == Expr::Tag::Var && peek().kind == Token::Kind::ASSIGN) {
            std::string name = e->varName;
            consume();
            ExprPtr rhs = parseExpr(0);
            auto s = std::make_unique<Stmt>();
            s->tag = Stmt::Tag::Assign;
            s->assignName = name;
            s->assignRhs = std::move(rhs);
            return s;
        }
        auto s = std::make_unique<Stmt>();
        s->tag = Stmt::Tag::Expr;
        s->expr = std::move(e);
        return s;
    }
};

inline StmtPtr parse(std::string input) {
    Parser p(std::move(input));
    return p.parseStmt();
}

#endif
