#ifndef RW_COMMAND_PARSER
#define RW_COMMAND_PARSER

#include "parsed_line.hpp"

#include <string_view>
#include <string>
#include <expected>
#include <fstream>

#include <filesystem>


namespace rewrite {

    template<typename Fn>
    concept IsProcessLineFn = requires (Fn fn, ParsedLine ln) {
	{ fn(ln) } -> std::same_as<std::expected<void, Message>>;
    };

    ///
    using CharCheckFn = bool(*)(const char);
    using HandleErrorFn = bool(*)(const Message&);

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

	auto get_number()
	    -> std::expected<double, Message>;


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
	auto get_command() -> std::expected<void, Message>;

	size_t error_distance() {
	    return std::distance(current_line.begin(), pos) - 2;
	}

	std::string error_pointer() {
	    return std::string(error_distance(), '~') + '^';
	}

    public:
	/**
	 * Will return the current ParsedLine object. Should only be called
	 * after successful call of `parse_line()`. On Failure, the content
	 * of the ParsedLine-object returned by this method is undefined.
	 */
	ParsedLine get_last_line() { return parsed_line; }

	static auto skip_indents(const std::string& line);


	/**
	 * Clears the last parsed line and sets `line` as new line
	 * for parsing. Returns `std::string::iterator` pointing at
	 * first non-whitespace character, or `line.end()` if line
	 * is empty.
	 */
	auto set_line(const std::string& line)
	    -> std::string::iterator;

	/**
	 * Parses one singular line. On Success the parsed line object can be
	 * retrieved with `get_last_line()`.
	 */
	auto parse_line()
	    -> std::expected<void, Message>;


	/**
	 * Reads `path` line by line, on success calls `proc` with the read
	 * ParsedLine object.
	 */
	template<typename ProcessLineFn>
	    requires IsProcessLineFn<ProcessLineFn>
	auto parse_file(std::ifstream& file, ProcessLineFn proc,
	    HandleErrorFn err_fn = nullptr) -> std::expected<void, Message> {

	    if (file.fail())
		return std::unexpected{ Message{ "Not a valid file" } };

	    std::string		line;

	    while (std::getline(file, line)) {
		set_line(line);

		// Parse Line
		const auto line_result = parse_line();

		// Process line on success
		if (line_result.has_value())
		    line_result = proc(parsed_line);

		// Handle errors from parsing or processing
		if (!line_result) {
		    const auto err = line_result.error();
		    const Message parser_error{ comp_error(
			    "[", parsed_line.line_nr, ':', error_distance(), "]: ",
			    err.message, '\n', current_line, '\n', error_pointer()),
			    err.type
			};

		    // Otherwise call user function if defined
		    if (err_fn && !err_fn(parser_error))
			return std::unexpected{ parser_error };
		}
	    }

	    return {};
	}
    };
}

#endif
