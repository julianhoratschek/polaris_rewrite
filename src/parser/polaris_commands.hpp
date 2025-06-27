#include "command_parser.hpp"
#include "../Parameters.hpp"

#include <array>

namespace rewrite {

    auto cmd_cmd(ParsedLine& line, parameters& param) -> std::expected<void, std::string>;
    auto cmd_delta0(ParsedLine& line, parameters& param) -> std::expected<void, std::string>;
    auto cmd_larm_f(ParsedLine& line, parameters& param) -> std::expected<void, std::string>;
    auto cmd_plot_list(ParsedLine& line, parameters& param) -> std::expected<void, std::string>;
    auto cmd_phase_function(ParsedLine& line, parameters& param) -> std::expected<void, std::string>;
    auto cmd_star_mass(ParsedLine& line, parameters& param) -> std::expected<void, std::string>;
    auto cmd_opiata_path_emi(ParsedLine& line, parameters& param) -> std::expected<void, std::string>;
    auto cmd_opiata_path_abs(ParsedLine& line, parameters& param) -> std::expected<void, std::string>;
    auto cmd_gas_species(ParsedLine& line, parameters& param) -> std::expected<void, std::string>;
    auto cmd_source_star(ParsedLine& line, parameters& param) -> std::expected<void, std::string>;
    auto cmd_source_starfield(ParsedLine& line, parameters& param) -> std::expected<void, std::string>;
    auto cmd_source_background(ParsedLine& line, parameters& param) -> std::expected<void, std::string>;
    auto cmd_source_laser(ParsedLine& line, parameters& param) -> std::expected<void, std::string>;
    auto cmd_axis1(ParsedLine& line, parameters& param) -> std::expected<void, std::string>;
    auto cmd_axis2(ParsedLine& line, parameters& param) -> std::expected<void, std::string>;
    auto cmd_align(ParsedLine& line, parameters& param) -> std::expected<void, std::string>;
    auto cmd_mu(ParsedLine& line, parameters& param) -> std::expected<void, std::string>;
    auto cmd_xy_min(ParsedLine& line, parameters& param) -> std::expected<void, std::string>;
    auto cmd_xy_max(ParsedLine& line, parameters& param) -> std::expected<void, std::string>;
    auto cmd_xy_steps(ParsedLine& line, parameters& param) -> std::expected<void, std::string>;
    auto cmd_xy_bins(ParsedLine& line, parameters& param) -> std::expected<void, std::string>;
    auto cmd_xy_label(ParsedLine& line, parameters& param) -> std::expected<void, std::string>;
    auto cmd_healpix_orientation(ParsedLine& line, parameters& param) -> std::expected<void, std::string>;
    auto cmd_path_grid(ParsedLine& line, parameters& param) -> std::expected<void, std::string>;
    auto cmd_path_grid_csg(ParsedLine& line, parameters& param) -> std::expected<void, std::string>;
    auto cmd_sub_dust(ParsedLine& line, parameters& param) -> std::expected<void, std::string>;
    auto cmd_vel_maps(ParsedLine& line, parameters& param) -> std::expected<void, std::string>;
    auto cmd_max_subpixel_lvl(ParsedLine& line, parameters& param) -> std::expected<void, std::string>;
    auto cmd_path_input(ParsedLine& line, parameters& param) -> std::expected<void, std::string>;
    auto cmd_dust_component(ParsedLine& line, parameters& param) -> std::expected<void, std::string>;
    auto cmd_path_out(ParsedLine& line, parameters& param) -> std::expected<void, std::string>;
    auto cmd_nr_plot_points(ParsedLine& line, parameters& param) -> std::expected<void, std::string>;
    auto cmd_nr_plot_vectors(ParsedLine& line, parameters& param) -> std::expected<void, std::string>;
    auto cmd_f_highJ(ParsedLine& line, parameters& param) -> std::expected<void, std::string>;
    auto cmd_Q_ref(ParsedLine& line, parameters& param) -> std::expected<void, std::string>;
    auto cmd_alpha_Q(ParsedLine& line, parameters& param) -> std::expected<void, std::string>;
    auto cmd_R_rayleigh(ParsedLine& line, parameters& param) -> std::expected<void, std::string>;
    auto cmd_f_c(ParsedLine& line, parameters& param) -> std::expected<void, std::string>;
    auto cmd_adj_tgs(ParsedLine& line, parameters& param) -> std::expected<void, std::string>;
    auto cmd_max_plot_lines(ParsedLine& line, parameters& param) -> std::expected<void, std::string>;
    auto cmd_start(ParsedLine& line, parameters& param) -> std::expected<void, std::string>;
    auto cmd_stop(ParsedLine& line, parameters& param) -> std::expected<void, std::string>;
    auto cmd_cons_dens(ParsedLine& line, parameters& param) -> std::expected<void, std::string>;
    auto cmd_conv_len(ParsedLine& line, parameters& param) -> std::expected<void, std::string>;
    auto cmd_conv_mag(ParsedLine& line, parameters& param) -> std::expected<void, std::string>;
    auto cmd_conv_vel(ParsedLine& line, parameters& param) -> std::expected<void, std::string>;
    auto cmd_mass_fraction(ParsedLine& line, parameters& param) -> std::expected<void, std::string>;
    auto cmd_mrw(ParsedLine& line, parameters& param) -> std::expected<void, std::string>;
    auto cmd_pda(ParsedLine& line, parameters& param) -> std::expected<void, std::string>;
    auto cmd_dust_offset(ParsedLine& line, parameters& param) -> std::expected<void, std::string>;
    auto cmd_gas_coupling(ParsedLine& line, parameters& param) -> std::expected<void, std::string>;
    auto cmd_radiation_field(ParsedLine& line, parameters& param) -> std::expected<void, std::string>;
    auto cmd_rt_scattering(ParsedLine& line, parameters& param) -> std::expected<void, std::string>;
    auto cmd_split_dust_emission(ParsedLine& line, parameters& param) -> std::expected<void, std::string>;
    auto cmd_full_dust_temp(ParsedLine& line, parameters& param) -> std::expected<void, std::string>;
    auto cmd_stochastic_heating(ParsedLine& line, parameters& param) -> std::expected<void, std::string>;
    auto cmd_source_dust(ParsedLine& line, parameters& param) -> std::expected<void, std::string>;
    auto cmd_source_isrf(ParsedLine& line, parameters& param) -> std::expected<void, std::string>;
    auto cmd_foreground_extinction(ParsedLine& line, parameters& param) -> std::expected<void, std::string>;
    auto cmd_enfsca(ParsedLine& line, parameters& param) -> std::expected<void, std::string>;
    auto cmd_peel_off(ParsedLine& line, parameters& param) -> std::expected<void, std::string>;
    auto cmd_acceptance_angle(ParsedLine& line, parameters& param) -> std::expected<void, std::string>;
    auto cmd_nr_threads(ParsedLine& line, parameters& param) -> std::expected<void, std::string>;
    auto cmd_vel_is_speed_of_sound(ParsedLine& line, parameters& param) -> std::expected<void, std::string>;
    auto cmd_amira_inp_points(ParsedLine& line, parameters& param) -> std::expected<void, std::string>;
    auto cmd_amira_out_points(ParsedLine& line, parameters& param) -> std::expected<void, std::string>;
    auto cmd_plot_inp_midplanes(ParsedLine& line, parameters& param) -> std::expected<void, std::string>;
    auto cmd_plot_out_midplanes(ParsedLine& line, parameters& param) -> std::expected<void, std::string>;
    auto cmd_write_3d_midplanes(ParsedLine& line, parameters& param) -> std::expected<void, std::string>;
    auto cmd_write_inp_midplanes(ParsedLine& line, parameters& param) -> std::expected<void, std::string>;
    auto cmd_write_out_midplanes(ParsedLine& line, parameters& param) -> std::expected<void, std::string>;
    auto cmd_write_radiation_field(ParsedLine& line, parameters& param) -> std::expected<void, std::string>;
    auto cmd_write_full_radiation_field(ParsedLine& line, parameters& param) -> std::expected<void, std::string>;
    auto cmd_write_g_zero(ParsedLine& line, parameters& param) -> std::expected<void, std::string>;
    auto cmd_write_dust_files(ParsedLine& line, parameters& param) -> std::expected<void, std::string>;
    auto cmd_midplane_zoom(ParsedLine& line, parameters& param) -> std::expected<void, std::string>;
    auto cmd_kepler_star_mass(ParsedLine& line, parameters& param) -> std::expected<void, std::string>;
    auto cmd_turbulent_velocity(ParsedLine& line, parameters& param) -> std::expected<void, std::string>;
    auto cmd_mc_lvl_pop_photons(ParsedLine& line, parameters& param) -> std::expected<void, std::string>;
    auto cmd_mc_lvl_pop_seed(ParsedLine& line, parameters& param) -> std::expected<void, std::string>;
    auto cmd_detector_opiate(ParsedLine& line, parameters& param) -> std::expected<void, std::string>;
    auto cmd_detector_opiate_healpix(ParsedLine& line, parameters& param) -> std::expected<void, std::string>;
    auto cmd_detector_line(ParsedLine& line, parameters& param) -> std::expected<void, std::string>;
    auto cmd_detector_line_healpix(ParsedLine& line, parameters& param) -> std::expected<void, std::string>;
    auto cmd_detector_line_polar(ParsedLine& line, parameters& param) -> std::expected<void, std::string>;
    auto cmd_detector_line_slice(ParsedLine& line, parameters& param) -> std::expected<void, std::string>;
    auto cmd_detector_dust(ParsedLine& line, parameters& param) -> std::expected<void, std::string>;
    auto cmd_detector_dust_healpix(ParsedLine& line, parameters& param) -> std::expected<void, std::string>;
    auto cmd_detector_dust_polar(ParsedLine& line, parameters& param) -> std::expected<void, std::string>;
    auto cmd_detector_dust_slice(ParsedLine& line, parameters& param) -> std::expected<void, std::string>;
    auto cmd_detector_dust_mc(ParsedLine& line, parameters& param) -> std::expected<void, std::string>;
    auto cmd_detector_sync(ParsedLine& line, parameters& param) -> std::expected<void, std::string>;
    auto cmd_detector_sync_slice(ParsedLine& line, parameters& param) -> std::expected<void, std::string>;
    auto cmd_detector_sync_healpix(ParsedLine& line, parameters& param) -> std::expected<void, std::string>;


    using CommandProcessFn = std::expected<bool, std::string> (*)(ParsedLine&, parameters&);
    constexpr auto make_map() {
	return std::map<std::string, CommandProcessFn> {
	};
    }


}
