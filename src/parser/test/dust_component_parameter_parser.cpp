#include <gtest/gtest.h>

#include <filesystem>
#include <fstream>
#include <string>
#include <random>

#define TEST_REWRITE

#include "../../CommandParser.hpp"
#include "../../MathSpline.hpp"
#include "../../DustComponent.hpp"
#include "../../DustComponent_1.hpp"
#include "../../Parameters.hpp"
#include "../../MathFunctions.hpp"

namespace rewrite::testing {
    // TEST (DustParser, CalcSizeDistribution) {
    //
    // }
    //
    // TEST (DustParser, CheckGrainSizeLimits) {
    //
    // }

    // TEST (DustParser, ReadScatteringMatrices) {
    //
    // }
    
    class TestDustComponent: public ::testing::Test {
    protected:
	std::filesystem::path	current_path;
	parameters		param;
	std::vector<double>	wavelengths;

	CDustComponent 		comp_new;
	legacy::CDustComponent 	comp_old;

	void generate_nk_file() {
	    std::mt19937			gen(std::random_device{}());
	    std::uniform_real_distribution<>	rdist;
	    std::uniform_int_distribution<>	idist(0, 100000);

	    current_path = std::format("dust_file{:04}.nk", idist(gen));

	    std::ofstream			out(current_path);
	    double				nr_wavelengths(idist(gen)),
						nr_inc_angles(idist(gen));

	    out << "# Comment lines" << endl;
	    out << "String ID" << endl;
	    out << nr_wavelengths << '\t';
	    out << nr_inc_angles << '\t';
	    // 0 -> nr_wavelengths
	    // 1 -> nr_inc_angles
	    for (auto i = 0; i < 5; i++)
		out << rdist(gen) << '\t';
	    out << endl;

	    for (auto row = 0; row < nr_wavelengths; row++)
		out << wavelengths[row] << '\t' << rdist(gen) << '\t' << rdist(gen) << endl;
	}


	template<typename Comp>
	void init_dust_component(
	    const size_t dust_component_choice,
	    Comp& single_component) {

	    // Get size distribution parameters
	    const std::string size_keyword = param.getDustSizeKeyword(dust_component_choice);
	    single_component.setSizeParameter(size_keyword, param.getDustSizeParameter(dust_component_choice));

            single_component.setMaterialDensity(param.getMaterialDensity(dust_component_choice));
	    // single_component.setPhaseFunctionID(param.getPhaseFunctionID(dust_component_choice));
	    single_component.setPhaseFunctionID(PH_MIE);


	    // Get global wavelength grid
	    single_component.setWavelengthList(wavelengths, 0);
	}
    };

    TEST_F(TestDustComponent, ReadDustRefractiveIndexFile) {

	std::mt19937				gen(std::random_device{}());
	std::uniform_real_distribution<>	rdist;
	std::uniform_int_distribution<>		idist(0, 5);
	std::vector<double>			size_param(14);
	std::array				keywords{
	    "plaw", "plaw-ed", "plaw-cv", "plaw-ed-cv", "logn", "zda"
	};

	std::ranges::generate(size_param, [&]() { return rdist(gen); });
	CMathFunctions::LogList(0.01, 1000.1, wavelengths, 10);

	generate_nk_file();

	param.AddDustComponentChoice(1);
	param.addDustComponent(
	    current_path.string(),	// dust_path
	    keywords[idist(gen)],	// size_keyword
	    rdist(gen),			// dust_fractions
	    rdist(gen),			// material_density
	    rdist(gen),			// a_min_global
	    rdist(gen),			// a_max_global
	    size_param);

	init_dust_component(0, comp_new);
	init_dust_component(0, comp_old);

	if (!comp_new.readDustRefractiveIndexFile(param, 0,
	    param.getSizeMin(0), param.getSizeMax(0))) {
	    FAIL() << "Error running new loader";
	    return;
	}

	if (!comp_old.readDustRefractiveIndexFile(param, 0,
	    param.getSizeMin(0), param.getSizeMax(0))) {
	    FAIL() << "Error running old loader";
	    return;
	}

	std::filesystem::remove(current_path);

	EXPECT_EQ(comp_old.nr_of_dust_species, comp_new.nr_of_dust_species);
	EXPECT_EQ(comp_old.nr_of_wavelength, comp_new.nr_of_wavelength);
	EXPECT_EQ(comp_old.nr_of_incident_angles, comp_new.nr_of_incident_angles);
	EXPECT_DOUBLE_EQ(comp_old.aspect_ratio, comp_new.aspect_ratio);
	EXPECT_DOUBLE_EQ(comp_old.material_density, comp_new.material_density);
	EXPECT_DOUBLE_EQ(comp_old.sub_temp, comp_new.sub_temp);
	EXPECT_DOUBLE_EQ(comp_old.delta_rat, comp_new.delta_rat);
	EXPECT_EQ(comp_old.is_align, comp_new.is_align);
	EXPECT_DOUBLE_EQ(comp_old.gold_g_factor, comp_new.gold_g_factor);

	for (auto i = 0; i < comp_old.nr_of_dust_species; i++) {
	    EXPECT_DOUBLE_EQ(comp_old.a_eff[i], comp_new.a_eff[i]);
	    EXPECT_DOUBLE_EQ(comp_old.a_eff_squared[i], comp_new.a_eff_squared[i]);
	    EXPECT_DOUBLE_EQ(comp_old.grain_distribution_x_aeff_sq[i], comp_new.grain_distribution_x_aeff_sq[i]);
	    EXPECT_DOUBLE_EQ(comp_old.mass[i], comp_new.mass[i]);

	    for (auto w = 0; w < comp_old.nr_of_wavelength; i++) {
		EXPECT_EQ(comp_old.nr_of_scat_theta[i][w], comp_new.nr_of_scat_theta[i][w]);
		EXPECT_DOUBLE_EQ(comp_old.Qext1[i][w], comp_new.Qext1[i][w]);
		EXPECT_DOUBLE_EQ(comp_old.Qext2[i][w], comp_new.Qext2[i][w]);
		EXPECT_DOUBLE_EQ(comp_old.Qabs1[i][w], comp_new.Qabs1[i][w]);
		EXPECT_DOUBLE_EQ(comp_old.Qabs2[i][w], comp_new.Qabs2[i][w]);
		EXPECT_DOUBLE_EQ(comp_old.Qsca1[i][w], comp_new.Qsca1[i][w]);
		EXPECT_DOUBLE_EQ(comp_old.Qsca2[i][w], comp_new.Qsca2[i][w]);
		EXPECT_DOUBLE_EQ(comp_old.Qcirc[i][w], comp_new.Qcirc[i][w]);
		EXPECT_DOUBLE_EQ(comp_old.HGg[i][w], comp_new.HGg[i][w]);
		EXPECT_DOUBLE_EQ(comp_old.HGg2[i][w], comp_new.HGg2[i][w]);
		EXPECT_DOUBLE_EQ(comp_old.HGg3[i][w], comp_new.HGg3[i][w]);
		EXPECT_DOUBLE_EQ(comp_old.CextMean[i][w], comp_new.CextMean[i][w]);
		EXPECT_DOUBLE_EQ(comp_old.CabsMean[i][w], comp_new.CabsMean[i][w]);
		EXPECT_DOUBLE_EQ(comp_old.CscaMean[i][w], comp_new.CscaMean[i][w]);

		EXPECT_EQ(comp_old.Qtrq[i * comp_old.nr_of_wavelength + w], comp_new.Qtrq[i * comp_old.nr_of_wavelength + w]);
		EXPECT_EQ(comp_old.HG_g_factor[i * comp_old.nr_of_wavelength + w], comp_new.HG_g_factor[i * comp_old.nr_of_wavelength + w]);
		EXPECT_EQ(comp_old.HG_g2_factor[i * comp_old.nr_of_wavelength + w], comp_new.HG_g2_factor[i * comp_old.nr_of_wavelength + w]);
		EXPECT_EQ(comp_old.HG_g3_factor[i * comp_old.nr_of_wavelength + w], comp_new.HG_g3_factor[i * comp_old.nr_of_wavelength + w]);

		for (auto sth = 0; sth < comp_old.nr_of_scat_theta[i][w]; sth++)
		    EXPECT_DOUBLE_EQ(comp_old.scat_theta[i][w][sth], comp_new.scat_theta[i][w][sth]);

		for (auto inc = 0; inc < comp_old.nr_of_incident_angles; inc++)
		    for (auto sph = 0; sph < comp_old.nr_of_scat_phi; sph++)
			for (size_t sth = 0; sth < comp_old.nr_of_scat_theta[i][w]; sth++)
			    EXPECT_EQ(comp_old.sca_mat[i][w][inc][sph][sth], comp_new.sca_mat[i][w][inc][sph][sth]);
	    }
	}

	EXPECT_EQ(comp_old.nr_of_scat_mat_elements, comp_new.nr_of_scat_mat_elements);
	EXPECT_EQ(comp_old.nr_of_scat_phi, comp_new.nr_of_scat_phi);
	EXPECT_EQ(comp_old.scat_loaded, comp_new.scat_loaded);
    }
    
}
