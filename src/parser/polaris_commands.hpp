#include <utility>
#include <string>
#include <array>

namespace rewrite {
    /*
     * When adding an option:
     * 	- Add option name to CommandFileOption
     * 	- Add option name as string to option_from_string
     * 	- Setup in file ....
     */

    enum class CommandFileOption {
	cmd, delta0, larm_f, plot_list, phase_function, star_mass, detector_opiate, detector_opiate_healpix, opiata_path_emi,
	opiata_path_abs, detector_line, detector_line_healpix, detector_line_polar, detector_line_slice, detector_dust,
	detector_dust_healpix, detector_dust_polar, detector_dust_slice, detector_dust_mc, detector_sync, detector_sync_slice,
	detector_sync_healpix, gas_species, source_star, source_starfield, source_background, source_laser, axis1, asix2, align,
	mu, xy_min, xy_max, xy_steps, xy_bins, xy_label, healpix_orientation, path_grid, path_grid_cgs, sub_dust, vel_maps, max_subpixel_lvl,
	path_input, dust_component, path_out, nr_plot_points, nr_plot_vectors, f_highJ,
	// opiata_param_path,
	// opiate_data_path, // e???
	Q_ref, alpha_Q, R_rayleigh, f_c, adj_tgas, max_plot_lines, start, stop, conv_dens, conv_len, conv_mag, conv_vel, mass_fraction,
	mrw, pda, dust_offset, dust_gas_coupling, radiation_field, rt_scattering, split_dust_emission, full_dust_temp, stochastic_heating,
	source_dust, source_isrf, foreground_extinction, enfsca, peel_off, acceptance_angle, nr_threads, vel_is_speed_of_sound,
	amira_inp_points, amira_out_points, plot_inp_midplanes, plot_out_midplanes, write_3d_midplanes, write_inp_midplanes,
	write_out_midplanes, write_radiation_field, write_full_radiation_field, write_g_zero, write_dust_files, midplane_zoom,
	kepler_star_mass, turbulent_velocity, mc_lvl_pop_photons, mc_lvl_pop_seed,

	// HACK: This should always remain last, as it is used to determine
	// the length of the command option list
	Invalid
    };

    /**
     * Returns an unsigned integer representation of cmd as type CommandFileOption
     * @param cmd String command
     * @returns unsigned integer as type CommandFileOption
     */
    constexpr CommandFileOption option_from_string(const std::string& cmd) {
	auto	options = std::array<
	    std::string, std::to_underlying(CommandFileOption::Invalid)> {

	    "cmd", "delta0", "larm_f", "plot_list", "phase_function", "star_mass", "detector_opiate", "detector_opiate_healpix",
	    "opiata_path_emi", "opiata_path_abs", "detector_line", "detector_line_healpix", "detector_line_polar", "detector_line_slice",
	    "detector_dust", "detector_dust_healpix", "detector_dust_polar", "detector_dust_slice", "detector_dust_mc", "detector_sync",
	    "detector_sync_slice", "detector_sync_healpix", "gas_species", "source_star", "source_starfield", "source_background",
	    "source_laser", "axis1", "asix2", "align", "mu", "xy_min", "xy_max", "xy_steps", "xy_bins", "xy_label", "healpix_orientation",
	    "path_grid", "path_grid_cgs", "sub_dust", "vel_maps", "max_subpixel_lvl", "path_input", "dust_component",
	    "path_out", "nr_plot_points", "nr_plot_vectors", "f_highJ",
	    // "opiata_param_path",
	    // "opiate_data_path", // e???
	    "Q_ref", "alpha_Q", "R_rayleigh", "f_c", "adj_tgas", "max_plot_lines", "start", "stop", "conv_dens", "conv_len", "conv_mag",
	    "conv_vel", "mass_fraction", "mrw", "pda", "dust_offset", "dust_gas_coupling", "radiation_field", "rt_scattering",
	    "split_dust_emission", "full_dust_temp", "stochastic_heating", "source_dust", "source_isrf", "foreground_extinction",
	    "enfsca", "peel_off", "acceptance_angle", "nr_threads", "vel_is_speed_of_sound", "amira_inp_points", "amira_out_points",
	    "plot_inp_midplanes", "plot_out_midplanes", "write_3d_midplanes", "write_inp_midplanes", "write_out_midplanes",
	    "write_radiation_field", "write_full_radiation_field", "write_g_zero", "write_dust_files", "midplane_zoom", "kepler_star_mass",
	    "turbulent_velocity", "mc_lvl_pop_photons", "mc_lvl_pop_seed"
	};

	const auto f = std::ranges::find(options, cmd);
	if (f == options.end())
	    return CommandFileOption::Invalid;
	return static_cast<CommandFileOption>(std::distance(options.begin(), f));
    }

}
