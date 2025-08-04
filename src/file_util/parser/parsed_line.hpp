#ifndef RW_PARSED_LINE
#define RW_PARSED_LINE

#include "../message.hpp"

#include <vector>
#include <string>
#include <string_view>
#include <map>
#include <expected>

namespace rewrite {

    /**
     * Chose multiple vector layout because of multiple conversions during cmd
     * file processing. Minor memory fragmentation is preferable to multiple
     * loops with conversions through the same arrays.
     */
    struct ParsedLine {

	enum class Type: unsigned char {
	    /// Only values were read (default)
	    ValueLine,

	    /// A command was found (<cmd> param1 param2 etc.)
	    Command,

	    /// A closing tag was found (</task>)
	    ClosingTag
	};

	enum class ParamType: unsigned char {
	    /// Query Identifier parameters
	    Identifier,

	    /// Query String parameters
	    String,

	    /// Query number parameters
	    Number
	};

	/// Command (if found)
	std::string_view				command;

	/// Original line
	std::string_view				line;

	/// Type of this line
	Type						type{Type::ValueLine};

	/// Line number
	size_t						line_nr{0};

	/// All found number-parameters
	std::vector<double>				num_params;

	/// All found string-parameters
	std::vector<std::string_view>			str_params;

	/// All found ID-Parameters
	std::vector<std::string_view>			id_params;

	// Using a map here is only syntactic sugar. It's easier to
	// use when writing new polaris cmds.
	/// All found named parameters <cmd named_param1 = "value">
	std::map<std::string_view, std::vector<double>>	named_params;

	/// Saves sequence of found parameters with associated type
	/// Holds the correct index for the corresponding vector
	std::vector<std::pair<ParamType, size_t>>	sequence;


	/**
	 * Convert ParamType to string for debugging and messages
	 */
	template<ParamType pt>
	static constexpr std::string_view param_type() {
	    using namespace std::literals;

	    if constexpr (pt == ParamType::Identifier)
		return "Identifier"sv;

	    if constexpr (pt == ParamType::Number)
		return "Number"sv;

	    return "String"sv;
	}

	/**
	 * Get vector for one parameter type
	 */
	template<ParsedLine::ParamType pt>
	constexpr auto get_vector() {
	    if constexpr (pt == ParamType::Number)
			return &num_params;

	    else if constexpr (pt == ParamType::Identifier)
			return &id_params;
		else
			return &str_params;
	}

	/**
	 * const variant of get_vector()
	 */
	template<ParsedLine::ParamType pt>
	constexpr auto get_vector() const {
	    if constexpr (pt == ParamType::Number)
		return &num_params;
	    else if constexpr (pt == ParamType::Identifier)
		return &id_params;
	    else
		return &str_params;
	}

	/**
	 * Add `val` to the correct vector designated by `pt`
	 * Updates `sequence` with the index of the current parameter
	 */
	template<ParsedLine::ParamType pt, typename T>
	void push_param(T val) {
	    auto	pv = get_vector<pt>();

	    sequence.emplace_back(pt, pv->size());
	    pv->push_back(val);
	}

	/**
	 * Get the parameter at index `idx`, returns an error if `pt` does not
	 * designate the correct type of the parameter at position `idx`
	 */
	template<ParsedLine::ParamType pt, typename T>
	auto get_param(const size_t idx) const
	    -> std::expected<T, Message>
	{
	    if (idx >= sequence.size())
		return std::unexpected{ Message { "Too few parameters" } };

	    const auto param = sequence[idx];
	    if (param.first != pt)
		return std::unexpected{ Message {
		    std::format("Expected {} at position {}", param_type<pt>(), idx) } };

	    auto pv = get_vector<pt>();
	    return pv->at(param.second);
	}

	/**
	 * Get value at parameter position `idx` as a number.
	 * Returns unexpected with error-string if type differs.
	 * @param idx Index in (global) parameter position
	 * @returns Parameter at position `idx` as a number or unexpected
	 */
	auto get_num(const size_t idx) const {
	    return get_param<ParamType::Number, double>(idx);
	}

	/**
	 * Get value at parameter position `idx` as an ID.
	 * Returns unexpected with error-string if type differs.
	 * @param idx Index in (global) parameter position
	 * @returns Parameter at position `idx` as an IDo r unexpected
	 */
	auto get_id(const size_t idx) const {
	    return get_param<ParamType::Identifier, std::string_view>(idx);
	}

	/**
	 * Get value at parameter position `idx` as a string.
	 * Returns unexpected with error-string if type differs.
	 * @param idx Index in (global) parameter position
	 * @returns Parameter at position `idx` as a string or unexpected
	 */
	auto get_str(const size_t idx) const {
	    return get_param<ParamType::String, std::string_view>(idx);
	}


	/**
	 * Get named parameters. Only 1 or 2 values can be derived from one
	 * named parameter (easily extendible here). Returns an std::array
	 * with the requested values, or std::unexpected, if name or values
	 * don't exist. Should be used with structured binding.
	 */
	template<size_t N = 1>
	    requires (N > 0) && (N < 3)
	auto get_named(const std::string_view name)
	    -> std::expected<std::array<double, N>, Message>
	{
	    if (!named_params.contains(name))
		return std::unexpected { Message {
		    std::format("Expected named parameter {}", name) } };

	    const auto& values = named_params[name];
	    if (values.size() != N)
		return std::unexpected{ Message {
		    std::format("Expected {} values for named parameter {}", N, name) } };

	    if constexpr (N == 1)
		return std::array{ values[0] };

	    if constexpr (N == 2)
		return std::array{ values[0], values[1] };
	}

	/**
	 * Completely empty line
	 */
	void clear();

	/*
	 * Returns true if nothing was written in this line (also true for pure
	 * comment lines)
	 */
	bool empty() const;
    };


    /**
     * Output stream operator for ParsedLine, mainly for debugging
     */
    std::ostream& operator<<(std::ostream& os, const ParsedLine& line);
}

#endif
