#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "command_parser.hpp"
#include "../CommandParser.hpp"
#include "polaris_commands.hpp"

#include <vector>
#include <sstream>
#include <string>
#include <random>


namespace rewrite::testing {
    class CommandParserTest : public ::testing::Test {
    protected:
	rewrite::CommandParser	parser_new;
	::CCommandParser 	parser_old;
	parameters		param_new, param_old;
	command_map		commands;

	void SetUp() override {
	    commands = make_cmd_map();
	}

	template<typename T>
	std::string make_param() {
	    std::random_device	rd;
	    std::mt19937		gen{rd()};

	    if constexpr (std::is_same_v<T, double>) {
		std::uniform_real_distribution<double> dist;

		return std::to_string(dist(gen));
	    }

	    if constexpr (std::is_same_v<T, int>) {
		std::uniform_int_distribution<int> dist;

		return std::to_string(dist(gen));
	    }

	    if constexpr (std::is_same_v<T, std::string>)
		return "\"This is a String\"";

	    return "Id3nt__1f13r__";
	}

	template<typename... Ts>
	std::string make_line(const std::string& cmd) {
	    auto	os = std::ostringstream("<") << cmd << "> ";
	    ((os << make_param<Ts>() << ' '), ...);
	    return os.str();
	}
    };


    TEST_F(CommandParserTest, DefaultParsing) {
	parser_new.parse_line("<cmd>");
	ASSERT_EQ(parser_new.get_last_line().command, "cmd");

	parser_new.parse_line("<cmd named1 = \"value1\" named2=\"value2\">");
	auto line = parser_new.get_last_line();
	ASSERT_EQ(line.named_params["named1"], "value1");
	ASSERT_EQ(line.named_params["named2"], "value2");

	parser_new.parse_line("<cmd> 1 2.3 4,56 7.89e-2 100,435e-2");
	auto comp_num = std::vector{1.0, 2.3, 4.56, 7.89e-2, 100.435e-2};
	line = parser_new.get_last_line();
	EXPECT_THAT(line.num_params, ::testing::ContainerEq(comp_num));
	//
	parser_new.parse_line("<cmd> \"String number 1\" \"String number 2\"");
	line = parser_new.get_last_line();
	EXPECT_EQ(line.str_params[0], "String number 1");
	EXPECT_EQ(line.str_params[1], "String number 2");

	parser_new.parse_line("<cmd named = \"value\"> \"string\" 1,34e-12 id_param \"string2\"");
	line = parser_new.get_last_line();
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

    TEST_F(CommandParserTest, Cmd) {
	auto	line = make_line<int>("cmd");

	if (const auto e = parser_new.parse_line(line); !e)
	    FAIL() << e.error();
	auto parsed = parser_new.get_last_line();
	cmd_cmd(parsed, param_new);

	parser_old
    }
}
