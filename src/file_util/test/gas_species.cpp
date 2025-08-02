#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <filesystem>
#include <fstream>
#include <string>
#include <random>
#include <expected>

#ifndef TEST_REWRITE
#define TEST_REWRITE
#endif

#include "../../GasSpecies.hpp"
#include "../../GasSpecies_1.hpp"

namespace rewrite::testing {
    
    class TestGasSpecies: public ::testing::TestWithParam<std::string> {
    protected:

	legacy::CGasSpecies	lc_data;
	CGasSpecies 		rw_data;

	TestGasSpecies() {
	}
    };

    TEST_P(TestGasSpecies, ReadGasParameterFile) {
	bool	lc_res = lc_data.readGasParamaterFile("path", 0, 0);
	auto 	rw_res = rw_data.readGasParameterFile("path", 0, 0);

	ASSERT_EQ(lc_res, rw_res.has_value());
	ASSERT_EQ(lc_data.stringID, rw_data.stringID);
	ASSERT_EQ(lc_data.molecular_weight, rw_data.molecular_weight);

	ASSERT_EQ(lc_data.nr_of_energy_level, rw_data.nr_of_energy_level);
	ASSERT_THAT(lc_data.energy_level, ::testing::ElementsAreArray(rw_data.energy_level, rw_data.nr_of_energy_level));
	ASSERT_THAT(lc_data.g_level, ::testing::ElementsAreArray(rw_data.g_level, rw_data.nr_of_energy_level));
	ASSERT_THAT(lc_data.quantum_numbers, ::testing::ElementsAreArray(rw_data.quantum_numbers, rw_data.nr_of_energy_level));
	ASSERT_THAT(lc_data.nr_of_sublevel, ::testing::ElementsAreArray(rw_data.nr_of_sublevel, rw_data.nr_of_energy_level));

	ASSERT_EQ(lc_data.nr_of_transitions, rw_data.nr_of_transitions);
	ASSERT_THAT(lc_data.upper_level, ::testing::ElementsAreArray(rw_data.upper_level, rw_data.nr_of_transitions));
	ASSERT_THAT(lc_data.lower_level, ::testing::ElementsAreArray(rw_data.lower_level, rw_data.nr_of_transitions));
	ASSERT_THAT(lc_data.trans_freq, ::testing::ElementsAreArray(rw_data.trans_freq, rw_data.nr_of_transitions));
	ASSERT_THAT(lc_data.trans_freq, ::testing::ElementsAreArray(rw_data.trans_freq, rw_data.nr_of_transitions));
	ASSERT_THAT(lc_data.trans_inner_energy, ::testing::ElementsAreArray(rw_data.trans_inner_energy, rw_data.nr_of_transitions));

	ASSERT_THAT(lc_data.trans_is_zeeman_split, ::testing::ElementsAreArray(rw_data.trans_is_zeeman_split, rw_data.nr_of_transitions));

	// TODO: correct for zeeman
	for (size_t i = 0; i < rw_data.nr_of_transitions; i++) {
	    ASSERT_THAT(lc_data.trans_einstA[i], ::testing::ElementsAreArray(rw_data.trans_einstA[i], rw_data.nr_of_transitions));
	    ASSERT_THAT(lc_data.trans_einstB_ul[i], ::testing::ElementsAreArray(rw_data.trans_einstB_ul[i], rw_data.nr_of_transitions));
	    ASSERT_THAT(lc_data.trans_einstB_lu[i], ::testing::ElementsAreArray(rw_data.trans_einstB_lu[i], rw_data.nr_of_transitions));
	}

	ASSERT_EQ(lc_data.nr_of_spectral_lines, rw_data.spectral_lines.size());
	ASSERT_THAT(lc_data.unique_spectral_lines, ::testing::ContainerEq(rw_data.unique_spectral_lines));

	ASSERT_EQ(lc_data.nr_of_col_partner, rw_data.nr_of_col_partner);
	ASSERT_THAT(lc_data.nr_of_col_transition, ::testing::ElementsAreArray(rw_data.nr_of_col_transition, rw_data.nr_of_col_partner));
	ASSERT_THAT(lc_data.nr_of_col_temp, ::testing::ElementsAreArray(rw_data.nr_of_col_temp, rw_data.nr_of_col_partner));
	ASSERT_THAT(lc_data.orientation_H2, ::testing::ElementsAreArray(rw_data.orientation_H2, rw_data.nr_of_col_partner));

	for (size_t i = 0; i < rw_data.nr_of_col_partner; i++) {
	    ASSERT_EQ(lc_data.nr_of_col_temp[i], rw_data.nr_of_col_temp[i]);
	    ASSERT_THAT(lc_data.collision_temp[i], ::testing::ElementsAreArray(rw_data.collision_temp[i], rw_data.nr_of_col_temp[i]));
	    
	    ASSERT_THAT(lc_data.col_upper[i], ::testing::ElementsAreArray(rw_data.col_upper[i], rw_data.nr_of_col_transition[i]));
	    ASSERT_THAT(lc_data.col_lower[i], ::testing::ElementsAreArray(rw_data.col_lower[i], rw_data.nr_of_col_transition[i]));

	    for(size_t j = 0; j < rw_data.nr_of_col_transition[i]; j++)
		ASSERT_THAT(lc_data.col_matrix[i][j], ::testing::ElementsAreArray(rw_data.col_matrix[i][j], rw_data.nr_of_col_temp[i]));
	}

    }
    

    INSTANTIATE_TEST_SUITE_P(DustNkFiles, TestDustComponent, ::testing::ValuesIn(get_nk_files()));
}
