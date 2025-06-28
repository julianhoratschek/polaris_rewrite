#ifndef RW_POLARIS_PARSER
#define RW_POLARIS_PARSER

#include "../Parameters.hpp"
#include "command_parser.hpp"
#include "polaris_commands.hpp"

#include <filesystem>
#include <vector>
#include <expected>
#include <string>
#include <functional>
#include <string_view>
#include <utility>

namespace rewrite {

    /**
     *
     */
    class PolarisParser {

	enum class BlockType: unsigned char {
	    None, Common, Task
	};

	/**
	 *
	 */
	static std::string_view block_to_str(BlockType tp) {
	    const auto arr = std::array{ "none"sv, "common"sv, "task"sv };
	    const size_t idx = std::to_underlying(tp);
	    return arr[idx];
	}

	/// List of parameters
	std::vector<parameters>		param_list;

	/// Common parameter values
	parameters			common_params;

	/// Currently processed parameter
	parameters			*param{ nullptr };

	/// Currently processed block
	BlockType			current_block{ BlockType::None };

	/// Maps "command name" -> command_function
	command_map			commands;


	/**
	 *
	 */
	auto process_polaris_cmd(ParsedLine& line)
	    -> std::expected<void, std::string> {

	    using namespace literals;

	    switch (line.type) {
		case ParsedLine::Type::ValueLine:
		    if (!line.empty())
			return std::unexpected{ "Missing <cmd>" };
		    return {};

		case ParsedLine::Type::ClosingTag:
		    if (block_to_str(current_block) != line.command)
			return std::unexpected{
			    comp_error(
				"Wrong closing tag, expected </",
				block_to_str(current_block), '>') };
		    current_block = BlockType::None;
		    param = nullptr;
		    return {};

		case ParsedLine::Type::Command:
		    if (current_block == BlockType::None) {
			if (line.command == "common"sv) {
			    param = &common_params;
			    break;
			}

			if (line.command == "task"sv) {
			    param_list.emplace_back();
			    param = &param_list.back();
			    break;
			}

			return std::unexpected{ "Expected <common> or <task> Block" };
		    }

		    try {
			if (const auto e = commands.at(line.command)(line, *param);
			    !e) return e;
		    }
		    catch(const std::out_of_range&) {
			return std::unexpected { comp_error(
			    "Unknown Command '", line.command, '\'') };
		    }

		    break;
	    }

	    return {};
	}

    public:

	/**
	 *
	 */
	auto parse_polaris_cmd(std::filesystem::path &path)
	    -> std::expected<void, std::string> {
	    
	    CommandParser	parser;
	    auto		fn = std::bind(
		&PolarisParser::process_polaris_cmd,
		this, std::placeholders::_1);

	    if (const auto e = parser.parse_file(path, fn))
		return e;

	    // TODO: would be easier to initialize param.start/stop to 0
	    for (auto& p: param_list) {
		const auto sz = p.getDetectorSize();
		auto	start = p.getStart();
		auto	stop = p.getStop();

		if (start >= sz)
		    start = 0;

		if (stop >= sz)
		    stop = sz > 0 ? sz - 1 : 0;

		p.setStart(start);
		p.setStop(stop);
	    }

	    return {};
	}
    };
}

#endif
