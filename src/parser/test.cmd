<common>
    <xy_steps> 100
</common>

<task>
    <cmd> CMD_TEMP
    <delta0> 10.5
    <larm_f> 394,2
    <plot_list> 3 34 5 2
    <phase_function id = "143"> PH_MIE
    <star_mass> 13353,24 2342.435 2434e23
    <opiata_path_emi> "Opiate ID Emi"
    <opiata_path_abs> "Opiate ID Abs"
    <gas_species> "path_to_species" POP_MC 3.5 "zeeman_path"
    <source_star nr_photons="1039948"> "Path_to_star_source" 4 6 5 
    <source_starfield nr_photons="39084"> "Path" 35 6 3
    <source_background nr_photons="4938"> "Path" 35 2 4
    <source_laser nr_photons="43094"> 23 4 53 6 5 7 3 4 2
    <axis1> 3 2 4
    <axis2> 4 6 3
    <align> ALIG_IDG
    <mu> 5
    <xy_min> 23
    <xy_max> 56
    <xy_steps> 2
    <xy_bin> 5
    <xy_label> "The label"
    <healpix_orientation> HEALPIX_CENTER
    <path_grid> "Path to grid"
    <path_grid_cgs> "Path to cgs"
    <sub_dust> 45
    <vel_maps> 59
    <max_subpixel_lvl> 98
    <path_input> "Input path"
    <dust_component id="1"> "dust_path" "plaw-ed" 34 5 4 6
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
    <mrw> 2
    <pda> 1
    # These are the same... more or less...
    <dust_offset min_gas_density="43.5"> 19
    <dust_gas_coupling min_gas_density="43.2"> 54
    <radiation_field> 32
    <rt_scattering> 94
    <split_dust_emission> 23
    <full_dust_temp> 456
    <stochastic_heating> 493
    <source_dust nr_photons="3948859">
    <source_isrf nr_photons="903489"> "pth to isrf source" 48 39
    <foreground_extinction> 34 54 65
    <enfsca> 49
    <peel_off> 90
    <acceptance_angle> 948
    <nr_threads> 10
    <vel_is_speed_of_sound> 1
    <amira_inp_points> 34
    <amita_out_points> 24
    <plot_inp_midplanes> 43
    <plot_out_midplanes> 34
    <write_3d_midplanes> 49 29 23
    <write_inp_midplanes> 1
    <write_out_midplanes> 0
    <write_radiation_field> 2
    <write_full_radiation_field>
    <write_g_zero> 43
    <write_dust_files> 1
    <midplane_zoom> 3.4
    <kepler_star_mass> 3928.34e23
    <turbulent_velocity> 3
    <mc_lvl_pop_photons> 4353
    <mc_lvl_pop_seed> 99483
</task>

<common>
</common>

# Comment line

<task> 0
    # Ignored
</task>

<task>
    <gas_species> "path_to_species.gas" 23.5 54.3
    <source_star nr_photons="348933"> 4 5 3 6 2
    <source_background nr_photons="489"> 3 43 5 4 32
    <dust_component> "dust_path2" 32 5 4 6
    <dust_offset> 43
</task>

<task>
    <source_star nr_photons="493893"> 3 5 8 5 4 5 2 3
</task>
