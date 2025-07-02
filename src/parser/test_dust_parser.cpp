#include <cstdlib>
#include <string>
#include <fstream>
#include <vector>
#include <chrono>

#include "../CommandParser.hpp"
#include "../MathSpline.hpp"
#include "dust_parameter_parser.hpp"

using namespace std;

spline *eff_wl_old{nullptr}, *Qtrq_wl_old{nullptr}, *HG_g_factor_wl_old{nullptr}, *HG_g2_factor_wl_old{nullptr}, *HG_g3_factor_wl_old{nullptr};
spline *eff_wl_new{nullptr}, *Qtrq_wl_new{nullptr}, *HG_g_factor_wl_new{nullptr}, *HG_g2_factor_wl_new{nullptr}, *HG_g3_factor_wl_new{nullptr};

uint nr_of_dust_species, nr_of_incident_angles;


bool readDustParameterFile_old(const string& path) {
    // Init variables
    CCommandParser ps;
    unsigned char ru[4] = { '|', '/', '-', '\\' };
    string line;
    vector<double> values, wavelength_list_dustcat;

    // temporary variables for wavelength interpolation
    unsigned int nr_of_wavelength_dustcat;

    // Init text file reader for dust cat file
    fstream reader(path.c_str());

    // Error message if the read does not work
    if(reader.fail())
    {
        cout << ERROR_LINE << "Cannot open dust parameters file:" << endl;
        cout << path << endl;
        return false;
    }

    // Init progress counter
    unsigned int line_counter = 0;
    unsigned int char_counter = 0;
    unsigned int cmd_counter = 0;
    unsigned int eff_counter = 0;

    while(getline(reader, line))
    {
        // Show progress
        // if(line_counter % 500 == 0)
        // {
        //     char_counter++;
        //     cout << "- reading dust parameters file: " << ru[(unsigned int)char_counter % 4] << "             \r";
        // }

        // Format the text file line
        ps.formatLine(line);

        // Increase line counter
        line_counter++;

        // If the line is empty -> skip
        if(line.size() == 0)
            continue;

        if(cmd_counter > 0)
        {
            // Parse the values of the current line
            values = ps.parseValues(line);

            // If no values found -> skip
            if(values.size() == 0)
                continue;
        }

        // Increase the command counter
        cmd_counter++;

        switch(cmd_counter)
        {
            case 1:
                // The first line contains the name of the dust component
                // stringID = line;
                break;

            case 2:
                // The second line needs 8 values
                if(values.size() != 8)
                {
                    cout << ERROR_LINE << "Line " << line_counter << " wrong amount of numbers!" << endl;
                    return false;
                }

                // The number of dust grain sizes
                nr_of_dust_species = (unsigned int)values[0];

                // The number of wavelength used by the dust catalog
                nr_of_wavelength_dustcat = (unsigned int)values[1];

                // The number of incident angles
                nr_of_incident_angles = (unsigned int)values[2];

                // The aspect ratio of minor to major dust grain axes
                // aspect_ratio = values[3];
                //
                // // The material density (only used if no one was set in the command file)
                // if(material_density == 0)
                // {
                //     if(values[4] == 0)
                //     {
                //         printIDs();
                //         cout << ERROR_LINE << "dust bulk mass is zero!" << endl;
                //         return false;
                //     }
                //     material_density = values[4];
                // }

                // The sublimation temperature
                // sub_temp = values[5];
                //
                // // The delta value fot the RAT alignment theory
                // delta_rat = values[6];
                //
                // // The boolean value if the dust catalog is aligned or not
                // if(values[7] == 1)
                //     is_align = true;
                // else
                //     is_align = false;

                // Calculate the GOLD alignment g factor
                // gold_g_factor = 0.5 * (aspect_ratio * aspect_ratio - 1);

                // Init splines for wavelength interpolation of the dust optical properties
                eff_wl_old = new spline[nr_of_dust_species * NR_OF_EFF - 1];
                Qtrq_wl_old = new spline[nr_of_dust_species * nr_of_incident_angles];
                HG_g_factor_wl_old = new spline[nr_of_dust_species * nr_of_incident_angles];
                HG_g2_factor_wl_old = new spline[nr_of_dust_species * nr_of_incident_angles];
                HG_g3_factor_wl_old = new spline[nr_of_dust_species * nr_of_incident_angles];

                break;

            case 3:
                // The second line needs a values per dust grain size
                if(values.size() != nr_of_dust_species)
                {
                    cout << ERROR_LINE << "Line " << line_counter << " wrong amount of effective radii!" << endl;
                    return false;
                }

                // Init arrays for grain size, size distribution, and mass
                // a_eff = new double[nr_of_dust_species];
                // a_eff_squared = new double[nr_of_dust_species];
                // grain_distribution_x_aeff_sq = new double[nr_of_dust_species];
                // grain_size_distribution = new double[nr_of_dust_species];
                // mass = new double[nr_of_dust_species];

                // Calculate the grain size distribution
                // calcSizeDistribution(values, mass);

                // Check if size limits are inside grain sizes and set global ones
                // if(!checkGrainSizeLimits(a_min, a_max))
                //     return false;
                break;

            case 4:
                // The fourth line needs a values per wavelength of the dust grain catalog
                if(values.size() != nr_of_wavelength_dustcat)
                {
                    cout << ERROR_LINE << "Line " << line_counter << " wrong amount of wavelength!" << endl;
                    return false;
                }

                // Init an array for the wavelengths of the dust grain catalog
                wavelength_list_dustcat.resize(nr_of_wavelength_dustcat);

                // Fill the wavelength array
                for(unsigned int w = 0; w < nr_of_wavelength_dustcat; w++)
                    wavelength_list_dustcat[w] = values[w];

                // if(wavelength_list[0] < wavelength_list_dustcat[0] ||
                //     wavelength_list[nr_of_wavelength - 1] > wavelength_list_dustcat[nr_of_wavelength_dustcat - 1])
                // {
                //     cout << WARNING_LINE << "The wavelength range is out of the limits of the catalog. This may cause problems!\n"
                //         << "         wavelength range          : " << wavelength_list[0] << " [m] to "
                //         << wavelength_list[nr_of_wavelength - 1] << " [m]\n"
                //         << "         wavelength range (catalog): " << wavelength_list_dustcat[0] << " [m] to "
                //         << wavelength_list_dustcat[nr_of_wavelength_dustcat - 1] << " [m]" << endl;
                //     if(!IGNORE_WAVELENGTH_RANGE)
                //     {
                //         cout << "         To continue, set 'IGNORE_WAVELENGTH_RANGE' to 'true' in src/Typedefs.h and recompile!" << endl;
                //         return false;
                //     }
                // }

                break;

            default:
                // Init boolean value to check if the lines contain the right values
                bool rec = false;

                // Get the wavelength and grain size indizes
                unsigned int w = int(eff_counter / nr_of_dust_species);
                unsigned int a = eff_counter % nr_of_dust_species;

                // At the first wavelength, resize the splines for the wavelengths
                if(w == 0)
                    for(unsigned int i = 0; i < NR_OF_EFF - 1; i++)
                        eff_wl_old[a * (NR_OF_EFF - 1) + i].resize(nr_of_wavelength_dustcat);

                // Each line contains NR_OF_EFF - 1 plus nr_of_incident_angles values
                if(values.size() == NR_OF_EFF - 1 + nr_of_incident_angles)
                {
                    // the current line contains enough values
                    rec = true;

                    // Set the dust grain optical properties
                    for(unsigned int i = 0; i < NR_OF_EFF - 1; i++)
                        eff_wl_old[a * (NR_OF_EFF - 1) + i].setValue(w, wavelength_list_dustcat[w], values[i]);

                    // For each incident angles, get Qtrq
                    for(unsigned int i_inc = 0; i_inc < nr_of_incident_angles; i_inc++)
                    {
                        // At the first wavelength, resize the splines for the wavelengths
                        if(w == 0)
                        {
                            Qtrq_wl_old[a * nr_of_incident_angles + i_inc].resize(nr_of_wavelength_dustcat);
                            HG_g_factor_wl_old[a * nr_of_incident_angles + i_inc].resize(nr_of_wavelength_dustcat);
                            HG_g2_factor_wl_old[a * nr_of_incident_angles + i_inc].resize(nr_of_wavelength_dustcat);
                            HG_g3_factor_wl_old[a * nr_of_incident_angles + i_inc].resize(nr_of_wavelength_dustcat);
                        }

                        // Set the radiative torque efficiency
                        Qtrq_wl_old[a * nr_of_incident_angles + i_inc].setValue(
                            w, wavelength_list_dustcat[w], values[NR_OF_EFF - 1 + i_inc]);

                        // Set the Henyey-Greenstein g factor
                        HG_g_factor_wl_old[a * nr_of_incident_angles + i_inc].setValue(
                            w,
                            wavelength_list_dustcat[w],
                            0.0);
                        
                        // Set the second Henyey-Greenstein factor to 0
                        HG_g2_factor_wl_old[a * nr_of_incident_angles + i_inc].setValue(
                            w,
                            wavelength_list_dustcat[w],
                            0.0);
                        
                        // Set the third Henyey-Greenstein factor to 1
                        HG_g3_factor_wl_old[a * nr_of_incident_angles + i_inc].setValue(
                            w,
                            wavelength_list_dustcat[w],
                            1.0);
                    }

                    // Increace the counter for the dust grain optical properties
                    eff_counter++;
                }

                // Alternatively, each line contains additional column with g for HG
                if(values.size() == NR_OF_EFF - 1 + 2 * nr_of_incident_angles)
                {
                    // the current line contains enough values
                    rec = true;

                    // Set the dust grain optical properties
                    for(unsigned int i = 0; i < NR_OF_EFF - 1; i++)
                        eff_wl_old[a * (NR_OF_EFF - 1) + i].setValue(w, wavelength_list_dustcat[w], values[i]);

                    // For each incident angles, get Qtrq and HG parameter
                    for(unsigned int i_inc = 0; i_inc < nr_of_incident_angles; i_inc++)
                    {
                        // At the first wavelength, resize the splines for the wavelengths
                        if(w == 0)
                        {
                            Qtrq_wl_old[a * nr_of_incident_angles + i_inc].resize(nr_of_wavelength_dustcat);
                            HG_g_factor_wl_old[a * nr_of_incident_angles + i_inc].resize(nr_of_wavelength_dustcat);
                            HG_g2_factor_wl_old[a * nr_of_incident_angles + i_inc].resize(nr_of_wavelength_dustcat);
                            HG_g3_factor_wl_old[a * nr_of_incident_angles + i_inc].resize(nr_of_wavelength_dustcat);
                        }

                        // Set the radiative torque efficiency
                        Qtrq_wl_old[a * nr_of_incident_angles + i_inc].setValue(
                            w, wavelength_list_dustcat[w], values[NR_OF_EFF - 1 + i_inc]);

                        // Set the Henyey-Greenstein g factor
                        HG_g_factor_wl_old[a * nr_of_incident_angles + i_inc].setValue(
                            w,
                            wavelength_list_dustcat[w],
                            values[NR_OF_EFF - 1 + i_inc + nr_of_incident_angles]);

                        // Set the second Henyey-Greenstein factor
                        HG_g2_factor_wl_old[a * nr_of_incident_angles + i_inc].setValue(
                            w,
                            wavelength_list_dustcat[w],
                            0.0);

                        // Set the third Henyey-Greenstein factor to 0
                        HG_g3_factor_wl_old[a * nr_of_incident_angles + i_inc].setValue(
                            w,
                            wavelength_list_dustcat[w],
                            1.0);
                    }

                    // Increace the counter for the dust grain optical properties
                    eff_counter++;
                }

                // Alternatively, each line contains additional two columns with g and alpha for Draine HG
                if(values.size() == NR_OF_EFF - 1 + 3 * nr_of_incident_angles)
                {
                    // the current line contains enough values
                    rec = true;

                    // Set the dust grain optical properties
                    for(unsigned int i = 0; i < NR_OF_EFF - 1; i++)
                        eff_wl_old[a * (NR_OF_EFF - 1) + i].setValue(w, wavelength_list_dustcat[w], values[i]);

                    // For each incident angles, get Qtrq and HG parameters
                    for(unsigned int i_inc = 0; i_inc < nr_of_incident_angles; i_inc++)
                    {
                        // At the first wavelength, resize the splines for the wavelengths
                        if(w == 0)
                        {
                            Qtrq_wl_old[a * nr_of_incident_angles + i_inc].resize(nr_of_wavelength_dustcat);
                            HG_g_factor_wl_old[a * nr_of_incident_angles + i_inc].resize(nr_of_wavelength_dustcat);
                            HG_g2_factor_wl_old[a * nr_of_incident_angles + i_inc].resize(nr_of_wavelength_dustcat);
                            HG_g3_factor_wl_old[a * nr_of_incident_angles + i_inc].resize(nr_of_wavelength_dustcat);
                        }

                        // Set the radiative torque efficiency
                        Qtrq_wl_old[a * nr_of_incident_angles + i_inc].setValue(
                            w, wavelength_list_dustcat[w], values[NR_OF_EFF - 1 + i_inc]);

                        // Set the Henyey-Greenstein g factor
                        HG_g_factor_wl_old[a * nr_of_incident_angles + i_inc].setValue(
                            w,
                            wavelength_list_dustcat[w],
                            values[NR_OF_EFF - 1 + i_inc + nr_of_incident_angles]);

                        // Set the second Henyey-Greenstein factor
                        HG_g2_factor_wl_old[a * nr_of_incident_angles + i_inc].setValue(
                            w,
                            wavelength_list_dustcat[w],
                            values[NR_OF_EFF - 1 + i_inc + 2 * nr_of_incident_angles]);

                        // Set the third Henyey-Greenstein factor to 1
                        HG_g3_factor_wl_old[a * nr_of_incident_angles + i_inc].setValue(
                            w,
                            wavelength_list_dustcat[w],
                            1.0);
                    }

                    // Increace the counter for the dust grain optical properties
                    eff_counter++;
                }

                // Alternatively, each line contains additional three columns with g1, g2 and weight for TTHG
                if(values.size() == NR_OF_EFF - 1 + 4 * nr_of_incident_angles)
                {
                    // the current line contains enough values
                    rec = true;

                    // Set the dust grain optical properties
                    for(unsigned int i = 0; i < NR_OF_EFF - 1; i++)
                        eff_wl_old[a * (NR_OF_EFF - 1) + i].setValue(w, wavelength_list_dustcat[w], values[i]);

                    // For each incident angles, get Qtrq and HG parameters
                    for(unsigned int i_inc = 0; i_inc < nr_of_incident_angles; i_inc++)
                    {
                        // At the first wavelength, resize the splines for the wavelengths
                        if(w == 0)
                        {
                            Qtrq_wl_old[a * nr_of_incident_angles + i_inc].resize(nr_of_wavelength_dustcat);
                            HG_g_factor_wl_old[a * nr_of_incident_angles + i_inc].resize(nr_of_wavelength_dustcat);
                            HG_g2_factor_wl_old[a * nr_of_incident_angles + i_inc].resize(nr_of_wavelength_dustcat);
                            HG_g3_factor_wl_old[a * nr_of_incident_angles + i_inc].resize(nr_of_wavelength_dustcat);
                        }

                        // Set the radiative torque efficiency
                        Qtrq_wl_old[a * nr_of_incident_angles + i_inc].setValue(
                            w, wavelength_list_dustcat[w], values[NR_OF_EFF - 1 + i_inc]);

                        // Set the Henyey-Greenstein g factor
                        HG_g_factor_wl_old[a * nr_of_incident_angles + i_inc].setValue(
                            w,
                            wavelength_list_dustcat[w],
                            values[NR_OF_EFF - 1 + i_inc + nr_of_incident_angles]);

                        // Set the second Henyey-Greenstein factor
                        HG_g2_factor_wl_old[a * nr_of_incident_angles + i_inc].setValue(
                            w,
                            wavelength_list_dustcat[w],
                            values[NR_OF_EFF - 1 + i_inc + 2 * nr_of_incident_angles]);

                        // Set the third Henyey-Greenstein factor
                        HG_g3_factor_wl_old[a * nr_of_incident_angles + i_inc].setValue(
                            w,
                            wavelength_list_dustcat[w],
                            values[NR_OF_EFF - 1 + i_inc + 3 * nr_of_incident_angles]);
                    }

                    // Increace the counter for the dust grain optical properties
                    eff_counter++;
                }

                // At the last wavelength, activate the splines
                if(w == nr_of_wavelength_dustcat - 1)
                {
                    // For the dust grain optical properties
                    for(unsigned int i = 0; i < NR_OF_EFF - 1; i++)
                        eff_wl_old[a * (NR_OF_EFF - 1) + i].createSpline();

                    // For the Qtrq and HG g factor for each incident angle
                    for(unsigned int i_inc = 0; i_inc < nr_of_incident_angles; i_inc++)
                    {
                        Qtrq_wl_old[a * nr_of_incident_angles + i_inc].createSpline();
                        HG_g_factor_wl_old[a * nr_of_incident_angles + i_inc].createSpline();
                        HG_g2_factor_wl_old[a * nr_of_incident_angles + i_inc].createSpline();
                        HG_g3_factor_wl_old[a * nr_of_incident_angles + i_inc].createSpline();
                    }
                }

                //  If a line was not correct, show error
                if(!rec)
                {
                    cout << "WARING: Wrong amount of values in line " << line_counter << "!" << endl;
                    return false;
                }
                break;
        }
    }

    cout << eff_counter << endl;

    // Close the text file reader for the dust catalog
    reader.close();

    return true;
}

bool readDustParameterFile_new(const string& path) {
    // Get Path to dust parameters file

    rewrite::DustParameterParser	parser;

    if (const auto res = parser.parse_file(path); !res) {
	cout << res.error().message << endl;
	return false;
    }

    auto data = parser.get_result();

    // Init variables
    // CCommandParser ps;
    // unsigned char ru[4] = { '|', '/', '-', '\\' };
    // string line;
    dlist values, wavelength_list_dustcat;

    // temporary variables for wavelength interpolation
    uint nr_of_wavelength_dustcat;

    // Init progress counter
    // uint line_counter = 0;
    // uint char_counter = 0;
    uint eff_counter = 0;

        // Show progress
        // if(line_counter % 500 == 0)
        // {
        //     char_counter++;
        //     printIDs();
        //     cout << "- reading dust parameters file: " << ru[(uint)char_counter % 4] << "             \r";
        // }
	
	//    stringID = data.stringID;
	//
	//    // The number of dust grain sizes
	  size_t  nr_of_dust_species = data.nr_dust_species;
	//
	//    // The number of wavelength used by the dust catalog
	nr_of_wavelength_dustcat = data.nr_wavelengths;
	//
	//    // The number of incident angles
	size_t   nr_of_incident_angles = data.nr_inc_angles;
	//
	//    // The aspect ratio of minor to major dust grain axes
	//    aspect_ratio = data.aspect_ratio;
	//
	//    // The material density (only used if no one was set in the command file)
	//    if(material_density == 0) {
	// // TODO
	// if(data.material_density == 0) {
	//     printIDs();
	//     cout << ERROR_LINE << "dust bulk mass is zero!" << endl;
	//     return false;
	// }
	// material_density = data.material_density;
	//    }
	//
	//    // The sublimation temperature
	//    sub_temp = data.sub_temp;
	//
	//    // The delta value fot the RAT alignment theory
	//    delta_rat = data.delta;
	//
	//    // The boolean value if the dust catalog is aligned or not
	//    is_align = data.align;
	//
	//    // Calculate the GOLD alignment g factor
	//    gold_g_factor = 0.5 * (aspect_ratio * aspect_ratio - 1);

    // Init splines for wavelength interpolation of the dust optical properties
    eff_wl_new = new spline[nr_of_dust_species * NR_OF_EFF-1];
    Qtrq_wl_new = new spline[nr_of_dust_species * nr_of_incident_angles];
    HG_g_factor_wl_new = new spline[nr_of_dust_species * nr_of_incident_angles];
    HG_g2_factor_wl_new = new spline[nr_of_dust_species * nr_of_incident_angles];
    HG_g3_factor_wl_new = new spline[nr_of_dust_species * nr_of_incident_angles];

    wavelength_list_dustcat.resize(data.nr_wavelengths);
    std::copy(data.wavelengths, data.wavelengths + data.nr_wavelengths, wavelength_list_dustcat.begin());

    // Get the wavelength and grain size indizes
    // uint w = int(eff_counter / nr_of_dust_species);
    // uint a = eff_counter % nr_of_dust_species;

    // At the first wavelength, resize the splines for the wavelengths
    for (size_t a = 0; a < nr_of_dust_species; a++) {
	for (size_t i = 0; i < 7; i++) {
	    eff_wl_new[a * 7 + i].resizeShared(
		data.nr_wavelengths,
		data.wavelengths,
		data.eff_wl + a * data.nr_wavelengths * 7 + i * data.nr_wavelengths);
	    eff_wl_new[a * 7 + i].createSpline();
	}

        for(size_t i = 0; i < nr_of_incident_angles; i++) {
	    const auto pos_mod = a * data.nr_wavelengths * nr_of_incident_angles + i * data.nr_wavelengths;
	
            Qtrq_wl_new[a * nr_of_incident_angles + i]
		.resizeShared(nr_of_wavelength_dustcat, data.wavelengths, data.Qtrq_wl + pos_mod);
	    Qtrq_wl_new[a * nr_of_incident_angles + i].createSpline();

            HG_g_factor_wl_new[a * nr_of_incident_angles + i]
		.resizeShared(nr_of_wavelength_dustcat, data.wavelengths, data.HG_g_factor_wl + pos_mod);
	    HG_g_factor_wl_new[a * nr_of_incident_angles + i].createSpline();

            HG_g2_factor_wl_new[a * nr_of_incident_angles + i]
		.resizeShared(nr_of_wavelength_dustcat, data.wavelengths, data.HG_g2_factor_wl + pos_mod);
	    HG_g2_factor_wl_new[a * nr_of_incident_angles + i].createSpline();

            HG_g3_factor_wl_new[a * nr_of_incident_angles + i]
		.resizeShared(nr_of_wavelength_dustcat, data.wavelengths, data.HG_g3_factor_wl + pos_mod);
	    HG_g3_factor_wl_new[a * nr_of_incident_angles + i].createSpline();
        }
    }

    // At the last wavelength, activate the splines
    // For the dust grain optical properties
    // TODO: Better to have these directly above after assignemnt?
	//   for(uint i = 0; i < nr_of_dust_species * NR_OF_EFF - 1; i++)
	// eff_wl_new[i].createSpline();
	//
	//    // For the Qtrq and HG g factor for each incident angle
	//    for(size_t i = 0; i < nr_of_dust_species * nr_of_incident_angles; i++) {
	// Qtrq_wl_new[i].createSpline();
	// HG_g_factor_wl_new[i].createSpline();
	// HG_g2_factor_wl_new[i].createSpline();
	// HG_g3_factor_wl_new[i].createSpline();
	//    }
    return true;
}

int main() {
    string path = "input/dust_cs/silicate_oblate.dat";


    cout << "Begin old load..." << endl;
    const auto begin2 = chrono::system_clock::now();
    if(!readDustParameterFile_old(path))
	cout << "Could not parse old" << endl;
    const auto res_old = std::chrono::duration_cast<std::chrono::milliseconds>(
	chrono::system_clock::now() - begin2);
    cout << "Duration old: " << res_old << endl;

    cout << endl << endl;

    cout << "Begin new load..." << endl;
    const auto begin = chrono::system_clock::now();
    if(const auto c = readDustParameterFile_new(path); !c)
	cout << "Could not parse new" << endl;
    const auto res_new = chrono::duration_cast<chrono::milliseconds>(
	chrono::system_clock::now() - begin);
    cout << "Duration new: " << res_new << endl;

    cout << endl << "Diff: " << res_old - res_new << endl << endl;

    for(uint i = 0; i < nr_of_dust_species * (NR_OF_EFF - 1); i++)
	if (!eff_wl_old[i].compare(eff_wl_new[i]))
	cout << "EFF_WL[" << setw(3) << i << "] " << endl;

    cout << endl << endl;

    for(size_t i = 0; i < nr_of_dust_species * nr_of_incident_angles; i++) {
	if (!Qtrq_wl_old[i].compare(Qtrq_wl_new[i]))
	    cout << "Qtrq[" << setw(3) << i << "]: " << boolalpha << Qtrq_wl_old[i].compare(Qtrq_wl_new[i]) << endl;
	if (!HG_g_factor_wl_old[i].compare(HG_g_factor_wl_new[i]))
	    cout << "HG[" << setw(3) << i << "]: " << boolalpha << HG_g_factor_wl_old[i].compare(HG_g_factor_wl_new[i]) << endl;
	if (!HG_g2_factor_wl_old[i].compare(HG_g2_factor_wl_new[i]))
	    cout << "HG2[" << setw(3) << i << "]: " << boolalpha << HG_g2_factor_wl_old[i].compare(HG_g2_factor_wl_new[i]) << endl;
	if (!HG_g3_factor_wl_old[i].compare(HG_g3_factor_wl_new[i]))
	    cout << "HG3[" << setw(3) << i << "]: " << boolalpha << HG_g3_factor_wl_old[i].compare(HG_g3_factor_wl_new[i]) << endl;
    }

    return 0;
}
