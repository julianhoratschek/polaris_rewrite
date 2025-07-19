#ifndef RW_SCAMATR_PARSER
#define RW_SCAMATR_PARSER

#include "../Typedefs.hpp"
#include "../MathInterp.hpp"

#include "basic_loader.hpp"

#include <cstddef>
#include <expected>
#include <filesystem>
#include <fstream>
#include <vector>

namespace rewrite {

    struct ScaMatrFile {
	size_t		nr_of_scat_phi;
	size_t		nr_of_scat_theta_tmp;
	size_t		nr_of_scat_mat_elements;
	int		elements[16];
	unsigned int	phID;
	interp*		sca_mat_wl;
	bool		disable_mie_scattering;
    };
    
    class ScaMatrParser: public BasicLoader {
	ScaMatrFile		result;

	void cleanup() override {
	    delete[] result.sca_mat_wl;
	}


    public:
	ScaMatrFile& get_result() {
	    return result;
	}

	
	auto parse_file(const std::filesystem::path& path,
	    const size_t nr_of_dust_species,
	    const size_t nr_of_wavelength_dustcat,
	    const size_t nr_of_incident_angles,
	    double* wavelength_list_dustcat)
		-> std::expected<void, Message> {

	    const auto inf_path = path.parent_path() / (path.stem().string() + "scat.inf");
	    // const size_t nr_of_scat_theta_tmp = 2 * NANG - 1;
	    size_t column = 0;
	    // std::vector<double>	values(5);

	    file.open(inf_path);

	    // Error message if the read does not work
	    if(file.fail())
		return safe_error("Cannot open scattering matrix info file:");

	    // The first line needs 5 values
	    if (!next_line(file))
		return safe_error( "Unexpected end of file" );

	    if (const auto res = read_values();
		!res) return safe_error( res.error().message );

		//    for (column = 0; is_or_next<is_number>() && column < 5; column++) {
		// if (const auto num = get_number();
		//     not num.has_value()) return safe_error( num.error().message );
		// else values[column] = num.value();
		//    }

	    if (values.size() != 5)
		return safe_error( "Expected 5 parameters in first line" );

	    // The number of dust grain sizes
	    if (values[0] != nr_of_dust_species)
		return safe_error(
		    "Number of dust species does not match the number in the dust parameters file!");

	    // The number of wavelength used by the dust catalog
	    if (values[1] != nr_of_wavelength_dustcat) 
		return safe_error(
		    "Number of wavelength does not match the number in the dust parameters file!");

	    // The number of incident angles
	    if (values[2] != nr_of_incident_angles) 
		return safe_error(
		    "Number of incident angles does not match the number in the dust parameters file!");

	    // The number of phi angles (outgoing radiation)
	    result.nr_of_scat_phi = static_cast<unsigned int>(values[3]);

	    // The number of theta angles (outgoing radiation)
	    result.nr_of_scat_theta_tmp = static_cast<unsigned int>(values[4]);

	    if (!next_line(file))
		return safe_error( "Unexpected end of file" );

	    if (const auto num = get_number();
		not num.has_value()) return safe_error( "Wrong amount of scattering matrix elements" );
	    else
		result.nr_of_scat_mat_elements = num.value();

	    // The third line needs 16 values
	    if (!next_line(file))
		return safe_error( "Unexpected end of file" );

	    // The relation which scattering matrix entry is used at which position in
	    // the 4x4 matrix
	    if (const auto res = read_values();
		!res.has_value()) return safe_error( res.error().message );

	    if (values.size() != 16)
		return safe_error( "Wrong amount of Matrix elements" );

	    std::copy(values.begin(), values.end(), result.elements);

	    // Close the file reader
	    file.close();

	    // If scattering matrix is empty, disable mie scattering for the corresponding dust
	    // component
	    result.disable_mie_scattering = std::any_of(result.elements, result.elements + 16,
		[](auto e) { return e != 0; });

	    if (result.disable_mie_scattering) {
		result.phID = PH_HG;
		return {};
	    }

	    // First init of the scattering matrix interp
	    // interp ***** sca_mat_wl = new interp ****[nr_of_dust_species];
	    //
	    const size_t full_size = nr_of_dust_species * nr_of_incident_angles *
	    result.nr_of_scat_phi * result.nr_of_scat_theta_tmp * result.nr_of_scat_mat_elements;

	    auto *rd_data = new float[full_size];
	    auto *data = new double[nr_of_wavelength_dustcat * full_size];

	// omp-loop?
	    //    for (size_t i = 0; i < full_size; i++)
	    // sca_mat_wl[i].resize(nr_of_wavelength_dustcat);

	    for (size_t w = 0; w < nr_of_wavelength_dustcat; w++) {
		auto		bin_path = path.parent_path() / 
		    (path.stem().string() + std::format("wID{:03}.sca", w + 1));
		std::ifstream	bin_file(bin_path, std::ios::in | std::ios::binary);

		if (bin_file.fail())
		    return safe_error( "Could not open binary file" );

		bin_file.read(
		    reinterpret_cast<char*>(rd_data),
		    full_size * sizeof(float));

		for (size_t i = 0; i < full_size; i++)
		    data[i * nr_of_wavelength_dustcat + w] = static_cast<double>(rd_data[i]);

		bin_file.close();
	    }

	    delete[] rd_data;

	    result.sca_mat_wl = new interp[full_size];

	    for (size_t i = 0; i < full_size; i++) {
		result.sca_mat_wl[i].resizeShared(
		    nr_of_wavelength_dustcat,
		    wavelength_list_dustcat,
		    &data[i * nr_of_wavelength_dustcat]);
	    }

	    return {};
	}
    };

}

#endif
