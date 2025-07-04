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
	auto parse_file(const std::filesystem::path& path,
	    const size_t nr_of_dust_species)
	    -> std::expected<void, Message> {

	    size_t		column;

	    file.open(path.parent_path() /
		(path.stem().string() + "calorimetry.dat"));

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
	    result.enthalpy = new double *[nr_of_dust_species];

	    // Add second dimension
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

	    
    // Init progress counter
    uint line_counter = 0;
    uint cmd_counter = 0;

    // Go through each line of the info file
    while(getline(calo_reader, line))
    {
        // Format the text file line
        ps.formatLine(line);

        // Increase line counter
        line_counter++;

        // If the line is empty -> skip
        if(line.size() == 0)
            continue;

        // Parse the values of the current line
        values = ps.parseValues(line);

        // If no values found -> skip
        if(values.size() == 0)
            continue;

        // Increase the command counter
        cmd_counter++;

        switch(cmd_counter)
        {

            default:
                // The other lines have either one or a value per dust grain size
                if(values.size() != 1 && values.size() != nr_of_dust_species)
                {
                    cout << ERROR_LINE << "Wrong amount of dust species in:" << endl;
                    cout << calo_filename.c_str() << " line " << line_counter << "!" << endl;
                    return false;
                }

                // Get temperature index
                uint t = cmd_counter - 4;

                for(uint a = 0; a < nr_of_dust_species; a++)
                {
                    // Use either a value per grain size or one value for all
                    double tmp_value;
                    if(values.size() == 1)
                        tmp_value = double(values[0]);
                    else
                        tmp_value = double(values[a]);

                    // If heat capacity, perform integration
                    if(calorimetry_type == CALO_HEAT_CAP)
                    {
                        if(t == 0)
                            enthalpy[a][t] = tmp_value * calorimetry_temperatures[t];
                        else
                            enthalpy[a][t] =
                                enthalpy[a][t - 1] +
                                tmp_value * (calorimetry_temperatures[t] - calorimetry_temperatures[t - 1]);
                    }
                    else if(calorimetry_type == CALO_ENTHALPY)
                    {
                        // Enthalpy is already in the right unit
                        enthalpy[a][t] = tmp_value;
                    }
                    else
                    {
                        // Reset enthalpy if wrong type
                        enthalpy[a][t] = 0;
                    }
                }
                break;
        }
    }
    // Close calorimetry file reader
    calo_reader.close();

    // Multiply the specific enthalpy with the grain size to get the enthalpy
    for(uint a = 0; a < nr_of_dust_species; a++)
        for(uint t = 0; t < nr_of_calorimetry_temperatures; t++)
            enthalpy[a][t] *= 4.0 / 3.0 * PI * a_eff[a] * a_eff[a] * a_eff[a];

    // Set that the calorimetry file was successfully loaded
    calorimetry_loaded = true;

    return true;
}

	}
};


}

#endif
