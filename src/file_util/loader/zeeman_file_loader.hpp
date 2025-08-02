#ifndef RW_ZEEMAN_FILE_LOADER
#define RW_ZEEMAN_FILE_LOADER

#include "basic_loader.hpp"
#include "gas_parameter_loader.hpp"

#include <cstddef>
#include <filesystem>
#include <cstring>
#include <string>

namespace rewrite {

    // TODO: do we need those?
#ifndef TRANS_SIGMA_P
    constexpr auto TRANS_SIGMA_P = 1;
#endif
#ifndef TRANS_PI
    constexpr auto TRANS_PI = 0;
#endif
#ifndef TRANS_SIGMA_M
    constexpr auto TRANS_SIGMA_M = -1;
#endif

    struct ZeemanFile {
	double*		lande_factor;
	double		gas_species_radius;
	size_t		nr_zeeman_spectral_lines;

	void cleanup()
	{
	    delete[] lande_factor;
	}
    };

    class ZeemanFileLoader: public BasicLoader<ZeemanFile> {
	void realloc_trans_einst(double*& trans_einst, const size_t new_size)
	{
	    const double tmp_einst = trans_einst[0];
	    delete[] trans_einst;
	    trans_einst = new double[new_size];
	    trans_einst[0] = tmp_einst;
	}

    public:
	// TODO: Mutable method!!!
	// gas_param.trans_is_teeman_split
	// gas_param.trans_einst...
	// gas_param.nr_of_sublevel
	std::expected<void, Message> parse_file(
	    const std::filesystem::path& path, GasParameterFile& gas_param)
	{
	    file.open(path);

	    if (file.fail())
		return safe_error(
		    std::format(
			"Cannot open Zeeman splitting catalog: {}", path.string() ));

	    result.lande_factor = new double[gas_param.nr_of_energy_level];
	    std::memset(result.lande_factor, 0, gas_param.nr_of_energy_level * sizeof(double));

	    // Init variables
	    std::vector<double> line_strength_pi, line_strength_sigma_p, line_strength_sigma_m;

	    if (!next_line(file))
		return safe_error( "Unexpected end of file" );

	    if (current_line != gas_param.stringID)
		return safe_error(
		    std::format( "Incorrect Zeeman file (<{}>, expected <{}>)",
			    current_line, gas_param.stringID) );

	    if (const auto num = rdline_number<double>("Gas species Radius");
		!num) return std::unexpected { num.error() };
	    else result.gas_species_radius = num.value();

	    if (const auto num = rdline_number<size_t>("Nr of Zeeman spectral lines");
		!num) return std::unexpected { num.error() };
	    else result.nr_zeeman_spectral_lines = num.value();

	    while (next_line(file)) {
		size_t i_trans;

		if (const auto num = rdline_number<size_t>("Transition Index");
		    !num) return std::unexpected{ num.error() };
		else i_trans = num.value() - 1;

		// TODO: what is is_zeeman_split, why do we need it?
		// TODO: jump if not in nr_of_transitions?
		// TODO THis should probably be a warning: Old colde just overwrites last transition, if i_trans is not in range
		if (0 <= i_trans && i_trans < gas_param.nr_of_transitions)
		    gas_param.trans_is_zeeman_split[i_trans] = true;

		const auto ul = gas_param.upper_level[i_trans],
			   ll = gas_param.lower_level[i_trans];

		if (const auto num = rdline_number<double>("Lande Factor Upper level");
		    !num) return std::unexpected { num.error() };
		else result.lande_factor[ul] = num.value();

		if (const auto num = rdline_number<double>("Lande Factor Lower level");
		    !num) return std::unexpected { num.error() };
		else result.lande_factor[ll] = num.value();

		// TODO: where is nr of dublevel allocated?
		if (const auto num = rdline_number<int>("Number of sublevel upper level");
		    !num) return std::unexpected { num.error() };
		else gas_param.nr_of_sublevel[ul] = num.value();

		if (const auto num = rdline_number<int>("Number of sublevel lower level");
		    !num) return std::unexpected { num.error() };
		else gas_param.nr_of_sublevel[ll] = num.value();

		// Set local number of sublevel for the involved energy levels
		const auto nr_of_sublevel_upper = gas_param.nr_of_sublevel[ul];
		const auto nr_of_sublevel_lower = gas_param.nr_of_sublevel[ll];

		// Calculate the numbers of transitions possible for sigma and pi transitions
		const auto nr_pi_spectral_lines = std::min(
		    nr_of_sublevel_upper, nr_of_sublevel_lower);

		const size_t nr_sigma_spectral_lines =
		    nr_of_sublevel_lower == nr_of_sublevel_upper ?
		    nr_of_sublevel_upper - 1 : nr_pi_spectral_lines;

		if (const auto res = rdlines_vector(
		    nr_pi_spectral_lines, "Line Strength Pi", line_strength_pi);
		    !res) return res;

		if (const auto res = rdlines_vector(
		    nr_sigma_spectral_lines, "Line Strength Sigma P", line_strength_sigma_p);
		    !res) return res;

		if (const auto res = rdlines_vector(
		    nr_sigma_spectral_lines, "Line Strength Sigma M", line_strength_sigma_m);
		    !res) return res;

		// Get maximum number of transitions between sublevel
		const size_t nr_sublevel_trans = nr_of_sublevel_upper * nr_of_sublevel_lower;

		// Save einstein coefficients of major level and extend pointer array for
		// the Zeeman transition
		// Take also into account that the sublevels are treated separately
		// const double tmp_einst_A = gas_param.trans_einstA[i_trans][0];
		// delete[] gas_param.trans_einstA[i_trans];
		// gas_param.trans_einstA[i_trans] = new double[nr_sublevel_trans + 1];
		// gas_param.trans_einstA[i_trans][0] = tmp_einst_A;

		realloc_trans_einst(gas_param.trans_einstA[i_trans], nr_sublevel_trans + 1);
		realloc_trans_einst(gas_param.trans_einstB_lu[i_trans], nr_sublevel_trans + 1);
		realloc_trans_einst(gas_param.trans_einstB_ul[i_trans], nr_sublevel_trans + 1);

		// -1 = TRANS_SIGMA_M, 0 = TRANS_PI, 1 = TRANS_SIGMA_P
		std::vector<double>::iterator line_strength_ptr[3] = {
		    line_strength_sigma_m.begin(),
		    line_strength_pi.begin(),
		    line_strength_sigma_p.begin() };

		// Factor 2/3 or 1/3 comes from normalization in the Larsson paper
		// See Deguchi & Watson 1984 as well!
		constexpr float factor[3] = { 2.0 / 3.0, 1.0 / 3.0, 2.0 / 3.0 };

		// Calculate the contribution of each allowed transition between Zeeman sublevels
		for (size_t i_sublvl_u = 0; i_sublvl_u < nr_of_sublevel_upper; i_sublvl_u++) {
		    // Calculate the quantum number of the upper energy level
		    const float sublvl_u = -static_cast<float>((nr_of_sublevel_upper - 1) / 2.0) + i_sublvl_u;

		    for (size_t i_sublvl_l = 0; i_sublvl_l < nr_of_sublevel_lower; i_sublvl_l++) {
			// Calculate the quantum number of the lower energy level
			const float sublvl_l = -static_cast<float>((nr_of_sublevel_lower - 1) / 2.0) + i_sublvl_l;
			const int trans_type = static_cast<int>(sublvl_l - sublvl_u) + 1;

			double line_strength;

			if (0 <= trans_type && trans_type < 2) {
			    line_strength = *line_strength_ptr[trans_type] * factor[trans_type];
			    ++line_strength_ptr[trans_type];
			}
			// Forbidden line
			else 
			    line_strength = 0;

			const auto i_sublvl = i_sublvl_u * nr_of_sublevel_lower + i_sublvl_l + 1;

			// Set the einstein coefficients for the sublevels
			gas_param.trans_einstA[i_trans][i_sublvl] =
			    gas_param.trans_einstA[i_trans][0] * line_strength * nr_of_sublevel_upper;

			gas_param.trans_einstB_ul[i_trans][i_sublvl] =
			    gas_param.trans_einstB_ul[i_trans][0] * line_strength * nr_of_sublevel_upper;

			gas_param.trans_einstB_lu[i_trans][i_sublvl] =
			    gas_param.trans_einstB_lu[i_trans][0] * line_strength * nr_of_sublevel_lower;
		    }
		}
	    }

	    file.close();
	    return {};
	}
    };
}

#endif
