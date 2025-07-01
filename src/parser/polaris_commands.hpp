#ifndef RW_POLARIS_COMMANDS
#define RW_POLARIS_COMMANDS

#include "command_parser.hpp"
#include "../Parameters.hpp"

#include <map>
#include <string_view>

/**
 * HOW-TO...
 * ...add a new POLARIS-Command?
 *   - Define a new function in "polaris_commands.hpp" with the signature
 *
 *   	  t_ret cmd_<your_command>(ParsedLine& line, parameters& param)
 *
 *   - Add a pair to `command_map` at the bottom of "polaris_commands.hpp"
 *     with the structure:
 *
 *	  { "<your_command>"sv, cmd_<your_command> }
 *
 *   - Implement `cmd_<your_command>` in "polaris_commands.cpp"
 *	- `line` is the currently parsed line, all found values
 *	  are conveniently split. Look at the documentation of `ParsedLine`
 *	  for more information
 *	- `param` are the currently processed parameters (one instance per
 *	  <task> or in the <common> block)
 *
 *   - Return-value for `cmd_<your_command>` on succes should be
 *
 *        return {}
 *
 *     On failure you can abort processing entirely by returning
 *
 *        return Message { "Error message" }
 *
 *     Information or Warnings (without aborting processing) can be returned
 *     using
 *
 *        return Message { "Info-Message", Message::Type::Info };
 *
 *     More complex Messages can be generated using `comp_error`:
 *
 *        return Message { comp_error("Expected ", n_params, " parameters") };
 *
 *
 * ...register a new Detector?
 *  TODO: docuementation. For now look at examples.
 */

namespace rewrite {

    using CommandProcessFn = t_ret (*)(ParsedLine&, parameters&);
    using command_map = std::map<std::string_view, CommandProcessFn>;

    t_ret cmd_cmd(ParsedLine& line, parameters& param);
    t_ret cmd_delta0(ParsedLine& line, parameters& param);
    t_ret cmd_larm_f(ParsedLine& line, parameters& param);
    t_ret cmd_plot_list(ParsedLine& line, parameters& param);
    t_ret cmd_phase_function(ParsedLine& line, parameters& param);
    t_ret cmd_star_mass(ParsedLine& line, parameters& param);
    t_ret cmd_opiata_path_emi(ParsedLine& line, parameters& param);
    t_ret cmd_opiata_path_abs(ParsedLine& line, parameters& param);
    t_ret cmd_gas_species(ParsedLine& line, parameters& param);
    t_ret cmd_source_star(ParsedLine& line, parameters& param);
    t_ret cmd_source_starfield(ParsedLine& line, parameters& param);
    t_ret cmd_source_background(ParsedLine& line, parameters& param);
    t_ret cmd_source_laser(ParsedLine& line, parameters& param);
    t_ret cmd_axis1(ParsedLine& line, parameters& param);
    t_ret cmd_axis2(ParsedLine& line, parameters& param);
    t_ret cmd_align(ParsedLine& line, parameters& param);
    t_ret cmd_mu(ParsedLine& line, parameters& param);
    t_ret cmd_xy_min(ParsedLine& line, parameters& param);
    t_ret cmd_xy_max(ParsedLine& line, parameters& param);
    t_ret cmd_xy_steps(ParsedLine& line, parameters& param);
    t_ret cmd_xy_bins(ParsedLine& line, parameters& param);
    t_ret cmd_xy_label(ParsedLine& line, parameters& param);
    t_ret cmd_healpix_orientation(ParsedLine& line, parameters& param);
    t_ret cmd_path_grid(ParsedLine& line, parameters& param);
    t_ret cmd_path_grid_cgs(ParsedLine& line, parameters& param);
    t_ret cmd_sub_dust(ParsedLine& line, parameters& param);
    t_ret cmd_vel_maps(ParsedLine& line, parameters& param);
    t_ret cmd_max_subpixel_lvl(ParsedLine& line, parameters& param);
    t_ret cmd_path_input(ParsedLine& line, parameters& param);
    t_ret cmd_dust_component(ParsedLine& line, parameters& param);
    t_ret cmd_path_out(ParsedLine& line, parameters& param);
    t_ret cmd_nr_plot_points(ParsedLine& line, parameters& param);
    t_ret cmd_nr_plot_vectors(ParsedLine& line, parameters& param);
    t_ret cmd_f_highJ(ParsedLine& line, parameters& param);
    t_ret cmd_Q_ref(ParsedLine& line, parameters& param);
    t_ret cmd_alpha_Q(ParsedLine& line, parameters& param);
    t_ret cmd_R_rayleigh(ParsedLine& line, parameters& param);
    t_ret cmd_f_c(ParsedLine& line, parameters& param);
    t_ret cmd_adj_tgas(ParsedLine& line, parameters& param);
    t_ret cmd_max_plot_lines(ParsedLine& line, parameters& param);
    t_ret cmd_start(ParsedLine& line, parameters& param);
    t_ret cmd_stop(ParsedLine& line, parameters& param);
    t_ret cmd_conv_dens(ParsedLine& line, parameters& param);
    t_ret cmd_conv_len(ParsedLine& line, parameters& param);
    t_ret cmd_conv_mag(ParsedLine& line, parameters& param);
    t_ret cmd_conv_vel(ParsedLine& line, parameters& param);
    t_ret cmd_mass_fraction(ParsedLine& line, parameters& param);
    t_ret cmd_mrw(ParsedLine& line, parameters& param);
    t_ret cmd_pda(ParsedLine& line, parameters& param);
    t_ret cmd_dust_offset(ParsedLine& line, parameters& param);
    t_ret cmd_dust_gas_coupling(ParsedLine& line, parameters& param);
    t_ret cmd_radiation_field(ParsedLine& line, parameters& param);
    t_ret cmd_rt_scattering(ParsedLine& line, parameters& param);
    t_ret cmd_split_dust_emission(ParsedLine& line, parameters& param);
    t_ret cmd_full_dust_temp(ParsedLine& line, parameters& param);
    t_ret cmd_stochastic_heating(ParsedLine& line, parameters& param);
    t_ret cmd_source_dust(ParsedLine& line, parameters& param);
    t_ret cmd_source_isrf(ParsedLine& line, parameters& param);
    t_ret cmd_foreground_extinction(ParsedLine& line, parameters& param);
    t_ret cmd_enfsca(ParsedLine& line, parameters& param);
    t_ret cmd_peel_off(ParsedLine& line, parameters& param);
    t_ret cmd_acceptance_angle(ParsedLine& line, parameters& param);
    t_ret cmd_nr_threads(ParsedLine& line, parameters& param);
    t_ret cmd_vel_is_speed_of_sound(ParsedLine& line, parameters& param);
    t_ret cmd_amira_inp_points(ParsedLine& line, parameters& param);
    t_ret cmd_amira_out_points(ParsedLine& line, parameters& param);
    t_ret cmd_plot_inp_midplanes(ParsedLine& line, parameters& param);
    t_ret cmd_plot_out_midplanes(ParsedLine& line, parameters& param);
    t_ret cmd_write_3d_midplanes(ParsedLine& line, parameters& param);
    t_ret cmd_write_inp_midplanes(ParsedLine& line, parameters& param);
    t_ret cmd_write_out_midplanes(ParsedLine& line, parameters& param);
    t_ret cmd_write_radiation_field(ParsedLine& line, parameters& param);
    t_ret cmd_write_full_radiation_field(ParsedLine& line, parameters& param);
    t_ret cmd_write_g_zero(ParsedLine& line, parameters& param);
    t_ret cmd_write_dust_files(ParsedLine& line, parameters& param);
    t_ret cmd_midplane_zoom(ParsedLine& line, parameters& param);
    t_ret cmd_kepler_star_mass(ParsedLine& line, parameters& param);
    t_ret cmd_turbulent_velocity(ParsedLine& line, parameters& param);
    t_ret cmd_mc_lvl_pop_photons(ParsedLine& line, parameters& param);
    t_ret cmd_mc_lvl_pop_seed(ParsedLine& line, parameters& param);
    t_ret cmd_detector_opiate(ParsedLine& line, parameters& param);
    t_ret cmd_detector_opiate_healpix(ParsedLine& line, parameters& param);
    t_ret cmd_detector_line(ParsedLine& line, parameters& param);
    t_ret cmd_detector_line_healpix(ParsedLine& line, parameters& param);
    t_ret cmd_detector_line_polar(ParsedLine& line, parameters& param);
    t_ret cmd_detector_line_slice(ParsedLine& line, parameters& param);
    t_ret cmd_detector_dust(ParsedLine& line, parameters& param);
    t_ret cmd_detector_dust_healpix(ParsedLine& line, parameters& param);
    t_ret cmd_detector_dust_polar(ParsedLine& line, parameters& param);
    t_ret cmd_detector_dust_slice(ParsedLine& line, parameters& param);
    t_ret cmd_detector_dust_mc(ParsedLine& line, parameters& param);
    t_ret cmd_detector_sync(ParsedLine& line, parameters& param);
    t_ret cmd_detector_sync_slice(ParsedLine& line, parameters& param);
    t_ret cmd_detector_sync_healpix(ParsedLine& line, parameters& param);

    constexpr auto make_cmd_map() {
	using namespace literals;

	return command_map {
	    { "cmd"sv, cmd_cmd },
	    { "delta0"sv, cmd_delta0 },
	    { "larm_f"sv, cmd_larm_f },
	    { "plot_list"sv, cmd_plot_list },
	    { "phase_function"sv, cmd_phase_function },
	    { "star_mass"sv, cmd_star_mass },
	    { "detector_opiate"sv, cmd_detector_opiate },
	    { "detector_opiate_healpix"sv, cmd_detector_opiate_healpix },
	    { "opiata_path_emi"sv, cmd_opiata_path_emi },
	    { "opiata_path_abs"sv, cmd_opiata_path_abs },
	    { "detector_line"sv, cmd_detector_line },
	    { "detector_line_healpix"sv, cmd_detector_line_healpix },
	    { "detector_line_polar"sv, cmd_detector_line_polar },
	    { "detector_line_slice"sv, cmd_detector_line_slice },
	    { "detector_dust"sv, cmd_detector_dust },
	    { "detector_dust_healpix"sv, cmd_detector_dust_healpix },
	    { "detector_dust_polar"sv, cmd_detector_dust_polar },
	    { "detector_dust_slice"sv, cmd_detector_dust_slice },
	    { "detector_dust_mc"sv, cmd_detector_dust_mc },
	    { "detector_sync"sv, cmd_detector_sync },
	    { "detector_sync_slice"sv, cmd_detector_sync_slice },
	    { "detector_sync_healpix"sv, cmd_detector_sync_healpix },
	    { "gas_species"sv, cmd_gas_species },
	    { "source_star"sv, cmd_source_star },
	    { "source_starfield"sv, cmd_source_starfield },
	    { "source_background"sv, cmd_source_background },
	    { "source_laser"sv, cmd_source_laser },
	    { "axis1"sv, cmd_axis1 },
	    { "axis2"sv, cmd_axis2 },
	    { "align"sv, cmd_align },
	    { "mu"sv, cmd_mu },
	    { "xy_min"sv, cmd_xy_min },
	    { "xy_max"sv, cmd_xy_max },
	    { "xy_steps"sv, cmd_xy_steps },
	    { "xy_bins"sv, cmd_xy_bins },
	    { "xy_label"sv, cmd_xy_label },
	    { "healpix_orientation"sv, cmd_healpix_orientation },
	    { "path_grid"sv, cmd_path_grid },
	    { "path_grid_cgs"sv, cmd_path_grid_cgs },
	    { "sub_dust"sv, cmd_sub_dust },
	    { "vel_maps"sv, cmd_vel_maps },
	    { "max_subpixel_lvl"sv, cmd_max_subpixel_lvl },
	    { "path_input"sv, cmd_path_input },
	    { "dust_component"sv, cmd_dust_component },
	    { "path_out"sv, cmd_path_out },
	    { "nr_plot_points"sv, cmd_nr_plot_points },
	    { "nr_plot_vectors"sv, cmd_nr_plot_vectors },
	    { "f_highJ"sv, cmd_f_highJ },
	    { "Q_ref"sv, cmd_Q_ref },
	    { "alpha_Q"sv, cmd_alpha_Q },
	    { "R_rayleigh"sv, cmd_R_rayleigh },
	    { "f_c"sv, cmd_f_c },
	    { "adj_tgas"sv, cmd_adj_tgas },
	    { "max_plot_lines"sv, cmd_max_plot_lines },
	    { "start"sv, cmd_start },
	    { "stop"sv, cmd_stop },
	    { "conv_dens"sv, cmd_conv_dens },
	    { "conv_len"sv, cmd_conv_len },
	    { "conv_mag"sv, cmd_conv_mag },
	    { "conv_vel"sv, cmd_conv_vel },
	    { "mass_fraction"sv, cmd_mass_fraction },
	    { "mrw"sv, cmd_mrw },
	    { "pda"sv, cmd_pda },
	    { "dust_offset"sv, cmd_dust_offset },
	    { "dust_gas_coupling"sv, cmd_dust_gas_coupling },
	    { "radiation_field"sv, cmd_radiation_field },
	    { "rt_scattering"sv, cmd_rt_scattering },
	    { "split_dust_emission"sv, cmd_split_dust_emission },
	    { "full_dust_temp"sv, cmd_full_dust_temp },
	    { "stochastic_heating"sv, cmd_stochastic_heating },
	    { "source_dust"sv, cmd_source_dust },
	    { "source_isrf"sv, cmd_source_isrf },
	    { "foreground_extinction"sv, cmd_foreground_extinction },
	    { "enfsca"sv, cmd_enfsca },
	    { "peel_off"sv, cmd_peel_off },
	    { "acceptance_angle"sv, cmd_acceptance_angle },
	    { "nr_threads"sv, cmd_nr_threads },
	    { "vel_is_speed_of_sound"sv, cmd_vel_is_speed_of_sound },
	    { "amira_inp_points"sv, cmd_amira_inp_points },
	    { "amira_out_points"sv, cmd_amira_out_points },
	    { "plot_inp_midplanes"sv, cmd_plot_inp_midplanes },
	    { "plot_out_midplanes"sv, cmd_plot_out_midplanes },
	    { "write_3d_midplanes"sv, cmd_write_3d_midplanes },
	    { "write_inp_midplanes"sv, cmd_write_inp_midplanes },
	    { "write_out_midplanes"sv, cmd_write_out_midplanes },
	    { "write_radiation_field"sv, cmd_write_radiation_field },
	    { "write_full_radiation_field"sv, cmd_write_full_radiation_field },
	    { "write_g_zero"sv, cmd_write_g_zero },
	    { "write_dust_files"sv, cmd_write_dust_files },
	    { "midplane_zoom"sv, cmd_midplane_zoom },
	    { "kepler_star_mass"sv, cmd_kepler_star_mass },
	    { "turbulent_velocity"sv, cmd_turbulent_velocity },
	    { "mc_lvl_pop_photons"sv, cmd_mc_lvl_pop_photons },
	    { "mc_lvl_pop_seed"sv, cmd_mc_lvl_pop_seed },
	};
    }
}

#endif
