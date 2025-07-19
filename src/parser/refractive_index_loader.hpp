#ifndef RW_REFRACTIVE_INDEX_LOADER
#define RW_REFRACTIVE_INDEX_LOADER

#include "basic_loader.hpp"

// #include "../MathSpline.hpp"
// #include "../MathFunctions.hpp"

#include <cstddef>
#include <filesystem>
#include <fstream>
#include <string>
#include <expected>

namespace rewrite {

    struct RefractiveIndexFile {
	std::string	stringID;

	size_t		nr_wavelengths;
	size_t		nr_inc_angles;
	double		aspect_ratio;
	double		material_density;
	double		sub_temp;
	double		delta;
	bool		align;
	double 		*wavelengths{nullptr};
	double		*real_part{nullptr};
	double		*imag_part{nullptr};
    };

    class RefractiveIndexFileLoader: public BasicLoader {
	RefractiveIndexFile	result;
	std::ifstream		file;

	void cleanup() override
	{
	    delete[] result.wavelengths;
	    delete[] result.real_part;
	    delete[] result.imag_part;
	}
    
    public:

	RefractiveIndexFile& get_result() { return result; }

	auto parse_file(
	    const std::filesystem::path& path)
	    -> std::expected<void, Message>
	{
	    std::size_t		column;

	    file.open(path);

	    // Error message if the read does not work
	    if (file.fail())
		return safe_error("Cannot open dust refractive index file");

	    if (!next_line(file))
		return safe_error( "Unexpected end of file" );

	    result.stringID = current_line;

	    if (!next_line(file))
		return safe_error( "Unexpected end of file" );

	    if (const auto res = read_values();
		!res) return safe_error( res.error().message );

	    if (values.size() != 7)
		return safe_error( std::format( "Expected 7 Values in line {}", line_nr ));

	    result.nr_wavelengths = values[0];
            result.nr_inc_angles = 1; // For non-spherical: (uint) values[1];
	    result.aspect_ratio = 1; // For non-spherical: values[2];
	    result.material_density = values[3];
	    result.sub_temp = values[4];
	    result.delta = 1;
	    result.align = values[6];

	    result.wavelengths = new double[result.nr_wavelengths],
	    result.real_part = new double[result.nr_wavelengths],
	    result.imag_part = new double[result.nr_wavelengths];
	    double *ptr[3]{result.wavelengths, result.real_part, result.imag_part};

	    std::size_t row;

	    for (row = 0; next_line(file) && row < result.nr_wavelengths; row++) {
		for (column = 0; is_or_next<is_number>() && column < 3; column++) {
		    if (const auto num = get_number();
			!num.has_value()) return safe_error(num.error().message);
		    else {
			*ptr[column] = num.value();
			++ptr[column];
		    }
		}

		if (column != 3)
		    return safe_error( "Expected 3 values per line" );
	    }

	    // If not a line per combination of grain size and wavelength was found in the
	    // catalog, show error
	    if (row != result.nr_wavelengths)
		return safe_error( "Wrong amount of efficiencies in file" );

	    // Close the text file reader for the dust catalog
	    file.close();
	    return {};
	}

    };

}

#endif

