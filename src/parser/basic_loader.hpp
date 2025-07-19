#ifndef RW_BASIC_LOADER_HPP
#define RW_BASIC_LOADER_HPP

#include "basic_parser.hpp"
#include <fstream>
#include <vector>

namespace rewrite {

    class BasicLoader: public BasicParser {
    protected:
	std::ifstream		file;
	std::vector<double>	values;

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


	std::expected<void, Message> read_values() {
	    std::expected<double, Message>	val(0.0);

	    values.clear();
	    while (val.has_value() && is_or_next<is_number>()) {
		val = get_number();
		values.push_back(val.value_or(0.0));
	    }

	    if (!val)
		return std::unexpected { val.error() };
	    return {};
	}
    };

}

#endif
