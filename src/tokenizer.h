#pragma once
#include <string>
#include <vector>
#include <cctype>
#include <algorithm>

enum class TokenType {
    // Keywords
    CREATE, TABLE, INSERT, INTO, VALUES, SELECT, FROM, WHERE, AND, OR,
    // Types
    INT, TEXT,
    // Symbols
    LPAREN, RPAREN, COMMA, SEMICOLON, STAR,
    // Operators
    EQ, NEQ, LT, GT, LTE, GTE,
    // Literals
    INTEGER_LITERAL, STRING_LITERAL, IDENTIFIER,
    // Special
    END_OF_INPUT, UNKNOWN
};

struct Token {
    TokenType type;
    std::string value;
    
    Token(TokenType t, std::string v) : type(t), value(std::move(v)) {}
};

class Tokenizer {
public:
    explicit Tokenizer(const std::string& input) : input_(input), pos_(0) {}

    std::vector<Token> tokenize() {
        std::vector<Token> tokens;
        while (pos_ < input_.size()) {
            skipWhitespace();
            if (pos_ >= input_.size()) break;

            char c = input_[pos_];

            if (c == '(') { tokens.emplace_back(TokenType::LPAREN, "("); pos_++; }
            else if (c == ')') { tokens.emplace_back(TokenType::RPAREN, ")"); pos_++; }
            else if (c == ',') { tokens.emplace_back(TokenType::COMMA, ","); pos_++; }
            else if (c == ';') { tokens.emplace_back(TokenType::SEMICOLON, ";"); pos_++; }
            else if (c == '*') { tokens.emplace_back(TokenType::STAR, "*"); pos_++; }
            else if (c == '=' ) { tokens.emplace_back(TokenType::EQ, "="); pos_++; }
            else if (c == '!' && peek() == '=') { tokens.emplace_back(TokenType::NEQ, "!="); pos_ += 2; }
            else if (c == '<' && peek() == '=') { tokens.emplace_back(TokenType::LTE, "<="); pos_ += 2; }
            else if (c == '>' && peek() == '=') { tokens.emplace_back(TokenType::GTE, ">="); pos_ += 2; }
            else if (c == '<') { tokens.emplace_back(TokenType::LT, "<"); pos_++; }
            else if (c == '>') { tokens.emplace_back(TokenType::GT, ">"); pos_++; }
            else if (c == '\'') { tokens.push_back(readString()); }
            else if (std::isdigit(c)) { tokens.push_back(readNumber()); }
            else if (std::isalpha(c) || c == '_') { tokens.push_back(readIdentifierOrKeyword()); }
            else { tokens.emplace_back(TokenType::UNKNOWN, std::string(1, c)); pos_++; }
        }
        tokens.emplace_back(TokenType::END_OF_INPUT, "");
        return tokens;
    }

private:
    std::string input_;
    size_t pos_;

    char peek() const {
        return (pos_ + 1 < input_.size()) ? input_[pos_ + 1] : '\0';
    }

    void skipWhitespace() {
        while (pos_ < input_.size() && std::isspace(input_[pos_])) pos_++;
    }

    Token readString() {
        pos_++; // skip opening quote
        std::string result;
        while (pos_ < input_.size() && input_[pos_] != '\'') {
            result += input_[pos_++];
        }
        if (pos_ < input_.size()) pos_++; // skip closing quote
        return Token(TokenType::STRING_LITERAL, result);
    }

    Token readNumber() {
        std::string result;
        while (pos_ < input_.size() && std::isdigit(input_[pos_])) {
            result += input_[pos_++];
        }
        return Token(TokenType::INTEGER_LITERAL, result);
    }

    Token readIdentifierOrKeyword() {
        std::string result;
        while (pos_ < input_.size() && (std::isalnum(input_[pos_]) || input_[pos_] == '_')) {
            result += input_[pos_++];
        }
        // Convert to uppercase for keyword matching
        std::string upper = result;
        std::transform(upper.begin(), upper.end(), upper.begin(), ::toupper);

        if (upper == "CREATE") return Token(TokenType::CREATE, upper);
        if (upper == "TABLE")  return Token(TokenType::TABLE, upper);
        if (upper == "INSERT") return Token(TokenType::INSERT, upper);
        if (upper == "INTO")   return Token(TokenType::INTO, upper);
        if (upper == "VALUES") return Token(TokenType::VALUES, upper);
        if (upper == "SELECT") return Token(TokenType::SELECT, upper);
        if (upper == "FROM")   return Token(TokenType::FROM, upper);
        if (upper == "WHERE")  return Token(TokenType::WHERE, upper);
        if (upper == "AND")    return Token(TokenType::AND, upper);
        if (upper == "OR")     return Token(TokenType::OR, upper);
        if (upper == "INT")    return Token(TokenType::INT, upper);
        if (upper == "TEXT")   return Token(TokenType::TEXT, upper);

        return Token(TokenType::IDENTIFIER, result);
    }
};
