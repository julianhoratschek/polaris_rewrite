#include <benchmark/benchmark.h>

#include "../../Parameters.hpp"
#include "../../DustComponent_1.hpp"
#include "../../DustComponent.hpp"

#include <vector>

// namespace rewrite::benchmark {
    template<typename Comp>
    void init_dust_component(
	const parameters& param,
	const size_t dust_component_choice,
	Comp& single_component) {

	// Get size distribution parameters
	const std::string size_keyword = param.getDustSizeKeyword(dust_component_choice);
	single_component.setSizeParameter(size_keyword, param.getDustSizeParameter(dust_component_choice));

	single_component.setMaterialDensity(param.getMaterialDensity(dust_component_choice));
	// single_component.setPhaseFunctionID(param.getPhaseFunctionID(dust_component_choice));
	single_component.setPhaseFunctionID(PH_MIE);


	// Get global wavelength grid
	std::vector<double>	wavelengths(WL_STEPS);
	CMathFunctions::LogList(WL_MIN, WL_MAX, wavelengths, 10);
	single_component.setWavelengthList(wavelengths, 0);
    }

    static void BM_Legacy_DustRefractiveIndexFile(benchmark::State& state) {
	std::vector<double>	size_param {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14};
	parameters 		param;
	rewrite::legacy::CDustComponent	comp;

	param.AddDustComponentChoice(1);
	param.addDustComponent(
	    // current_path.string(),	// dust_path
	    // keywords[idist(gen)],	// size_keyword
	    "input/dust_nk/iron_p94.nk",
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

	init_dust_component(param, 0, comp);

	for (auto _: state)
	    comp.readDustRefractiveIndexFile(param, 0, param.getSizeMin(0), param.getSizeMax(0));
    }

    static void BM_Rewrite_DustRefractiveIndexFile(benchmark::State& state) {
	std::vector<double>	size_param {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14};
	parameters 		param;
	CDustComponent		comp;

	param.AddDustComponentChoice(1);
	param.addDustComponent(
	    // current_path.string(),	// dust_path
	    // keywords[idist(gen)],	// size_keyword
	    "input/dust_nk/iron_p94.nk",
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

	init_dust_component(param, 0, comp);

	for (auto _: state)
	    comp.readDustRefractiveIndexFile(param, 0, param.getSizeMin(0), param.getSizeMax(0));
    }
// }

BENCHMARK(BM_Legacy_DustRefractiveIndexFile);
BENCHMARK(BM_Rewrite_DustRefractiveIndexFile);

BENCHMARK_MAIN();
