#include "command_parser.hpp"
#include "../Parameters.hpp"

#include <array>

namespace rewrite {

    using CommandProcessFn = std::expected<bool, std::string> (*)(ParsedLine&, parameters&);
    constexpr auto make_map() {
	return std::map<std::string, CommandProcessFn> {
	    { "cmd", cmd_cmd }
	};
    }


}
