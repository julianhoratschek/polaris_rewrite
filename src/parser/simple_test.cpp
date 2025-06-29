#include <iostream>
#include <string>
#include <string_view>
#include "polaris_commands.hpp"
#include "polaris_parser.hpp"

#include "../CommandParser.hpp"

using namespace std;


bool error_msg(const rewrite::Message& msg) {
    string_view	label = msg.type == rewrite::Message::Type::Warning ?
	"WARNING " : "INFO ";

    cout << rewrite::msg_color(msg.type, label) << msg.message << endl;
    return true;
}


int main() {
    const string	filename{"./src/parser/small.cmd"};

    cout << "New Parser" << endl;
    rewrite::PolarisParser	parser;

    if (const auto e = parser.parse_polaris_cmd(
	filename, error_msg); !e)
	error_msg(e.error());

    cout << endl << "--------------------------------" << endl << endl;

    cout << "Old Parser" << endl;
    CCommandParser		old_parser(filename);

    if (!old_parser.parse())
	cout << "Error parsing" << endl;

    auto	new_list = parser.get_param_list(),
		old_list = old_parser.getParameterList();

    for(int i=0;i<new_list.size();i++)
	cout << '[' << i << "]: " << new_list[i].compare(old_list[i]) << endl;

    return 0;
}
