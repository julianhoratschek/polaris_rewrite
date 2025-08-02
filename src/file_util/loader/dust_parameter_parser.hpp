#ifndef RW_DUST_PARAMETER_PARSER
#define RW_DUST_PARAMETER_PARSER

#include "basic_loader.hpp"

#include <cstddef>
#include <filesystem>
#include <fstream>
#include <expected>
#include <string>
#include <vector>

namespace rewrite {

    /*
     * Notes to spline-object:
     * 	- x - values don't change internally
     * 	- so a shared x is possible
     * 	- however, if any of the shared objects goes out of scope
     * 	  it will free the pointer (dangerous!)
     */

    struct DustParameterFile {
	//TODO: Free resources here
	std::string		stringID;
	size_t			nr_dust_species;
	size_t			nr_wavelengths;
	size_t			nr_inc_angles;
	double			aspect_ratio;
	double			material_density;
	double			sub_temp;
	double			delta;
	bool			align;

	std::vector<double>	a_eff;
	double			*wavelengths{nullptr};
	double			*eff_wl{nullptr};
	double			*Qtrq_wl{nullptr};
	double			*HG_g_factor_wl{nullptr};
	double			*HG_g2_factor_wl{nullptr};
	double			*HG_g3_factor_wl{nullptr};

	void cleanup() {
	    delete[] wavelengths;
	    delete[] eff_wl;
	    delete[] Qtrq_wl;
	    delete[] HG_g_factor_wl;
	    delete[] HG_g2_factor_wl;
	    delete[] HG_g3_factor_wl;
	}
    };


    class DustParameterParser: public BasicLoader<DustParameterFile> {
    public:
	// template<ErrorHandlerFn ErrorFn>
	// auto parse_file(std::filesystem::path path, ErrorFn error_handler)
	auto parse_file(std::filesystem::path path)
		-> std::expected<void, Message> {

	    std::array<double, 8>	values;
	    size_t			column = 0;
	    size_t			data_length = 0;


	    file.open(path, std::ios::binary);

	    if (file.fail())
		return safe_error( "Not a valid file");

	    // Read String ID

	    if (!next_line(file))
		return safe_error( "Unexpected End of File" );

	    result.stringID = current_line;

	    // Read 8 Values: general parameters

	    if (!next_line(file))
		return safe_error( "Unexpected End of File" );

	    while (is_or_next<is_number>() && column < 8)
		if (const auto num = get_number();
		    not num.has_value()) return safe_error( num.error().message );
		else values[column++] = num.value();

	    if (column != 8)
		// TODO: Correct line
		return safe_error( "Expected 8 Values in line 2" );

	    std::tie(
		result.nr_dust_species,
		result.nr_wavelengths,
		result.nr_inc_angles,
		result.aspect_ratio,
		result.material_density,
		result.sub_temp,
		result.delta,
		result.align) = values;

	    // result.nr_dust_species = static_cast<size_t>(values[0]);
	    // result.nr_wavelengths = static_cast<size_t>(values[1]);
	    // result.nr_inc_angles = static_cast<size_t>(values[2]);
	    // result.aspect_ratio = values[3];
	    //
	    // result.material_density = values[4];
	    // result.sub_temp = values[5];
	    // result.delta = values[6];
	    // result.align = values[7] != 0;
	    
	    // Read a_eff values per dust grain size

	    if (!next_line(file))
		return safe_error( "Unexpected End of File" );

	    result.a_eff.resize(result.nr_dust_species);
	    for (column = 0; is_or_next<is_number>() && column < result.nr_dust_species; column++) {
		if (const auto num = get_number();
		    not num.has_value()) return safe_error( num.error().message );
		else result.a_eff[column] = num.value();
	    }

	    if (result.a_eff.size() != result.nr_dust_species)
		return safe_error( "Mismatching dust species counts" );

	    // Read wavelengths

	    if (!next_line(file))
		return safe_error( "Unexpected End of File" );

	    result.wavelengths = new double[result.nr_wavelengths];
	    for (column = 0; column < result.nr_wavelengths && is_or_next<is_number>(); column++)
		if (const auto num = get_number();
		    not num.has_value()) return safe_error( num.error().message );
		else result.wavelengths[column] = num.value();

	    if (column != result.nr_wavelengths)
		return safe_error( "Mismatching wavelengths count" );

	    // Get all relevant remaining lines

	    auto pos = file.tellg();
	    data_length = 0;
	    while (next_line(file))
		++data_length;
	    file.clear();
	    file.seekg(pos, std::ios::beg);

	    // If not a line per combination of grain size and wavelength was found in the
	    // catalog, show error
	    if (data_length != result.nr_wavelengths * result.nr_dust_species)
		return safe_error( "Wrong amount of efficiencies" );

	    // Allocate memory
	    // WARNING: This will not be freed by this class! The receier
	    // is obliged to free this
	    // TODO: Rather have shared pointers? runtime-cost?

	    result.eff_wl = new double[data_length * 7];
	    result.Qtrq_wl = new double[data_length * result.nr_inc_angles];
	    result.HG_g_factor_wl = new double[data_length * result.nr_inc_angles];
	    result.HG_g2_factor_wl = new double[data_length * result.nr_inc_angles];
	    result.HG_g3_factor_wl = new double[data_length * result.nr_inc_angles];

	    for (auto i = 0; i < data_length * result.nr_inc_angles; i++) {
		result.HG_g_factor_wl[i] = 0.0;
		result.HG_g2_factor_wl[i] = 0.0;
		result.HG_g3_factor_wl[i] = 1.0;
	    }

	    // Read and transform values while reading
	    // This makes it easier to simply transfer the pointers to
	    // the memory locations to the splines
	    
	    size_t cnt = 0;

	    for (auto w = 0; w < result.nr_wavelengths; w++) {
		for (auto a = 0; a < result.nr_dust_species; a++) {
		    if (!next_line(file))
			return safe_error( "Unexpected end of file");

		    for(column = 0; column < 7 && is_or_next<is_number>(); column++) {
			if (const auto num = get_number();
			    not num.has_value()) return safe_error( num.error().message );
			else
			    result.eff_wl[a * result.nr_wavelengths * 7 + column * result.nr_wavelengths + w] = num.value();
		    }

		    for(column = 0; column < result.nr_inc_angles && expect_next<is_number>(); column++) {
			if (const auto num = get_number();
			    not num.has_value()) return safe_error( num.error().message );
			else
			    result.Qtrq_wl[a * result.nr_wavelengths * result.nr_inc_angles + column * result.nr_wavelengths + w] = num.value();
		    }

		    for(column = 0; column < result.nr_inc_angles && expect_next<is_number>(); column++) {
			if (const auto num = get_number();
			    not num.has_value()) return safe_error( num.error().message );
			else
			    result.HG_g_factor_wl[a * result.nr_wavelengths * result.nr_inc_angles + column * result.nr_wavelengths + w] = num.value();
		    }

		    for(column = 0; column < result.nr_inc_angles && expect_next<is_number>(); column++) {
			if (const auto num = get_number();
			    not num.has_value()) return safe_error( num.error().message );
			else
			    result.HG_g2_factor_wl[a * result.nr_wavelengths * result.nr_inc_angles + column * result.nr_wavelengths + w] = num.value();
		    }

		    for(column = 0; column < result.nr_inc_angles && expect_next<is_number>(); column++) {
			if (const auto num = get_number();
			    not num.has_value()) return safe_error( num.error().message );
			else
			    result.HG_g3_factor_wl[a * result.nr_wavelengths * result.nr_inc_angles + column * result.nr_wavelengths + w] = num.value();
		    }

		}
	    }

	    // Close the text file reader for the dust catalog
	    file.close();

	    return {};
	}

    };
}

#endif
