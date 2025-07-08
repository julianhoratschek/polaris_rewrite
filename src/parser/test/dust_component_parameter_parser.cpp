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

    TEST(DustParser, DustParameterFile) {
	CDustComponent 		comp_new;
	legacy::CDustComponent	comp_old;

	parameters 		param;
    }
    
}
