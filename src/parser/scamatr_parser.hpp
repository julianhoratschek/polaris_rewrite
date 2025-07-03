#ifndef RW_SCAMATR_PARSER
#define RW_SCAMATR_PARSER

#include "../Typedefs.hpp"

#include "basic_parser.hpp"

#include <expected>
#include <filesystem>
#include <fstream>
#include <vector>

namespace rewrite {
    
    class ScaMatrParser: public BasicParser {
	public:
	    auto parse_file(const std::filesystem::path& path)
		-> std::expected<void, Message> {

		bool disable_mie_scattering = true;
		const auto inf_path = path.parent_path() / (path.stem().string() + "scat.inf");
		const size_t nr_of_scat_theta_tmp = 2 * NANG - 1;
		size_t column = 0;
		std::vector<double>	values(5);

		std::ifstream inf_reader(inf_path);

		// Error message if the read does not work
		if(inf_reader.fail())
		    return std::unexpected { Message {
		    "Cannot open scattering matrix info file:"
		} };

		// The first line needs 5 values
		if (!get_next_line(inf_file))
		    return;

		for (column = 0; is_or_next<is_number>() && column < 5; column++) {
		    if (const auto num = get_number();
			not num.has_value()) return;
		    values[column] = num.value();
		}

		if (column != 5)
		    return std::unexpected{};

		// The number of dust grain sizes
		if(values[0] != nr_of_dust_species)
		    return std::unexpected { Message {
			"Number of dust species does not match the number in the dust parameters file!"
		    } };

		// The number of wavelength used by the dust catalog
		if(values[1] != nr_of_wavelength_dustcat) 
		    return std::unexpected { Message {
			"Number of wavelength does not match the number in the dust parameters file!" 
		    } };

		// The number of incident angles
		if(values[2] != nr_of_incident_angles) 
		    return std::unexpected { Message {
			"Number of incident angles does not match the number in the dust parameters file!"
		    } };

		// The number of phi angles (outgoing radiation)
		nr_of_scat_phi = static_cast<unsigned int>(values[3]);

		// The number of theta angles (outgoing radiation)
		nr_of_scat_theta_tmp = static_cast<unsigned int>(values[4]);

		if (!get_next_line())
		    return;

		if (const auto num = get_number();
		    not num.has_value()) return "Wrong amount of scattering matrix elements";
		else
		    nr_of_scat_mat_elements = num.value();

		if (!get_next_line(inf_file))
		    return;




		    case 3:
			// The third line needs 16 values
			if(values.size() != 16)
			{
			    cout << ERROR_LINE << "Wrong amount of matrix elements in:" << endl;
			    cout << inf_filename.c_str() << " line 3!" << endl;
			    return false;
			}

			// The relation which scattering matrix entry is used at which position in
			// the 4x4 matrix
			for(uint i = 0; i < 16; i++)
			{
			    elements[i] = int(values[i]);
			    if(elements[i] != 0)
				disable_mie_scattering = false;
			}
			break;

	    // Close the file reader
	    inf_reader.close();

	    // If scattering matrix is empty, disable mie scattering for the corresponding dust
	    // component
	    if(disable_mie_scattering)
	    {
		phID = PH_HG;
		return true;
	    }

	    // Init counter and percentage to show progress
	    ullong per_counter = 0;
	    float last_percentage = 0;

	    // Init maximum counter value
	    uint max_counter = nr_of_incident_angles * nr_of_dust_species * nr_of_scat_phi;

	    // First init of the scattering matrix interp
	    interp ***** sca_mat_wl = new interp ****[nr_of_dust_species];

	    #pragma omp parallel for
	    for(int a = 0; a < int(nr_of_dust_species); a++)
	    {
		// Second init of the scattering matrix interp
		sca_mat_wl[a] = new interp ***[nr_of_incident_angles];
		for(uint inc = 0; inc < nr_of_incident_angles; inc++)
		{
		    // Third init of the scattering matrix interp
		    sca_mat_wl[a][inc] = new interp **[nr_of_scat_phi];
		    for(uint sph = 0; sph < nr_of_scat_phi; sph++)
		    {
			// Increase counter used to show progress
			per_counter++;

			// Calculate percentage of total progress per source
			float percentage = 100.0 * float(per_counter) / float(max_counter);

			// Show only new percentage number if it changed
			if((percentage - last_percentage) > PERCENTAGE_STEP)
			{
			    #pragma omp critical
			    {
				printIDs();
				cout << "- allocating memory: " << percentage << " [%]                      \r";
				last_percentage = percentage;
			    }
			}

			// Fourth init of the scattering matrix interp
			sca_mat_wl[a][inc][sph] = new interp *[nr_of_scat_theta_tmp];
			for(uint sth = 0; sth < nr_of_scat_theta_tmp; sth++)
			{
			    // Fifth init of the scattering matrix interp
			    sca_mat_wl[a][inc][sph][sth] = new interp[nr_of_scat_mat_elements];

			    // Resize the linear for each scattering matrix element
			    for(uint mat = 0; mat < nr_of_scat_mat_elements; mat++)
				sca_mat_wl[a][inc][sph][sth][mat].resize(nr_of_wavelength_dustcat);
			}
		    }
		}
	    }

	    // Set counter and percentage to show progress
	    per_counter = 0;
	    last_percentage = 0;

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
		ifstream bin_reader(bin_filename.c_str(), ios::in | ios::binary);

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
		    // Increase counter used to show progress
		    per_counter++;

		    // Calculate percentage of total progress per source
		    float percentage = 100.0 * float(per_counter) / float(max_counter);

		    // Show only new percentage number if it changed
		    if((percentage - last_percentage) > PERCENTAGE_STEP)
		    {
			#pragma omp critical
			{
			    printIDs();
			    cout << "- loading matrices: " << percentage << " [%]                      \r";
			    last_percentage = percentage;
			}
		    }

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
