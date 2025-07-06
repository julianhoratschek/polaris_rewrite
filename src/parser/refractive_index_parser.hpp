#ifndef RW_REFRACTIVE_INDEX_PARSER
#define RW_REFRACTIVE_INDEX_PARSER

#include "basic_parser.hpp"

#include "../MathSpline.hpp"
#include "../MathFunctions.hpp"

#include <cstddef>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>
#include <expected>

namespace rewrite {

    struct RefractiveIndexFile {
	std::string	stringID;
	size_t			nr_dust_species;
	size_t			nr_wavelengths;
	size_t			nr_inc_angles;
	double			aspect_ratio;
	double			material_density;
	double			sub_temp;
	double			delta;
	bool			align;

    };

    class RefractiveIndexFileParser: public BasicParser {
	RefractiveIndexFile	result;
	std::ifstream		file;

	std::unexpected<Message> safe_error(const std::string& msg) {
	    file.close();
	    return std::unexpected { Message {
		msg
	    }};
	}
    
    public:

	auto parse_file(const std::filesystem::path& path,
	    const double a_min_mixture, const double a_max_mixture)
		-> std::expected<void, Message> {

	    // Init variables
	    std::vector<double>		values_aeff;
	    std::array<double, 7> 	values;

	    // temporary variables for wavelength interpolation
	    spline refractive_index_real, refractive_index_imag;
	    unsigned int nr_of_wavelength_dustcat;
	    std::size_t		column;

	    // Get min and max dust grain size
	    // double a_min = param.getSizeMin(dust_component_choice);
	    // double a_max = param.getSizeMax(dust_component_choice);

	    // Set number of grain sizes for Mie theory (1 if only one grain size is used)
	    if(a_min_mixture == a_max_mixture)
		result.nr_dust_species = 1;
	    else
		result.nr_dust_species = MIE_NR_DUST_SIZE;
	    values_aeff.resize(result.nr_dust_species);

	    // Init dust grain sizes
	    CMathFunctions::LogList(a_min_mixture, a_max_mixture, values_aeff, 10);

	    file.open(path);

	    // Error message if the read does not work
	    if (file.fail())
		return safe_error("Cannot open dust refractive index file");

	    if (!next_line(file))
		return safe_error( "Unexpected ed of file" );

	    result.stringID = current_line;

	    if (!next_line(file))
		return safe_error( "Unexpected End of File" );

	    while (is_or_next<is_number>() && column < 7)
		if (const auto num = get_number();
		    not num.has_value()) return safe_error( num.error().message );
		else values[column++] = num.value();

	    if (column != 7)
		// TODO: Correct line
		return safe_error( "Expected 7 Values in line 2" );

	    result.nr_wavelengths = values[0];
            result.nr_inc_angles = 1; // For non-spherical: (uint) values[1];
	    result.aspect_ratio = 1; // For non-spherical: values[2];
	    result.material_density = values[3];
	    result.sub_temp = values[4];
	    result.delta = 1;
	    result.align = values[6];

	    // Init splines for wavelength interpolation of the dust optical properties
	    refractive_index_real.resize(nr_of_wavelength_dustcat);
	    refractive_index_imag.resize(nr_of_wavelength_dustcat);
	    // Init progress counter
	    // uint line_counter = 0;
	    // uint char_counter = 0;
	    // uint cmd_counter = 0;
	    // uint wl_counter = 0;

    // while(getline(reader, line))
    // {
        // Show progress
        // if(line_counter % 500 == 0)
        // {
        //     char_counter++;
        //     printIDs();
        //     cout << "- reading dust parameters file: " << ru[(uint)char_counter % 4] << "             \r";
        // }

        // Format the text file line
        // ps.formatLine(line);

        // Increase line counter
        // line_counter++;

        // If the line is empty -> skip

        //
        // if(cmd_counter > 0)
        // {
        //     // Parse the values of the current line
        //     values = ps.parseValues(line);
        //
        //     // If no values found -> skip
        //     if(values.size() == 0)
        //         continue;
        // }
        //
        // Increase the command counter
        // cmd_counter++;

                // Calculate the GOLD alignment g factor
                // gold_g_factor = 0.5 * (aspect_ratio * aspect_ratio - 1);
                //
                // // Init splines for wavelength interpolation of the dust optical properties
                // refractive_index_real.resize(nr_of_wavelength_dustcat);
                // refractive_index_imag.resize(nr_of_wavelength_dustcat);
                //
                // // Set size parameters
                // a_eff = new double[nr_of_dust_species];
                // a_eff_squared = new double[nr_of_dust_species];
                // grain_distribution_x_aeff_sq = new double[nr_of_dust_species];
                // grain_size_distribution = new double[nr_of_dust_species];
                // mass = new double[nr_of_dust_species];
                //
                // // Calculate the grain size distribution
                // calcSizeDistribution(values_aeff, mass);
                //
                // // Check if size limits are inside grain sizes and set global ones
                // if(!checkGrainSizeLimits(a_min, a_max))
                //     return false;
                // break;
                //
            default:
                // Init boolean value to check if the lines contain the right values
                bool rec = false;

                // Each line contains NR_OF_EFF - 1 plus 2 times nr_of_incident_angles
                // values
                if(values.size() == 3)
                {
                    // he current line contains enough values
                    rec = true;

                    // Set the dust grain optical properties
                    refractive_index_real.setValue(wl_counter, values[0], values[1]);
                    refractive_index_imag.setValue(wl_counter, values[0], values[2]);

                    // Increace the counter for the dust grain optical properties
                    wl_counter++;
                }

                //  If a line was not correct, show error
                if(!rec)
                {
                    cout << "WARING: Wrong amount of values in line " << line_counter << "!" << endl;
                    return false;
                }
                break;
        }
    // }

    if(wavelength_list[0] < refractive_index_real.getX(0) ||
        wavelength_list[nr_of_wavelength - 1] > refractive_index_real.getX(nr_of_wavelength_dustcat - 1))
    {
        cout << WARNING_LINE << "The wavelength range is out of the limits of the catalog. This may cause problems!\n"
            << "         wavelength range          : " << wavelength_list[0] << " [m] to "
            << wavelength_list[nr_of_wavelength - 1] << " [m]\n"
            << "         wavelength range (catalog): " << refractive_index_real.getX(0) << " [m] to "
            << refractive_index_real.getX(nr_of_wavelength_dustcat - 1) << " [m]" << endl;
        if(!IGNORE_WAVELENGTH_RANGE)
        {
            cout << "         To continue, set 'IGNORE_WAVELENGTH_RANGE' to 'true' in src/Typedefs.h and recompile!" << endl;
            return false;
        }
    }

    // At the last wavelength, activate the splines
    refractive_index_real.createSpline();
    refractive_index_imag.createSpline();

    // If not a line per combination of grain size and wavelength was found in the
    // catalog, show error
    if(wl_counter != nr_of_wavelength_dustcat)
    {
        cout << stringID << endl;
        cout << ERROR_LINE << "Wrong amount of efficiencies in file!" << endl;
        return false;
    }

    // Close the text file reader for the dust catalog
    reader.close();

    // Init pointer arrays for dust optical properties
    Qext1 = new double *[nr_of_dust_species];
    Qext2 = new double *[nr_of_dust_species];
    Qabs1 = new double *[nr_of_dust_species];
    Qabs2 = new double *[nr_of_dust_species];
    Qsca1 = new double *[nr_of_dust_species];
    Qsca2 = new double *[nr_of_dust_species];
    Qcirc = new double *[nr_of_dust_species];
    HGg = new double *[nr_of_dust_species];
    HGg2 = new double *[nr_of_dust_species];
    HGg3 = new double *[nr_of_dust_species];

    CextMean = new double *[nr_of_dust_species];
    CabsMean = new double *[nr_of_dust_species];
    CscaMean = new double *[nr_of_dust_species];

    for(uint a = 0; a < nr_of_dust_species; a++)
    {
        Qext1[a] = new double[nr_of_wavelength];
        Qext2[a] = new double[nr_of_wavelength];
        Qabs1[a] = new double[nr_of_wavelength];
        Qabs2[a] = new double[nr_of_wavelength];
        Qsca1[a] = new double[nr_of_wavelength];
        Qsca2[a] = new double[nr_of_wavelength];
        Qcirc[a] = new double[nr_of_wavelength];
        HGg[a] = new double[nr_of_wavelength];
        HGg2[a] = new double[nr_of_wavelength];
        HGg3[a] = new double[nr_of_wavelength];

        CextMean[a] = new double [nr_of_wavelength];
        fill(CextMean[a], CextMean[a] + nr_of_wavelength, 0);
        CabsMean[a] = new double [nr_of_wavelength];
        fill(CabsMean[a], CabsMean[a] + nr_of_wavelength, 0);
        CscaMean[a] = new double [nr_of_wavelength];
        fill(CscaMean[a], CscaMean[a] + nr_of_wavelength, 0);
    }

    // Init splines for incident angle interpolation of Qtrq and parameters for Henyey-Greenstein phase function
    Qtrq = new spline[nr_of_dust_species * nr_of_wavelength];
    HG_g_factor = new spline[nr_of_dust_species * nr_of_wavelength];
    HG_g2_factor = new spline[nr_of_dust_species * nr_of_wavelength];
    HG_g3_factor = new spline[nr_of_dust_species * nr_of_wavelength];

    // Set variables for scattering via Mie theory
    nr_of_scat_mat_elements = 4;

    // --- Phi angle
    nr_of_scat_phi = 1;

    // --- Theta angle
    uint nr_of_scat_theta_start = 2 * NANG - 1;

    // Init normal scattering matrix array
    initNrOfScatThetaArray();
    initScatThetaArray();
    initScatteringMatrixArray();

    // Init counter and percentage to show progress
    ullong per_counter = 0;
    float last_percentage = 0;

    // Init error check
    bool error = false;

    // Init error in refractive index data check
    bool nk_error = false;

    double max_rel_diff = 0.0;

    // Init maximum counter value
    uint max_counter = nr_of_dust_species * nr_of_wavelength;

    if(USE_SPLINE_FOR_REFRACTIVE_INDEX){
        cout << WARNING_LINE << "USE_SPLINE_FOR_REFRACTIVE_INDEX was set to true in 'Typedefs.h'!" << endl;
        cout << "(When the wavelength list in the input .nk-file has gaps that are too large compared with the change of the complex refractive index n+ik, using Splines can cause large errors and negative values of n or k.)" << endl;
    }

    #pragma omp parallel for schedule(dynamic) collapse(2)
    for(int a = 0; a < int(nr_of_dust_species); a++)
    {
        for(int w = 0; w < int(nr_of_wavelength); w++)
        {
            // Skip everything else if error was found
            if(error)
                continue;

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
                    cout << "- calculating optical properties: " << percentage
                         << " [%]                      \r";
                    last_percentage = percentage;
                }
            }

            // Resize the splines of Qtrq and HG g factor for each wavelength
            Qtrq[w * nr_of_dust_species + a].resize(nr_of_incident_angles);
            HG_g_factor[w * nr_of_dust_species + a].resize(nr_of_incident_angles);
            HG_g2_factor[w * nr_of_dust_species + a].resize(nr_of_incident_angles);
            HG_g3_factor[w * nr_of_dust_species + a].resize(nr_of_incident_angles);

            if(sizeIndexUsed(a))
            {
                // Init variables and pointer arrays
                double *S11_start, *S12_start, *S33_start, *S34_start;
                S11_start = new double[nr_of_scat_theta_start];
                S12_start = new double[nr_of_scat_theta_start];
                S33_start = new double[nr_of_scat_theta_start];
                S34_start = new double[nr_of_scat_theta_start];

                dlist scat_angle_start(nr_of_scat_theta_start);
                for(uint i_scat_ang=0; i_scat_ang < nr_of_scat_theta_start; i_scat_ang++)
                {
                    scat_angle_start[i_scat_ang] = i_scat_ang * PI/(nr_of_scat_theta_start-1);
                }

                // Set size index and refractive index as complex number
                double x = 2.0 * PI * a_eff[a] / wavelength_list[w];
                dcomplex refractive_index;
#if BENCHMARK == PINTE
                refractive_index = dcomplex(refractive_index_real.getValue(wavelength_list[w], LOGLINEAR),
                                            refractive_index_imag.getValue(wavelength_list[w], LOGLINEAR));
#else
                if(USE_SPLINE_FOR_REFRACTIVE_INDEX)
                {
                    refractive_index = dcomplex(refractive_index_real.getValue(wavelength_list[w], LOGLINEAR),
                                                refractive_index_imag.getValue(wavelength_list[w], LOGLINEAR));
                }
                else
                {
                    refractive_index = dcomplex(refractive_index_real.getLinearValue(wavelength_list[w]),
                                                refractive_index_imag.getLinearValue(wavelength_list[w]));
                }
#endif
                if(refractive_index.imag() < 0)
                {
                    error = true;
                    nk_error = true;
                    continue;
                }

                if(refractive_index.real() < 0)
                {
                    error = true;
                    nk_error = true;
                    continue;
                }

                // Calculate Mie-scattering
                if(!CMathFunctions::calcWVMie(x,
                                              scat_angle_start,
                                              refractive_index,
                                              Qext1[a][w],
                                              Qabs1[a][w],
                                              Qsca1[a][w],
                                              HGg[a][w],
                                              S11_start,
                                              S12_start,
                                              S33_start,
                                              S34_start))
                    error = true;

                dlist S11_final(1), S12_final(1), S33_final(1), S34_final(1), scat_angle_final(1);
                S11_final[0] = S11_start[0];
                S12_final[0] = S12_start[0];
                S33_final[0] = S33_start[0];
                S34_final[0] = S34_start[0];
                scat_angle_final[0] = scat_angle_start[0];

                // set default HG values if optical properties are calcualted with Mie-scattering
                HGg2[a][w] = 0.0;
                HGg3[a][w] = 1.0;

                double current_S11_rel_diff;

                for(uint i_scat_ang=0; i_scat_ang < nr_of_scat_theta_start -1; i_scat_ang++)
                {
                    dlist S11_tmp(1), S12_tmp(1), S33_tmp(1), S34_tmp(1);
                    dlist scat_angle_tmp(2);

                    S11_tmp[0] = S11_start[i_scat_ang+1];
                    S12_tmp[0] = S12_start[i_scat_ang+1];
                    S33_tmp[0] = S33_start[i_scat_ang+1];
                    S34_tmp[0] = S34_start[i_scat_ang+1];
                    scat_angle_tmp[0] = scat_angle_start[i_scat_ang+1];
                    scat_angle_tmp[1] = 0.5 * (scat_angle_final.back() + scat_angle_start[i_scat_ang+1]);

                    while(true)
                    {
                        current_S11_rel_diff = abs( S11_tmp.back() - S11_final.back() ) / max( S11_tmp.back(), S11_final.back() );

                        // Subdivide scattering angles only if size parameter x is not too large.
                        // Large x lead to extrem forward scattering and the subdivision criterion will
                        // get almost impossible to achive for small scattering angles.
                        // The limit of x=100 is somewhat arbitrary.
                        while(x < 100.0 && current_S11_rel_diff > MAX_MIE_SCA_REL_DIFF)
                        {
                            double *pointer_s11_tmp, *pointer_s12_tmp, *pointer_s33_tmp, *pointer_s34_tmp;
                            pointer_s11_tmp = new double[1];
                            pointer_s12_tmp = new double[1];
                            pointer_s33_tmp = new double[1];
                            pointer_s34_tmp = new double[1];

                            dlist scat_angle_calc(1);
                            scat_angle_calc[0] = scat_angle_tmp.back();

                            if(!CMathFunctions::calcWVMie(x,
                                                          scat_angle_calc,
                                                          refractive_index,
                                                          Qext1[a][w],
                                                          Qabs1[a][w],
                                                          Qsca1[a][w],
                                                          HGg[a][w],
                                                          pointer_s11_tmp,
                                                          pointer_s12_tmp,
                                                          pointer_s33_tmp,
                                                          pointer_s34_tmp))
                                error = true;

                            S11_tmp.push_back(pointer_s11_tmp[0]);
                            S12_tmp.push_back(pointer_s12_tmp[0]);
                            S33_tmp.push_back(pointer_s33_tmp[0]);
                            S34_tmp.push_back(pointer_s34_tmp[0]);
                            scat_angle_tmp.push_back( 0.5 * (scat_angle_final.back() + scat_angle_tmp.back()) );

                            delete[] pointer_s11_tmp;
                            delete[] pointer_s12_tmp;
                            delete[] pointer_s33_tmp;
                            delete[] pointer_s34_tmp;

                            current_S11_rel_diff = abs( S11_tmp.back() - S11_final.back() ) / max( S11_tmp.back(), S11_final.back() );
                        }
                        scat_angle_tmp.pop_back();

                        S11_final.push_back(S11_tmp.back());
                        S12_final.push_back(S12_tmp.back());
                        S33_final.push_back(S33_tmp.back());
                        S34_final.push_back(S34_tmp.back());
                        scat_angle_final.push_back(scat_angle_tmp.back());

                        S11_tmp.pop_back();
                        S12_tmp.pop_back();
                        S33_tmp.pop_back();
                        S34_tmp.pop_back();
                        scat_angle_tmp.pop_back();

                        if(S11_tmp.size() < 1)
                            break;
                        else
                            scat_angle_tmp.push_back( 0.5 * (scat_angle_final.back() + scat_angle_tmp.back()) );
                    }
                    S11_tmp.clear();
                    S12_tmp.clear();
                    S33_tmp.clear();
                    S34_tmp.clear();
                    scat_angle_tmp.clear();
                }
                scat_angle_start.clear();
                delete[] S11_start;
                delete[] S12_start;
                delete[] S33_start;
                delete[] S34_start;

                uint nr_of_scat_theta_final = scat_angle_final.size();

                // Set missing Efficiencies for other axis
                Qext2[a][w] = Qext1[a][w];
                Qabs2[a][w] = Qabs1[a][w];
                Qsca2[a][w] = Qsca1[a][w];
                Qcirc[a][w] = 0;

                if(scat_theta[a][w] != 0){
                    delete[] scat_theta[a][w];
                }

                double diff_tmp;
                for(uint sth = 1; sth < nr_of_scat_theta_final; sth++)
                {
                    diff_tmp = abs(S11_final[sth-1] - S11_final[sth]) / max(S11_final[sth-1], S11_final[sth]);
                    max_rel_diff = max(diff_tmp, max_rel_diff);
                }

                scat_theta[a][w] = new double[nr_of_scat_theta_final];

                for(uint inc = 0; inc < nr_of_incident_angles; inc++)
                {
                    sca_mat[a][w][inc] = new Matrix2D*[nr_of_scat_phi];
                    for(uint sph = 0; sph < nr_of_scat_phi; sph++)
                    {
                        sca_mat[a][w][inc][sph] = new Matrix2D[nr_of_scat_theta_final];
                        for(uint sth = 0; sth < nr_of_scat_theta_final; sth++)
                        {
                            sca_mat[a][w][inc][sph][sth].resize(4, 4);

                            sca_mat[a][w][inc][sph][sth](0, 0) = S11_final[sth]; // S11
                            sca_mat[a][w][inc][sph][sth](1, 1) = S11_final[sth]; // S22

                            sca_mat[a][w][inc][sph][sth](0, 1) = S12_final[sth]; // S12
                            sca_mat[a][w][inc][sph][sth](1, 0) = S12_final[sth]; // S21

                            sca_mat[a][w][inc][sph][sth](2, 2) = S33_final[sth]; // S33
                            sca_mat[a][w][inc][sph][sth](3, 3) = S33_final[sth]; // S44

                            sca_mat[a][w][inc][sph][sth](2, 3) = S34_final[sth]; // S34
                            sca_mat[a][w][inc][sph][sth](3, 2) = -S34_final[sth]; // S43

                            scat_theta[a][w][sth] = scat_angle_final[sth];
                        }
                    }
                }

                S11_final.clear();
                S12_final.clear();
                S33_final.clear();
                S34_final.clear();

                nr_of_scat_theta[a][w] = nr_of_scat_theta_final;
            }
            else
            {
                Qext1[a][w] = 0;
                Qext2[a][w] = 0;
                Qabs1[a][w] = 0;
                Qabs2[a][w] = 0;
                Qsca1[a][w] = 0;
                Qsca2[a][w] = 0;
                Qcirc[a][w] = 0;
                HGg[a][w] = 0;
                HGg2[a][w] = 0;
                HGg3[a][w] = 1;
            }

            // Activate the splines of Qtrq and HG g factor
            Qtrq[w * nr_of_dust_species + a].createSpline();
            HG_g_factor[w * nr_of_dust_species + a].createSpline();
            HG_g2_factor[w * nr_of_dust_species + a].createSpline();
            HG_g3_factor[w * nr_of_dust_species + a].createSpline();

            CextMean[a][w] = PI * a_eff_squared[a] * (2.0 * Qext1[a][w] + Qext2[a][w]) / 3.0;
            CabsMean[a][w] = PI * a_eff_squared[a] * (2.0 * Qabs1[a][w] + Qabs2[a][w]) / 3.0;
            CscaMean[a][w] = PI * a_eff_squared[a] * (2.0 * Qsca1[a][w] + Qsca2[a][w]) / 3.0;
        } // end of wavelength loop
    } // end of grain size loop

    // Set that the scattering matrix was successfully read
    scat_loaded = true;

    if(nk_error)
    {
        cout << ERROR_LINE << "Either the real or the complex part of the refractive index is negative." << endl;
    }

    if(error)
    {
        cout << ERROR_LINE << "Problem with optical properties calculation" << endl;
        return false;
    }

    printIDs();
    cout << "- calculating optical properties: done          " << endl;

    if(max_rel_diff > 0.5) // arbitrary limit
    {
        cout << WARNING_LINE << "Number of scattering angles might be too low (max rel diff = " << max_rel_diff << ")." << endl;
        cout << "  If required, increase 'NANG' or decrease 'MAX_MIE_SCA_REL_DIFF' (for x < 100) in src/Typedefs.h and recompile!" << endl;
    }

    return true;
}
	}

    };

}

#endif

