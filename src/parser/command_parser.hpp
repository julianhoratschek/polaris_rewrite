#ifndef RW_COMMAND_PARSER
#define RW_COMMAND_PARSER

#include <cstddef>
#include <string_view>
#include <string>
#include <expected>
#include <vector>
#include <sstream>

#include <filesystem>
#include <fstream>
#include <map>

// TODO: we don't want this
#include <iostream>

namespace rewrite {

    /**
     *
     */
    template<typename... Args>
    std::string comp_error(const std::string& msg, Args... args) {
	return (std::ostringstream(msg) << ... << args).str();
    }


    /**
     *
     */
    using CharCheckFn = bool(*)(const char);

    bool is_whitespace(const char c);
    bool is_number(const char c);
    bool is_identifier(const char c);
    bool is_quote(const char c);
    bool is_string(const char c);
    bool is_equals(const char c);
    bool is_slash(const char c);


    /**
     * Chose multiple vector layout because of multiple conversions during cmd
     * file processing. Memory fragmentation is preferable to multiple
     * loops through the same arrays.
     */
    struct ParsedLine {
	enum class Type {
	    ClosingTag, Command
	};

	enum class ParamType {
	    Identifier, String, Number
	};

	std::string_view				command;
	Type						type{Type::Command};
	size_t						line_nr{0};
	std::vector<double>				num_params;
	std::vector<std::string_view>			str_params;
	std::vector<std::string_view>			id_params;
	std::map<std::string, std::string_view> 	named_params;

	std::vector<std::pair<ParamType, size_t>>	sequence;

	// TODO test with consteval
	template<ParamType tp>
	static constexpr std::string param_type() {
	    if constexpr (tp == ParamType::Identifier)
		return "Identifier";
	    else if constexpr (tp == ParamType::Number)
		return "String";
	    else
		return "Number";
	}

	// TODO test with consteval
	template<ParamType pt>
	constexpr auto get_vector() {
	    if constexpr (pt == ParamType::Number)
		return &num_params;
	    else if constexpr (pt == ParamType::Identifier)
		return &id_params;
	    else
		return &str_params;
	}

	template<ParamType pt>
	constexpr auto get_vector() const {
	    if constexpr (pt == ParamType::Number)
		return &num_params;
	    else if constexpr (pt == ParamType::Identifier)
		return &id_params;
	    else
		return &str_params;
	}

	template<ParamType pt, typename T>
	void push_param(T val) {
	    auto	pv = get_vector<pt>();
	    const auto idx = pv->size();

	    sequence.emplace_back(pt, idx);
	    pv->push_back(val);
	}

	template<ParamType pt, typename T>
	auto get_param(const size_t idx) const
	    -> std::expected<T, std::string> {

	    if (idx >= sequence.size())
		return std::unexpected{ "Too few parameters" };

	    const auto param = sequence[idx];
	    if (param.first != pt)
		return std::unexpected{ comp_error("Expected ", param_type<pt>(), " at position ", idx) };

	    auto pv = get_vector<pt>();
	    return pv->at(param.second);
	}

	auto get_num(const size_t idx) const {
	    return get_param<ParamType::Number, double>(idx);
	}

	auto get_id(const size_t idx) const {
	    return get_param<ParamType::Identifier, std::string_view>(idx);
	}

	auto get_str(const size_t idx) const {
	    return get_param<ParamType::String, std::string_view>(idx);
	}

	void clear() {
	    type = Type::Command;
	    sequence.clear();
	    num_params.clear();
	    str_params.clear();
	    id_params.clear();
	    named_params.clear();
	}
    };

    std::ostream& operator<<(std::ostream& os, const ParsedLine& line) {
	os << "Parsed Line (" << line.line_nr << ")\n\nCommand: " << line.command << "\nParameter Sequence:\n";
	for (auto& v: line.sequence)
	    os << v.second << " ";
	os << "\nNamed Parameters:\n";
	for (auto& v: line.named_params)
	    os << "\t" << v.first << ": " << v.second << "\n";
	os << "\nNumber Parameters:\n";
	for (auto& v: line.num_params)
	    os << v << "; ";
	os << "\nID Parameters:\n";
	for (auto& v: line.id_params)
	    os << v << "; ";
	os << "\nString Parameters:\n";
	for (auto& v: line.str_params)
	    os << "\t" << v << "\n";

	return os;
    }


    /**
     *
     */
    class CommandParser {
	private:
	    std::string				current_line;
	    std::string::iterator		pos;

	    ParsedLine				parsed_line;

	    static std::string_view unquote(const std::string_view& str) {
		return str.substr(1, str.size() - 2);
	    }
	
	    /**
	     *
	     */
	    template<CharCheckFn check>
	    std::string_view read_while() {
		const auto	start = pos;

		while (++pos < current_line.end() && check(*pos))
		    if constexpr (check == is_number)
			if(*pos == ',')
			    *pos = '.';

		return {current_line.substr(
		    std::distance(current_line.begin(), start),
		    std::distance(start, pos))};
	    }

	    /**
	     * Skips whitespace and then returns true if check returns
	     * true for the next character.
	     */
	    template<CharCheckFn check>
	    bool expect_next() {
		read_while<is_whitespace>();
		return pos < current_line.cend() && check(*pos);
	    }

	    /**
	     *
	     */
	    auto get_command()
		-> std::expected<void, std::string>;

	public:
	    ParsedLine get_last_line() {
		return parsed_line;
	    }

	    auto parse_line(const std::string& line)
		-> std::expected<void, std::string>;

	    using ProcessFn = std::expected<void, std::string>(*)(const ParsedLine&);
	    auto parse_file(const std::filesystem::path& path, ProcessFn proc)
		-> std::expected<void, std::string> {

		std::ifstream		file(path);
		std::string		line;

		while(std::getline(file, line)) {
		    if (const auto res = parse_line(line); not res)
			return std::unexpected { comp_error(
			    "Parsing Error [", parsed_line.line_nr, ":", std::distance(current_line.begin(), pos), "]: ",
			    res.error()) };
		    if (const auto res = proc(parsed_line); !res)
			return std::unexpected { comp_error(
			    "Processing Error [", parsed_line.line_nr, ":", std::distance(current_line.begin(), pos), "]:",
			    res.error()) };
		}

		return {};
	    }
    };
}

#endif
