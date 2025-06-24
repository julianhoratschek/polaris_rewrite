#ifndef RW_COMMAND_PARSER
#define RW_COMMAND_PARSER

#include <string_view>
#include <string>
#include <expected>
#include <optional>
#include <vector>
#include <charconv>
#include <sstream>

#include <filesystem>
#include <fstream>

// TODO: we don't want this
#include <iostream>

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
    bool is_slash(const char c);


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
	// TODO: Do we need to unquote strings before from_chars?
	auto as_number() const -> std::expected<double, std::string> {
	    double	result{0};
	    auto 	[ptr, ec] = std::from_chars(value.begin(), value.end(), result);

	    if (ec == std::errc{})
		return result;
	    if (ec == std::errc::invalid_argument)
		return std::unexpected{ "Could not convert number" };
	    return std::unexpected{ "Unknown conversion Error" };
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
	enum class Type {
	    ClosingTag, Command
	};

	std::string_view		command;
	Type				type { Type::Command };
	std::vector<ParsedParameter>	parameters;
	unsigned int			named_parameter_count{0};

	/**
	 * Get named parameter by its name
	 * @param name: String representing the desired parameter
	 * @returns: An unquoted string of the found named parameter or
	 * 	     std::unexpected if no matching parameter was found.
	 */
	auto get_named(const std::string& name) const
	    -> std::optional<ParsedParameter> {
	    for(auto i = 0; i < named_parameter_count; i += 2)
		if (parameters[i].value == name)
		    return parameters[i + 1];
	    return {};
	}


	// TODO safer
	auto num_values(std::vector<double>& out, const size_t from=0) 
	    -> std::optional<std::string> {

	    const size_t start = named_parameter_count * 2 + from;
	    size_t skipped = 0;
	    for (auto i = start; i < parameters.size(); i++)
		if (const auto n = parameters[i].as_number(); n.has_value())
		    out[i - start - skipped] = n.value();
		else ++skipped;
	}

	auto get_optional(const size_t i) const
	    -> std::optional<ParsedParameter> {

	    const size_t idx = named_parameter_count * 2 + i;
	    if (idx < parameters.size())
		return parameters.at(idx);
	    return {};
	}

	/**
	 *
	 */
	auto get_value(const unsigned int i) const {
	    return parameters.at(named_parameter_count * 2 + i);
	}

	auto operator[](const unsigned int i) const {
	    return get_value(i);
	}

	size_t size() const {
	    return parameters.size() - (named_parameter_count * 2);
	}

	void clear() {
	    parameters.clear();
	    type = Type::Command;
	    named_parameter_count = 0;
	}
    };


    /**
     *
     */
    class CommandParser {
	private:
	    std::string				current_line;
	    std::string::iterator		pos;
	    unsigned int			line_nr{0};
	
	    /**
	     *
	     */
	    template<CharCheckFn check>
	    std::string_view read_while() {
		const auto	start = pos;

		while (++pos < current_line.end() && check(*pos))
		    if constexpr (check == is_number)
			if(*pos == ',')
			    *pos = '.';

		return {current_line.substr(
		    std::distance(current_line.begin(), start),
		    std::distance(start, pos))};
	    }

	    /**
	     * Skips whitespace and then returns true if check returns
	     * true for the next character.
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
		-> std::expected<bool, std::string>;

	public:
	    /**
	     * @param out: Reuse ParsedLine to have its memory hotloaded
	     */
	    auto parse_line(const std::string& line, ParsedLine &out)
		-> std::expected<ParsedLine*, std::string> {

		std::string_view	tmp_string;

		out.parameters.clear();

		current_line = line;
		line_nr++;
		pos = current_line.begin();

		while (pos < current_line.end()) {
		    const char c = *pos;

		    switch (c) {
			case '#':
			case '!':
			    return &out;

			case '"':
			    tmp_string = read_while<is_string>();
			    out.parameters.emplace_back(
				    tmp_string.substr(1, tmp_string.size() - 2),
				    ParsedParameter::Type::String);
			    if (pos == current_line.end())
				return std::unexpected{ "Missing '\"'" };
			    continue;

			case '<':
			    if (const auto e = get_command(out);
				not e) return std::unexpected{ e.error() };
			    continue;

			default:
			    if (is_identifier(c))
				out.parameters.emplace_back(
				    read_while<is_identifier>(), ParsedParameter::Type::Identifier);

			    else if (is_number(c))
				out.parameters.emplace_back(
				    read_while<is_number>(), ParsedParameter::Type::Number);

			    else
				return std::unexpected{
				    comp_error( "Unknown Token '", c, "'" ) };
		    }

		    ++pos;
		}

		return &out;
	    }

	    using ProcessFn = bool(*)(const ParsedLine&);
	    void parse_file(const std::filesystem::path& path, ProcessFn proc) {
		std::ifstream		file(path);
		std::string		line;
		ParsedLine		parsed;

		while(std::getline(file, line)) {
		    if (const auto res = parse_line(line, parsed); not res) {
			std::cout << "Parsing Error ["
			    << line_nr << ":" << std::distance(current_line.cbegin(), pos) << "]: "
			    << res.error();
			continue;
		    }

		    proc(parsed);
		}

	    }
    };
}

#endif
