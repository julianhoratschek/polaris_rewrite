#ifndef RW_MESSAGE
#define RW_MESSAGE

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

	/// Default is Processor, as most messages will be sent from
	/// parsing POLARIS CMD files
	Sender			sender;


	explicit Message(const std::string& msg, const Type tp = Type::Error, const Sender snd = Sender::Processor)
	    : message{msg}, type{tp}, sender{ snd } {}

	Message(const std::string& msg, const Sender snd)
	    : message{msg}, sender{ snd } {}
    };


    /**
     * Simple output operator for `Message`
     * TODO: use format
     */
    inline std::ostream& operator<<(std::ostream& os, const Message& msg) {
	return os << msg.message;
    }


    /**
     * Return Escape coded coloured text depending on `color` value.
     */
    template<typename T>
    std::string msg_color(const Message::Type color, const T& msg) {
	std::string	col;

	switch (color) {
	    case Message::Type::Warning:
		col = "\033[1m\033[33m";
		break;

	    case Message::Type::Error:
		col = "\033[1m\033[31m";
		break;

	    case Message::Type::Info:
		col = "\033[1m\033[32m";
		break;
	}

	return std::format("{}{}{}", col, msg, "\033[0m");
    }


    bool default_error_handler(const Message& msg) {
	constexpr auto	senders = std::array{ "", "Parser", "Processor" };
	const auto		sender = senders[std::to_underlying(msg.sender)];

	constexpr auto	labels = std::array{
	    "ERROR ", "WARNING ", "INFO " };
	const auto 		label = labels[std::to_underlying(msg.type)];

	std::cout << rewrite::msg_color(msg.type, label) << sender << ' ' << msg.message << std::endl;

	// Abort processing if message type was an error
	return msg.type != rewrite::Message::Type::Error;
    }
}

#endif
