#include "parsed_line.hpp"

namespace rewrite {
    std::ostream& operator<<(std::ostream& os, const ParsedLine& line) {
	os << "Parsed Line (Nr. " << line.line_nr << ")\n\n"
	    << "Command: '" << line.command << "'\nParameter Sequence:\n";

	for (auto& v: line.sequence)
	    os << '\t' << ParsedLine::param_type(v.first) << " @ " << v.second << '\n';

	os << "\nNamed Parameters:\n";
	for (auto& v: line.named_params)
	    os << '\t' << v.first << ": " << v.second << '\n';

	os << "\nNumber Parameters:\n";
	for (auto& v: line.num_params)
	    os << '\t' << v << '\n';

	os << "\nID Parameters:\n";
	for (auto& v: line.id_params)
	    os << '\t' << v << '\n';

	os << "\nString Parameters:\n";
	for (auto& v: line.str_params)
	    os << '\t' << v << '\n';

	return os;
    }

	//    template<ParsedLine::ParamType pt>
	//    constexpr auto ParsedLine::get_vector() {
	// if constexpr (pt == ParamType::Number)
	//     return &num_params;
	// else if constexpr (pt == ParamType::Identifier)
	//     return &id_params;
	// else
	//     return &str_params;
	//    }
	//
	//    template<ParsedLine::ParamType pt>
	//    constexpr auto ParsedLine::get_vector() const {
	// if constexpr (pt == ParamType::Number)
	//     return &num_params;
	// else if constexpr (pt == ParamType::Identifier)
	//     return &id_params;
	// else
	//     return &str_params;
	//    }

	//    template<ParsedLine::ParamType pt, typename T>
	//    void ParsedLine::push_param(T val) {
	// auto	pv = get_vector<pt>();
	// const auto idx = pv->size();
	//
	// sequence.emplace_back(pt, idx);
	// pv->push_back(val);
	//    }

	//    template<ParsedLine::ParamType pt, typename T>
	//    auto ParsedLine::get_param(const size_t idx) const
	// -> std::expected<T, std::string> {
	//
	// if (idx >= sequence.size())
	//     return std::unexpected{ "Too few parameters" };
	//
	// const auto param = sequence[idx];
	// if (param.first != pt)
	//     return std::unexpected{ comp_error("Expected ", param_type<pt>(), " at position ", idx) };
	//
	// auto pv = get_vector<pt>();
	// return pv->at(param.second);
	//    }

    void ParsedLine::clear() {
	type = Type::ValueLine;
	command = "";
	sequence.clear();
	num_params.clear();
	str_params.clear();
	id_params.clear();
	named_params.clear();
    }

    bool ParsedLine::empty() const {
	return command == "" && sequence.empty();
    }
}
