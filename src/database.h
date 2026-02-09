#pragma once
#include "ast.h"
#include <string>
#include <vector>
#include <unordered_map>
#include <variant>
#include <stdexcept>

// A row is just a vector of Values (one per column, in order)
using Row = std::vector<Value>;

struct Table {
    std::string            name;
    std::vector<ColumnDef> columns;
    std::vector<Row>       rows;

    // Lookup column index by name, returns -1 if not found
    int columnIndex(const std::string& colName) const {
        for (size_t i = 0; i < columns.size(); i++) {
            if (columns[i].name == colName) return static_cast<int>(i);
        }
        return -1;
    }
};

class Database {
public:
    // Create a new table
    void createTable(const std::string& name,
                     const std::vector<ColumnDef>& columns) {
        if (tables_.count(name)) {
            throw std::runtime_error("Table '" + name + "' already exists");
        }
        Table table;
        table.name    = name;
        table.columns = columns;
        tables_[name] = table;
    }

    // Insert a row into a table
    void insertRow(const std::string& tableName,
                   const std::vector<Value>& values) {
        Table& table = getTable(tableName);
        if (values.size() != table.columns.size()) {
            throw std::runtime_error(
                "Column count mismatch: table has " +
                std::to_string(table.columns.size()) + " columns, got " +
                std::to_string(values.size()) + " values");
        }
        // Basic type checking
        for (size_t i = 0; i < values.size(); i++) {
            bool isInt  = std::holds_alternative<int>(values[i]);
            bool expectInt = (table.columns[i].type == ColumnType::INT);
            if (isInt != expectInt) {
                throw std::runtime_error(
                    "Type mismatch for column '" + table.columns[i].name +
                    "': expected " + (expectInt ? "INT" : "TEXT"));
            }
        }
        table.rows.push_back(values);
    }

    // Get table reference (throws if not found)
    Table& getTable(const std::string& name) {
        auto it = tables_.find(name);
        if (it == tables_.end()) {
            throw std::runtime_error("Table '" + name + "' not found");
        }
        return it->second;
    }

    const Table& getTable(const std::string& name) const {
        auto it = tables_.find(name);
        if (it == tables_.end()) {
            throw std::runtime_error("Table '" + name + "' not found");
        }
        return it->second;
    }

private:
    std::unordered_map<std::string, Table> tables_;
};
