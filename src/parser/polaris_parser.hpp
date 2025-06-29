#ifndef RW_POLARIS_PARSER
#define RW_POLARIS_PARSER

#include "../Parameters.hpp"
#include "command_parser.hpp"
#include "polaris_commands.hpp"

#include <filesystem>
#include <vector>
#include <expected>
#include <string_view>
#include <utility>


namespace rewrite {


    /**
     * Flags controlling parser processing
     */
    enum class PolarisParserFlags {
	None = 0,
	CommonProcessed = 0x1,
	Skipping = 0x2
    };

    template<>
    consteval bool enable_enum_flag<PolarisParserFlags>() { return true; }

    /**
     * Per-line parser for POLARIS *.cmd-files
     */
    class PolarisParser {
    private:

	/**
	 * Describes the currently processed command block
	 * <common> Commands will be set for all parameters before
	 * <task> commands overwrite the <common> setting for each parameters
	 */
	enum class BlockType: unsigned char {
	    None, Common, Task
	};


	/**
	 * Helper function to convert `BlockType` to string. Used
	 * for debugging and output.
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
	 * Main method looking for <common> and <task> blocks, allocating
	 * new parameters per block and calling methods from `commands`
	 * according to found commands.
	 * @param line Currently parsed line
	 */
	auto process_polaris_cmd(ParsedLine& line)
	    -> std::expected<void, Message>;

    public:


	PolarisParser()
	    : commands{make_cmd_map()} {}

	/**
	 * Exposes parsing functionality.
	 * @param path Path to the *.cmd file to parse
	 * @param err_fn optional function to handle non-fatal error messages
	 * @returns on failure Message with fatal error message
	 */
	auto parse_polaris_cmd(
	    std::filesystem::path path,
	    HandleErrorFn err_fn = nullptr)
	    -> std::expected<void, Message>; 

	/**
	 * Returns complete parameters-list. Should be called after
	 * `parse_polaris_cmd`
	 */
	auto& get_param_list() { return param_list; }
    };

}

#endif
