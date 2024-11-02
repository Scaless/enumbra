// The following file was generated by ChatGPT 4o
// The output is "good enough", but not optimized and doesn't cover every edge case.

#pragma once

#include <iostream>
#include <string>
#include <cctype>
#include <stdexcept>
#include <absl/numeric/int128.h>

// Function declarations
absl::int128 parseExpression(const std::string& expression, size_t& index);
absl::int128 parseTerm(const std::string& expression, size_t& index);
absl::int128 parseFactor(const std::string& expression, size_t& index);
absl::int128 parseNumber(const std::string& expression, size_t& index);

// Main function to evaluate an expression
absl::int128 evaluate(const std::string& expression) {
    size_t index = 0;
    absl::int128 result = parseExpression(expression, index);

    // Ensure the entire expression has been parsed
    if (index < expression.length()) {
        throw std::runtime_error("Unexpected characters after expression");
    }
    return result;
}

absl::int128 parseExpression(const std::string& expression, size_t& index) {
    absl::int128 result = parseTerm(expression, index);

    while (index < expression.length()) {
        char op = expression[index];
        if (op == '+') {
            index++;
            result += parseTerm(expression, index);
        }
        else if (op == '-') {
            index++;
            result -= parseTerm(expression, index);
        }
        else {
            break;
        }
    }
    return result;
}

absl::int128 parseTerm(const std::string& expression, size_t& index) {
    absl::int128 result = parseFactor(expression, index);

    while (index < expression.length()) {
        char op = expression[index];
        if (op == '|') {
            index++;
            result |= parseFactor(expression, index);
        }
        else if (op == '<' && index + 1 < expression.length() && expression[index + 1] == '<') {
            index += 2; // Skip '<<'
            result <<= int(parseFactor(expression, index));
        }
        else if (op == '>' && index + 1 < expression.length() && expression[index + 1] == '>') {
            index += 2; // Skip '>>'
            result >>= int(parseFactor(expression, index));
        }
        else {
            break;
        }
    }
    return result;
}

absl::int128 parseFactor(const std::string& expression, size_t& index) {
    if (index >= expression.length()) {
        throw std::runtime_error("Unexpected end of expression");
    }

    char current = expression[index];

    // Check for negative numbers
    if (current == '-') {
        index++; // Skip the '-'
        return -parseFactor(expression, index); // Negate the factor
    }

    if (std::isdigit(current)) {
        return parseNumber(expression, index);
    }
    else if (current == '(') {
        index++; // Skip '('
        absl::int128 result = parseExpression(expression, index);
        if (index >= expression.length() || expression[index] != ')') {
            throw std::runtime_error("Mismatched parentheses");
        }
        index++; // Skip ')'
        return result;
    }
    else {
        throw std::runtime_error("Unexpected character: " + std::string(1, current));
    }
}

absl::int128 parseNumber(const std::string& expression, size_t& index) {
    size_t start = index; // Use size_t to prevent conversion issues
    while (index < expression.length() && std::isdigit(expression[index])) {
        index++;
    }
    return absl::int128{ std::stoll(expression.substr(start, index - start)) }; // Use stoll for 64-bit conversion
}