#pragma once
#include "ast.h"
#include "database.h"
#include <string>
#include <vector>
#include <sstream>
#include <iomanip>
#include <algorithm>

struct QueryResult {
    bool        ok = true;
    std::string message;
    // For SELECT queries: column names + result rows
    std::vector<std::string> columnNames;
    std::vector<Row>         rows;
};

class Executor {
public:
    explicit Executor(Database& db) : db_(db) {}

    QueryResult execute(const Statement& stmt) {
        switch (stmt.type) {
            case StmtType::CREATE_TABLE: return execCreate(stmt.createTable);
            case StmtType::INSERT:       return execInsert(stmt.insert);
            case StmtType::SELECT:       return execSelect(stmt.select);
        }
        return errorResult("Unknown statement type");
    }

    // Format a QueryResult as a pretty ASCII table string
    static std::string formatResult(const QueryResult& result) {
        if (!result.ok)          return "ERROR: " + result.message;
        if (result.rows.empty() && result.columnNames.empty())
            return result.message;

        // Compute column widths
        size_t numCols = result.columnNames.size();
        std::vector<size_t> widths(numCols, 0);
        for (size_t c = 0; c < numCols; c++) {
            widths[c] = result.columnNames[c].size();
        }
        // Convert values to strings first
        std::vector<std::vector<std::string>> strRows;
        for (auto& row : result.rows) {
            std::vector<std::string> sr;
            for (size_t c = 0; c < numCols; c++) {
                std::string s = valueToString(row[c]);
                widths[c] = std::max(widths[c], s.size());
                sr.push_back(s);
            }
            strRows.push_back(sr);
        }

        // Build output
        std::ostringstream out;
        // Separator line
        auto sep = [&]() {
            out << "+";
            for (size_t c = 0; c < numCols; c++) {
                out << std::string(widths[c] + 2, '-') << "+";
            }
            out << "\n";
        };

        sep();
        // Header
        out << "|";
        for (size_t c = 0; c < numCols; c++) {
            out << " " << std::left << std::setw(static_cast<int>(widths[c]))
                << result.columnNames[c] << " |";
        }
        out << "\n";
        sep();
        // Rows
        for (auto& sr : strRows) {
            out << "|";
            for (size_t c = 0; c < numCols; c++) {
                out << " " << std::left
                    << std::setw(static_cast<int>(widths[c]))
                    << sr[c] << " |";
            }
            out << "\n";
        }
        sep();
        out << strRows.size() << " row(s) returned";
        return out.str();
    }

private:
    Database& db_;

    static QueryResult errorResult(const std::string& msg) {
        QueryResult r;
        r.ok = false;
        r.message = msg;
        return r;
    }

    static std::string valueToString(const Value& v) {
        if (std::holds_alternative<int>(v))
            return std::to_string(std::get<int>(v));
        return std::get<std::string>(v);
    }

    // ---- CREATE TABLE ----
    QueryResult execCreate(const CreateTableStmt& stmt) {
        db_.createTable(stmt.tableName, stmt.columns);
        QueryResult r;
        r.message = "Table '" + stmt.tableName + "' created";
        return r;
    }

    // ---- INSERT ----
    QueryResult execInsert(const InsertStmt& stmt) {
        db_.insertRow(stmt.tableName, stmt.values);
        QueryResult r;
        r.message = "1 row inserted into '" + stmt.tableName + "'";
        return r;
    }

    // ---- SELECT ----
    QueryResult execSelect(const SelectStmt& stmt) {
        const Table& table = db_.getTable(stmt.tableName);

        // Resolve which column indices to output
        std::vector<int>         colIndices;
        std::vector<std::string> colNames;
        if (stmt.selectAll) {
            for (size_t i = 0; i < table.columns.size(); i++) {
                colIndices.push_back(static_cast<int>(i));
                colNames.push_back(table.columns[i].name);
            }
        } else {
            for (auto& name : stmt.columns) {
                int idx = table.columnIndex(name);
                if (idx < 0) throw std::runtime_error(
                    "Column '" + name + "' not found in table '" +
                    stmt.tableName + "'");
                colIndices.push_back(idx);
                colNames.push_back(name);
            }
        }

        // Filter rows
        std::vector<Row> results;
        for (auto& row : table.rows) {
            bool include = true;
            if (stmt.hasWhere) {
                include = evaluateExpression(stmt.where, table, row);
            }
            if (include) {
                // Project selected columns
                Row projected;
                for (int idx : colIndices) {
                    projected.push_back(row[static_cast<size_t>(idx)]);
                }
                results.push_back(projected);
            }
        }

        QueryResult r;
        r.columnNames = colNames;
        r.rows        = results;
        return r;
    }

    // ---- WHERE evaluation ----
    bool evaluateExpression(const Expression& expr,
                            const Table& table,
                            const Row& row) const {
        if (expr.conditions.empty()) return true;

        bool result = evaluateCondition(expr.conditions[0], table, row);
        for (size_t i = 0; i < expr.operators.size(); i++) {
            bool next = evaluateCondition(expr.conditions[i + 1], table, row);
            if (expr.operators[i] == LogicOp::AND)
                result = result && next;
            else
                result = result || next;
        }
        return result;
    }

    bool evaluateCondition(const Condition& cond,
                           const Table& table,
                           const Row& row) const {
        int idx = table.columnIndex(cond.column);
        if (idx < 0) throw std::runtime_error(
            "WHERE: column '" + cond.column + "' not found");

        const Value& cellValue = row[static_cast<size_t>(idx)];

        // Compare based on types
        if (std::holds_alternative<int>(cellValue) &&
            std::holds_alternative<int>(cond.value)) {
            int a = std::get<int>(cellValue);
            int b = std::get<int>(cond.value);
            return applyOp(a, b, cond.op);
        }
        if (std::holds_alternative<std::string>(cellValue) &&
            std::holds_alternative<std::string>(cond.value)) {
            const auto& a = std::get<std::string>(cellValue);
            const auto& b = std::get<std::string>(cond.value);
            return applyOp(a, b, cond.op);
        }
        throw std::runtime_error("Type mismatch in WHERE condition for column '"
                                 + cond.column + "'");
    }

    template <typename T>
    bool applyOp(const T& a, const T& b, CompareOp op) const {
        switch (op) {
            case CompareOp::EQ:  return a == b;
            case CompareOp::NEQ: return a != b;
            case CompareOp::LT:  return a <  b;
            case CompareOp::GT:  return a >  b;
            case CompareOp::LTE: return a <= b;
            case CompareOp::GTE: return a >= b;
        }
        return false;
    }
};
