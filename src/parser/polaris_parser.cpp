#include "polaris_parser.hpp"

namespace rewrite {

    auto PolarisParser::get_command()
	-> std::expected<void, Message> {

	// Look for closing tag, if present
	if (expect_next<is_slash>()) {
	    parsed_line.type = ParsedLine::Type::ClosingTag;
	    expect_next<is_identifier>();
	}
	else
	    parsed_line.type = ParsedLine::Type::Command;

	if (!is_identifier(*pos))
	    return std::unexpected { Message { "Expected Polaris command after '<[/]'" } };

	// --pos is needed between read_while and expect_next, to look at
	// the current character
	parsed_line.command = read_while<is_identifier>();
	--pos;

	// Read named parameters
	while (expect_next<is_identifier>()) {
	    const auto param_name = read_while<is_identifier>();
	    --pos;

	    if (!expect_next<is_equals>())
		return std::unexpected{ Message { "Expected '=' after named parameter", } };

	    if (!expect_next<is_quote>())
		return std::unexpected { Message { "Expected String after named parameter", } };

	    std::vector<double>	named_params;
	    while (expect_next<is_number>()) {
		const auto num = get_number();
		if (!num.has_value())
		    return std::unexpected{ num.error() };
		named_params.push_back(num.value());
		--pos;
	    }

	    parsed_line.named_params[param_name] = std::move(named_params);
	}

	if (pos >= current_line.end() || *pos != '>')
	    return std::unexpected{ Message { "Expected '>' after command", } };

	return {};
    }


    auto PolarisParser::process_polaris_cmd()
	-> std::expected<void, Message> {

	using namespace literals;

	// Do not read lines of skipped block until closing tag is encountered
	if (flag_isset(flags, PolarisParserFlags::Skipping)
	    && parsed_line.type != ParsedLine::Type::ClosingTag)
	    return {};


	switch (parsed_line.type) {
	    
	    // Includes empty and comment lines
	    case ParsedLine::Type::ValueLine:
		if (!parsed_line.empty())
		    return std::unexpected{ Message{
			"Missing <cmd>" } };
		return {};


	    case ParsedLine::Type::ClosingTag:
		if (block_to_str(current_block) != parsed_line.command)
		    return std::unexpected{ Message{
			std::format("Wrong closing tag, expected </{}>",
			    block_to_str(current_block)) } };

		current_block = BlockType::None;
		flag_unset(flags, PolarisParserFlags::Skipping);
		param = nullptr;
		return {};


	    case ParsedLine::Type::Command:

		// Handle block commands

		// If not inside a block, look for common or task block
		if (current_block == BlockType::None) {

		    // Do we have a parameter to define skipping behaviour?
		    if (const auto e = parsed_line.get_num(0);
			e.has_value() && e.value() == 0) {
			flags |= PolarisParserFlags::Skipping;
		    }

		    if (parsed_line.command == "common"sv) {
			if (!flag_isset(flags, PolarisParserFlags::Skipping)
			    && flag_isset(flags, PolarisParserFlags::CommonProcessed))
			    return std::unexpected { Message{
				"<common> Blocks MUST now precede <task> Blocks" } };
			
			param = &common_params;
			flags |= PolarisParserFlags::CommonProcessed;
			current_block = BlockType::Common;
			break;
		    }

		    if (parsed_line.command == "task"sv) {
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
		    if (const auto e = commands.at(parsed_line.command)(parsed_line, *param);
			!e.has_value()) return e;
		}
		catch(const std::out_of_range&) {
		    return std::unexpected { Message {
			std::format("Unknown Command '{}'", parsed_line.command) } };
		}

		break;
	}

	return {};
    }


    auto PolarisParser::set_line(const std::string& line)
	-> std::string::iterator {
	parsed_line.clear();

	current_line = line;
	pos = current_line.begin();

	++parsed_line.line_nr;
	parsed_line.line = current_line;

	return pos;
    }


    auto PolarisParser::parse_line()
	-> std::expected<void, Message> {

	while (pos < current_line.end()) {
	    const char c = *pos;

	    switch (c) {
		// Get Comments
		case '#':
		case '!':
		    return {};

		// Get Strings
		case '"':
		    if (const auto str = get_string();
			!str.has_value()) return std::unexpected{ str.error() };
		    else parsed_line.push_param<ParsedLine::ParamType::String>(str.value());
		    break;

		// Get commands (tags)
		case '<':
		    if (const auto e = get_command();
			not e) return std::unexpected{ e.error() };
		    break;

		// Get whitespace, numbers or identifiers
		default:
		    if (is_whitespace(c))
			break;

		    else if (is_identifier_start(c))
			parsed_line.push_param<ParsedLine::ParamType::Identifier>(
			    read_while<is_identifier>());

		    else if (is_number(c)) {
			const auto num = get_number();
			if (!num.has_value())
			    return std::unexpected{ num.error() };

			parsed_line.push_param<ParsedLine::ParamType::Number>(num.value());
		    }

		    else
			return std::unexpected{ Message { std::format("Unknown Token '{}'", c), } };
	    }

	    // Skip all whitespace until next character is found
	    read_while<is_whitespace>();
	}

	return {};
    }
}
