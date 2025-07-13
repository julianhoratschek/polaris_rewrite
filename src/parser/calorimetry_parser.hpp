#ifndef RW_CALORIMETRY_PARSER
#define RW_CALORIMETRY_PARSER

#include "../Typedefs.hpp"

#include "basic_parser.hpp"
#include <cstddef>
#include <fstream>
#include <expected>
#include <string>
#include <filesystem>

namespace rewrite {

    struct CalorimetryFile {
	size_t		nr_of_calorimetry_temperatures;
	double		*calorimetry_temperatures;
	double		**enthalpy;
	unsigned int	calorimetry_type;
    };

    class CalorimetryParser: public BasicParser {
	std::ifstream	file;
	CalorimetryFile	result;

	std::unexpected<Message> safe_error(const std::string& msg) {
	    delete[] result.enthalpy;
	    delete[] result.calorimetry_temperatures;
	    file.close();
	    return std::unexpected { Message {
		msg
	    }};
	}


    public:
	CalorimetryFile& get_result() { return result; }

	auto parse_file(const std::filesystem::path& path,
	    const size_t nr_of_dust_species)
		-> std::expected<void, Message> {

	    size_t		column;

	    file.open(
		path.parent_path() /
		std::format("{}/calorimetry.dat", path.stem().string()));

	    if (file.fail())
		return safe_error( "Could not open calorimetry file" );

	    if (!next_line(file))
		return safe_error( "Unexpected end of file" );

	    if (const auto num = get_number();
		not num.has_value()) return safe_error( "Wrong amount of calorimetry temperatures");
	    else
		result.nr_of_calorimetry_temperatures = num.value();

	    // Init array for the calorimetry temperatures
	    result.calorimetry_temperatures = new double[result.nr_of_calorimetry_temperatures];

	    // Init 2D array for the enthalpy
	    result.enthalpy = new double*[nr_of_dust_species];

	    // Add second dimension
	    // TODO: zero?
	    for(size_t a = 0; a < nr_of_dust_species; a++)
		result.enthalpy[a] = new double[result.nr_of_calorimetry_temperatures];

	    // The second line needs a value per calorimetric temperature
	    if (!next_line(file))
		return safe_error( "Unexpected end of file" );

	    for (column = 0; is_or_next<is_number>() && column < result.nr_of_calorimetry_temperatures; column++) {
		if (const auto num = get_number();
		    !num.has_value()) return safe_error( num.error().message );
		else result.calorimetry_temperatures[column] = num.value();
	    }

	    if (column != result.nr_of_calorimetry_temperatures)
		return safe_error( "Wrong calorimetry temperatures" );

	    // The third line needs one value
	    if (!next_line(file))
		return safe_error( "Unexpected end of file" );

	    // The unit of the calorimetry data
	    if (const auto num = get_number();
		!num.has_value()) return safe_error( "Wrong calorimetry type" );
	    else result.calorimetry_type = static_cast<unsigned int>(num.value());

	    // Only heat capacity or enthalpy are possible
	    if(result.calorimetry_type != CALO_HEAT_CAP && result.calorimetry_type != CALO_ENTHALPY)
		return safe_error( "Wrong calorimetry type" );

	    // Get special case: first line of temperatures

	    if (!next_line(file))
		return safe_error( "Unexpected end of file" );

	    const auto fact = result.calorimetry_type == CALO_HEAT_CAP ?
		result.calorimetry_temperatures[0] : 1;
	    double last_num;

	    for (column = 0; is_or_next<is_number>() && column < nr_of_dust_species; column++) {
		last_num = get_number().value_or(0.0);
		result.enthalpy[column][0] = last_num * fact;
	    }

		//    size_t enthalpy_counter = 0;
		//
		//    while (next_line(file)) {
		//               // Get temperature index
		//               // uint t = cmd_counter - 4;
		//
		// ++enthalpy_counter;
	    for (size_t enthalpy_counter = 0; next_line(file); enthalpy_counter++) {
                for(column = 0; is_or_next<is_number>() && column < nr_of_dust_species; column++) {
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
                
			//              if(values.size() != 1 && values.size() != nr_of_dust_species)
			//    return safe_error (
			// "Wrong amount of dust species in:" );
	    }
	    
	    // Close calorimetry file reader
	    file.close();

	    return {};

	}
    };


}

#endif
