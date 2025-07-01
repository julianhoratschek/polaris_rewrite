#include "basic_parser.hpp"
#include <string_view>

namespace rewrite {

    auto BasicParser::get_number()
	    -> std::expected<double, Message> {

	const auto	tmp_string = read_while<is_number>();
	double val = 0;

	try {
	    std::from_chars(tmp_string.begin(), tmp_string.end(), val);
	}
	catch (const std::out_of_range&) {
	    return std::unexpected{ Message { "Number Parameter is too large", } };
	}
	catch (const std::invalid_argument&) {
	    return std::unexpected{ Message { "Ill-formed Number", } };
	}

	return val;
    }


    auto BasicParser::get_string()
	    -> std::expected<std::string_view, Message> {

	auto res = read_while<is_string>();
	if (pos >= current_line.end())
	    return std::unexpected{ Message { "Missing '\"'" } };
	return res.substr(1, res.size() - 1);
    }


    size_t BasicParser::error_distance() {
	return std::distance(current_line.begin(), pos) - 2;
    }


    std::string BasicParser::error_pointer() {
	return std::string(error_distance(), '~') + '^';
    }
}
