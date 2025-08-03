<common>
<axis1> -1 0 0
<axis2> 0 1 0
<write_inp_midplanes> 1000
<write_out_midplanes> 1000
<nr_threads> -1
</common>

<task> 1
<dust_component id="0"> "/star/home/nluettkemoeller/git/POLARIS/input/dust_nk/dsharp_b18.nk" "plaw" 1.0 1675 0.1e-06 10.0e-6 -3.5
<dust_component id="1"> "/star/home/nluettkemoeller/git/POLARIS/input/dust_cs/dsharp_b18_obl_0.67_ddscat_miex_mflock_100um.dat" "logn" 1.0 1675 20.0e-06 500.0e-6 100.0e-6 0.25
<dust_component id="2"> "/star/home/nluettkemoeller/git/POLARIS/input/dust_nk/dsharp_b18.nk" "logn" 1.0 1675 200.0e-06 5000.0e-6 1000.0e-6 0.25
<detector_dust_polar nr_pixel = "1000"> 0.87e-3 0.87e-3 1 1 0 0 4.319948614054068224e+18
<detector_dust_polar nr_pixel = "1000"> 1.30e-3 1.30e-3 1 1 0 0 4.319948614054068224e+18
<detector_dust_polar nr_pixel = "1000"> 3.00e-3 3.00e-3 1 1 0 0 4.319948614054068224e+18
<detector_dust_polar nr_pixel = "1000"> 7.00e-3 7.00e-3 1 1 0 0 4.319948614054068224e+18
<detector_dust_polar nr_pixel = "1000"> 10.0e-3 10.0e-3 1 1 0 0 4.319948614054068224e+18
<detector_dust_polar nr_pixel = "1000"> 0.87e-3 0.87e-3 1 1 45 0 4.319948614054068224e+18
<detector_dust_polar nr_pixel = "1000"> 1.30e-3 1.30e-3 1 1 45 0 4.319948614054068224e+18
<detector_dust_polar nr_pixel = "1000"> 3.00e-3 3.00e-3 1 1 45 0 4.319948614054068224e+18
<detector_dust_polar nr_pixel = "1000"> 7.00e-3 7.00e-3 1 1 45 0 4.319948614054068224e+18
<detector_dust_polar nr_pixel = "1000"> 10.0e-3 10.0e-3 1 1 45 0 4.319948614054068224e+18
<detector_dust_polar nr_pixel = "1000"> 0.87e-3 0.87e-3 1 1 90 0 4.319948614054068224e+18
<detector_dust_polar nr_pixel = "1000"> 1.30e-3 1.30e-3 1 1 90 0 4.319948614054068224e+18
<detector_dust_polar nr_pixel = "1000"> 3.00e-3 3.00e-3 1 1 90 0 4.319948614054068224e+18
<detector_dust_polar nr_pixel = "1000"> 7.00e-3 7.00e-3 1 1 90 0 4.319948614054068224e+18
<detector_dust_polar nr_pixel = "1000"> 10.0e-3 10.0e-3 1 1 90 0 4.319948614054068224e+18
<cmd> CMD_DUST_EMISSION
<align> ALIG_NONPA
<R_rayleigh> -1
<max_subpixel_lvl> 1
<path_grid> "/star/data/nluettkemoeller/POLCA/results/orbit_1000_m1.00_bin1/grid_files/grid_1000_alig_100micron.dat"
<path_out> "/star/data/nluettkemoeller/POLCA/results/orbit_1000_m1.00_bin1/dust_alig_100micron/"
</task>
