#include <gtest/gtest.h>
#include <gmock/gmock.h>


#ifndef TEST_REWRITE
#define TEST_REWRITE
#endif

#include "../../Parameters.hpp"

#include "../../SourceStar.hpp"
#include "../../SourceStar_1.hpp"
#include "../../SourceBackground.hpp"
#include "../../SourceBackground_1.hpp"

namespace rewrite::testing {
    
    class TestSourceStar: public ::testing::TestWithParam<std::string> {
	
    protected:
	parameters		param;

	::CSourceStar		lc_data;
	CSourceStar		rw_data;

	TestSourceStar() {
	    std::vector<double>		val = {0.0, 0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7};
	    param.addPointSource(val, "path");
	}
    };

    TEST_P(TestSourceStar, SourceStarLoader) {
	bool lc_res = lc_data.setParameterFromFile(param, 0);
	bool rw_res = rw_data.setParameterFromFile(param, 0);

	ASSERT_EQ(lc_res, rw_res);

	ASSERT_EQ(lc_data.is_ext, rw_data.is_ext);
	EXPECT_THAT(lc_data.pos, ::testing::ContainerEq(rw_data.pos));
	ASSERT_EQ(lc_data.R, rw_data.R);
	ASSERT_EQ(lc_data.T, rw_data.T);
	ASSERT_EQ(lc_data.sp_ext, rw_data.sp_ext);
	ASSERT_EQ(lc_data.sp_ext_q, rw_data.sp_ext_q);
	ASSERT_EQ(lc_data.sp_ext_u, rw_data.sp_ext_u);
    }


    class TestSourceBackground: public: ::testing::TestWithParam<std::string> {
    protected:
	::
    }


    INSTANTIATE_TEST_SUITE_P(DustNkFiles, TestDustComponent, ::testing::ValuesIn(get_nk_files()));
}
