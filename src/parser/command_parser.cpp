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


    auto CommandParser::get_command()
	-> std::expected<void, std::string> {

	if (expect_next<is_slash>()) {
	    parsed_line.type = ParsedLine::Type::ClosingTag;
	    ++pos;
	}

	if (!is_identifier(*pos))
	    return std::unexpected { "Expected Polaris command after '<'" };

	parsed_line.command = read_while<is_identifier>();

	while (expect_next<is_identifier>()) {
	    const auto	param_name = read_while<is_identifier>();

	    if (!expect_next<is_equals>())
		return std::unexpected{ "Expected '=' after named parameter" };

	    if (!expect_next<is_quote>())
		return std::unexpected { "Expected String after named parameter" };

	    parsed_line.named_params[std::string{param_name}] = unquote(read_while<is_string>());
	}

	if (pos >= current_line.cend() || *pos != '>')
	    return std::unexpected{ "Expected '>' after command" };
	++pos;

	return {};
    }

    auto CommandParser::parse_line(const std::string& line)
	-> std::expected<void, std::string> {

	parsed_line.clear();

	current_line = line;
	++parsed_line.line_nr;
	pos = current_line.begin();

	while (pos < current_line.end()) {
	    read_while<is_whitespace>();
	    const char c = *pos;

	    switch (c) {
		case '#':
		case '!':
		    return {};

		case '"':
		    parsed_line.push_param<ParsedLine::ParamType::String>(
			unquote(read_while<is_string>()));

		    if (pos == current_line.end())
			return std::unexpected{ "Missing '\"'" };

		    continue;

		case '<':
		    if (const auto e = get_command();
			not e) return std::unexpected{ e.error() };
		    continue;

		default:
		    if (is_identifier(c))
			parsed_line.push_param<ParsedLine::ParamType::Identifier>(
			    read_while<is_identifier>());

		    else if (is_number(c)) {
			const auto	tmp_string = read_while<is_number>();

			try {
			    double val = 0;
			    std::from_chars(tmp_string.begin(), tmp_string.end(), val);
			    parsed_line.push_param<ParsedLine::ParamType::Number>(val);
			}
			catch ( std::out_of_range ) {
			    return std::unexpected{ "Number Parameter is too large" };
			}
			catch ( std::invalid_argument ) {
			    return std::unexpected{ "Ill-formed Number" };
			}
		    }
		    else
			return std::unexpected{ comp_error( "Unknown Token '", c, "'" ) };
	    }
	}

	return {};
    }
}
