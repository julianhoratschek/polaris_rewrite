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
    
    class TestDustComponent: public ::testing::TestWithParam<std::string> {
    protected:
	parameters		param;
	std::vector<double>	wavelengths;

	CDustComponent 		comp_new;
	legacy::CDustComponent 	comp_old;

	TestDustComponent() {
	    wavelengths.resize(WL_STEPS);
	}

	auto generate_nk_file() {
	    std::mt19937			gen(std::random_device{}());
	    std::uniform_real_distribution<>	rdist;
	    std::uniform_int_distribution<>	idist(0, 100000);

	    auto current_path = std::format("dust_file{:04}.nk", idist(gen));

	    std::ofstream			out(current_path);
	    double				nr_inc_angles(idist(gen));

	    out << "# Comment lines" << endl;
	    out << "String ID" << endl;
	    out << wavelengths.size() << '\t';
	    out << nr_inc_angles << '\t';
	    // 0 -> nr_wavelengths
	    // 1 -> nr_inc_angles
	    for (auto i = 0; i < 5; i++)
		out << rdist(gen) << '\t';
	    out << endl;

	    for (const auto& wl: wavelengths)
		out << wl << '\t' << rdist(gen) << '\t' << rdist(gen) << endl;
	    out << endl;

	    return current_path;
	}

	auto generate_calorimetry_file(
	    const std::filesystem::path& filename,
	    const size_t nr_of_dust_species)
	    -> std::expected<std::filesystem::path, std::string>
	{
	    std::mt19937			gen(std::random_device{}());
	    std::uniform_real_distribution<>	rdist;
	    std::uniform_int_distribution<>	idist(0, 10000);
	    std::error_code 			ec;

	    auto current_path = filename.parent_path() / filename.stem() / "calorimetry.dat";

	    filesystem::create_directories(current_path.parent_path(), ec);

	    std::ofstream			out(current_path);

	    if (out.fail())
		return "Failed file creation";

	    int					nr_of_calorimetry_temperatures{idist(gen)};

	    out << "# Comment lines" << endl;
	    out << nr_of_calorimetry_temperatures << endl;
	    for (auto i = 0; i < nr_of_calorimetry_temperatures; i++)
		out << rdist(gen) << '\t';
	    out << endl;

	    out << (idist(gen) < 5000 ? 0 : 1) << endl;

	    for (auto i = 0; i < nr_of_calorimetry_temperatures; i++) {
		if (idist(gen) < 5000)
		    out << rdist(gen) << endl;
		else {
		    for (auto a = 0; a < nr_of_dust_species; a++)
			out << rdist(gen) << '\t';
		    out << endl;
		}
	    }

	    out.close();
	    return current_path;
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

    TEST_P(TestDustComponent, ReadDustRefractiveIndexFile) {
	std::mt19937				gen(std::random_device{}());
	std::uniform_real_distribution<>	rdist;
	std::uniform_int_distribution<>		idist(0, 5);
	std::vector<double>			size_param(14);
	std::array				keywords{
	    "plaw", "plaw-ed", "plaw-cv", "plaw-ed-cv", "logn", "zda"
	};


	std::ranges::generate(size_param, [&]() { return rdist(gen); });

	//1.00000e-07-1.00000e-01
	//6.19920000e-11-1.23984000e-01
	//1.00000000e-09-1.00000000e-02
	CMathFunctions::LogList(WL_MIN, WL_MAX, wavelengths, 10);

	// generate_nk_file();

	double a_min_global = rdist(gen);
	double a_max_global = a_min_global;

	param.AddDustComponentChoice(1);
	param.addDustComponent(
	    // current_path.string(),	// dust_path
	    // keywords[idist(gen)],	// size_keyword
	    GetParam(),
	    // "input/dust_nk/iron_p94.nk",
	    "plaw",
	    // rdist(gen),			// dust_fractions
	    0.25, //0.625
	    // rdist(gen),			// material_density
	    2250, //3500
	    // a_min_global,			// a_min_global
	    5e-09,
	    // a_max_global,			// a_max_global
	    2.5e-07,
	    size_param);

	init_dust_component(0, comp_new);
	init_dust_component(0, comp_old);

	bool p_new_res = comp_new.readDustRefractiveIndexFile(param, 0,
	    param.getSizeMin(0), param.getSizeMax(0));

	bool p_old_res = comp_old.readDustRefractiveIndexFile(param, 0,
	    param.getSizeMin(0), param.getSizeMax(0));

	// std::filesystem::remove(current_path);

	ASSERT_EQ(p_old_res, p_new_res);

	if (!p_old_res)
	    GTEST_SKIP() << "File not readable" << endl;

	ASSERT_EQ(comp_old.nr_of_dust_species, comp_new.nr_of_dust_species);
	ASSERT_EQ(comp_old.nr_of_wavelength, comp_new.nr_of_wavelength);
	ASSERT_EQ(comp_old.nr_of_incident_angles, comp_new.nr_of_incident_angles);
	ASSERT_EQ(comp_old.nr_of_scat_phi, comp_new.nr_of_scat_phi);
	ASSERT_EQ(comp_old.nr_of_scat_mat_elements, comp_new.nr_of_scat_mat_elements);

	EXPECT_DOUBLE_EQ(comp_old.aspect_ratio, comp_new.aspect_ratio);
	EXPECT_DOUBLE_EQ(comp_old.material_density, comp_new.material_density);
	EXPECT_DOUBLE_EQ(comp_old.sub_temp, comp_new.sub_temp);
	EXPECT_DOUBLE_EQ(comp_old.delta_rat, comp_new.delta_rat);
	EXPECT_EQ(comp_old.is_align, comp_new.is_align);
	EXPECT_DOUBLE_EQ(comp_old.gold_g_factor, comp_new.gold_g_factor);

	ASSERT_THAT(comp_old.real_list, ::testing::ContainerEq(comp_new.real_list));

	for (auto i = 0; i < comp_old.nr_of_dust_species; i++) {
	    EXPECT_DOUBLE_EQ(comp_old.a_eff[i], comp_new.a_eff[i]);
	    EXPECT_DOUBLE_EQ(comp_old.a_eff_squared[i], comp_new.a_eff_squared[i]);
	    EXPECT_DOUBLE_EQ(comp_old.grain_distribution_x_aeff_sq[i], comp_new.grain_distribution_x_aeff_sq[i]);
	    EXPECT_DOUBLE_EQ(comp_old.mass[i], comp_new.mass[i]);

	    for (auto w = 0; w < comp_old.nr_of_wavelength; w++) {
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

		ASSERT_EQ(comp_old.nr_of_scat_theta[i][w], comp_new.nr_of_scat_theta[i][w]);

		for (auto sth = 0; sth < comp_old.nr_of_scat_theta[i][w]; sth++)
		    ASSERT_DOUBLE_EQ(comp_old.scat_theta[i][w][sth], comp_new.scat_theta[i][w][sth]) << i << " " << w << " " << sth;

		for (auto inc = 0; inc < comp_old.nr_of_incident_angles; inc++)
		    for (auto sph = 0; sph < comp_old.nr_of_scat_phi; sph++)
			for (size_t sth = 0; sth < comp_old.nr_of_scat_theta[i][w]; sth++)
			    ASSERT_EQ(comp_old.sca_mat[i][w][inc][sph][sth], comp_new.sca_mat[i][w][inc][sph][sth]) << i << " " << w << " " << inc << " " << sph << " " << sth;
	    }
	}

	EXPECT_EQ(comp_old.scat_loaded, comp_new.scat_loaded);
    }

    TEST_P(TestDustComponent, ReadCalorimetryFile)
    {
	std::vector<double>	size_param(14);
	std::ranges::iota(size_param, 0);

	CMathFunctions::LogList(WL_MIN, WL_MAX, wavelengths, 10);
	param.AddDustComponentChoice(1);
	param.addDustComponent(
	    // current_path.string(),	// dust_path
	    // keywords[idist(gen)],	// size_keyword
	    GetParam(),
	    // "input/dust_nk/iron_p94.nk",
	    "plaw",
	    // rdist(gen),			// dust_fractions
	    0.25, //0.625
	    // rdist(gen),			// material_density
	    2250, //3500
	    // a_min_global,			// a_min_global
	    5e-09,
	    // a_max_global,			// a_max_global
	    2.5e-07,
	    size_param);

	init_dust_component(0, comp_new);
	init_dust_component(0, comp_old);

	if (!comp_new.readDustRefractiveIndexFile(param, 0, param.getSizeMin(0), param.getSizeMax(0))
	    || !comp_old.readDustRefractiveIndexFile(param, 0, param.getSizeMin(0), param.getSizeMax(0)))
	    GTEST_SKIP() << "Files not readable" << endl;

	ASSERT_EQ(comp_old.nr_of_dust_species, comp_new.nr_of_dust_species);

	std::filesystem::path cal_path;
	if (const auto fgen = generate_calorimetry_file(param.getDustPath(0), comp_old.nr_of_dust_species);
	    !fgen.has_value()) { FAIL(); return; }
	else cal_path = fgen.value();

	bool p_new_res = comp_new.readCalorimetryFile(param, 0);
	bool p_old_res = comp_old.readCalorimetryFile(param, 0);

	// std::filesystem::remove_all(cal_path.parent_path());

	ASSERT_EQ(p_old_res, p_new_res);

	EXPECT_EQ(comp_old.calorimetry_type, comp_new.calorimetry_type);

	ASSERT_EQ(comp_old.nr_of_dust_species, comp_new.nr_of_dust_species);
	ASSERT_EQ(comp_old.nr_of_calorimetry_temperatures, comp_new.nr_of_calorimetry_temperatures);

	for (auto i{0}; i < comp_old.nr_of_calorimetry_temperatures; i++)
	    EXPECT_DOUBLE_EQ(comp_old.calorimetry_temperatures[i], comp_new.calorimetry_temperatures[i]);

	for (auto a{0}; a < comp_old.nr_of_dust_species; a++)
	    for (auto c{0}; c < comp_old.nr_of_calorimetry_temperatures; c++)
		EXPECT_DOUBLE_EQ(comp_old.enthalpy[a][c], comp_new.enthalpy[a][c]);

	EXPECT_EQ(comp_old.calorimetry_loaded, comp_new.calorimetry_loaded);
    }

    
    std::vector<std::string> get_nk_files() {
	std::vector<std::string>	result;

	for (auto& dir: std::filesystem::directory_iterator("input/dust_nk/")) {
	    if (dir.path().extension() != ".nk")
		continue;
	    result.push_back(dir.path().string());
	}

	return result;
    }


    INSTANTIATE_TEST_SUITE_P(DustNkFiles, TestDustComponent, ::testing::ValuesIn(get_nk_files()));
}
