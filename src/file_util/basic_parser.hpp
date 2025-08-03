#ifndef RW_COMMAND_PARSER
#define RW_COMMAND_PARSER

#include "message.hpp"

#include <cstddef>
#include <string_view>
#include <string>
#include <expected>
#include <fstream>


namespace rewrite {

    /**
     * Default error handling function. Should take a Message-class as
     * parameter and output boolean.
     * An ErrorHandlerFn will be called, when any Message is propagated
     * by a parser or processor.
     * When an ErrorHandlerFn returns true, execution will resume. When
     * an ErrorHandlerFn returns false, processing of the file will end.
     */
    template<typename Fn>
    concept ErrorHandlerFn = requires (Fn fn, const Message msg) {
	{ fn(msg) } -> std::same_as<bool>;
    };

    // TODO unused, do we need a tokenizer?
	//    struct Token {
	// enum class Type: unsigned char {
	//     Number, Identifier, String, Comment, 
	//     Equals = '=', Slash = '/', CommandBegin = '<', CommandEnd = '>'
	// };
	//
	// unsigned long long value;
	//
	// std::string_view get_text(const std::string_view& text) const {
	//     return text.substr(
	// 	value >> (sizeof(unsigned long long) / 2),
	// 	value & 0x11111111);
	// }
	//
	// double get_number() const {
	//     return static_cast<double>(value);
	// }
	//    };
    
    /**
     * Strict base class for static inheritance. Provides
     * multiple useful methods for file parsing
     */
    class BasicParser {

	/// Used to determine the type of a character
	using CharCheckFn = bool(*)(const char);

    protected:

	/// Treat ;?* as whitespace for backwards-compatibility
	static bool is_whitespace(const char c) {
	    return std::isspace(c) || c == ';' || c == '?' || c == '*'; }

	/// Tries to find anything that is a number
	//TODO split into number_begin and number?
	static bool is_number(const char c) {
	    return std::isdigit(c) || c == '+' || c == '-' || c == ',' || c == '.' || c == 'e' || c == 'E'; }

	/// Identifiers may not start with numbers
	static bool is_identifier_start(const char c) {
	    return std::isalpha(c) || c == '_'; }

	/// Identifiers may contain numbers in their name
	static bool is_identifier(const char c) {
	    return std::isalpha(c) || c == '_' || std::isdigit(c); }

	/// Finds quotes as string delimiter
	static bool is_quote(const char c) {
	    return c == '"'; }

	/// Inverse method of is_quote
	static bool is_string(const char c) {
	    return c != '"'; }

	/// Multiple comment types are supported
	static bool is_comment(const char c) {
	    return c == '#' || c == '!'; }

	/// Mostly used for polaris parser
	static bool is_equals(const char c) {
	    return c == '='; }

	/// Mostly used for polaris parser
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
	 * Skip over whitespace, ignores pos at the beginning
	 */
	inline void eat_whitespace() {
	    while (++pos < current_line.end() && is_whitespace(*pos));
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
	    eat_whitespace();
	    return pos < current_line.end() && check(*pos);
	}


	/**
	 * Checks if the current pos is true for check, if it is
	 * whitespace, reads whitespace until the next non-whitespace
	 * pos is found and the result for check(*pos) is returned.
	 */
	template<CharCheckFn check>
	bool is_or_next() {
	    return pos < current_line.end() 
		&& (check(*pos)
		    || (is_whitespace(*pos) && expect_next<check>()));
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


	/**
	 * Reads line from in_file, skips commented lines.
	 * Counts line_nr up, sets current_line and sets pos to the beginning
	 * of current_line.
	 * @param in_file opened file to read from
	 * @returns true if a line could be read, false otherwise
	 */
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
	 * Reads a number in format double from pos
	 */
	auto get_number()
	    -> std::expected<double, Message>;

	/**
	 * Reads a string from pos, removes quotes
	 */
	auto get_string()
	    -> std::expected<std::string_view, Message>;

	/**
	 * Calculates position of pos in current line
	 */
	size_t error_distance();

	/**
	 * Returns a string "~~~^" pointing at pos
	 */
	std::string error_pointer();

	/**
	 * Standard formatted error message
	 */
	Message error_message(const Message& msg);
    };
}

#endif
