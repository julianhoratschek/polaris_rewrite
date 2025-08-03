
#include <gtest/gtest.h>
#include <gmock/gmock.h>


#ifndef TEST_REWRITE
#define TEST_REWRITE
#endif

#include "../parser/polaris_parser.hpp"
#include "../../CommandParser.hpp"

#include "../../Parameters.hpp"

#include <filesystem>
#include <vector>

namespace rewrite::testing {
    
    class TestPolarisParser: public ::testing::TestWithParam<std::string> {
	
    protected:
	std::vector<parameters>		lc_param;
	std::vector<parameters>		rw_param;

	CCommandParser		lc_parser;
	PolarisParser		rw_parser;

	TestPolarisParser() { }
    };

    TEST_P(TestPolarisParser, ParserTest) {
	const auto file_name = GetParam();

	lc_parser.setCommandFile(file_name);

	bool lc_res = lc_parser.parse();
	bool rw_res = rw_parser.parse_file(file_name, default_error_handler).has_value();

	ASSERT_EQ(lc_res, rw_res);

	lc_param = lc_parser.getParameterList();
	rw_param = std::move(rw_parser.get_param_list());

	ASSERT_EQ(lc_param.size(), rw_param.size());
	for (size_t i = 0; i < lc_param.size(); i++)
	    ASSERT_EQ(lc_param[i], rw_param[i]);
    }


    std::vector<std::string> get_cmd_files() {
	std::filesystem::path		path("./src/file_util/test/cmd/");
	std::vector<std::string>	res;

	for (const auto& p: std::filesystem::directory_iterator{path}) {
	    const auto fname = p.path();
	    if (fname.extension() != ".cmd")
		continue;
	    res.emplace_back(fname.string());
	}

	return res;
    }

    INSTANTIATE_TEST_SUITE_P(CommandFiles, TestPolarisParser, ::testing::ValuesIn(get_cmd_files()));
}
