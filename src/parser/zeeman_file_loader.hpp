#ifndef RW_ZEEMAN_FILE_LOADER
#define RW_ZEEMAN_FILE_LOADER

#include "basic_loader.hpp"

#include <cstddef>
#include <filesystem>
#include <cstring>
#include <string>

namespace rewrite {

    struct ZeemanFile {
	std::string	stringID;
	double*		lande_factor;
	int*		nr_of_sublevel;
	double		gas_species_radius;
	size_t		nr_zeeman_spectral_lines;
    };

    class ZeemanFileLoader: public BasicLoader {
	ZeemanFile 	result;

    protected:
	void cleanup() override
	{

	}

    public:
	ZeemanFile& get_result() { return result; }

	std::expected<void, Message> parse_file(
	    const std::filesystem::path& path,
	    const size_t nr_of_energy_level,
	    const size_t nr_of_transitions,
	    const std::string& stringID,
	    std::vector<int>& lower_level,
	    std::vector<int>& upper_level,
	    bool* trans_is_zeeman_split)
	{
	    file.open(path);

	    if (file.fail())
		return safe_error(
		    std::format(
			"Cannot open Zeeman splitting catalog: {}", path.string() ));

	    result.lande_factor = new double[nr_of_energy_level];
	    std::memset(result.lande_factor, 0, nr_of_energy_level * sizeof(double));

	    // Init variables
	    std::vector<double> line_strength_pi, line_strength_sigma_p, line_strength_sigma_m;

	    if (!next_line(file))
		return safe_error( "Unexpected end of file" );

	    result.stringID = current_line;

	    if (const auto num = rdline_number<double>("Gas species Radius"); !num.has_value())
		return std::unexpected{ num.error() };
	    else result.gas_species_radius = num.value();

	    if (const auto num = rdline_number<size_t>("Nr of Zeeman spectral lines"); !num.has_value())
		return std::unexpected{ num.error() };
	    else result.nr_zeeman_spectral_lines = num.value();

	    while (next_line(file)) {
		size_t i_trans = 0;

		if (const auto num = rdline_number<size_t>("Transition Index"); !num.has_value())
		    return std::unexpected{ num.error() };
		else i_trans = num.value() - 1;

		// TODO: what is is_zeeman_split, why do we need it?
		// TODO: jump if not in nr_of_transitions?
		// TODO THis should probably be a warning: Old colde just overwrites last transition, if i_trans is not in range
		if (0 <= i_trans && i_trans < nr_of_transitions)
		    trans_is_zeeman_split[i_trans] = true;

		const auto 	ul = upper_level[i_trans],
				ll = lower_level[i_trans];

		if (const auto num = rdline_number<double>("Lande Factor Upper level"); !num.has_value())
		    return std::unexpected{ num.error() };
		else result.lande_factor[ul] = num.value();

		if (const auto num = rdline_number<double>("Lande Factor Lower level"); !num.has_value())
		    return std::unexpected{ num.error() };
		else result.lande_factor[ll] = num.value();

		// TODO: where is nr of dublevel allocated?
		if (const auto num = rdline_number<int>("Number of sublevel upper level"); !num.has_value())
		    return std::unexpected{ num.error() };
		else result.nr_of_sublevel[ul] = num.value();

		if (const auto num = rdline_number<int>("Number of sublevel lower level"); !num.has_value())
		    return std::unexpected{ num.error() };
		else result.nr_of_sublevel[ll] = num.value();

		// TODO: switch with upper
		// Set local number of sublevel for the involved energy levels
		const auto nr_of_sublevel_upper = result.nr_of_sublevel[ul];
		const auto nr_of_sublevel_lower = result.nr_of_sublevel[ll];

		// Calculate the numbers of transitions possible for sigma and pi transitions
		const auto nr_pi_spectral_lines = std::min(nr_of_sublevel_upper, nr_of_sublevel_lower);
		const auto nr_sigma_spectral_lines = nr_of_sublevel_lower == nr_of_sublevel_upper ? nr_of_sublevel_upper - 1 : nr_pi_spectral_lines;

		line_strength_pi.resize(nr_pi_spectral_lines);
		for (size_t i = 0; i < nr_pi_spectral_lines; i++) {
		    if (const auto num = rdline_number<double>("Line strength pi"); !num.has_value())
			return std::unexpected{ num.error() };
		    else line_strength_pi[i] = num.value();
		}

		line_strength_sigma_p.resize(nr_sigma_spectral_lines);
		for (size_t i = 0; i < nr_sigma_spectral_lines; i++) {
		    if (const auto num = rdline_number<double>("Line strength sigma p"); !num.has_value())
			return std::unexpected{ num.error() };
		    else line_strength_sigma_p[i] = num.value();
		}

		line_strength_sigma_m.resize(nr_sigma_spectral_lines);
		for (size_t i = 0; i < nr_sigma_spectral_lines; i++) {
		    if (const auto num = rdline_number<double>("Line strength sigma m"); !num.has_value())
			return std::unexpected{ num.error() };
		    else line_strength_sigma_m[i] = num.value();
		}

	    // TODO: do this at the very end? It only affects einstein trans and only the last of those
	    {
            // Init indices
            unsigned int i_pi = 0, i_sigma_p = 0, i_sigma_m = 0;

            // Init variables
            double line_strength;

            // Get maximum number of transitions between sublevel
            unsigned int nr_sublevel_trans = getNrOfTransBetweenSublevels(i_trans_zeeman);

	    // TODO: can't we just count up and do all at once???
            // Save einstein coefficients of major level and extend pointer array for
            // the Zeeman transition
            // Take also into account that the sublevels are treated separately
            // double tmp_einst_A = trans_einstA[i_trans_zeeman][0];
            // delete[] trans_einstA[i_trans_zeeman];
            // trans_einstA[i_trans_zeeman] = new double[nr_sublevel_trans + 1];
            //
            // double tmp_einst_Bul = trans_einstB_ul[i_trans_zeeman][0];
            // delete[] trans_einstB_ul[i_trans_zeeman];
            // trans_einstB_ul[i_trans_zeeman] = new double[nr_sublevel_trans + 1];
            //
            // double tmp_einst_Blu = trans_einst_Blu[i_trans_zeeman][0];
            // delete[] trans_einstB_lu[i_trans_zeeman];
            // trans_einstB_lu[i_trans_zeeman] = new double[nr_sublevel_trans + 1];
            //
            // for(size_t ie = 0; ie < nr_sublevel_trans + 1; ie++) {
            //     trans_einstA[i_trans_zeeman][ie] = 0;
            //     trans_einstB_ul[i_trans_zeeman][ie] = 0;
            //     trans_einstB_lu[i_trans_zeeman][ie] = 0;
            // }
            //
            // trans_einstA[i_trans_zeeman][0] = tmp_einst_A;
            // trans_einstB_ul[i_trans_zeeman][0] = tmp_einst_Bul;
            // trans_einstB_lu[i_trans_zeeman][0] = tmp_einst_Blu;

            // Calculate the contribution of each allowed transition between Zeeman sublevels
            for (size_t i_sublvl_u = 0; i_sublvl_u < nr_of_sublevel[i_lvl_u]; i_sublvl_u++) {
                // Calculate the quantum number of the upper energy level
                float sublvl_u = -getMaxM(i_lvl_u) + i_sublvl_u;

                for (size_t i_sublvl_l = 0; i_sublvl_l < nr_of_sublevel[i_lvl_l]; i_sublvl_l++) {
                    // Calculate the quantum number of the lower energy level
                    float sublvl_l = -getMaxM(i_lvl_l) + i_sublvl_l;
                    unsigned int i_sublvl = getSublevelIndex(i_trans_zeeman, i_sublvl_u, i_sublvl_l);


                    switch(int(sublvl_l - sublvl_u)) {

                        // Factor 2/3 or 1/3 comes from normalization in the Larsson paper
                        // See Deguchi & Watson 1984 as well!
                        case TRANS_SIGMA_P:
                            // Get the relative line strength from Zeeman file
                            line_strength = line_strength_sigma_p[i_sigma_p] * (2.0 / 3.0);

                            // Increase the sigma_+ counter to circle through the line strengths
                            i_sigma_p++;
                            break;

                        case TRANS_PI:
                            // Get the relative line strength from Zeeman file
                            line_strength = line_strength_pi[i_pi] * (1.0 / 3.0);

                            // Increase the pi counter to circle through the line strengths
                            i_pi++;
                            break;

                        case TRANS_SIGMA_M:
                            // Get the relative line strength from Zeeman file
                            line_strength = line_strength_sigma_m[i_sigma_m] * (2.0 / 3.0);

                            // Increase the sigma_- counter to circle through the line strengths
                            i_sigma_m++;
                            break;

                        default:
                            // Forbidden line
                            line_strength = 0;
                            break;
                    }

                    // Set the einstein coefficients for the sublevels
                    trans_einstA[i_trans_zeeman][i_sublvl + 1] =
                        tmp_einst_A * line_strength * nr_of_sublevel[i_lvl_u];
                    trans_einstB_ul[i_trans_zeeman][i_sublvl + 1] =
                        tmp_einst_Bul * line_strength * nr_of_sublevel[i_lvl_u];
                    trans_einstB_lu[i_trans_zeeman][i_sublvl + 1] = tmp_einst_Blu * line_strength * nr_of_sublevel[i_lvl_l];
                }
            }
	    }

        else if(cmd_counter == 9 + nr_pi_spectral_lines + 2 * nr_sigma_spectral_lines)
        {
            if(values.size() != 1)
            {
                cout << ERROR_LINE << "Line " << line_counter << " wrong amount of numbers (Zeeman file)!" << endl;
                return false;
            }
            for(uint i_trans = 0; i_trans < nr_of_transitions; i_trans++)
            {
                if(i_trans == int(values[0] - 1))
                {
                    // Set current zeeman transition index
                    i_trans_zeeman = int(values[0] - 1);
                    trans_is_zeeman_split[i_trans] = true;
                    break;
                }
            }

            cmd_counter = 4;
        }


	    file.close();
	    return {};
	}
    };
}

#endif
