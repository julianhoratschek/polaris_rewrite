#ifndef RW_COMMAND_REGISTER
#define RW_COMMAND_REGISTER

#include "parsed_line.hpp"
#include "../../Parameters.hpp"

#include <unordered_map>
#include <string_view>
#include <expected>


namespace rewrite {

    /**
     * Call signature for POLARIS command processing methods
     *
     * ParsedLine is the currently parsed line with a fitting command-member
     * for the current function.
     *
     * parameters is the currently processed
     * parameters instance, which can be modified directly.
     *
     * POLARIS command processing functions should return void (return {}) on
     * success and an Instance of std::unexpected{ Message{}} on failure.
     *
     * If an Instance of Message with Message::type Error is returned,
     * processing of the current file will be aborted.
     */
    using CommandFunction = std::expected<void, Message>(*)(ParsedLine&, parameters&);


    /**
     * Singleton class holding all POLARIS commands and their defined
     * functions. Will be filled automatically when using the
     * POLARIS_COMMAND and DEFINE_COMMAND Macros.
     */
    class PolarisCommands {
	PolarisCommands() = default;

	/// List of commands and their respective callbacks
	std::unordered_map<std::string_view, CommandFunction>	commands;

    public:

	static PolarisCommands& self() {
	    static PolarisCommands me;
	    return me;
	}


	static void register_command(const std::string_view& name, CommandFunction& fn) {
	    self().commands[name] = fn;
	}


	static auto get_command(const std::string_view& name)
	    -> std::expected<CommandFunction, Message>
	{
	    if (self().commands.contains(name))
		return self().commands[name];

	    return std::unexpected { Message {
		std::format("Unknown POLARIS Command '{}'", name) 
	    } };
	}
    };


    /**
     * Helper-class to call PolarisCommands::register_command on
     * construction.
     */
    struct RegisterCommand {
	RegisterCommand(std::string_view name, CommandFunction fn) {
	    PolarisCommands::register_command(name, fn);
	}
    };

    // TODO as views instead of strings?

    /// Macro to register a function as POLARIS callback (header-side)
#define POLARIS_COMMAND(name) \
    std::expected<void, Message> cmd_##name(ParsedLine& line, parameters& param);\
    static RegisterCommand polaris_command_registered_##name(#name, cmd_##name);

    /// Macro to ease the definition of a POLARIS callback (cpp-side)
#define DEFINE_COMMAND(name) \
    std::expected<void, Message> cmd_##name(ParsedLine& line, parameters& param)
}


#endif
