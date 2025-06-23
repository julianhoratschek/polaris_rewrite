#include <string_view>
#include <string>
#include <cctype>
#include <expected>
#include <vector>

#include "polaris_commands.hpp"

namespace rewrite {

    using CharCheckFn = bool(*)(const char);

    bool is_whitespace(const char c) { return std::isspace(c); }
    bool is_number(const char c) { return std::isdigit(c) || c == '+' || c == '-' || c == ',' || c == '.' || c == 'e' || c == 'E'; }
    bool is_identifier(const char c) { return std::isalpha(c) || c == '_'; }
    bool is_quote(const char c) { return c == '"'; }

    struct ParsedParameter {
	enum class Type {
	    Identifier,
	    String,
	    Number
	}					type;
	std::string_view			value;

	double as_number() {

	}

    }

    struct ParsedLine {
	CommandFileOption			command;
	unsigned char				names_parameter_count;
	std::vector<std::string_view>		parameters;


    };

    class CommandParser {
	private:
	    struct Token {
		enum class Type {
		    Number, Identifier, String, Quote = '"',
		    OpenTag = '<', CloseTag = '>', Slash = '/'
		};

		std::string_view	value;
	    };

	    std::string_view			current_line;
	    std::string_view::const_iterator	pos;
	    unsigned int			line_nr{0};
	
	    template<CharCheckFn check>
	    std::string_view read_while() {
		const auto	start = pos;

		while (++pos != current_line.end() && check(*pos));

		return current_line.substr(
		    std::distance(current_line.begin(), start),
		    std::distance(start, pos));
	    }

	public:
	    auto parse_line(const std::string_view& line)
		-> std::expected<ParsedLine, std::string> {

		bool		is_comment = false;

		current_line = line;
		line_nr++;
		pos = current_line.cbegin();

		while (!is_comment && pos != current_line.cend()) {
		    const char c = *pos;

		    switch (c) {
			case '#':
			case '!':
			    is_comment = true;
			    break;

			case '"':
			    break;

			case '<':
			    break;

			default:
			    if (is_identifier(c))
				read_while<is_identifier>();
			    else if (is_number(c))
				read_while<is_number>();
			    else
				return std::unexpected{ "Unknown Command" };
		    }
		}
	    }

    };
}
