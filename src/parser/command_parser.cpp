#include "command_parser.hpp"

namespace rewrite {

    bool is_whitespace(const char c) {
	return std::isspace(c) || c == ';' || c == '?' || c == '*';
    }

    bool is_number(const char c) {
	return std::isdigit(c) || c == '+' || c == '-' || c == ',' || c == '.' || c == 'e' || c == 'E';
    }

    bool is_identifier_start(const char c) {
	return std::isalpha(c) || c == '_';
    }

    bool is_identifier(const char c) {
	return std::isalpha(c) || c == '_' || std::isdigit(c);
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

    std::ostream& operator<<(std::ostream& os, const ParsedLine& line) {
	os << "Parsed Line (" << line.line_nr << ")\n\nCommand: " << line.command << "\nParameter Sequence:\n";
	for (auto& v: line.sequence)
	    os << v.second << " ";
	os << "\nNamed Parameters:\n";
	for (auto& v: line.named_params)
	    os << "\t" << v.first << ": " << v.second << "\n";
	os << "\nNumber Parameters:\n";
	for (auto& v: line.num_params)
	    os << v << "; ";
	os << "\nID Parameters:\n";
	for (auto& v: line.id_params)
	    os << v << "; ";
	os << "\nString Parameters:\n";
	for (auto& v: line.str_params)
	    os << "\t" << v << "\n";

	return os;
    }

    auto CommandParser::get_command()
	-> std::expected<void, std::string> {
	
	if (expect_next<is_slash>()) {
	    parsed_line.type = ParsedLine::Type::ClosingTag;
	    expect_next<is_identifier>();
	}

	if (!is_identifier(*pos))
	    return std::unexpected { "Expected Polaris command after '<'" };

	parsed_line.command = read_while<is_identifier>();
	--pos;

	while (expect_next<is_identifier>()) {
	    const auto	param_name = read_while<is_identifier>();
	    --pos;

	    if (!expect_next<is_equals>())
		return std::unexpected{ "Expected '=' after named parameter" };

	    if (!expect_next<is_quote>())
		return std::unexpected { "Expected String after named parameter" };

	    parsed_line.named_params[std::string{param_name}] = unquote(read_while<is_string>());
	}

	if (pos >= current_line.end() || *pos != '>')
	    return std::unexpected{ "Expected '>' after command" };

	return {};
    }

    auto CommandParser::parse_line(const std::string& line)
	-> std::expected<void, std::string> {

	parsed_line.clear();

	current_line = line;
	++parsed_line.line_nr;
	pos = current_line.begin();

	while (pos < current_line.end()) {
	    const char c = *pos;

	    switch (c) {
		case '#':
		case '!':
		    return {};

		case '"':
		    parsed_line.push_param<ParsedLine::ParamType::String>(
			unquote(read_while<is_string>()));

		    if (pos >= current_line.end())
			return std::unexpected{ "Missing '\"'" };
		    break;

		case '<':
		    if (const auto e = get_command();
			not e) return std::unexpected{ e.error() };
		    break;

		default:
		    if (is_whitespace(c))
			break;

		    else if (is_identifier_start(c))
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

	    read_while<is_whitespace>();
	}

	return {};
    }
}
