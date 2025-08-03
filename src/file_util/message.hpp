#ifndef RW_MESSAGE
#define RW_MESSAGE

#include <cstddef>
#include <expected>
#include <string>
#include <array>
#include <utility>

#include <iostream>

namespace rewrite {
    /**
     * Return-value for parsers containing a message and a thread-level
     */
    struct Message {
	enum class Sender: unsigned char {
	    None, Parser, Processor
	};

	enum class Type: unsigned char {
	    Error, Warning, Info
	};

	std::string		message;
	Type			type;
	Sender			sender;
	size_t			line, col;


	explicit Message(const std::string& msg, const Type tp = Type::Error, const Sender snd = Sender::Processor, size_t in_line = 0, size_t in_col = 0)
	    : message{msg}, type{tp}, sender{snd}, line{in_line}, col{in_col} {}

	Message(const std::string& msg, const Sender snd)
	    : message{msg}, sender{snd} {}
    };


    /// Return value for most POLARIS cmd methods
    using t_ret = std::expected<void, Message>;

    inline bool default_error_handler(const Message& msg) {
	constexpr auto	senders = std::array{ "", "Parser", "Processor" };
	const auto sender = senders[std::to_underlying(msg.sender)];

	constexpr auto	labels = std::array{
	    "ERROR ", "WARNING ", "INFO " };
	const auto label = labels[std::to_underlying(msg.type)];

	constexpr auto colors = std::array{
	    "\033[1;31m", "\033[1;33m", "\033[1;32m"
	};
	const auto color = colors[std::to_underlying(msg.type)];

	std::cout << color << label << "\033[0m" << sender << ' ' << msg.message << std::endl;

	// Abort processing if message type was an error
	return msg.type != rewrite::Message::Type::Error;
    }
}

#endif
