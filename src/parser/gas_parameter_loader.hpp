#ifndef RW_GAS_PARAMETER_LOADER
#define RW_GAS_PARAMETER_LOADER

#include <cstddef>
#include <filesystem>
#include <string>
#include <cstring>
#include <cmath>
#include <algorithm>

#include "basic_loader.hpp"

#include "../Typedefs.hpp"

namespace rewrite {

    struct GasParameterFile {
	std::string	stringID;
	double		molecular_weight;
	
	size_t		nr_of_energy_level;
	double*		energy_level;
	double*		g_level;
	double*		quantum_numbers;
	int*		nr_of_sublevel;

	size_t		nr_of_transitions;
	int*		upper_level;
	int*		lower_level;
	double*		trans_freq;
	double*		trans_inner_energy;
	double**	trans_einstA;
	double**	trans_einstB_ul;
	double**	trans_einstB_lu;
	bool*		trans_is_zeeman_split;
	std::vector<unsigned int> unique_spectral_lines;

	size_t		nr_of_col_partner;
	int*		nr_of_col_transition;
	int*		nr_of_col_temp;
	int*		orientation_H2;
	double**	collision_temp;
	unsigned int**	col_upper;
	unsigned int**	col_lower;
	double***	col_matrix;
    };

    class GasParameterLoader: public BasicLoader {
	GasParameterFile result;

	void cleanup() override {
	    delete[] result.energy_level;
	    delete[] result.g_level;
	    delete[] result.quantum_numbers;
	    delete[] result.nr_of_sublevel;

	    delete[] result.upper_level;
	    delete[] result.lower_level;
	    delete[] result.trans_freq;
	    delete[] result.trans_inner_energy;
	    delete[] result.trans_is_zeeman_split;
	    for (auto i=0; i < result.nr_of_transitions; i++) {
		delete[] result.trans_einstA[i];
		delete[] result.trans_einstB_ul[i];
		delete[] result.trans_einstB_lu[i];
	    }
	    delete[] result.trans_einstA;
	    delete[] result.trans_einstB_ul;
	    delete[] result.trans_einstB_lu;

	    delete[] result.nr_of_col_transition;
	    delete[] result.nr_of_col_temp;
	    delete[] result.orientation_H2;
	    for (auto i=0; i < result.nr_of_col_partner; i++) {
		delete[] result.collision_temp[i];
		delete[] result.col_upper[i];
		delete[] result.col_lower[i];

		for (auto j=0; j < result.nr_of_transitions; j++)
		    delete[] result.col_matrix[i][j];
		delete[] result.col_matrix[i];
	    }
	    delete[] result.collision_temp;
	    delete[] result.col_upper;
	    delete[] result.col_lower;
	    delete[] result.col_matrix;
	}

    public:
	GasParameterFile& get_result() { return result; }

	auto parse_file(
	    const std::filesystem::path& path,
	    const std::vector<int>& spectral_lines)
	    -> std::expected<void, Message>
	{
	    file.open(path);

	    if (file.fail())
		return safe_error( 
		    std::format( "Could not open gas_species catalog: {}", path.string() ));

	    if (!next_line(file))
		return safe_error( "Unexpected end of file" );

	    result.stringID = current_line;

	    if (const auto res = rdline_number<double>("Molecular Weight"); !res.has_value())
		return std::unexpected{ res.error() };
	    else result.molecular_weight = res.value();

	    if (const auto res = rdline_number<size_t>("Number of Energy Level"); !res.has_value())
		return std::unexpected{ res.error() };
	    else result.nr_of_energy_level = res.value();

	    const size_t nel = result.nr_of_energy_level;
	    result.energy_level = new double[nel];
	    result.g_level = new double[nel];
	    result.quantum_numbers = new double[nel];
	    result.nr_of_sublevel = new int[nel];
	    std::memset(result.nr_of_sublevel, 1, nel * sizeof(int));

	    for (size_t e = 0; e < nel; e++) {
		if (const auto res = rdline_values(4, "<none>, energy level, g level and quantum number"); !res.has_value())
		    return res;

		// TODO: what happens to values[0]?

		// ENERGIES(cm^-1)
		result.energy_level[e] = values[1];

		// WEIGHT
		result.g_level[e] = values[2];

		// Quantum numbers for corresponding energy level
		result.quantum_numbers[e] = values[3];
	    }

	    if (const auto res = rdline_number<size_t>("Number of Transitions"); !res.has_value())
		return std::unexpected{ res.error() };
	    else result.nr_of_transitions = res.value();

	    const size_t nt = result.nr_of_transitions;

	    result.upper_level = new int[nt];
	    result.lower_level = new int[nt];
	    result.trans_freq = new double[nt];
	    result.trans_inner_energy = new double[nt];

	    result.trans_einstA = new double*[nt];
	    result.trans_einstB_ul = new double*[nt];
	    result.trans_einstB_lu = new double*[nt];

            // Only one entry, but more if Zeeman sublevels are treated
	    for (size_t i = 0; i < nt; i++) {
		result.trans_einstA[i] = new double;
		result.trans_einstB_ul[i] = new double;
		result.trans_einstB_lu[i] = new double;
	    }

            // Init list if a transition is zeeman split
	    // TODO: this is horrible...
	    result.trans_is_zeeman_split = new bool[nt];
	    std::memset(result.trans_is_zeeman_split, 0, nt * sizeof(bool));

	    for (size_t t = 0; t < nt; t++) {
		if (const auto res = rdline_values(6, "Spectral Line, Upper Level, Lower Level, Einstein A, Transition Frequency, Inner Energy"); !res.has_value())
		    return res;

		const int spectral_line = values[0] - 1;
		if (std::ranges::find(spectral_lines, spectral_line) != spectral_lines.end())
		    result.unique_spectral_lines.push_back(spectral_line);

		// UP (as index starting with 0)
		const int ul = values[1] - 1;
		result.upper_level[t] = ul;

		// LOW (as index starting with 0)
		const int ll = values[2] - 1;
		result.lower_level[t] = ll;

		// EINSTEIN A(s^-1)
		result.trans_einstA[t][0] = values[3];

		// FREQ(GHz -> Hz)
		const double tf = values[4] * 1e9;
		result.trans_freq[t] = tf;

		// E_u(K)
		result.trans_inner_energy[t] = values[5];

		// EINSTEIN B_ul(s^-1)
		const double bul = values[3]
		    * std::pow(con_c / tf, 2.0)
		    / (2.0 * con_h * tf);
		result.trans_einstB_ul[t][0] = bul;

		// EINSTEIN B_lu(s^-1)
		result.trans_einstB_lu[t][0] =
		    result.g_level[ul]
		    / result.g_level[ll] * bul;
	    }

	    if (const auto res = rdline_number<size_t>("Number of Col Partner"); !res.has_value())
		return std::unexpected{ res.error() };
	    else result.nr_of_col_partner = res.value();

	    const size_t nc = result.nr_of_col_partner;
	    
            result.nr_of_col_transition = new int[nc];
            result.nr_of_col_temp = new int[nc];
            result.orientation_H2 = new int[nc];

            result.collision_temp = new double*[nc];
            result.col_upper = new unsigned int*[nc];
            result.col_lower = new unsigned int*[nc];

            result.col_matrix = new double**[nc];

	    for (size_t icp = 0; icp < result.nr_of_col_partner; icp++) {

		if (const auto res = rdline_number<int>("Orientation H2 Collision Partner"); !res.has_value())
		    return std::unexpected{ res.error() };
		else result.orientation_H2[icp] = res.value();

		if (const auto res = rdline_number<int>("Number of Collision Transition"); !res.has_value())
		    return std::unexpected{ res.error() };
		else result.nr_of_col_transition[icp] = res.value();

		const int ctrans = result.nr_of_col_transition[icp];

		result.col_upper[icp] = new unsigned int[ctrans];
		result.col_lower[icp] = new unsigned int[ctrans];
		result.col_matrix[icp] = new double*[ctrans];

		if (const auto res = rdline_number<int>("Number of Col Temp"); !res.has_value())
		    return std::unexpected{ res.error() };
		else result.nr_of_col_temp[icp] = res.value();

		const size_t ctemp = result.nr_of_col_temp[icp];

		result.collision_temp[icp] = new double[ctemp];

		if (const auto res = rdline_values(ctemp, "Col Temps"); !res.has_value())
		    return res;

		std::copy_n(values.begin(), ctemp, result.collision_temp[icp]);

		for (size_t ict = 0; ict < ctrans; ict++) {
		    if (const auto res = rdline_values(ctemp + 3, "<none>, Col Upper, Col Lower, Col Matrix..."); !res.has_value())
			return res;

		    // TODO: what happens to values[0]?

		    result.col_upper[icp][ict] = values[1] - 1;
		    result.col_lower[icp][ict] = values[2] - 1;

		    result.col_matrix[icp][ict] = new double[ctemp];

		    for(uint i = 0; i < ctemp; i++)
			result.col_matrix[icp][ict][i] = values[3 + i] * 1e-6;
		}
	    }

	    // Close calorimetry file reader
	    file.close();

	    return {};

	}
    };


}
#endif
