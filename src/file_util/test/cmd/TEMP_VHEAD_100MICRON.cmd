<common>
<write_inp_midplanes> 1000
<write_out_midplanes> 1000
<nr_threads> -1
</common>

<task> 1
<dust_component id="0"> "/star/home/nluettkemoeller/git/POLARIS/input/dust_nk/dsharp_b18.nk" "plaw" 1.0 1675 0.1e-06 10.0e-6 -3.5
<dust_component id="1"> "/star/home/nluettkemoeller/git/POLARIS/input/dust_nk/dsharp_b18.nk" "logn" 1.0 1675 20.0e-06 500.0e-6 100.0e-6 0.25
<dust_component id="2"> "/star/home/nluettkemoeller/git/POLARIS/input/dust_nk/dsharp_b18.nk" "logn" 1.0 1675 200.0e-06 5000.0e-6 1000.0e-6 0.25
<phase_function> PH_MIE
<source_star nr_photons = "1e9"> 0 0 0 2.0 4000
<cmd> CMD_TEMP
<path_grid> "/star/data/nluettkemoeller/POLCA/results/orbit_1000_m1.00_bin1/grid_files/grid_1000_vhead_100micron.dat"
<path_out> "/star/data/nluettkemoeller/POLCA/results/orbit_1000_m1.00_bin1/temp_vhead_100micron/"
</task>
