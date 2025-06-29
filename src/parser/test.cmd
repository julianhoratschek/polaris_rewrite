<common>
    <xy_steps> 100
</common>

<task>
    <cmd> CMD_LINE_EMISSION
    <delta0> 10.5
    <larm_f> 394,2
    <plot_list> 3 34 5 2
    <phase_function id = "143"> PH_MIE
    <star_mass> 13353,24 2342.435 2434e23
    <opiata_path_emi> "Opiate ID Emi"
    <opiata_path_abs> "Opiate ID Abs"
    <gas_species> "path_to_species" POP_MC 3.5 "zeeman_path"
    <source_star nr_photons="1039948"> "Path_to_star_source" 4 6 5 
    <source_starfield nr_photons="39084"> "Path" 35 6 3 43
    # <source_background nr_photons="4938"> "Path" 35 2 4
    <source_laser nr_photons="43094"> 23 4 53 6 5 7 3 4 2
    <axis1> 3 2 4
    <axis2> 4 6 3
    <align> ALIG_IDG
    <mu> 5
    <xy_min> 23
    <xy_max> 56
    <xy_steps> 2
    <xy_bins> 5
    <xy_label> "The label"
    <healpix_orientation> HEALPIX_CENTER
    <path_grid> "Path to grid"
    <path_grid_cgs> "Path to cgs"
    <sub_dust> 1
    <vel_maps> 1
    <max_subpixel_lvl> 98
    <path_input> "Input path"
    <dust_component id="1"> "dust_path" "plaw-ed" 34 5 4 6 43 2 5 3
    <path_out> "Output path"
    <nr_plot_points> 4
    <nr_plot_vectors> 2
    <f_highJ> 9
    <Q_ref> 94
    <alpha_Q> 3
    <R_rayleigh> 235
    <f_c> 34
    <adj_tgas> 23
    <max_plot_lines> 3
    <start> 2
    <stop> 12
    <conv_dens> 9
    <conv_len>94
    <conv_mag>943
    <conv_vel>23
    <mass_fraction> 58
    <mrw> 1
    <pda> 1
    # These are the same... more or less...
    <dust_offset min_gas_density="43.5"> 0
    <dust_gas_coupling min_gas_density="43.2"> 1
    <radiation_field> 1
    <rt_scattering> 1
    <split_dust_emission> 1
    <full_dust_temp> 1
    <stochastic_heating> 493
    <source_dust nr_photons="3948859">
    <source_isrf nr_photons="903489"> "pth to isrf source" 39
    <foreground_extinction> 34 54 65
    <enfsca> 1
    <peel_off> 1
    <acceptance_angle> 948
    <nr_threads> 10
    <vel_is_speed_of_sound> 1
    <amira_inp_points> 34
    <amira_out_points> 24
    <plot_inp_midplanes> 1
    <plot_out_midplanes> 1
    <write_3d_midplanes> 2 21 23 58
    <write_inp_midplanes> 1
    <write_out_midplanes> 0
    <write_radiation_field> 1
    <write_full_radiation_field>
    <write_g_zero> 1
    <write_dust_files> 1
    <midplane_zoom> 3.4
    <kepler_star_mass> 3928.34e23
    <turbulent_velocity> 3
    <mc_lvl_pop_photons> 4353
    <mc_lvl_pop_seed> 99483
</task>

<task>
    <detector_line nr_pixel = "245" vel_channels = "3"> 23 4 3 5 4 6
</task>

<task>
    <detector_line_healpix nr_sides = "4" vel_channels = "23"> 23 4 3 5 4 6 5
</task>

<task>
    <detector_line_polar nr_pixel = "2342984294" vel_channels = "34"> 2 3 4 5 4 3
</task>

<task>
    <detector_line_slice nr_pixel = "33242" vel_channels = "342"> 2 4 3 5 4 6
</task>

<task>
    <detector_dust nr_pixel="23423"> 2 34 5 5 65 3
</task>

<task>
    <detector_dust_healpix nr_sides="8"> 3 34 5 4 3 5 6
</task>

<task>
    <detector_dust_polar nr_pixel="42544"> 3 4 5 3 4 5
</task>

<task>
    <detector_dust_slice nr_pixel="345345"> 3 45 4 3 5 3
</task>

<task>
    <detector_dust_mc nr_pixel="34234"> 3 4 5 4 3 5
</task>

<task>
    <detector_sync nr_pixel="344234"> 3 5 4 6 5 7
</task>

<task>
    <detector_sync_slice nr_pixel="534534"> 4 3 6 5 4 3
</task>

<task>
    <detector_sync_healpix nr_sides="4"> 3 4 5 6 4 5 6
</task>
