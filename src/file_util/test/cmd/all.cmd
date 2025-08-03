<common>
</common>


# <task> 0
# </task>

<task>
    <cmd> CMD_TEMP
    <delta0> 10.434
    <larm_f> 4923,5235
    <plot_list> 23 5 34 2
    <phase_function id="23"> PH_MIE
    <star_mass> 3923,4234 34.3 54 12.43e-45
    <opiata_path_emi> "Path or something"
    <opiata_path_abs> "Another one"
    <gas_species> "Path to gas" POP_LTE 12.4 "zeeman path"
    <source_star nr_photons="129384"> "path to star source" 293 484 54
    <source_starfield nr_photons="19384"> "path to star field" 39 390 23 09
    <source_background nr_photons = "1293"> "bg source path" 123,3 43 45.2 34.5 34.5
    <source_laser nr_photons="1949"> 39 49 30 2  54 324 2 3 4
    <axis1> 32.43 445.34 63.345
    <axis2> 645.53 234,245 34.54e-34
    <align> ALIG_NONPA
    <mu> 34.234
    <xy_min> 5452.34
    <xy_max> 43523.534
    <xy_steps> 23423.5324
    <xy_bins> 43.34
    <xy_label> "xy label value"
    <healpix_orientation> HEALPIX_CENTER
    <path_grid> "path to grid"
    <path_grid_cgs> "path cgs grid"
    <sub_dust> 0
    <vel_maps> 0
    <max_subpixel_lvl> 3934
    <path_input> "path for input"
    <dust_component id = "439"> "path to dust" "plaw-ed" 234.234 452.234 324.43 4235 534 342 345 34
    <path_out> "Write path out"
    <nr_plot_points> 34
    <nr_plot_vectors> 3435
    <f_highJ> 23
    <Q_ref> 45245
    <alpha_Q> 3943
    <R_rayleigh> 4934
    <f_c> 390234
    <adj_tgas> 3423
    <max_plot_lines> 23
    <start> 32
    <stop> 54
    <conv_dens> 34
    <conv_len> 54
    <conv_mag> 948
    <conv_vel> 3984
    <mass_fraction> 349
    <mrw> 1			## Should generate a Warning
    <pda> 0			## Should generate a Warning
    <dust_offset> 0
    <dust_gas_coupling> 0
    <radiation_field> 1
    <rt_scattering> 1
    <split_dust_emission> 0
    <full_dust_temp> 1
    <stochastic_heating> 23.54
    <source_dust nr_photons = "43.4">
    # <source_isrf nr_photons = "32.4"> "any path" 32.4 23.4
    <foreground_extinction> 34.3 2.54e-23 43.5
    <enfsca> 1
    <peel_off> 1
    <acceptance_angle> 234.5
    <nr_threads> 10
    <vel_is_speed_of_sound> 1
    <amira_inp_points> 192
    <amira_out_points> 392
    <plot_inp_midplanes> 1
    <plot_out_midplanes> 0
    <write_3d_midplanes> 2 23 35 143
    <write_inp_midplanes> 34
    <write_out_midplanes> 43
    <write_radiation_field> 1
    <write_full_radiation_field> 0	# Should generate a warning
    <write_g_zero> 0
    <write_dust_files> 1
    <midplane_zoom> 12
    <kepler_star_mass> 392.45e13
    <turbulent_velocity> 34
    <mc_lvl_pop_photons> 192384
    <mc_lvl_pop_seed> 29384
    # <detector_opiate>
    # <detector_opiate_healpix>
    # <detector_line>
    # <detector_line_healpix>
    # <detector_line_polar>
    # <detector_line_slice>
    # <detector_dust>
    # <detector_dust_healpix>
    # <detector_dust_polar>
    # <detector_dust_slice>
    # <detector_dust_mc>
    # <detector_sync>
    # <detector_sync_slice>
    # <detector_sync_healpix>
</task>

<task>
    <cmd> CMD_DUST_EMISSION
    <phase_function id="54"> PH_ISO
    <gas_species> "Path to gas" 4 12.4
    <align> ALIG_INTERNAL
    <conv_dens> -43		# Should generate a warning
    <conv_vel> -54		# Should generate a warning
    <dust_offset min_gas_density="23.45"> 1
    <dust_gas_coupling min_gas_density = "34,34"> 0
    <foreground_extinction> 32.54
    <nr_threads> -1
    <vel_is_speed_of_sound> 0
    # <write_3d_midplanes> 0 43 2834 1	# This is an error
</task>

<task>
    <phase_function id="43"> PH_HG
    <cmd> CMD_DUST_SCATTERING
    # <gas_species> "Path to gas" POP_DEGUCHI_LVG 43.54 	# POP index not supported in legacy
    <align> ALIG_PA
    <nr_threads> 100203
</task>

<task>
    <phase_function id="43"> PH_DHG
    # <cmd> CMD_PROBING						# Not supported by legacy parser
    <align> ALIG_IDG
</task>

<task>
    <phase_function id="43"> PH_TTHG
    <cmd> CMD_RAT
    <align> ALIG_RAT
</task>

<task>
    <cmd> CMD_TEMP_RAT
    <align> ALIG_GOLD
</task>

<task>
    <cmd> CMD_LINE_EMISSION
    # <align> ALIG_KRAT		# Not supported in old parser
</task>

<task>
    <cmd> CMD_FORCE
</task>

<task>
    <cmd> CMD_OPIATE
</task>

<task>
    <cmd> CMD_SYNCHROTRON
</task>
