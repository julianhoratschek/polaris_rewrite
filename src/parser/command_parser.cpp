#include "command_parser.hpp"

#include <fstream>

namespace rewrite {

    namespace {
	/// Treat ;?* as whitespace for backwards-compatibility
	inline bool is_whitespace(const char c) {
	    return std::isspace(c) || c == ';' || c == '?' || c == '*'; }

	inline bool is_number(const char c) {
	    return std::isdigit(c) || c == '+' || c == '-' || c == ',' || c == '.' || c == 'e' || c == 'E'; }

	/// Identifiers may not start with numbers
	inline bool is_identifier_start(const char c) {
	    return std::isalpha(c) || c == '_'; }

	/// Identifiers may contain numbers in their name
	inline bool is_identifier(const char c) {
	    return std::isalpha(c) || c == '_' || std::isdigit(c); }

	inline bool is_quote(const char c) {
	    return c == '"'; }

	inline bool is_string(const char c) {
	    return c != '"'; }

	inline bool is_equals(const char c) {
	    return c == '='; }

	inline bool is_slash(const char c) {
	    return c == '/'; }
    }


    template<CharCheckFn check>
    inline std::string_view CommandParser::read_while() {
	const auto	start = pos;

	while (++pos < current_line.end() && check(*pos))
	    if constexpr (check == is_number)
		if(*pos == ',') *pos = '.';

	return {start, pos};
    }


    template<CharCheckFn check>
    inline bool CommandParser::expect_next() {
	read_while<is_whitespace>();
	return pos < current_line.end() && check(*pos);
    }


    auto CommandParser::get_command()
	-> std::expected<void, std::string> {

	if (expect_next<is_slash>()) {
	    parsed_line.type = ParsedLine::Type::ClosingTag;
	    expect_next<is_identifier>();
	}
	else
	    parsed_line.type = ParsedLine::Type::Command;

	if (!is_identifier(*pos))
	    return std::unexpected { "Expected Polaris command after '<[/]'" };

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

			// TODO: rather have numbers converted in one go
			//       after saving them as stringviews
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


    auto CommandParser::parse_file(const std::filesystem::path& path, ProcessLineFn proc)
	-> std::expected<void, std::string> {

	std::ifstream	file(path);
	std::string	line;

	while(std::getline(file, line)) {
	    if (const auto res = parse_line(line); not res)
		return std::unexpected { comp_error(
		    "Parsing Error [", parsed_line.line_nr, ":", std::distance(current_line.begin(), pos), "]: ",
		    res.error()) };

	    if (const auto res = proc(parsed_line); !res)
		return std::unexpected { comp_error(
		    "Processing Error [", parsed_line.line_nr, ":", std::distance(current_line.begin(), pos), "]:",
		    res.error()) };
	}

	return {};
    }
}
