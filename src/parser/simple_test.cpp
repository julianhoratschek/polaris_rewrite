#include <iostream>
#include <string_view>
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
    const filesystem::path 		path{"./src/parser/cmd/"};

    for(const auto& filename: filesystem::directory_iterator(path)) {
	cout << endl << endl << "----------------------------------" << endl << endl;
	cout << "Current Test: " << filename.path() << endl << endl;

	cout << "New Parser" << endl;
	rewrite::PolarisParser	parser;

	if (const auto e = parser.parse_file(filename.path(), error_msg); !e)
	    error_msg(e.error());

	cout << endl << "--------------------------------" << endl << endl;

	cout << "Old Parser" << endl;
	CCommandParser		old_parser(filename.path().string());

	if (!old_parser.parse())
	    cout << "Error parsing" << endl;

	auto	new_list = parser.get_param_list(),
		    old_list = old_parser.getParameterList();

	for(int i=0;i<new_list.size();i++) {
	    cout << "Start: " << new_list[i].getStart() << " " << old_list[i].getStart() << endl;
	    cout << "Stop: " << new_list[i].getStop() << " " << old_list[i].getStop() << endl;
	    cout << '[' << setw(2) << setfill('0') << i << "]: " << new_list[i].compare(old_list[i]) << endl;
	}
    }

    return 0;
}
