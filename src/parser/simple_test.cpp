#include <iostream>
#include "polaris_commands.hpp"
#include "polaris_parser.hpp"

#include "../CommandParser.hpp"

using namespace std;


int main() {
    cout << "New Parser" << endl;
    rewrite::PolarisParser	parser;

    if (const auto e = parser.parse_polaris_cmd("./src/parser/test.cmd"); !e)
	cout << e.error() << endl;


    cout << "Old Parser" << endl;
    CCommandParser		old_parser("./src/parser/test.cmd");

    if (!old_parser.parse())
	cout << "Error parsing" << endl;

    auto	new_list = parser.get_param_list(),
		old_list = old_parser.getParameterList();

    for(int i=0;i<new_list.size();i++)
	cout << '[' << i << "]: " << new_list[i].compare(old_list[i]) << endl;

    return 0;
}
