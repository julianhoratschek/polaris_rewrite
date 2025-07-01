#ifndef RW_COMMAND_PARSER
#define RW_COMMAND_PARSER

#include "message.hpp"

#include <string_view>
#include <string>
#include <expected>


namespace rewrite {

    template<typename Fn>
    concept IsErrorHandlerFn = requires (Fn fn, const Message msg) {
	{ fn(msg) } -> std::same_as<bool>;
    };
    
    /**
     * This is a strict per-line parser. Parsed lines are invalidated as soon
     * as the next line is read. Processing should be done by passing a
     * processing function pointer, which will be called after each line is
     * successfully parsed.
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

	static bool is_equals(const char c) {
	    return c == '='; }

	static bool is_slash(const char c) {
	    return c == '/'; }


	/// Currently processed line
	std::string				current_line;

	/// Current position in current_line
	std::string::iterator			pos;


	/**
	 * Reads text while `check` returns true.
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
	 * true for the next character.
	 */
	template<CharCheckFn check>
	bool expect_next() {
	    read_while<is_whitespace>();
	    return pos < current_line.end() && check(*pos);
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
    };
}

#endif
