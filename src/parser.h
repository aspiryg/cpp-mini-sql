#pragma once
#include "tokenizer.h"
#include "ast.h"
#include <stdexcept>
#include <string>

class Parser {
public:
    explicit Parser(const std::vector<Token>& tokens)
        : tokens_(tokens), pos_(0) {}

    Statement parse() {
        if (check(TokenType::CREATE))  return parseCreateTable();
        if (check(TokenType::INSERT))  return parseInsert();
        if (check(TokenType::SELECT))  return parseSelect();
        throw std::runtime_error("Expected CREATE, INSERT, or SELECT");
    }

private:
    std::vector<Token> tokens_;
    size_t pos_;

    // ---- helpers ----
    const Token& current() const { return tokens_[pos_]; }
    bool check(TokenType t) const { return current().type == t; }
    bool atEnd() const { return check(TokenType::END_OF_INPUT); }

    Token advance() {
        Token tok = current();
        if (!atEnd()) pos_++;
        return tok;
    }

    Token expect(TokenType t, const std::string& msg) {
        if (!check(t)) {
            throw std::runtime_error("Parse error: expected " + msg +
                " but got '" + current().value + "'");
        }
        return advance();
    }

    // ---- CREATE TABLE name (col1 TYPE, col2 TYPE, ...) ----
    Statement parseCreateTable() {
        expect(TokenType::CREATE, "CREATE");
        expect(TokenType::TABLE,  "TABLE");
        std::string tableName = expect(TokenType::IDENTIFIER, "table name").value;
        expect(TokenType::LPAREN, "(");

        std::vector<ColumnDef> columns;
        do {
            ColumnDef col;
            col.name = expect(TokenType::IDENTIFIER, "column name").value;
            if (check(TokenType::INT)) {
                advance();
                col.type = ColumnType::INT;
            } else if (check(TokenType::TEXT)) {
                advance();
                col.type = ColumnType::TEXT;
            } else {
                throw std::runtime_error("Expected column type INT or TEXT");
            }
            columns.push_back(col);
        } while (check(TokenType::COMMA) && (advance(), true));

        expect(TokenType::RPAREN, ")");
        optionalSemicolon();

        Statement stmt;
        stmt.type = StmtType::CREATE_TABLE;
        stmt.createTable.tableName = tableName;
        stmt.createTable.columns   = columns;
        return stmt;
    }

    // ---- INSERT INTO name VALUES (v1, v2, ...) ----
    Statement parseInsert() {
        expect(TokenType::INSERT, "INSERT");
        expect(TokenType::INTO,   "INTO");
        std::string tableName = expect(TokenType::IDENTIFIER, "table name").value;
        expect(TokenType::VALUES, "VALUES");
        expect(TokenType::LPAREN, "(");

        std::vector<Value> values;
        do {
            values.push_back(parseValue());
        } while (check(TokenType::COMMA) && (advance(), true));

        expect(TokenType::RPAREN, ")");
        optionalSemicolon();

        Statement stmt;
        stmt.type = StmtType::INSERT;
        stmt.insert.tableName = tableName;
        stmt.insert.values    = values;
        return stmt;
    }

    // ---- SELECT cols FROM name [WHERE ...] ----
    Statement parseSelect() {
        expect(TokenType::SELECT, "SELECT");

        SelectStmt sel;
        if (check(TokenType::STAR)) {
            advance();
            sel.selectAll = true;
        } else {
            do {
                sel.columns.push_back(
                    expect(TokenType::IDENTIFIER, "column name").value);
            } while (check(TokenType::COMMA) && (advance(), true));
        }

        expect(TokenType::FROM, "FROM");
        sel.tableName = expect(TokenType::IDENTIFIER, "table name").value;

        if (check(TokenType::WHERE)) {
            advance();
            sel.hasWhere = true;
            sel.where    = parseExpression();
        }

        optionalSemicolon();

        Statement stmt;
        stmt.type   = StmtType::SELECT;
        stmt.select = sel;
        return stmt;
    }

    // ---- WHERE expression: condition (AND|OR condition)* ----
    Expression parseExpression() {
        Expression expr;
        expr.conditions.push_back(parseCondition());
        while (check(TokenType::AND) || check(TokenType::OR)) {
            LogicOp op = (current().type == TokenType::AND)
                         ? LogicOp::AND : LogicOp::OR;
            advance();
            expr.operators.push_back(op);
            expr.conditions.push_back(parseCondition());
        }
        return expr;
    }

    // ---- condition: column OP value ----
    Condition parseCondition() {
        Condition cond;
        cond.column = expect(TokenType::IDENTIFIER, "column name").value;
        cond.op     = parseCompareOp();
        cond.value  = parseValue();
        return cond;
    }

    CompareOp parseCompareOp() {
        TokenType t = current().type;
        advance();
        switch (t) {
            case TokenType::EQ:  return CompareOp::EQ;
            case TokenType::NEQ: return CompareOp::NEQ;
            case TokenType::LT:  return CompareOp::LT;
            case TokenType::GT:  return CompareOp::GT;
            case TokenType::LTE: return CompareOp::LTE;
            case TokenType::GTE: return CompareOp::GTE;
            default:
                throw std::runtime_error("Expected comparison operator");
        }
    }

    Value parseValue() {
        if (check(TokenType::INTEGER_LITERAL)) {
            int v = std::stoi(advance().value);
            return Value(v);
        }
        if (check(TokenType::STRING_LITERAL)) {
            return Value(advance().value);
        }
        throw std::runtime_error("Expected integer or string value, got '" +
                                 current().value + "'");
    }

    void optionalSemicolon() {
        if (check(TokenType::SEMICOLON)) advance();
    }
};
