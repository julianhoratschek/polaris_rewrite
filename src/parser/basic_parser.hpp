#ifndef RW_COMMAND_PARSER
#define RW_COMMAND_PARSER

#include "message.hpp"

#include <cstddef>
#include <string_view>
#include <string>
#include <expected>
#include <fstream>


namespace rewrite {

    template<typename Fn>
    concept ErrorHandlerFn = requires (Fn fn, const Message msg) {
	{ fn(msg) } -> std::same_as<bool>;
    };
    
    /**
     * This is a strict per-line parser. Parsed lines are invalidated as soon
     * as the next line is read. Processing should be done by passing a
     * processing function pointer, which will be called after each line is
     * successfully parsed.
     */

    /**
     * Strict base class for static inheritance. Provides
     * multiple useful methods for file parsing
     */
    class BasicParser {
	using CharCheckFn = bool(*)(const char);

    protected:

	/// Treat ;?* as whitespace for backwards-compatibility
	static bool is_whitespace(const char c) {
	    return std::isspace(c) || c == ';' || c == '?' || c == '*'; }

	static bool is_number(const char c) {
	    return std::isdigit(c) || c == '+' || c == '-' || c == ',' || c == '.' || c == 'e' || c == 'E'; }

	/// Identifiers may not start with numbers
	static bool is_identifier_start(const char c) {
	    return std::isalpha(c) || c == '_'; }

	/// Identifiers may contain numbers in their name
	static bool is_identifier(const char c) {
	    return std::isalpha(c) || c == '_' || std::isdigit(c); }

	static bool is_quote(const char c) {
	    return c == '"'; }

	static bool is_string(const char c) {
	    return c != '"'; }

	static bool is_comment(const char c) {
	    return c == '#' || c == '!';
	}

	static bool is_equals(const char c) {
	    return c == '='; }

	static bool is_slash(const char c) {
	    return c == '/'; }


	/// Currently processed line
	std::string				current_line;

	/// Current position in current_line
	std::string::iterator			pos;

	size_t					line_nr{0};


	/**
	 * Reads text while `check` returns true.
	 * After a call, pos points at the first char, check()
	 * returns false for or current_line.end().
	 * @tparam check Function returning true as long as reading should be
	 * 		 continued
	 * @returns string_view of read characters.
	 */
	template<CharCheckFn check>
	inline std::string_view read_while() {
	    const auto	start = pos;

	    while (++pos < current_line.end() && check(*pos))
		if constexpr (check == &BasicParser::is_number)
		    if(*pos == ',') *pos = '.';

	    return {start, pos};
	}


	/**
	 * Skips whitespace and then returns true if check returns
	 * true for the next character. Starts with the next
	 * char after pos.
	 * After a call, pos points to the first expected char or
	 * at current_line.end()
	 */
	template<CharCheckFn check>
	bool expect_next() {
	    read_while<is_whitespace>();
	    return pos < current_line.end() && check(*pos);
	}


	template<CharCheckFn check>
	bool is_or_next() {
	    if (check(*pos))
		return true;
	    return expect_next<check>();
	}


	/**
	 * Checks, if the current char is a comment char,
	 * otherwise, if current char is whitespace, eats
	 * whitespace until another char ist found. Returns
	 * true, if that is a comment char.
	 */
	bool is_comment_line() {
	    return is_comment(*pos)
		|| (is_whitespace(*pos) && expect_next<is_comment>());
	}


	bool next_line(std::ifstream& in_file) {
	    while (std::getline(in_file, current_line)) {
		++line_nr;
		pos = current_line.begin();
		if (!is_comment_line() && pos < current_line.end())
		    return true;
	    }

	    return false;
	}

	/**
	 *
	 */
	auto get_number()
	    -> std::expected<double, Message>;

	auto get_string()
	    -> std::expected<std::string_view, Message>;

	size_t error_distance();

	std::string error_pointer();

	Message error_message(const Message& msg);
    };
}

#endif
