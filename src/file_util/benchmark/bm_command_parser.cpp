#include <benchmark/benchmark.h>
#include <filesystem>
#include <fstream>

#include "../../CommandParser.hpp"
#include "../parser/polaris_parser.hpp"

static const std::filesystem::path	path = "./src/file_util/test/cmd/all.cmd";
// static const std::filesystem::path		path("../../../file_util/test/cmd/");

static void BM_Legacy_ParseCommands(benchmark::State& state) {
    std::ofstream		nul("NUL");
    const auto buf = std::cout.rdbuf();
    std::cout.rdbuf(nul.rdbuf());

    for (auto _: state) {
	// CCommandParser		parser("./src/file_util/test/cmd/all.cmd");
	CCommandParser		parser(path.string());
	parser.parse();
    }

    std::cout.rdbuf(buf);
}


static void BM_Rewrite_ParseCommands(benchmark::State& state) {
    std::ofstream		nul("NUL");

    const auto buf = std::cout.rdbuf();
    std::cout.rdbuf(nul.rdbuf());

    for (auto _: state) {
	rewrite::PolarisParser	parser;
	const auto e = parser.parse_file(path, rewrite::default_error_handler);
    }
    std::cout.rdbuf(buf);
}

BENCHMARK(BM_Legacy_ParseCommands);
BENCHMARK(BM_Rewrite_ParseCommands);

BENCHMARK_MAIN();
