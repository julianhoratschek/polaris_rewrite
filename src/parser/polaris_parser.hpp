#ifndef RW_POLARIS_PARSER
#define RW_POLARIS_PARSER

#include "../Parameters.hpp"
#include "command_parser.hpp"
#include "polaris_commands.hpp"

#include <filesystem>
#include <vector>
#include <expected>
#include <string>
#include <string_view>
#include <utility>


namespace rewrite {


    enum class PolarisParserFlags {
	None = 0,
	CommonProcessed = 0x1,
	Skipping = 0x2
    };

    template<>
    consteval bool enable_enum_flag<PolarisParserFlags>() { return true; }


    /**
     *
     */
    class PolarisParser {
    private:

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

	/// Control flow of parser
	PolarisParserFlags		flags{ PolarisParserFlags::None };


	/**
	 *
	 */
	auto process_polaris_cmd(ParsedLine& line)
	    -> std::expected<void, std::string>;

    public:


	PolarisParser()
	    : commands{make_cmd_map()} {}
	/**
	 *
	 */
	auto parse_polaris_cmd(std::filesystem::path path)
	    -> std::expected<void, std::string>; 
    };

}

#endif
