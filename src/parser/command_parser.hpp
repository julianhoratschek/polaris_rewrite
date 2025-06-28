#ifndef RW_COMMAND_PARSER
#define RW_COMMAND_PARSER

#include "parsed_line.hpp"

#include <string_view>
#include <string>
#include <expected>
#include <fstream>

#include <filesystem>
#include <iostream>


namespace rewrite {

    template<typename Fn>
    concept IsProcessLineFn = requires (Fn fn, ParsedLine ln) {
	{ fn(ln) } -> std::same_as<std::expected<void, std::string>>;
    };

    ///
    using CharCheckFn = bool(*)(const char);

    /**
     * This is a strict per-line parser. Parsed lines are invalidated as soon
     * as the next line is read. Processing should be done by passing a
     * processing function pointer, which will be called after each line is
     * successfully parsed.
     */
    class CommandParser {

	/// Currently processed line
	std::string				current_line;

	/// Current position in current_line
	std::string::iterator			pos;

	/// Result of currently parsed line (invalidated when a new line is read)
	ParsedLine				parsed_line;

	/**
	 * Removes beginning quote from read strings
	 * @param str view to process
	 * @returns `str` without its first character
	 */
	static std::string_view unquote(const std::string_view& str) {
	    return str.substr(1, str.size() - 1);
	}

	/**
	 * Reads text while `check` returns true.
	 * @tparam check Function returning true as long as reading should be
	 * 		 continued
	 * @returns string_view of read characters.
	 */
	template<CharCheckFn check>
	inline std::string_view read_while();

	/**
	 * Skips whitespace and then returns true if check returns
	 * true for the next character.
	 */
	template<CharCheckFn check>
	inline bool expect_next();

	/**
	 *
	 */
	auto get_command() -> std::expected<void, std::string>;

    public:
	// using ProcessLineFn = std::expected<void, std::string>(*)(ParsedLine&);

	/**
	 *
	 */
	ParsedLine get_last_line() { return parsed_line; }

	/**
	 * Parses one singular line. On Success the parsed line object can be
	 * retrieved with `get_last_line()`.
	 */
	auto parse_line(const std::string& line)
	    -> std::expected<void, std::string>;

	/**
	 * Reads `path` line by line, on success calls `proc` with the read
	 * ParsedLine object.
	 */
	template<typename ProcessLineFn>
	    requires IsProcessLineFn<ProcessLineFn>
	auto parse_file(const std::filesystem::path& path, ProcessLineFn proc)
	    -> std::expected<void, std::string> {

	    std::ifstream	file(path);
	    std::string		line;

	    if (file.fail())
		return std::unexpected{ "Could not open cmd file" };

	    while (std::getline(file, line)) {
		if (const auto res = parse_line(line); !res)
		    return std::unexpected { comp_error(
			"Parsing Error [", parsed_line.line_nr, ':', std::distance(current_line.begin(), pos), "]:",
			res.error()) };

		if (const auto res = proc(parsed_line); !res)
		    return std::unexpected { comp_error(
			"Processing Error [", parsed_line.line_nr, ':', std::distance(current_line.begin(), pos), "]:",
			res.error()) };
	    }

	    return {};
	}
    };
}

#endif
