#ifndef RW_BASIC_LOADER_HPP
#define RW_BASIC_LOADER_HPP

#include "basic_parser.hpp"
#include <fstream>

namespace rewrite {

    class BasicLoader: public BasicParser {
    protected:
	std::ifstream	file;

	// TODO: use crtp instead?
	virtual void cleanup() = 0;

	std::unexpected<Message> safe_error(const std::string& msg)
	{
	    cleanup();
	    file.close();

	    return std::unexpected{ Message {
		msg,
		Message::Type::Error,
		Message::Sender::Processor,
		line_nr,
		error_distance()
	    }};
	}
    };

}

#endif
