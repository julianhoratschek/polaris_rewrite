#ifndef RW_NK_PARSER
#define RW_NK_PARSER

#include "basic_parser.hpp"

#include <cmath>
#include <cstddef>
#include <filesystem>
#include <fstream>
#include <expected>
#include <string>
#include <vector>

namespace rewrite {

    struct NKFileResult {
	std::string		stringID;
	size_t			nr_dust_species;
	size_t			nr_wavelengths;
	size_t			nr_inc_angles;
	double			aspect_ratio;
	double			material_density;
	double			sub_temp;
	double			delta;
	bool			align;

	std::vector<double>	a_eff;
	std::vector<double>	wavelengths;
	size_t			line_length;
	size_t			data_length;
	std::vector<double>	data;
    };


    class NKDataParser: public BasicParser {
	std::ifstream		file;
	NKFileResult		result;

	bool next_line() {
	    while (std::getline(file, current_line)) {
		pos = current_line.begin();
		if (!is_comment_line() && pos < current_line.end())
		    return true;
	    }

	    return false;
	}

    public:
	template<ErrorHandlerFn ErrorFn>
	auto parse_file(const std::filesystem::path& path, ErrorFn error_handler)
		-> std::expected<void, Message> {

	    std::vector<double>	values;

	    file.open(path, std::ios::binary);

	    if (file.fail())
		return std::unexpected{ Message{
		    "Not a valid file", Message::Sender::Parser } };

	    if (!next_line())
		return std::unexpected{ Message {
		    "Unexpected End of File" }};

	    result.stringID = current_line;

	    if (!next_line())
		return std::unexpected{ Message {
		    "Unexpected End of File" }};

	    values.reserve(8);
	    while (expect_next<is_number>())
		if (const auto num = get_number();
		    not num.has_value()) return std::unexpected { num.error() };
		else values.push_back(num.value());

	    result.nr_dust_species = values[0];
	    result.nr_wavelengths = values[1];
	    result.nr_inc_angles = values[2];
	    result.aspect_ratio = values[3];

	    // TODO: check if set in cmd
	    result.material_density = values[4];
	    result.sub_temp = values[5];
	    result.delta = values[6];
	    result.align = values[7] != 0;

	    if (!next_line())
		return std::unexpected{ Message {
		    "Unexpected End of File" }};

	    result.a_eff.reserve(result.nr_dust_species);
	    while (expect_next<is_number>())
		if (const auto num = get_number();
		    not num.has_value()) return std::unexpected { num.error() };
		else result.a_eff.push_back(num.value());

	    if (result.a_eff.size() != result.nr_dust_species)
		return std::unexpected{ Message {
		    "Mismatching dust species counts"
		}};

	    if (!next_line())
		return std::unexpected{ Message {
		    "Unexpected End of File" }};

	    result.wavelengths.reserve(result.nr_wavelengths);
	    while (expect_next<is_number>())
		if (const auto num = get_number();
		    not num.has_value()) return std::unexpected { num.error() };
		else result.wavelengths.push_back(num.value());

	    if (result.wavelengths.size() != result.nr_wavelengths)
		return std::unexpected { Message {
		    "Mismatching wavelengths cound"
		}};

	    // Here: Testline for number of columns
	    if (!next_line())
		return std::unexpected{ Message {
		    "Unexpected End of File" }};

	    values.clear();
	    while (expect_next<is_number>())
		if (const auto num = get_number();
		    not num.has_value()) return std::unexpected { num.error() };
		else values.push_back(num.value());

	    // TODO: This should be profiled: Is it faster than 
	    // "On the go" allocation?
	    auto pos = file.tellg();
	    result.data_length = 1;
	    while (next_line())
		++result.data_length;
	    file.seekg(pos);

	    result.line_length = values.size();
	    result.data.resize(result.line_length * result.data_length);
	    std::ranges::copy(values, result.data.begin());

	    size_t	row = 1;

	    while (next_line()) {
		size_t	col = 0;

		while (expect_next<is_number>()) {
		    if (const auto num = get_number();
			not num.has_value()) return std::unexpected { num.error() };
		    else
			result.data[col * result.data_length + row] = num.value();
		}

		if (col != result.line_length)
		    return std::unexpected{ Message {
			"Mismatching column count" }};
	    }


	    return {};
	}

    };
}

#endif
