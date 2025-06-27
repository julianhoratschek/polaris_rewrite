#include "command_parser.hpp"

int main() {
    using namespace std;

    rewrite::CommandParser	parser;
    vector<string>		tests = {
	"\t<cmd> POLARIS_TEST",
	"\t \t<line> 1 2 43 5 23.4 23e-2",
	"<comma> 1,4 23,54 2.3"
    };

    for(auto& line: tests) {
	cout << "Testing: " << line << endl;
	if(const auto e = parser.parse_line(line); !e)
	    cout << "Error: " << e.error();
	cout << parser.get_last_line() << endl;
    }

    
    return 0;
}
