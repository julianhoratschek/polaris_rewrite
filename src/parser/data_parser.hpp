#ifndef RW_DATA_PARSER
#define RW_DATA_PARSER

#include "parsed_line.hpp"
#include "command_parser.hpp"
#include "parser/message.hpp"

#include <filesystem>
#include <fstream>
#include <functional>

namespace rewrite {
    // I would like to implement this as a call-stack of function-pointer,
    // each one is called after a valid line is processed.
    // Each function itself can push new functions onto the callstack.
    // Those must be inserted between the current function (HEAD) and
    // the immediate neighbour

    // How is this better than switch/case?
    // - More flexibility: Implementing other file formats should be easy
    // - Better readability: THe code blocks currently span around 200-300
    //   lines. This is easier to digest in a method than in a case block
    // - Scope: It is not necessary to have all helper variables in scope
    //   all the time. Maybe more stack-allocations are possible?
    //
    // What are drawbacks?
    // - More complicated and error prone code
    // - More code jumps over large distances (but: also with switch/case)
    // - Introduction of another object
    // - How does concurrency work with this?
    //  -> It doesn't, but it's also not needed (few parameters)
	

    template<typename T>
    class DataParser: public CommandParser {
	using FunctionType = std::expected<void, Message>(T::*)(ParsedLine&);

	std::vector<FunctionType>	method_stack;

	/**
	 * Calls next method, even if parser has found an error
	 * This enables work with the raw line, even if ParsedLine
	 * result might be undefined
	 */
	bool process_error(const Message& msg) {

	    // Ignore parser errors, as we can also use raw line input
	    if (msg.sender == Message::Sender::Parser) {
		if (const auto res = call_method(parsed_line);
		    not res) return res.error().type != Message::Type::Error;
		return true;
	    }

	    // Output error or warning
	    return default_error_handler(msg);
	}


	auto process_line(ParsedLine& line)
	    -> std::expected<void, Message> {
	    return call_method(line);
	}


    public:

	/**
	 *
	 */
	void push_method(FunctionType fn, size_t n = 1) {
	    if (n <= 1) {
		method_stack.push_back(fn);
		return;
	    }

	    method_stack.resize(method_stack.size() + n, fn);
	}

	/**
	 *
	 */
	DataParser<T>& operator<<(FunctionType fn) {
	    method_stack.push_back(fn);
	    return *this;
	}

	/**
	 *
	 */
	void pop_method() {
	    if (!method_stack.empty())
		method_stack.pop_back();
	}

	/**
	 *
	 */
	auto call_method(ParsedLine& line) -> std::expected<void, Message> {
	    if (method_stack.empty())
		return std::unexpected { Message {
		    "No more Methods defined for line processing" } };

	    const auto callback = method_stack.back();
	    method_stack.pop_back();

	    return std::invoke(callback, *this, line);
	}

	/**
	 *
	 */
	auto parse_file(const std::filesystem::path& path)
		-> std::expected<void, Message> {

	    const auto		proc_fn = std::bind(&DataParser::process_line,
		*this, std::placeholders::_1);

	    const auto		err_fn = std::bind(&DataParser::process_error,
		*this, std::placeholders::_1);

	    std::ifstream	file(path);

	    if (const auto res = parse_file(file, proc_fn, err_fn);
		not res) return res; 

	    return {};
	}
    };

}


#endif
