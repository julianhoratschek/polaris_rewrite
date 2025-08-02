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
#include "../../SourceStarField.hpp"
#include "../../SourceStarField_1.hpp"
#include "../../SourceISRF.hpp"
#include "../../SourceISRF_1.hpp"
#include "../../SourceAGN.hpp"
#include "../../SourceAGN_1.hpp"

namespace rewrite::testing {
    
    class TestSourceStar: public ::testing::TestWithParam<std::string> {
	
    protected:
	parameters		param;

	legacy::CSourceStar	lc_data;
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


    class TestSourceBackground: public ::testing::TestWithParam<std::string> {
    protected:
	parameters			param;

	legacy::CSourceBackground 	lc_data;
	CSourceBackground 		rw_data;

	TestSourceBackground() {
	    std::vector<double>		val = {0.0, 0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8};
	    param.addDiffuseSource(val, "path");
	    param.addBackgroundSource("path", val);
	}
    };

    TEST_P(TestSourceBackground, SourceBackgroundLoader) {
	bool lc_res = lc_data.setParameterFromFile(param, 0);
	bool rw_res = rw_data.setParameterFromFile(param, 0);

	ASSERT_EQ(lc_res, rw_res);

	ASSERT_EQ(lc_data.rot_angle1, rw_data.rot_angle1);
	ASSERT_EQ(lc_data.rot_angle2, rw_data.rot_angle2);
	ASSERT_EQ(lc_data.nr_of_photons, rw_data.nr_of_photons);
	ASSERT_EQ(lc_data.bins, rw_data.bins);
	ASSERT_EQ(lc_data.max_len, rw_data.max_len);
	ASSERT_EQ(lc_data.f, rw_data.f);
	ASSERT_EQ(lc_data.temp, rw_data.temp);
	ASSERT_EQ(lc_data.q, rw_data.q);
	ASSERT_EQ(lc_data.u, rw_data.u);
	ASSERT_EQ(lc_data.v, rw_data.v);
	ASSERT_EQ(lc_data.constant, rw_data.constant);
    }


    class TestSourceStarField: public ::testing::TestWithParam<std::string> {
	
    protected:
	parameters			param;

	legacy::CSourceStarField	lc_data;
	CSourceStarField		rw_data;

	TestSourceStarField() {
	    std::vector<double>		val = {0.0, 0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8};
	    param.addDiffuseSource(val, "path");
	}
    };

    TEST_P(TestSourceStarField, SourceStarFieldLoader) {
	bool lc_res = lc_data.setParameterFromFile(param, 0);
	bool rw_res = rw_data.setParameterFromFile(param, 0);

	ASSERT_EQ(lc_res, rw_res);

	ASSERT_EQ(lc_data.is_ext, rw_data.is_ext);
	EXPECT_THAT(lc_data.pos, ::testing::ContainerEq(rw_data.pos));
	ASSERT_EQ(lc_data.R, rw_data.R);
	ASSERT_EQ(lc_data.T, rw_data.T);
	ASSERT_EQ(lc_data.var, rw_data.var);
	ASSERT_EQ(lc_data.nr_of_photons, rw_data.nr_of_photons);

	ASSERT_EQ(lc_data.sp_ext, rw_data.sp_ext);
	ASSERT_EQ(lc_data.sp_ext_q, rw_data.sp_ext_q);
	ASSERT_EQ(lc_data.sp_ext_u, rw_data.sp_ext_u);
    }

    class TestSourceISRF: public ::testing::TestWithParam<std::string> {
	
    protected:
	parameters			param;

	legacy::CSourceISRF 		lc_data;
	CSourceISRF			rw_data;

	TestSourceISRF() {
	    param.setISRF("path", 0.0, 0.0);
	    param.setNrOfISRFPhotons(10000);
	}
    };

    TEST_P(TestSourceISRF, SourceISRFLoader) {
	EXPECT_THAT(lc_data.wavelength_list, ::testing::ContainerEq(rw_data.wavelength_list));

	bool lc_res = lc_data.setParameterFromFile(param, 0);
	bool rw_res = rw_data.setParameterFromFile(param, 0);

	ASSERT_EQ(lc_res, rw_res);

	ASSERT_EQ(lc_data.nr_of_photons, rw_data.nr_of_photons);
	ASSERT_EQ(lc_data.radius, rw_data.radius);

	ASSERT_EQ(lc_data.c_q, rw_data.c_q);
	ASSERT_EQ(lc_data.c_u, rw_data.c_u);
	ASSERT_EQ(lc_data.c_v, rw_data.c_v);

	ASSERT_EQ(lc_data.sp_ext, rw_data.sp_ext);
    }


    class TestSourceAGN: public ::testing::TestWithParam<std::string> {
	
    protected:
	parameters		param;

	legacy::CSourceAGN 	lc_data;
	CSourceAGN		rw_data;

	TestSourceAGN() {
	    std::vector<double>		val = {0.0, 0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7};
	    param.addPointSource(val, "path");
	}
    };

    TEST_P(TestSourceAGN, SourceAGNLoader) {
	bool lc_res = lc_data.setParameterFromFile(param, 0);
	bool rw_res = rw_data.setParameterFromFile(param, 0);

	ASSERT_EQ(lc_res, rw_res);

	ASSERT_EQ(lc_data.is_ext, rw_data.is_ext);
	EXPECT_THAT(lc_data.pos, ::testing::ContainerEq(rw_data.pos));
	ASSERT_EQ(lc_data.R, rw_data.R);
	ASSERT_EQ(lc_data.T, rw_data.T);
	ASSERT_EQ(lc_data.nr_of_photons, rw_data.nr_of_photons);
	ASSERT_EQ(lc_data.sp_ext, rw_data.sp_ext);
	ASSERT_EQ(lc_data.sp_ext_q, rw_data.sp_ext_q);
	ASSERT_EQ(lc_data.sp_ext_u, rw_data.sp_ext_u);
    }

    INSTANTIATE_TEST_SUITE_P(DustNkFiles, TestDustComponent, ::testing::ValuesIn(get_nk_files()));
}
