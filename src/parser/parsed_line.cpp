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
	    os << "\t'" << v << "'\n";

	os << "\nString Parameters:\n";
	for (auto& v: line.str_params)
	    os << "\t'" << v << "'\n";

	return os;
    }


    auto ParsedLine::get_num(const size_t idx) const {
	return get_param<ParamType::Number, double>(idx);
    }


    auto ParsedLine::get_id(const size_t idx) const {
	return get_param<ParamType::Identifier, std::string_view>(idx);
    }


    auto ParsedLine::get_str(const size_t idx) const {
	return get_param<ParamType::String, std::string_view>(idx);
    }


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
