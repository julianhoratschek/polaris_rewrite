#ifndef RW_BASIC_LOADER_HPP
#define RW_BASIC_LOADER_HPP

#include "../basic_parser.hpp"
#include <fstream>
#include <vector>

namespace rewrite {

    template<typename T>
    concept FileType = requires(T t) {
	{ t.cleanup() };
    };


    /**
     * Basic loader providing general utility methods
     * Child classes should implement "parse_file" to take
     * A pathlike input and parse as well as load FileType.
     */
    template<FileType LoadFile>
    class BasicLoader: public BasicParser {

    protected:
	std::ifstream		file;
	std::vector<double>	values;

	LoadFile		result;

	/**
	 * Close current file and calls result.cleanup.
	 * Returns an error Message with msg as content
	 * @param msg string to display in returned message
	 * @returns std::unexpected<Message> with current line-nr and msg
	 */
	std::unexpected<Message> safe_error(const std::string& msg)
	{
	    result.cleanup();
	    file.close();

	    return std::unexpected{ Message {
		msg,
		Message::Type::Error,
		Message::Sender::Processor,
		line_nr,
		error_distance()
	    } };
	}

	/**
	 * Reads all numerical values found from current pos
	 * and saves it in values.
	 */
	std::expected<void, Message> read_values()
	{
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


	/**
	 * Reads a single numerical value from the next line
	 */
	template<typename T>
	std::expected<T, Message> rdline_number(const std::string& param_name)
	{
	    if (!next_line(file) || !is_or_next<is_number>())
		return safe_error( 
		    std::format( 
			"Expected single numerical value for {}",
			param_name ) );

	    if (const auto num = get_number(); !num)
		return safe_error( 
		    std::format(
			    "Could not read {}: {}",
			    param_name, num.error().message ) );

	    else return static_cast<T>(num.value());
	}


	/**
	 * Reads next line as all numeric values, expects exactly length
	 * values in the next line, uses param names as marker in error
	 * messages.
	 * @param length number of values expected numbers to read
	 * @param param_names string declaring all values representing the
	 * 		      read numbers.
	 */
	std::expected<void, Message> rdline_values(
	    const size_t length,
	    const std::string& param_names)
	{
	    if (!next_line(file) || !is_or_next<is_number>())
		return safe_error(
		    std::format(
			"Expected {} numeric values: {}",
			length, param_names ) );

	    if (const auto res = read_values(); !res)
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


	auto rdlines_vector(const size_t size, const std::string& param_name, std::vector<double>& out) 
	    -> std::expected<void, Message>
	{
	    std::expected<double, Message> num;
	    size_t i;

	    out.resize(size);

	    for (i = 0; next_line(file) && (num = get_number()) && i < size; i++)
		out[i] = *num;

	    if (!num)
		return safe_error( 
		    std::format( "Could not read {}: {}", param_name, num.error().message) );

	    if (i != size)
		return safe_error(
		std::format( 
			"Expected {} lines with single values for {}, got {}", size, param_name, i) );
	    return {};
	}

    public:
	LoadFile& get_result() { return result; }
    };

}

#endif
