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

    bool is_slash(const char c) {
	return c == '/';
    }


    auto CommandParser::get_command(ParsedLine& result)
	-> std::expected<bool, std::string> {

	if (expect_next<is_slash>())
	    result.type = ParsedLine::Type::ClosingTag;
	else if (!is_identifier(*pos))
	    return std::unexpected { "Expected Polaris command after '<'" };

	result.command = read_while<is_identifier>();
	while (expect_next<is_identifier>()) {
	    const auto	param_name = read_while<is_identifier>();
	    if (!expect_next<is_equals>())
		return std::unexpected{ "Expected '=' after named parameter" };
	    if (!expect_next<is_quote>())
		return std::unexpected { "Expected String after named parameter" };
	    const auto param_value = read_while<is_string>();

	    result.parameters.emplace_back(param_name, ParsedParameter::Type::Identifier);
	    result.parameters.emplace_back(param_value, ParsedParameter::Type::String);
	}

	if (pos >= current_line.cend() || *pos != '>')
	    return std::unexpected{ "Expected '>' after command" };
	++pos;

	return true;
    }
}
