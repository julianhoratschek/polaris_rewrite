#include "command_parser.hpp"

#include <fstream>
#include <iostream>
#include <string>

using namespace std;
using namespace rewrite;

auto process_line(const ParsedLine& line)
    -> expected<void, Message> {

    cout << line << endl;
    return {};
}


int main() {
    CommandParser	parser;
    ifstream		fl("input/dust_nk/dsharp_b18.nk");

    if (fl.fail()) {
	cout << "Could not open file" << endl;
	return 0;
    }

    string	line;
    while(getline(fl, line)) {
	if (const auto res = parser.parse_line(line);
	    !res) cout << msg_color(res.error().type, "ERROR: ") << res.error().message << endl;
	else
	    process_line(parser.get_last_line());
    }

    cout << endl << endl << "DONE";
    return 0;
}
