#ifndef RW_COMMAND_REGISTER
#define RW_COMMAND_REGISTER

#include "parsed_line.hpp"
#include "../../Parameters.hpp"

#include <unordered_map>
#include <string_view>
#include <expected>


namespace rewrite {

    using CommandFunction = t_ret(*)(ParsedLine&, parameters&);

    class PolarisCommands {
	PolarisCommands() = default;

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

    struct RegisterCommand {
	RegisterCommand(std::string_view name, CommandFunction fn) {
	    PolarisCommands::register_command(name, fn);
	}
    };

    // TODO as views instead of strings?

#define POLARIS_COMMAND(name) \
    t_ret cmd_##name(ParsedLine& line, parameters& param);\
    static RegisterCommand polaris_command_registered_##name(#name, cmd_##name);

#define DEFINE_COMMAND(name) \
    t_ret cmd_##name(ParsedLine& line, parameters& param)
}


#endif
