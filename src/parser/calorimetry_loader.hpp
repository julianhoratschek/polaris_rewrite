#ifndef RW_CALORIMETRY_LOADER
#define RW_CALORIMETRY_LOADER

#include "../Typedefs.hpp"

#include "basic_loader.hpp"
#include <cstddef>
#include <fstream>
#include <expected>
#include <string>
#include <filesystem>

namespace rewrite {

    struct CalorimetryFile {
	size_t		nr_of_calorimetry_temperatures;
	size_t		nr_of_dust_species;	// HACK: Only needed for cleanup
	double		*calorimetry_temperatures;
	double		**enthalpy;
	unsigned int	calorimetry_type;
    };

    class CalorimetryLoader: public BasicLoader {
	CalorimetryFile	result;

	void cleanup() override
	{
	    for (auto i = 0; i < result.nr_of_dust_species; i++)
		delete[] result.enthalpy[i];
	    delete[] result.enthalpy;
	    delete[] result.calorimetry_temperatures;
	}


    public:
	CalorimetryFile& get_result() { return result; }

	auto parse_file(
	    const std::filesystem::path& path,
	    const size_t nr_of_dust_species)
	    -> std::expected<void, Message>
	{
	    file.open(path.parent_path() / path.stem() / "calorimetry.dat");

	    result.nr_of_dust_species = 0;

	    if (file.fail())
		return safe_error( "Could not open calorimetry file" );

	    if (const auto num = rdline_number<size_t>("Nr of calorimetry temperatures"); !num.has_value())
		return std::unexpected{ num.error() };
	    else result.nr_of_calorimetry_temperatures = num.value();

	    // Init array for the calorimetry temperatures
	    result.calorimetry_temperatures = new double[result.nr_of_calorimetry_temperatures];

	    // Init 2D array for the enthalpy
	    result.enthalpy = new double*[nr_of_dust_species];

	    // Add second dimension
	    // TODO: zero?
	    for(size_t a = 0; a < nr_of_dust_species; a++)
		result.enthalpy[a] = new double[result.nr_of_calorimetry_temperatures];

	    result.nr_of_dust_species = nr_of_dust_species;

	    // The second line needs a value per calorimetric temperature
	    if (const auto res = rdline_values(result.nr_of_calorimetry_temperatures, "Calorimetry temperatures");
		!res.has_value()) return res;

	    std::copy(values.begin(), values.end(), result.calorimetry_temperatures);

	    // The third line needs one value
	    if (const auto num = rdline_number<unsigned int>("Calorimetry Type"); !num.has_value())
		return std::unexpected{ num.error() };
	    else result.calorimetry_type = num.value();

	    // Only heat capacity or enthalpy are possible
	    if(result.calorimetry_type != CALO_HEAT_CAP && result.calorimetry_type != CALO_ENTHALPY)
		return safe_error( "Wrong calorimetry type: Only accept HEAT_CAP and ENTHALPY" );

	    // Get special case: first line of temperatures

	    if (!next_line(file) || !is_or_next<is_number>())
		return safe_error( "Unexpected end of file" );

	    const auto fact = result.calorimetry_type == CALO_HEAT_CAP ?
		result.calorimetry_temperatures[0] : 1;
	    double last_num;

	    for (size_t column = 0; column < nr_of_dust_species; column++) {
		if (is_or_next<is_number>())
		    last_num = get_number().value_or(0.0);
		result.enthalpy[column][0] = last_num * fact;
	    }

	    size_t enthalpy_counter;
	    for (enthalpy_counter = 1; next_line(file) && is_or_next<is_number>(); enthalpy_counter++) {
                for(size_t column = 0; column < nr_of_dust_species; column++) {
		    if (is_or_next<is_number>())
			last_num = get_number().value_or(0.0);

                    // If heat capacity, perform integration
                    if(result.calorimetry_type == CALO_HEAT_CAP) {
			result.enthalpy[column][enthalpy_counter] =
			    result.enthalpy[column][enthalpy_counter - 1] +
			    last_num * (result.calorimetry_temperatures[enthalpy_counter] - result.calorimetry_temperatures[enthalpy_counter - 1]);
                    }
                    else
                        // Enthalpy is already in the right unit
                        result.enthalpy[column][enthalpy_counter] = last_num;
		}
	    }

	    if (enthalpy_counter != result.nr_of_calorimetry_temperatures)
		return safe_error("Wrong amount of lines");
	    
	    // Close calorimetry file reader
	    file.close();

	    return {};

	}
    };


}

#endif
