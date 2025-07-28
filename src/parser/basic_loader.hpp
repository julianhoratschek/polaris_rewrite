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


	template<typename T>
	auto rdline_number(
	    const std::string& param_name)
	    -> std::expected<T, Message>
	{
	    if (!next_line(file) || !is_or_next<is_number>())
		return safe_error( 
		    std::format( "Expected single numerical value for {}", param_name ) );

	    if (const auto num = get_number(); !num.has_value())
		return safe_error( 
		    std::format( "Could not read {}: {}", param_name, num.error().message ) );
	    else return static_cast<T>(num.value());
	}


	auto rdline_values(const size_t length, const std::string& param_names)
	    -> std::expected<void, Message>
	{
	    if (!next_line(file) || !is_or_next<is_number>())
		return safe_error(
		    std::format(
			"Expected {} numeric values: {}",
			length, param_names ) );

	    if (const auto res = read_values(); !res.has_value())
		return safe_error(
		    std::format( "Could not read values {}: {}",
			param_names, res.error().message ) );

	    if (values.size() != length)
		return safe_error(
		    std::format(
			"Expected {} numeric values: {}",
			length, param_names ) );

	    return {};
	}


	auto rdlines_vector(const size_t size) 
	    -> std::expected<std::vector<double>, Message> {
	    std::expected<double, Message> num{0.0};
	    std::vector<double>	res(size);
	    size_t i = 0;

	    while (num && next_line(file)) {
		num = get_number();
		res[i] = num.value_or(0.0);
		++i;
	    }

	    if (!num)
		return std::unexpected{ num.error() };
	    if (i != size)
		return safe_error(
		    std::format( "Expected {} lines with single values", size) );
	    return res;
	}
    };

}

#endif
