#ifndef RW_ZEEMAN_FILE_LOADER
#define RW_ZEEMAN_FILE_LOADER

#include "basic_loader.hpp"

#include <filesystem>
#include <cstring>

namespace rewrite {

    struct ZeemanFile {
	double*		lande_factor;
	int*		nr_of_sublevel;
	double		gas_species_radius;
	double		nr_zeeman_spectral_lines;
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
	    unsigned int i_trans_zeeman = 0;

	    if (!next_line(file))
		return safe_error( "Unexpected end of file" );

	    if (current_line != stringID)
		return safe_error( "Wrong Zeeman splitting catalog file chosen!" );

	    if (!next_line(file) || !is_or_next<is_number>())
		return safe_error( "Expected number (Zeeman file)" );

	    if (const auto num = get_number(); !num.has_value())
		return safe_error( num.error().message );
	    else result.gas_species_radius = num.value();

	    if (!next_line(file) || !is_or_next<is_number>())
		return safe_error( "Expected number (Zeeman file)" );

	    if (const auto num = get_number(); !num.has_value())
		return safe_error( num.error().message );
	    else result.nr_zeeman_spectral_lines = num.value();

	    while (next_line(file)) {

		unsigned int i_trans = 0;
		if (!is_or_next<is_number>())
		    return safe_error( "Expected number (Zeeman file)" );

		if (const auto num = get_number(); !num.has_value())
		    return safe_error( num.error().message );
		else i_trans = num.value() - 1;

		// TODO: what is is_zeeman_split, why do we need it?
		// TODO: jump if not in nr_of_transitions?
		// TODO: where does nr_of_transitions come from?
		if (0 <= i_trans && i_trans < nr_of_transitions) {
		    i_trans_zeeman = i_trans;
		    trans_is_zeeman_split[i_trans] = true;
		}

		if (!next_line(file) || !is_or_next<is_number>())
		    return safe_error( "Expected number (Zeeman file)" );

		// TODO: where does upper_level come from?
		if (const auto num = get_number(); !num.has_value())
		    return safe_error( num.error().message );
		else result.lande_factor[upper_level[i_trans_zeeman]] = num.value();

		if (!next_line(file) || !is_or_next<is_number>())
		    return safe_error( "Expected number (Zeeman file)" );

		// TODO: where does lower_level come from?
		if (const auto num = get_number(); !num.has_value())
		    return safe_error( num.error().message );
		else result.lande_factor[lower_level[i_trans_zeeman]] = num.value();

		if (!next_line(file) || !is_or_next<is_number>())
		    return safe_error( "Expected number (Zeeman file)" );

		// TODO: allocate nr_of_sublevel
		if (const auto num = get_number(); !num.has_value())
		    return safe_error( num.error().message );
		else result.nr_of_sublevel[upper_level[i_trans_zeeman]] = num.value();

		if (!next_line(file) || !is_or_next<is_number>())
		    return safe_error( "Expected number (Zeeman file)" );

		// Save the number of sublevel per energy level
		if (const auto num = get_number(); !num.has_value())
		    return safe_error( num.error().message );
		else result.nr_of_sublevel[lower_level[i_trans_zeeman]] = num.value();

		// TODO: switch with upper
		// Set local number of sublevel for the involved energy levels
		const size_t nr_of_sublevel_upper = result.nr_of_sublevel[upper_level[i_trans_zeeman]];
		const size_t nr_of_sublevel_lower = result.nr_of_sublevel[lower_level[i_trans_zeeman]];

		// Calculate the numbers of transitions possible for sigma and pi transitions
		const size_t nr_pi_spectral_lines = std::min(nr_of_sublevel_upper, nr_of_sublevel_lower);
		size_t nr_sigma_spectral_lines = nr_of_sublevel_upper - 1;

		if (nr_of_sublevel_upper != nr_of_sublevel_lower)
		    nr_sigma_spectral_lines = std::min(nr_of_sublevel_upper, nr_of_sublevel_lower);

		line_strength_pi.resize(nr_pi_spectral_lines);
		for (size_t i = 0; i < nr_pi_spectral_lines; i++) {
		    if (!next_line(file) || !is_or_next<is_number>())
			return safe_error( "Expected number (Zeeman file)" );

		    if (const auto num = get_number(); !num.has_value())
			return safe_error( num.error().message );
		    else line_strength_pi[i] = num.value();
		}

		line_strength_sigma_p.resize(nr_sigma_spectral_lines);
		for (size_t i = 0; i < nr_sigma_spectral_lines; i++) {
		    if (!next_line(file) || !is_or_next<is_number>())
			return safe_error( "Expected number (Zeeman file)" );

		    if (const auto num = get_number(); !num.has_value())
			return safe_error( num.error().message );
		    else line_strength_sigma_p[i] = num.value();
		}

		line_strength_sigma_m.resize(nr_sigma_spectral_lines);
		for (size_t i = 0; i < nr_sigma_spectral_lines; i++) {
		    if (!next_line(file) || !is_or_next<is_number>())
			return safe_error( "Expected number (Zeeman file)" );

		    if (const auto num = get_number(); !num.has_value())
			return safe_error( num.error().message );
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

            // Get indices to involved energy level
            unsigned int i_lvl_u = upper_level[i_trans_zeeman];
            unsigned int i_lvl_l = lower_level[i_trans_zeeman];

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

    reader.close();

    for(uint i_line = 0; i_line < nr_of_spectral_lines; i_line++)
    {
        uint i_trans = getTransitionFromSpectralLine(i_line);
        
        if(getLandeUpper(i_trans) == 0) // || getLandeLower(i_trans) == 0
        {
            cout << SEP_LINE;
            cout << ERROR_LINE << "For transition number " << uint(i_trans + 1)
                 << " exists no Zeeman splitting data" << endl;
            return false;
        }
        else
        {
            trans_is_zeeman_split[i_trans] = true;
        }
    }

    return true;
}

	    file.close();
	    return {};
	}
    };
}

#endif
