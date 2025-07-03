#ifndef RW_SCAMATR_PARSER
#define RW_SCAMATR_PARSER

#include "../Typedefs.hpp"

#include "basic_parser.hpp"

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
    };
    
    class ScaMatrParser: public BasicParser {
	private:
	    std::ifstream	inf_file;
	    ScaMatrFile		result;

	    std::unexpected<Message> safe_error(const std::string& msg) {
		if (inf_file.is_open())
		    inf_file.close();
		return std::unexpected { Message { msg } };
	    }


	public:
	    auto parse_file(const std::filesystem::path& path,
		const size_t nr_of_dust_species,
		const size_t nr_of_wavelength_dustcat,
		const size_t nr_of_incident_angles)
		    -> std::expected<void, Message> {

		const auto inf_path = path.parent_path() / (path.stem().string() + "scat.inf");
		// const size_t nr_of_scat_theta_tmp = 2 * NANG - 1;
		size_t column = 0;
		std::vector<double>	values(5);

		inf_file.open(inf_path);

		// Error message if the read does not work
		if(inf_file.fail())
		    return std::unexpected { Message {
		    "Cannot open scattering matrix info file:"
		} };

		// The first line needs 5 values
		if (!next_line(inf_file))
		    return safe_error( "Unexpected end of file" );

		for (column = 0; is_or_next<is_number>() && column < 5; column++) {
		    if (const auto num = get_number();
			not num.has_value()) return safe_error( num.error().message );
		    else values[column] = num.value();
		}

		if (column != 5)
		    return safe_error( "Expected 5 parameters in first line" );

		// The number of dust grain sizes
		if(values[0] != nr_of_dust_species)
		    return safe_error(
			"Number of dust species does not match the number in the dust parameters file!");

		// The number of wavelength used by the dust catalog
		if(values[1] != nr_of_wavelength_dustcat) 
		    return safe_error(
			"Number of wavelength does not match the number in the dust parameters file!");

		// The number of incident angles
		if(values[2] != nr_of_incident_angles) 
		    return safe_error(
			"Number of incident angles does not match the number in the dust parameters file!");

		// The number of phi angles (outgoing radiation)
		result.nr_of_scat_phi = static_cast<unsigned int>(values[3]);

		// The number of theta angles (outgoing radiation)
		result.nr_of_scat_theta_tmp = static_cast<unsigned int>(values[4]);

		if (!next_line(inf_file))
		    return safe_error( "Unexpected end of file" );

		if (const auto num = get_number();
		    not num.has_value())
		    return safe_error( "Wrong amount of scattering matrix elements" );
		else
		    result.nr_of_scat_mat_elements = num.value();

		// The third line needs 16 values
		if (!next_line(inf_file))
		    return safe_error( "Unexpected end of file" );

		// The relation which scattering matrix entry is used at which position in
		// the 4x4 matrix
		for (column = 0; column < 16; column++) {
		    if (const auto num = get_number();
			not num.has_value()) return safe_error( "Wrong amount of matrix elements" );
		    else result.elements[column] = num.value();
		}


		// Close the file reader
		inf_file.close();

		// If scattering matrix is empty, disable mie scattering for the corresponding dust
		// component
		const bool disable_mie_scattering = std::any_of(result.elements, result.elements + 16,
		    [](auto e) { e != 0; });

		if (disable_mie_scattering) {
		    result.phID = PH_HG;
		    return {};
		}

	    // First init of the scattering matrix interp
	    // interp ***** sca_mat_wl = new interp ****[nr_of_dust_species];
	    //
	    const size_t full_size = nr_of_dust_species * nr_of_incident_angles *
	    nr_of_scat_phi * nr_of_scat_theta_tmp * nr_of_scat_mat_elements;

	    interp *sca_mat_wl = new interp[full_size];

	    // omp-loop?
	    for (size_t i = 0; i < full_size; i++)
		sca_mat_wl[i].resize(nr_of_wavelength_dustcat);

	    // Init error bool
	    bool error = false;

	    // Set maximum counter value
	    max_counter = nr_of_wavelength_dustcat * nr_of_dust_species;

	    #pragma omp parallel for
	    for(int w = 0; w < int(nr_of_wavelength_dustcat); w++)
	    {
		if(error)
		    continue;

		const auto str_ID_end = std::format("wID{:03}.sca", w + 1);

		float tmp_val = 0;

		// Create the filename
		string bin_filename = path;
		bin_filename += SEP;
		bin_filename += str_ID_end;

		// Init text file reader for scattering binary file
		ifstream bin_reader(bin_filename, ios::in | ios::binary);

		// Error message if the read does not work
		if(bin_reader.fail())
		{
		    cout << ERROR_LINE << "Cannot open scattering matrix file:" << endl;
		    cout << bin_filename.c_str() << endl;
		    bin_reader.close();
		    error = true;
		}

		for(uint a = 0; a < nr_of_dust_species; a++)
		{
		    for(uint inc = 0; inc < nr_of_incident_angles; inc++)
		    {
			for(uint sph = 0; sph < nr_of_scat_phi; sph++)
			{
			    for(uint sth = 0; sth < nr_of_scat_theta_tmp; sth++)
			    {
				for(uint mat = 0; mat < nr_of_scat_mat_elements; mat++)
				{
				    // Read value from binary file
				    bin_reader.read((char *)&tmp_val, 4);

				    // Set value of interp to the read value
				    sca_mat_wl[a][inc][sph][sth][mat].setValue(
					w, wavelength_list_dustcat[w], tmp_val);
				}
			    }
			}
		    }
		}
		// Close scattering binary file
		bin_reader.close();
	    }

	    if(error)
		return false;

	    // Init normal scattering matrix array
	    initScatteringMatrixArray(nr_of_scat_theta_tmp);

	    // Fill values of the scattering matrix via interpolation
	    #pragma omp parallel for
	    for(int a = 0; a < int(nr_of_dust_species); a++)
	    {
		if(sizeIndexUsed(a))
		{
		    for(uint inc = 0; inc < nr_of_incident_angles; inc++)
		    {
			for(uint sph = 0; sph < nr_of_scat_phi; sph++)
			{
			    for(uint sth = 0; sth < nr_of_scat_theta_tmp; sth++)
			    {
				for(uint e = 0; e < 16; e++)
				{
				    double sign = CMathFunctions::sgn(elements[e]);
				    int pos = abs(elements[e]);
				    if(pos > 0)
				    {
					for(uint w = 0; w < nr_of_wavelength; w++)
					{
					    sca_mat[a][w][inc][sph][sth](e) =
						sign *
						sca_mat_wl[a][inc][sph][sth][pos - 1].getValue(wavelength_list[w]);
					    scat_theta[a][w][sth] = PI * double(sth) / double(nr_of_scat_theta_tmp - 1);
					}
				    }
				}
				delete[] sca_mat_wl[a][inc][sph][sth];
			    }
			    delete[] sca_mat_wl[a][inc][sph];
			}
			delete[] sca_mat_wl[a][inc];
		    }
		}
		delete[] sca_mat_wl[a];
	    }
	    delete[] sca_mat_wl;

	    // Set that the scattering matrix was successfully read
	    scat_loaded = true;

	    return true;
	    }
    };

}

#endif
