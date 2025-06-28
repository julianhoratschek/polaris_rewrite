#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "command_parser.hpp"
#include "../CommandParser.hpp"

#include <vector>


namespace rewrite::testing {
    class CommandParserTest : public ::testing::Test {
    protected:
	rewrite::CommandParser	parser;

	void SetUp() override {
	}

    };

    TEST_F(CommandParserTest, DefaultParsing) {
	parser.parse_line("<cmd>");
	ASSERT_EQ(parser.get_last_line().command, "cmd");

	parser.parse_line("<cmd named1 = \"value1\" named2=\"value2\">");
	auto line = parser.get_last_line();
	ASSERT_EQ(line.named_params["named1"], "value1");
	ASSERT_EQ(line.named_params["named2"], "value2");

	parser.parse_line("<cmd> 1 2.3 4,56 7.89e-2 100,435e-2");
	auto comp_num = std::vector{1.0, 2.3, 4.56, 7.89e-2, 100.435e-2};
	line = parser.get_last_line();
	EXPECT_THAT(line.num_params, ::testing::ContainerEq(comp_num));
	//
	parser.parse_line("<cmd> \"String number 1\" \"String number 2\"");
	line = parser.get_last_line();
	EXPECT_EQ(line.str_params[0], "String number 1");
	EXPECT_EQ(line.str_params[1], "String number 2");

	parser.parse_line("<cmd named = \"value\"> \"string\" 1,34e-12 id_param \"string2\"");
	line = parser.get_last_line();
	ASSERT_EQ(line.named_params["named"], "value");

	auto a = line.get_param<ParsedLine::ParamType::String, std::string_view>(0).value();
	auto b = line.get_param<ParsedLine::ParamType::Number, double>(1).value();
	auto c = line.get_param<ParsedLine::ParamType::Identifier, std::string_view>(2).value();
	auto d = line.get_param<ParsedLine::ParamType::String, std::string_view>(3).value();

	ASSERT_EQ(a, "string");
	ASSERT_EQ(b, 1.34e-12);
	ASSERT_EQ(c, "id_param");
	ASSERT_EQ(d, "string2");
    }
}
