#ifndef RW_PARSED_LINE
#define RW_PARSED_LINE

#include "util.hpp"

#include <vector>
#include <string>
#include <string_view>
#include <map>
#include <expected>

#include <sstream>

namespace rewrite {


    /**
     * Chose multiple vector layout because of multiple conversions during cmd
     * file processing. Memory fragmentation is preferable to multiple
     * loops through the same arrays.
     */
    struct ParsedLine {

	enum class Type {
	    /// Only values were read (default)
	    ValueLine,

	    /// A command was found (<cmd> param1 param2 etc.)
	    Command,

	    /// A closing tag was found (</task>)
	    ClosingTag
	};

	enum class ParamType {
	    /// Query Identifier parameters
	    Identifier,

	    /// Query String parameters
	    String,

	    /// Query number parameters
	    Number
	};

	/// Command (if found)
	std::string_view				command;

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

	/// All found named parameters <cmd named_param1 = "value">
	std::map<std::string, std::string_view> 	named_params;

	/// Saves sequence of found parameters with associated type
	/// Holds the correct index for the corresponding vector
	std::vector<std::pair<ParamType, size_t>>	sequence;


	/**
	 * Convert ParamType to string for debugging and messages
	 */
	// TODO test with consteval
	static std::string param_type(ParamType pt) {
	    if (pt == ParamType::Identifier)
		return "Identifier";
	    else if (pt == ParamType::Number)
		return "Number";
	    else
		return "String";
	}

	/**
	 * Get vector for one parameter type
	 */
	// TODO test with consteval
	template<ParsedLine::ParamType pt>
	constexpr auto get_vector() {
	    if constexpr (pt == ParamType::Number)
		return &num_params;
	    else if constexpr (pt == ParamType::Identifier)
		return &id_params;
	    else
		return &str_params;
	}

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
	    const auto idx = pv->size();

	    sequence.emplace_back(pt, idx);
	    pv->push_back(val);
	}


	/**
	 * Get the parameter at index `idx`, returns an error if `pt` does not
	 * designate the correct type of the parameter at position `idx`
	 */
	// TODO experiment with:
	// decltype(std::declval<decltype(*get_vector<pt>())>().back())
	template<ParsedLine::ParamType pt, typename T>
	auto get_param(const size_t idx) const
	    -> std::expected<T, std::string> {
		//    -> std::expected<
		// decltype(std::declval<decltype(*get_vector<pt>())>().back()), std::string> {

	    if (idx >= sequence.size())
		return std::unexpected{ "Too few parameters" };

	    const auto param = sequence[idx];
	    if (param.first != pt)
		return std::unexpected{ comp_error("Expected ", param_type(pt), " at position ", idx) };

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

	void clear();
	bool empty() const;
    };

    /**
     *
     */
    std::ostream& operator<<(std::ostream& os, const ParsedLine& line);
}

#endif
