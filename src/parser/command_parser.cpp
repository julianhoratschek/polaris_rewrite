#include "command_parser.hpp"
#include <string_view>

namespace rewrite {

    // Don't leak these functions outside translation unit
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


    auto CommandParser::get_number()
	-> std::expected<double, Message> {
	// TODO: rather have numbers converted in one go
	//       after saving them as stringviews
	//
	const auto	tmp_string = read_while<is_number>();
	double val = 0;

	try {
	    std::from_chars(tmp_string.begin(), tmp_string.end(), val);
	}
	catch (const std::out_of_range&) {
	    return std::unexpected{ Message { "Number Parameter is too large" } };
	}
	catch (const std::invalid_argument&) {
	    return std::unexpected{ Message { "Ill-formed Number" } };
	}

	return val;
    }


    auto CommandParser::get_command()
	-> std::expected<void, Message> {

	if (expect_next<is_slash>()) {
	    parsed_line.type = ParsedLine::Type::ClosingTag;
	    expect_next<is_identifier>();
	}
	else
	    parsed_line.type = ParsedLine::Type::Command;

	if (!is_identifier(*pos))
	    return std::unexpected { Message { "Expected Polaris command after '<[/]'" } };

	// --pos is needed between read_while and expect_next, to look at
	// the current character
	parsed_line.command = read_while<is_identifier>();
	--pos;


	while (expect_next<is_identifier>()) {
	    const auto param_name = read_while<is_identifier>();
	    --pos;

	    if (!expect_next<is_equals>())
		return std::unexpected{ Message { "Expected '=' after named parameter" } };

	    if (!expect_next<is_quote>())
		return std::unexpected { Message { "Expected String after named parameter" } };

	    std::vector<double>	named_params;
	    while (expect_next<is_number>()) {
		const auto num = get_number();
		if (!num.has_value())
		    return std::unexpected{ num.error() };
		named_params.push_back(num.value());
		--pos;
	    }

	    parsed_line.named_params[param_name] = std::move(named_params);
	}

	if (pos >= current_line.end() || *pos != '>')
	    return std::unexpected{ Message { "Expected '>' after command" } };

	return {};
    }


    auto CommandParser::set_line(const std::string& line)
	-> std::string::iterator {
	parsed_line.clear();

	current_line = line;
	++parsed_line.line_nr;
	pos = current_line.begin();
	read_while<is_whitespace>();

	return pos;
    }


    auto CommandParser::parse_line()
	-> std::expected<void, Message> {

	while (pos < current_line.end()) {
	    const char c = *pos;

	    switch (c) {

		// Get Comments
		case '#':
		case '!':
		    return {};


		// Get Strings
		case '"':
		    parsed_line.push_param<ParsedLine::ParamType::String>(
			unquote(read_while<is_string>()));

		    if (pos >= current_line.end())
			return std::unexpected{ Message { "Missing '\"'" } };
		    break;


		// Get commands (tags)
		case '<':
		    if (const auto e = get_command();
			not e) return std::unexpected{ e.error() };
		    break;


		// Get whitespace, numbers or identifiers
		default:
		    if (is_whitespace(c))
			break;

		    else if (is_identifier_start(c))
			parsed_line.push_param<ParsedLine::ParamType::Identifier>(
			    read_while<is_identifier>());

		    else if (is_number(c)) {
			const auto num = get_number();
			if (!num.has_value())
			    return std::unexpected{ num.error() };

			parsed_line.push_param<ParsedLine::ParamType::Number>(num.value());
		    }

		    else
			return std::unexpected{ Message { comp_error( "Unknown Token '", c, "'" ) } };
	    }

	    // Skip all whitespace until next character is found
	    read_while<is_whitespace>();
	}

	return {};
    }
}
