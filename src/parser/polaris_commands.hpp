#ifndef RW_POLARIS_COMMANDS
#define RW_POLARIS_COMMANDS

#include "command_parser.hpp"
#include "../Parameters.hpp"

#include <map>
#include <expected>

namespace rewrite {

    using t_ret = std::expected<void, std::string>;

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


    using CommandProcessFn = t_ret (*)(ParsedLine&, parameters&);
    constexpr auto make_cmd_map() {
	return std::map<std::string, CommandProcessFn> {
	    { "cmd", cmd_cmd },
	    { "delta0", cmd_delta0 },
	    { "larm_f", cmd_larm_f },
	    { "plot_list", cmd_plot_list },
	    { "phase_function", cmd_phase_function },
	    { "star_mass", cmd_star_mass },
	    { "detector_opiate", cmd_detector_opiate },
	    { "detector_opiate_healpix", cmd_detector_opiate_healpix },
	    { "opiata_path_emi", cmd_opiata_path_emi },
	    { "opiata_path_abs", cmd_opiata_path_abs },
	    { "detector_line", cmd_detector_line },
	    { "detector_line_healpix", cmd_detector_line_healpix },
	    { "detector_line_polar", cmd_detector_line_polar },
	    { "detector_line_slice", cmd_detector_line_slice },
	    { "detector_dust", cmd_detector_dust },
	    { "detector_dust_healpix", cmd_detector_dust_healpix },
	    { "detector_dust_polar", cmd_detector_dust_polar },
	    { "detector_dust_slice", cmd_detector_dust_slice },
	    { "detector_dust_mc", cmd_detector_dust_mc },
	    { "detector_sync", cmd_detector_sync },
	    { "detector_sync_slice", cmd_detector_sync_slice },
	    { "detector_sync_healpix", cmd_detector_sync_healpix },
	    { "gas_species", cmd_gas_species },
	    { "source_star", cmd_source_star },
	    { "source_starfield", cmd_source_starfield },
	    { "source_background", cmd_source_background },
	    { "source_laser", cmd_source_laser },
	    { "axis1", cmd_axis1 },
	    { "axis2", cmd_axis2 },
	    { "align", cmd_align },
	    { "mu", cmd_mu },
	    { "xy_min", cmd_xy_min },
	    { "xy_max", cmd_xy_max },
	    { "xy_steps", cmd_xy_steps },
	    { "xy_bins", cmd_xy_bins },
	    { "xy_label", cmd_xy_label },
	    { "healpix_orientation", cmd_healpix_orientation },
	    { "path_grid", cmd_path_grid },
	    { "path_grid_cgs", cmd_path_grid_cgs },
	    { "sub_dust", cmd_sub_dust },
	    { "vel_maps", cmd_vel_maps },
	    { "max_subpixel_lvl", cmd_max_subpixel_lvl },
	    { "path_input", cmd_path_input },
	    { "dust_componenct", cmd_dust_component },
	    { "path_out", cmd_path_out },
	    { "nr_plot_points", cmd_nr_plot_points },
	    { "nr_plot_vectors", cmd_nr_plot_vectors },
	    { "f_highJ", cmd_f_highJ },
	    { "Q_ref", cmd_Q_ref },
	    { "alpha_Q", cmd_alpha_Q },
	    { "R_rayleigh", cmd_R_rayleigh },
	    { "f_c", cmd_f_c },
	    { "adj_tgas", cmd_adj_tgas },
	    { "max_plot_lines", cmd_max_plot_lines },
	    { "start", cmd_start },
	    { "stop", cmd_stop },
	    { "conv_dens", cmd_conv_dens },
	    { "conv_len", cmd_conv_len },
	    { "conv_mag", cmd_conv_mag },
	    { "conv_vel", cmd_conv_vel },
	    { "mass_fraction", cmd_mass_fraction },
	    { "mrw", cmd_mrw },
	    { "pda", cmd_pda },
	    { "dust_offset", cmd_dust_offset },
	    { "dust_gas_coupling", cmd_dust_gas_coupling },
	    { "radiation_field", cmd_radiation_field },
	    { "rt_scattering", cmd_rt_scattering },
	    { "split_dust_emission", cmd_split_dust_emission },
	    { "full_dust_temp", cmd_full_dust_temp },
	    { "stochastic_heating", cmd_stochastic_heating },
	    { "source_dust", cmd_source_dust },
	    { "source_isrf", cmd_source_isrf },
	    { "foreground_extinction", cmd_foreground_extinction },
	    { "enfsca", cmd_enfsca },
	    { "peel_off", cmd_peel_off },
	    { "acceptance_angle", cmd_acceptance_angle },
	    { "nr_threads", cmd_nr_threads },
	    { "vel_is_speed_of_sound", cmd_vel_is_speed_of_sound },
	    { "amira_inp_points", cmd_amira_inp_points },
	    { "amira_out_points", cmd_amira_out_points },
	    { "plot_inp_midplanes", cmd_plot_inp_midplanes },
	    { "plot_out_midplanes", cmd_plot_out_midplanes },
	    { "write_3d_midplanes", cmd_write_3d_midplanes },
	    { "write_inp_midplanes", cmd_write_inp_midplanes },
	    { "write_out_midplanes", cmd_write_out_midplanes },
	    { "write_radiation_field", cmd_write_radiation_field },
	    { "write_full_radiation_field", cmd_write_full_radiation_field },
	    { "write_g_zero", cmd_write_g_zero },
	    { "write_dust_files", cmd_write_dust_files },
	    { "midplane_zoom", cmd_midplane_zoom },
	    { "kepler_star_mass", cmd_kepler_star_mass },
	    { "turbulent_velocity", cmd_turbulent_velocity },
	    { "mc_lvl_pop_photons", cmd_mc_lvl_pop_photons },
	    { "mc_lvl_pop_seed", cmd_mc_lvl_pop_seed },
	};
    }
}

#endif
