#include "polaris_parser.hpp"
#include "parser/command_parser.hpp"

namespace rewrite {

    auto PolarisParser::process_polaris_cmd(ParsedLine& line)
	-> std::expected<void, Message> {

	using namespace literals;

	// Do not read lines of skipped block until it is closed
	if (flag_isset(flags, PolarisParserFlags::Skipping)
	    && line.type != ParsedLine::Type::ClosingTag)
	    return {};


	switch (line.type) {
	    
	    // Includes empty and comment lines
	    case ParsedLine::Type::ValueLine:
		if (!line.empty())
		    return std::unexpected{ Message{
			"Missing <cmd>" } };
		return {};


	    case ParsedLine::Type::ClosingTag:
		if (block_to_str(current_block) != line.command)
		    return std::unexpected{ Message{ comp_error(
			"Wrong closing tag, expected </", block_to_str(current_block), '>') } };

		current_block = BlockType::None;
		flag_unset(flags, PolarisParserFlags::Skipping);
		param = nullptr;
		return {};


	    case ParsedLine::Type::Command:

		// Handle block commands

		if (current_block == BlockType::None) {

		    // Do we have a parameter to define skipping behaviour?
		    if (const auto e = line.get_num(0);
			e.has_value() && e.value() == 0) {
			flags |= PolarisParserFlags::Skipping;
		    }

		    if (line.command == "common"sv) {
			if (!flag_isset(flags, PolarisParserFlags::Skipping)
			    && flag_isset(flags, PolarisParserFlags::CommonProcessed))
			    return std::unexpected { Message{
				"<common> Blocks MUST now precede <task> Blocks" } };
			
			param = &common_params;
			flags |= PolarisParserFlags::CommonProcessed;
			current_block = BlockType::Common;
			break;
		    }

		    if (line.command == "task"sv) {
			param_list.emplace_back();
			param = &param_list.back();
			*param = common_params;
			param->setTaskID(param_list.size());
			current_block = BlockType::Task;
			break;
		    }

		    return std::unexpected{ Message{
			"Expected <common> or <task> Block" } };
		}

		// Handle line commands inside blocks
		try {
		    if (const auto e = commands.at(line.command)(line, *param);
			!e) return e;
		}
		catch(const std::out_of_range&) {
		    return std::unexpected { Message { comp_error(
			"Unknown Command '", line.command, '\'') } };
		}

		break;
	}

	return {};
    }
}
