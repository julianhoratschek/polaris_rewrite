#include <iostream>
#include "polaris_commands.hpp"
#include "polaris_parser.hpp"

using namespace std;


int main() {
    rewrite::PolarisParser	parser;

    if (const auto e = parser.parse_polaris_cmd("./src/parser/test.cmd"); !e)
	cout << e.error() << endl;

    cout << "Done" << endl;
    return 0;
}
