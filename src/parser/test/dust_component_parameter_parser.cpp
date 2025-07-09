#include <gtest/gtest.h>

#include <filesystem>
#include <fstream>
#include <string>
#include "../../CommandParser.hpp"
#include "../../MathSpline.hpp"
#include "../../DustComponent.hpp"
#include "../../DustComponent_1.hpp"
#include "../../Parameters.hpp"

namespace rewrite::testing {
    using namespace std;

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
	parameters		param;

	CDustComponent 		comp_new;
	legacy::CDustComponent 	comp_old;

	template<typename Comp>
	void init_dust_component(
	    const size_t dust_component_choice,
	    Comp& single_component) {

	    double fraction = param.getDustFraction(dust_component_choice);
	    double fraction_sum = 1.0;

	    // Set mass fractions of each
	    if(param.getIndividualDustMassFractions()) {
		single_component.setIndividualDustMassFractions(true);
		single_component.setDustMassFraction(fraction);
		single_component.setFraction(fraction / fraction_sum);
	    }
	    else {
		single_component.setDustMassFraction(fraction * param.getDustMassFraction() /
							     fraction_sum);
		single_component.setFraction(fraction / fraction_sum);
	    }

	    // Get size distribution parameters
	    const string size_keyword = param.getDustSizeKeyword(dust_component_choice);
	    single_component.setSizeParameter(size_keyword, param.getDustSizeParameter(dust_component_choice));

	    // Get material density and similar user defined parameters
	    single_component.setMaterialDensity(param.getMaterialDensity(dust_component_choice));
	    single_component.setFHighJ(param.getFHighJ());
	    single_component.setFcorr(param.getFcorr());
	    single_component.setQref(param.getQref());
	    single_component.setAlphaQ(param.getAlphaQ());
	    single_component.setRayleighReductionFactor(param.getRayleighReductionFactor());

	    single_component.setDelta0(param.getDelta0());
	    single_component.setLarmF(param.getLarmF());
	    single_component.setMu(param.getMu());
	    // single_component.setPhaseFunctionID(param.getPhaseFunctionID());
	    single_component.setPhaseFunctionID(param.getPhaseFunctionID(dust_component_choice));

	    // Get global wavelength grid
	    single_component.setWavelengthList(wavelength_list, wavelength_offset);
	}
    };

    TEST_F(TestDustComponent, ReadDustRefractiveIndexFile) {

	parameters 		param;
	vector<double>		size_param(14, 0.0);


	param.AddDustComponentChoice(1);
	param.addDustComponent("path.nk", "plaw", 1.0, 1.0, 0.1, 1e5, size_param);

	comp_new.readDustParameterFile(param, 0);
	comp_old.readDustParameterFile(param, 0);
    }
    
}
