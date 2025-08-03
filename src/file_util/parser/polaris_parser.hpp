#ifndef RW_POLARIS_PARSER
#define RW_POLARIS_PARSER

#include "../../Parameters.hpp"
#include "../basic_parser.hpp"
#include "../util.hpp"

#include "parsed_line.hpp"

#include <filesystem>
#include <vector>
#include <expected>
#include <string_view>
#include <utility>
#include <fstream>

#include <array>
#include <utility>


namespace rewrite {

    // TODO: Maybe do a tokenizer? Parse first, interpret later?

    /**
     * Flags controlling parser processing
     */
    enum class CommandProcessingMode {
	None = 0,
	CommonProcessed = 0x1,
	Skip = 0x2
    };

    template<>
    consteval bool enable_enum_flag<CommandProcessingMode>() { return true; }

    /**
     * Per-line parser for POLARIS *.cmd-files
     */
    class PolarisParser: public BasicParser {
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
	// command_map			commands;

	/// Control flow of parser
	CommandProcessingMode		processing_mode{ CommandProcessingMode::None };

	/// Result of currently parsed line (invalidated when a new line is read)
	ParsedLine			parsed_line;

	/**
	 *
	 */
	auto get_command() -> std::expected<void, Message>;

	/**
	 * Main method looking for <common> and <task> blocks, allocating
	 * new parameters per block and calling methods from `commands`
	 * according to found commands.
	 * @param line Currently parsed line
	 */
	auto process_polaris_cmd()
	    -> std::expected<void, Message>;


	/**
	 * Parses one singular line. On Success the parsed line object can be
	 * retrieved with `get_last_line()`.
	 */
	auto parse_line()
	    -> std::expected<void, Message>;


    public:

	PolarisParser() = default;
	    // : commands{make_cmd_map()} {}


	/**
	 * Will return the current ParsedLine object. Should only be called
	 * after successful call of `parse_line()`. On Failure, the content
	 * of the ParsedLine-object returned by this method is undefined.
	 */
	ParsedLine get_last_line() { return parsed_line; }


	/**
	 * Returns complete parameters-list. Should be called after
	 * `parse_polaris_cmd`
	 */
	auto& get_param_list() { return param_list; }


	/**
	 * Exposes parsing functionality.
	 * @param path Path to the *.cmd file to parse
	 * @param err_fn optional function to handle non-fatal error messages
	 * @returns on failure Message with fatal error message
	 */
	template<ErrorHandlerFn ErrorFn>
	auto parse_file(const std::filesystem::path& path,
		ErrorFn err_fn = nullptr) -> std::expected<void, Message> {

	    std::ifstream	file(path);

	    if (file.fail())
		return std::unexpected{ Message{
		    "Not a valid file", Message::Sender::Parser } };

	    // while (std::getline(file, line)) {
	    while (next_line(file)) {
		parsed_line.clear();

		parsed_line.line_nr = line_nr;
		parsed_line.line = current_line;

		// Parse Line
		auto line_result = parse_line();

		// Handle errors from parsing
		// Here, Parser errors are marked as "sender: Parser"
		// TODO: Correct format parameter for lines
		if (!line_result) {
		    auto line_error = line_result.error();
		    line_error.sender = Message::Sender::Parser;

		    const auto parser_error = error_message(line_error);

		    if (!err_fn || !err_fn(parser_error))
			return std::unexpected{ parser_error };
		}

		// Process line on success
		line_result = process_polaris_cmd();

		// Handle errors from processing
		if (!line_result) {
		    const auto line_error = line_result.error();
		    const auto parser_error = error_message(line_error);
			//    const Message parser_error {
			// std::format("[{:04}]: {}\n",
			//     line_nr, err.message),
			// err.type,
			// Message::Sender::Processor,
			// line_nr
			//    };

		    if (!err_fn || !err_fn(parser_error))
			return std::unexpected{ parser_error };
		}
	    }

	    // Slightly different from original:
	    // Sets start/stop to 0 if not used, not to UINT_MAX
	    for (auto& p: param_list) {
		const auto sz = p.getDetectorSize();
		auto	start = p.getStart();
		auto	stop = p.getStop();

		if (start >= sz)
		    start = 0;

		if (stop >= sz)
		    stop = sz == 0 ? 0 : sz - 1;

		p.setStart(start);
		p.setStop(stop);
	    }

	    return {};
	}

    };

}

#endif
