#include <string_view>
#include <string>
#include <cctype>
#include <expected>
#include <vector>
#include <charconv>
#include <sstream>

#include "polaris_commands.hpp"

namespace rewrite {

    /**
     *
     */
    template<typename... Args>
    std::string comp_error(const std::string& msg, Args... args) {
	return (std::ostringstream(msg) << ... << args).str();
    }


    /**
     *
     */
    using CharCheckFn = bool(*)(const char);

    bool is_whitespace(const char c);
    bool is_number(const char c);
    bool is_identifier(const char c);
    bool is_quote(const char c);
    bool is_string(const char c);
    bool is_equals(const char c);


    /**
     *
     */
    struct ParsedParameter {
	enum class Type {
	    Identifier,
	    String,
	    Number
	};

	std::string_view	value;
	Type			type;

	/**
	 *
	 */
	auto as_number() -> std::expected<double, std::string> {
	    double	result{0};
	    auto 	[ptr, ec] = std::from_chars(value.begin(), value.end(), result);

	    if (ec == std::errc{})
		return result;
	    if (ec == std::errc::invalid_argument)
		return std::unexpected { "Could not convert number" };
	    return std::unexpected{ "Unknown conversion Error" };
	}

	/**
	 *
	 */
	auto unquoted() -> std::string_view {
	    return value.substr(1, value.length() - 2);
	}
    };


    /**
     * Fully parsed line with all found parameters
     * The decision to use the parameters-vector to store named parameters
     * as prepended name-value pairs was made as maps won't have much of
     * a performance increase with small numbers of parameters, but will
     * take much more space.
     *
     * @ivar command: Holds the command (<command>) of this line
     * @ivar paramters: List of all found parameters. Named parameters are
     * 			prepended as pairs (2 indices per named parameter as
     * 			name - value pair). The effective index of positional
     * 			parameters starts at named_parameter_count * 2 + 0
     * 	@ivar named_parameter_count: Number of named parameters
     *
     */
    struct ParsedLine {
	std::string_view		command;
	std::vector<ParsedParameter>	parameters;
	unsigned int			named_parameter_count{0};

	/**
	 * Get named parameter by its name
	 * @param name: String representing the desired parameter
	 * @returns: An unquoted string of the found named parameter or
	 * 	     std::unexpected if no matching parameter was found.
	 */
	auto get_named(const std::string& name)
	    -> std::expected<std::string_view, std::string> {
	    for(auto i = 0; i < named_parameter_count; i += 2)
		if (parameters[i].value == name)
		    return parameters[i + 1].unquoted();
	    return std::unexpected { comp_error( "Unknown named parameter ", name) };
	}

	/**
	 *
	 */
	auto get_value(const unsigned int i) const {
	    return parameters[named_parameter_count * 2 + i];
	}

	auto operator[](const unsigned int i) const {
	    return get_value(i);
	}
    };


    /**
     *
     */
    class CommandParser {
	private:
	    struct Token {
		enum class Type {
		    Number, Identifier, String, Quote = '"',
		    OpenTag = '<', CloseTag = '>', Slash = '/'
		};

		std::string_view	value;
	    };

	    std::string_view			current_line;
	    std::string_view::const_iterator	pos;
	    unsigned int			line_nr{0};
	
	    /**
	     *
	     */
	    template<CharCheckFn check>
	    std::string_view read_while() {
		const auto	start = pos;

		while (++pos < current_line.cend() && check(*pos));

		return current_line.substr(
		    std::distance(current_line.cbegin(), start),
		    std::distance(start, pos));
	    }

	    /**
	     *
	     */
	    template<CharCheckFn check>
	    bool expect_next() {
		read_while<is_whitespace>();
		return pos < current_line.cend() && check(*pos);
	    }

	    /**
	     *
	     */
	    auto get_command(ParsedLine& result)
		-> std::expected<bool, std::string> {

		if (!expect_next<is_identifier>())
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

		return true;
	    }

	public:
	    /**
	     *
	     */
	    auto parse_line(const std::string& line)
		-> std::expected<ParsedLine, std::string> {

		ParsedLine		result;

		current_line = line;
		line_nr++;
		pos = current_line.cbegin();

		while (pos < current_line.cend()) {
		    const char c = *pos;

		    switch (c) {
			case '#':
			case '!':
			    return result;

			case '"':
			    result.parameters.emplace_back(
				    read_while<is_string>(),
				    ParsedParameter::Type::String);
			    if (pos == current_line.end())
				return std::unexpected{ "Missing '\"'" };
			    continue;

			case '<':
			    if (const auto e = get_command(result);
				not e) return std::unexpected{ e.error() };
			    continue;

			default:
			    if (is_identifier(c))
				result.parameters.emplace_back(
				    read_while<is_identifier>(), ParsedParameter::Type::Identifier);

			    else if (is_number(c))
				result.parameters.emplace_back(
				    read_while<is_number>(), ParsedParameter::Type::Number);

			    else
				return std::unexpected{
				    comp_error( "Unknown Token '", c, "'" ) };
		    }

		    ++pos;
		}

		return result;
	    }
    };
}
