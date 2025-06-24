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
#include <map>

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
     * Chose multiple vector layout because of multiple conversions during cmd
     * file processing. Memory fragmentation is preferable to multiple
     * loops through the same arrays.
     */
    struct ParsedLine {
	enum class Type {
	    ClosingTag, Command
	};

	std::string_view		command;
	Type				type{Type::Command};
	std::vector<double>		num_params;
	std::vector<std::string_view>	str_params;
	std::vector<std::string_view>	id_params;
	std::map<std::string, std::string_view> named_params;

	void clear() {
	    type = Type::Command;
	    num_params.clear();
	    str_params.clear();
	    id_params.clear();
	    named_params.clear();
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

	    ParsedLine				parsed_line;
	
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
	    auto get_command()
		-> std::expected<bool, std::string>;

	public:
	    /**
	     * @param out: Reuse ParsedLine to have its memory hotloaded
	     */
	    auto parse_line(const std::string& line)
		-> std::expected<ParsedLine*, std::string> {

		std::string_view	tmp_string;

		parsed_line.clear();

		current_line = line;
		line_nr++;
		pos = current_line.begin();

		while (pos < current_line.end()) {
		    const char c = *pos;

		    switch (c) {
			case '#':
			case '!':
			    return;

			case '"':
			    tmp_string = read_while<is_string>();
			    parsed_line.str_params.emplace_back(
				tmp_string.substr(1, tmp_string.size() - 2));
			    if (pos == current_line.end())
				return std::unexpected{ "Missing '\"'" };
			    continue;

			case '<':
			    if (const auto e = get_command(out);
				not e) return std::unexpected{ e.error() };
			    continue;

			default:
			    if (is_identifier(c))
				parsed_line.id_params.emplace_back(
				    read_while<is_identifier>());

			    else if (is_number(c)) {
				tmp_string = read_while<is_number>();
				try {
				    double val = 0;
				    std::from_chars(tmp_string.begin(), tmp_string.end(), val);
				    parsed_line.num_params.push_back(val);
				}
				catch ( std::out_of_range ) {
				    return std::unexpected{""};
				}
				catch ( std::invalid_argument ) {
				    return std::unexpected{""};
				}
			    }
			    else
				return std::unexpected{
				    comp_error( "Unknown Token '", c, "'" ) };
		    }

		    ++pos;
		}
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
