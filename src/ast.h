#pragma once
#include <string>
#include <vector>
#include <variant>
#include <memory>

// ============================================================
// Column definition (for CREATE TABLE)
// ============================================================
enum class ColumnType { INT, TEXT };

struct ColumnDef {
    std::string name;
    ColumnType type;
};

// ============================================================
// Expressions (for WHERE clauses)
// ============================================================
enum class CompareOp { EQ, NEQ, LT, GT, LTE, GTE };
enum class LogicOp   { AND, OR };

using Value = std::variant<int, std::string>;

struct Condition {
    std::string column;
    CompareOp   op;
    Value       value;
};

struct Expression {
    // A simple expression tree: single condition or (left OP right)
    // For simplicity, we flatten to a list of conditions joined by AND/OR
    // This keeps the parser simple while supporting basic logic.
    std::vector<Condition> conditions;
    std::vector<LogicOp>   operators; // operators[i] joins conditions[i] and conditions[i+1]
};

// ============================================================
// Statements
// ============================================================
struct CreateTableStmt {
    std::string              tableName;
    std::vector<ColumnDef>   columns;
};

struct InsertStmt {
    std::string              tableName;
    std::vector<Value>       values;
};

struct SelectStmt {
    std::string              tableName;
    std::vector<std::string> columns; // empty = SELECT *
    bool                     selectAll = false;
    bool                     hasWhere  = false;
    Expression               where;
};

// Wrapper for any SQL statement
enum class StmtType { CREATE_TABLE, INSERT, SELECT };

struct Statement {
    StmtType type;
    CreateTableStmt createTable;
    InsertStmt      insert;
    SelectStmt      select;
};
