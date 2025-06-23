#include "command_parser.hpp"

namespace rewrite {

    bool is_whitespace(const char c) {
	return std::isspace(c) || c == ';' || c == '?' || c == '*';
    }

    bool is_number(const char c) {
	return std::isdigit(c) || c == '+' || c == '-' || c == ',' || c == '.' || c == 'e' || c == 'E';
    }

    bool is_identifier(const char c) {
	return std::isalpha(c) || c == '_';
    }

    bool is_quote(const char c) {
	return c == '"';
    }

    bool is_string(const char c) {
	return c != '"';
    }

    bool is_equals(const char c) {
	return c == '=';
    }
}
