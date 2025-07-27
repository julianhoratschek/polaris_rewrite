/************************************************************************************
*                      POLARIS: POLArized RadIation Simulator                       *
*                         Copyright (C) 2018 Stefan Reissl                          *
************************************************************************************/

#ifndef PARAMETERS_H
#define PARAMETERS_H

#include "Typedefs.hpp"
#include "Vector3D.hpp"

class parameters
{
public:
    parameters()
    {
        path_grid = "";
        path_output = "";
        // phID = PH_HG;
        conv_l_in_SI = 1;
        conv_dH_in_SI = 1;
        conv_B_in_SI = 1;
        conv_V_in_SI = 1;
        conv_mass_fraction = 0.01;
        align = 0;
        nr_ofThreads = 1;

        mu = 2.0;

        min_detector_pixel_x = MAX_UINT;
        max_detector_pixel_x = 0;
        min_detector_pixel_y = MAX_UINT;
        max_detector_pixel_y = 0;

        min_rot_angle_1 = 360;
        max_rot_angle_1 = 0;
        min_rot_angle_2 = 360;
        max_rot_angle_2 = 0;

        min_sidelength_x = 1e300;
        max_sidelength_x = 0;
        min_sidelength_y = 1e300;
        max_sidelength_y = 0;
        use_grid_sidelength_x = false;
        use_grid_sidelength_y = false;

        min_ray_map_shift_x = 1e300;
        max_ray_map_shift_x = 0;
        min_ray_map_shift_y = 1e300;
        max_ray_map_shift_y = 0;

        min_obs_distance = 1e300;
        max_obs_distance = -1e300;

        delta0 = 8.28e23 * 2.5e-12 * 1e8 * 1e-6 * 1e6;
        larm_f = 4.1e-19;

        nr_of_mc_lvl_pop_photons = 0;
        mc_lvl_pop_seed = 0;

        kepler_star_mass = 0;
        turbulent_velocity = 0;
        offset_min_gas_dens = 0;
        stochastic_heating_max_size = 0;

        task_id = 0;

        rt_grid_description = "";

        b_mrw = false;
        b_pda = false;
        b_enforced = false;
        peel_off = true;
        is_speed_of_sound = false;
        vel_maps = false;
        dust_offset = false;
        dust_gas_coupling = false;
        full_dust_temp = false;
        save_radiation_field = false;
        scattering_to_raytracing = false;
        split_dust_emision = false;
        sublimate = false;
        individual_dust_fractions = false;

        nr_ofISRFPhotons = 0;
        nr_ofDustPhotons = 0;

        nrOfPlotPoints = 0;
        nrOfPlotVectors = 0;
        maxPlotLines = 0;
        cmd = -1;

        healpix_orientation = HEALPIX_YAXIS;

        start = MAX_UINT;
        stop = MAX_UINT;

        nr_ofInpAMIRAPoints = 0;
        nr_ofOutAMIRAPoints = 0;

        plot_inp_points = false;
        plot_out_points = false;
        write_radiation_field = 0;
        write_g_zero = false;
        write_dust_files = false;
        nr_ofInpMidDataPoints = 0;
        nr_ofOutMidDataPoints = 0;

        f_highJ = 0.25;
        Q_ref = 0.4;
        alpha_Q = 3.0;
        R_rayleigh = 1.0;

        f_cor = 0.6;
        adjTgas = 0;
        isrf_g_zero = 0;
        isrf_radius = 0;

        isrf_path = "";

        max_subpixel_lvl = 3;
        midplane_zoom = 1;
        max_dust_component_choice = 0;

        extinction_magnitude = 0;
        extinction_magnitude_wavelength = 0;
        extinction_i_mixture = MAX_UINT;

        reset_dust_files = false;
        acceptance_angle = 1.0;

        xymin = -1;
        xymax = 1;
        xysteps = 2;
        xy_bins = MAX_UINT;
        xylabel = "[a.u.]";
        autoscale = true;

        axis1.set(1, 0, 0);
        axis2.set(0, 1, 0);

        // opiate parmeters
        opiata_path_emi="";
        opiata_path_abs="";
    }

    ~parameters()
    {}

    string getOpiatePathEmission();

    string getOpiatePathAbsorption();

    string getOpiateSpec(uint pos);

    uint getNrOfOPIATESpecies();

    const Vector3D & getAxis1() const;

    const Vector3D & getAxis2() const;

    uint getOutAMIRAPoints() const;

    uint getInpAMIRAPoints() const;

    bool plotInpMidPoints() const;

    bool plotOutMidPoints() const;

    dlist getMidplane3dParams() const;

    const uilist & getPlotList() const;

    bool isInPlotList(uint id);

    uint getInpMidDataPoints() const;

    uint getOutMidDataPoints() const;

    /*
    const dlist & getOpiateSequence() const;
    */

    uint getMidplaneZoom() const;

    int getCommand() const;

    bool isRatSimulation() const;

    bool isMonteCarloSimulation() const;

    bool isRaytracingSimulation() const;

    bool isTemperatureSimulation() const;

    double getStarMass(uint i) const;

    string getPathGrid() const;

    string getPathOutput() const;

    string getPathInput() const;

    uint getMinDetectorPixelX() const;

    uint getMaxDetectorPixelX() const;

    uint getMinDetectorPixelY() const;

    uint getMaxDetectorPixelY() const;

    double getMinDetectorAngle1() const;

    double getMaxDetectorAngle1() const;

    double getMinDetectorAngle2() const;

    double getMaxDetectorAngle2() const;

    double getMinSidelengthX() const;

    double getMaxSidelengthX() const;

    double getMinSidelengthY() const;

    double getMaxSidelengthY() const;

    bool getUseGridSidelengthX() const;

    bool getUseGridSidelengthY() const;

    double getMinMapShiftX() const;

    double getMinMapShiftY() const;

    double getMaxMapShiftX() const;

    double getMaxMapShiftY() const;

    double getSIConvLength() const;

    double getSIConvDH() const;

    double getDelta0() const;

    double getLarmF() const;

    double getSIConvBField() const;

    double getSIConvVField() const;

    bool getDustOffset() const;

    bool getDustGasCoupling() const;

    double getOffsetMinGasDensity() const;

    bool getDustTempMulti() const;

    double getSizeMin(uint i) const;

    double getSizeMax(uint i) const;

    double getMaterialDensity(uint i) const;

    bool getDustSource() const;

    bool getISRFSource() const;

    ullong getNrOfDustPhotons() const;

    double getDustMassFraction() const;

    uint getAlign() const;

    bool getAligRANDOM() const;

    bool getAligPA() const;

    bool getAligNONPA() const;

    bool getAligIDG() const;

    bool getAligRAT() const;

    bool getAligGOLD() const;

    bool getAligINTERNAL() const;

    double getMu() const;

    bool getMRW() const;

    bool getPDA() const;

    bool getEnfScattering() const;

    double getStochasticHeatingMaxSize() const;

    bool getSaveRadiationField() const;

    bool getScatteringToRay() const;

    bool splitDustEmission() const;

    bool getIndividualDustMassFractions() const;

    bool getIsSpeedOfSound() const;

    bool getPeelOff() const;

    double getForegroundExtinctionMagnitude() const;

    double getForegroundExtinctionWavelength() const;

    uint getForegroundExtinctionDustMixture() const;

    bool getVelFieldType() const;

    uint getWriteRadiationField() const;

    bool getWriteGZero() const;

    bool getWriteDustFiles() const;

    double getISRFGZero() const;

    double getISRFRadius() const;

    string getISRFPath() const;

    string getZeemanCatalog(uint i_species) const;

    uint getAlignmentMechanism() const;

    double getMinObserverDistance() const;

    double getMaxObserverDistance() const;

    double getKeplerStarMass() const;

    double getTurbulentVelocity() const;

    uint getMCLvlPopNrOfPhotons() const;

    uint getMCLvlPopSeed() const;

    uint getTaskID() const;

    uint getNrOfThreads() const;

    ullong getNrOfISRFPhotons() const;

    uint getNrOfMixtures() const;

    uilist getDustComponentChoices() const;

    /*
    uint getPhaseFunctionID() const;
    */

    uint getPhaseFunctionID(uint i) const;

    double getFHighJ() const;

    double getQref() const;

    double getAlphaQ() const;

    double getRayleighReductionFactor() const;

    double getFcorr() const;

    double getAdjTgas() const;

    uint getNrOfDiffuseSources() const;

    uint getNrOfPointSources() const;

    uint getNrOfLaserSources() const;

    uint getNrOfBackgroundSources() const;

    double getXYMin() const;

    double getXYMax() const;

    double getXYSteps() const;

    uint getXYBins() const;

    string getXYLabel() const;

    bool isAutoScale() const;

    uint getNrOfSources() const;

    uint getNrOfPlotPoints() const;

    uint getNrOfPlotVectors() const;

    uint getMaxPlotLines() const;

    uint getStart() const;

    uint getStop() const;

    void setOpiatePathEmission(string str);

    void setOpiatePathAbsorption(string str);

    void setXYMin(double val);

    void setXYMax(double val);

    void setXYSteps(double val);

    void setXYBins(uint val);

    void setXYLabel(string val);

    void setAutoScale(bool val);

    void setAxis1(double x, double y, double z);

    void setAxis2(double x, double y, double z);

    void setStart(uint val);

    void setStop(uint val);

    void setMu(double val);

    void setNrOfPlotPoints(uint val);

    void setnrOfPlotVectors(uint val);

    void setMaxPlotLines(uint val);

    void setNrOfThreads(uint val);

    void setCommand(int val);

    void setISRF(string path, double g_zero = 0, double radius = 2);

    void setNrOfISRFPhotons(long val);

    void addOpiateSpec(string str);

    void AddDustComponentChoice(uint dust_component_choice);

    uint getMaxDustComponentChoice();

    void setTaskID(uint val);

    void addStarMass(double val);

    void setStarMass(dlist val);

    void setPathGrid(string val);

    void setPathInput(string val);

    void setPathOutput(string val);

    void setDustOffset(bool val);

    void setDustOffset(double _offset_min_gas_dens);

    void setDustGasCoupling(bool val);

    void setDustGasCoupling(double _offset_min_gas_dens);

    void setFullDustTemp(bool val);

    void setNrOfDustPhotons(long val);

    void setDelta0(double val);

    void addToPlotList(uint id);

    void setLarmF(double val);

    void setMRW(bool val);

    void setPDA(bool val);

    void setEnfScattering(bool val);

    void setIsSpeedOfSound(bool val);

    void setPeelOff(bool val);

    void setForegroundExtinction(double _extinction_magnitude,
                                 double _extinction_magnitude_wavelength = 0.55e-6,
                                 uint _extinction_i_mixture = MAX_UINT);

    void setInpAMIRAPoints(uint val);

    void setOutAMIRAPoints(uint val);

    void setInpMidPlot(bool val);

    void setOutMidPlot(bool val);

    void set3dMidplane(uint plane, uint nr_of_slices, double z_min, double z_max);

    void setInpMidDataPoints(uint val);

    void setOutMidDataPoints(uint val);

    void setMidplaneZoom(uint val);

    void setWriteRadiationField(uint val);

    void setWriteGZero(bool val);

    void setWriteDustFiles(bool val);

    void updateDetectorPixel(uint pixel_x, uint pixel_y);

    void updateDetectorAngles(double rot_angle_1, double rot_angle_2);

    void updateMapSidelength(double sidelength_x, double sidelength_y);

    void updateRayGridShift(double map_shift_x, double map_shift_y);

    void setStochasticHeatingMaxSize(double val);

    void setSaveRadiationField(bool val);

    void setScatteringToRay(bool val);

    void setSplitDustEmission(bool val);

    void setSIConvLength(double val);

    void setSIConvDH(double val);

    void setSIConvBField(double val);

    void setSIConvVField(double val);

    void updateSIConvLength(double val);

    void updateSIConvDH(double val);

    void updateSIConvBField(double val);

    void updateSIConvVField(double val);

    void setDustMassFraction(double val);

    void setIndividualDustMassFractions(bool val);

    void addAlignmentMechanism(uint val);

    void updateObserverDistance(double val);

    void setKeplerStarMass(double val);

    void setMCLvlPopNrOfPhotons(uint val);

    void setMCLvlPopSeed(uint val);

    void setTurbulentVelocity(double val);

    void addZeemanCatalog(string val);

    /*
    void setPhaseFunctionID(uint val);
    */

    void setPhaseFunctionID(uint val, uint pos);

    void setFhighJ(double val);

    void setQref(double val);

    void setAlphaQ(double val);

    void setRayleighReductionFactor(double val);

    void setFcorr(double val);

    void setAdjTgas(double val);

    void setVelMaps(bool val);

    void setAcceptanceAngle(double angle);

    void setMaxSubpixelLvl(int val);

    dlist & getDustRayDetectors();

    dlist & getSyncRayDetectors();

    dlist & getOPIATERayDetectors();

    dlist & getPointSources();

    dlist & getLaserSources();

    strlist & getPointSourceStringList();

    string & getPointSourceString(uint i_str);

    strlist & getDiffuseSourceStringList();

    string & getDiffuseSourceString(uint i_str);

    dlist & getDiffuseSources();

    dlist & getBackgroundSources();

    strlist & getBackgroundSourceStringList();

    string & getBackgroundSourceString(uint i_str);

    void addDustRayDetector(dlist & val);

    void addOpiateRayDetector(dlist & val);

    void addSyncRayDetector(dlist & val);

    /*
    void addLineOpiateDetector(dlist & val);

    void setOpiateParamPath(string val);

    void setOpiateDataPath(string val);
    */

    void addLineRayDetector(dlist & val);

    void addDustMCDetector(dlist & val);

    uint getNrOfDustMCDetectors();

    dlist getDustMCDetectors();

    void addPointSource(dlist & val, string path);

    void addLaserSource(dlist & val);

    void addBackgroundSource(dlist & val);

    void addBackgroundSource(string path, dlist & val);

    void addBackgroundSource(string path);

    void resetDustFiles();

    void addGasSpecies(string gas_species_path, string zeeman_path, dlist & val);

    uint getNrOfGasSpecies();

    uint getNrOfSpectralLines(uint i_species);

    std::vector<int> getSpectralLines(uint i_species);

    uint getNrOfDustRayDetectors();

    uint getNrOfSyncRayDetectors();

    void setSublimate(bool val);

    void setHealpixOrientation(uint val);

    uint getHealpixOrientation() const;

    const dlist & getLineRayDetector(uint i_species) const;

    const maplist & getLineRayDetectors() const;

    void addDustComponent(string path,
                          string size_key,
                          double fr,
                          double mat_dens,
                          double a_min,
                          double a_max,
                          dlist size_parameter);

    bool getVelMaps() const;

    uint getDustChoiceFromComponentId(uint i) const;

    uint getDustChoiceFromMixtureId(uint i) const;

    void printRTGridDescription();

    double getAcceptanceAngle() const;

    string getDustPath(uint i) const;

    bool isSublimate();

    double getDustFraction(uint i) const;

    string getDustSizeKeyword(uint i) const;

    dlist getDustSizeParameter(uint i_comp) const;

    string getGasSpeciesCatalogPath(uint i_species) const;

    double getGasSpeciesAbundance(uint i_species) const;

    uint getGasSpeciesLevelPopType(uint i_species) const;

    bool isGasSpeciesLevelPopMC() const;

    uint getMaxSubpixelLvl() const;

    uint getTotalNrOfDustComponents() const;

    void resetNrOfDustComponents();

    void addDiffuseSource(dlist & val, string path);

    size_t getDetectorSize() const {
	std::array<size_t, 10>	detectors{
	    0, dust_ray_detectors.size() / NR_OF_RAY_DET, 0,
	    0, 0, 0, gas_species_abundance.size(),
	    0, opiate_spec_ids.size(), sync_ray_detectors.size() / NR_OF_RAY_DET};

	return detectors[cmd];
    }

    string compare(const parameters& other) const {
	if (!(cmd == other.cmd)) return to_string(cmd) + " " + to_string(other.cmd);
	if (!(path_grid == other.path_grid)) return "path_grid";
	if (!(path_input == other.path_input)) return "path_input";
	if (!(path_output == other.path_output)) return "path_output";

	if (!(max_subpixel_lvl == other.max_subpixel_lvl)) return "max_subpixel_lvl";
	if (!(nr_ofThreads == other.nr_ofThreads)) return "nr_ofThreads: " + to_string(nr_ofThreads) + " " + to_string(other.nr_ofThreads);
	if (!(task_id == other.task_id)) return "task_id";

	if (!(conv_l_in_SI == other.conv_l_in_SI)) return "conv_l_in_SI";
	if (!(conv_dH_in_SI == other.conv_dH_in_SI)) return "conv_dH_in_SI";
	if (!(conv_B_in_SI == other.conv_B_in_SI)) return "conv_B_in_SI";
	if (!(conv_V_in_SI == other.conv_V_in_SI)) return "conv_V_in_SI";
	if (!(conv_mass_fraction == other.conv_mass_fraction)) return "conv_mass_fraction";
	if (!(mu == other.mu)) return "mu";

	if (!(min_rot_angle_1 == other.min_rot_angle_1)) return "min_rot_angle_1";
	if (!(max_rot_angle_1 == other.max_rot_angle_1)) return "max_rot_angle_1";
	if (!(min_rot_angle_2 == other.min_rot_angle_2)) return "min_rot_angle_2";
	if (!(max_rot_angle_2 == other.max_rot_angle_2)) return "max_rot_angle_2";

	if (!(min_sidelength_x == other.min_sidelength_x)) return "min_sidelength_x";
	if (!(max_sidelength_x == other.max_sidelength_x)) return "max_sidelength_x";
	if (!(min_sidelength_y == other.min_sidelength_y)) return "min_sidelength_y";
	if (!(max_sidelength_y == other.max_sidelength_y)) return "max_sidelength_y";
	if (!(use_grid_sidelength_x == other.use_grid_sidelength_x)) return "use_grid_sidelength_x";
	if (!(use_grid_sidelength_y == other.use_grid_sidelength_y)) return "use_grid_sidelength_y";

	if (!(min_ray_map_shift_x == other.min_ray_map_shift_x)) return "min_ray_map_shift_x";
	if (!(min_ray_map_shift_y == other.min_ray_map_shift_y)) return "min_ray_map_shift_y";
	if (!(max_ray_map_shift_x == other.max_ray_map_shift_x)) return "max_ray_map_shift_x";
	if (!(max_ray_map_shift_y == other.max_ray_map_shift_y)) return "max_ray_map_shift_y";

	if (!(align == other.align)) return "align";
	if (!(min_detector_pixel_x == other.min_detector_pixel_x)) return "min_detector_pixel_x";
	if (!(max_detector_pixel_x == other.max_detector_pixel_x)) return "max_detector_pixel_x";
	if (!(min_detector_pixel_y == other.min_detector_pixel_y)) return "min_detector_pixel_y";
	if (!(max_detector_pixel_y == other.max_detector_pixel_y)) return "max_detector_pixel_y";
	if (!(nr_ofInpAMIRAPoints == other.nr_ofInpAMIRAPoints)) return "nr_ofInpAMIRAPoints";
	if (!(nr_ofOutAMIRAPoints == other.nr_ofOutAMIRAPoints)) return "nr_ofOutAMIRAPoints";
	if (!(nr_ofInpMidDataPoints == other.nr_ofInpMidDataPoints)) return "nr_ofInpMidDataPoints";
	if (!(nr_ofOutMidDataPoints == other.nr_ofOutMidDataPoints)) return "nr_ofOutMidDataPoints";
	if (!(midplane_zoom == other.midplane_zoom)) return "midplane_zoom";
	if (!(max_dust_component_choice == other.max_dust_component_choice)) return "max_dust_component_choice";

	if (!(plot_inp_points == other.plot_inp_points)) return "plot_inp_points";
	if (!(plot_out_points == other.plot_out_points)) return "plot_out_points";
	if (!(write_radiation_field == other.write_radiation_field)) return "write_radiation_field";
	if (!(write_g_zero == other.write_g_zero)) return "write_g_zero";
	if (!(write_dust_files == other.write_dust_files)) return "write_dust_files";

	if (!(midplane_3d_param == other.midplane_3d_param)) return "midplane_3d_param";
	if (!(star_mass == other.star_mass)) return "star_mass";

	if (!(plot_list == other.plot_list)) return "plot_list";

	if (!(b_mrw == other.b_mrw)) return "b_mrw";
	if (!(b_pda == other.b_pda)) return "b_pda";
	if (!(b_enforced == other.b_enforced)) return "b_enforced";
	if (!(is_speed_of_sound == other.is_speed_of_sound)) return "is_speed_of_sound";
	if (!(peel_off == other.peel_off)) return "peel_off";
	if (!(vel_maps == other.vel_maps)) return "vel_maps";

	if (!(dust_offset == other.dust_offset)) return "dust_offset";
	if (!(dust_gas_coupling == other.dust_gas_coupling)) return "dust_gas_coupling";
	if (!(full_dust_temp == other.full_dust_temp)) return "full_dust_temp";
	if (!(save_radiation_field == other.save_radiation_field)) return "save_radiation_field";
	if (!(scattering_to_raytracing == other.scattering_to_raytracing)) return "scattering_to_raytracing";
	if (!(split_dust_emision == other.split_dust_emision)) return "split_dust_emision";
	if (!(individual_dust_fractions == other.individual_dust_fractions)) return "individual_dust_fractions";

	if (!(zeeman_catalog_path == other.zeeman_catalog_path)) return "zeeman_catalog_path";

	// if (!(phID == other.phID)) return "phID";
	if (!(phIDs == other.phIDs)) return "phIDs";

	if (!(min_obs_distance == other.min_obs_distance)) return "min_obs_distance";
	if (!(max_obs_distance == other.max_obs_distance)) return "max_obs_distance";
	if (!(kepler_star_mass == other.kepler_star_mass)) return "kepler_star_mass";
	if (!(turbulent_velocity == other.turbulent_velocity)) return "turbulent_velocity";
	if (!(stochastic_heating_max_size == other.stochastic_heating_max_size)) return "stochastic_heating_max_size";
	if (!(delta0 == other.delta0)) return "delta0";
	if (!(larm_f == other.larm_f)) return "larm_f";
	if (!(acceptance_angle == other.acceptance_angle)) return "acceptance_angle";
	if (!(offset_min_gas_dens == other.offset_min_gas_dens)) return "offset_min_gas_dens";

	if (!(extinction_magnitude == other.extinction_magnitude)) return "extinction_magnitude";
	if (!(extinction_magnitude_wavelength == other.extinction_magnitude_wavelength)) return "extinction_magnitude_wavelength";
	if (!(extinction_i_mixture == other.extinction_i_mixture)) return "extinction_i_mixture";

	if (!(nrOfPlotPoints == other.nrOfPlotPoints)) return "nrOfPlotPoints";
	if (!(nrOfPlotVectors == other.nrOfPlotVectors)) return "nrOfPlotVectors";
	if (!(maxPlotLines == other.maxPlotLines)) return "maxPlotLines";

	if (!(nr_of_mc_lvl_pop_photons == other.nr_of_mc_lvl_pop_photons)) return "nr_of_mc_lvl_pop_photons";
	if (!(mc_lvl_pop_seed == other.mc_lvl_pop_seed)) return "mc_lvl_pop_seed";

	if (!(healpix_orientation == other.healpix_orientation)) return "healpix_orientation";

	if (!(line_ray_detector_list == other.line_ray_detector_list)) return "line_ray_detector_list";

	if (!(rt_grid_description == other.rt_grid_description)) return "rt_grid_description";

	if (!(dust_mc_detectors == other.dust_mc_detectors)) return "dust_mc_detectors";
	if (!(dust_ray_detectors == other.dust_ray_detectors)) return "dust_ray_detectors";
	if (!(sync_ray_detectors == other.sync_ray_detectors)) return "sync_ray_detectors";

	if (!(point_sources == other.point_sources)) return "point_sources";
	if (!(diffuse_sources == other.diffuse_sources)) return "diffuse_sources";
	if (!(laser_sources == other.laser_sources)) return "laser_sources";
	if (!(background_sources == other.background_sources)) return "background_sources";
	if (!(gas_species_abundance == other.gas_species_abundance)) return "gas_species_abundance";

	if (!(gas_species_level_pop_type == other.gas_species_level_pop_type)) return "gas_species_level_pop_type";

	if (!(gas_species_cat_path == other.gas_species_cat_path)) return "gas_species_cat_path";
	if (!(point_sources_str == other.point_sources_str)) return "point_sources_str";
	if (!(diffuse_sources_str == other.diffuse_sources_str)) return "diffuse_sources_str";
	if (!(background_sources_path == other.background_sources_path)) return "background_sources_path";
	if (!(isrf_path == other.isrf_path)) return "isrf_path: " + isrf_path + " << " + other.isrf_path;

	if (!(xymin == other.xymin)) return "xymin";
	if (!(xymax == other.xymax)) return "xymax";
	if (!(xysteps == other.xysteps)) return "xysteps";
	if (!(xy_bins == other.xy_bins)) return "xy_bins";
	if (!(xylabel == other.xylabel)) return "xylabel";
	if (!(autoscale == other.autoscale)) return "autoscale";
	if (!(sublimate == other.sublimate)) return "sublimate";

	if (!(axis1 == other.axis1)) return "axis1";
	if (!(axis2 == other.axis2)) return "axis2";


	if (!(f_highJ == other.f_highJ)) return "f_highJ";
	if (!(f_cor == other.f_cor)) return "f_cor";
	if (!(Q_ref == other.Q_ref)) return "Q_ref";
	if (!(alpha_Q == other.alpha_Q)) return "alpha_Q";
	if (!(R_rayleigh == other.R_rayleigh)) return "R_rayleigh";

	if (!(adjTgas == other.adjTgas)) return "adjTgas";
	if (!(isrf_g_zero == other.isrf_g_zero)) return "isrf_g_zero";
	if (!(isrf_radius == other.isrf_radius)) return "isrf_radius";

	if (!(nr_ofISRFPhotons == other.nr_ofISRFPhotons)) return "nr_ofISRFPhotons";
	if (!(nr_ofDustPhotons == other.nr_ofDustPhotons)) return "nr_ofDustPhotons";

	if (!(dust_fractions == other.dust_fractions)) return "dust_fractions";
	if (!(material_density == other.material_density)) return "material_density";
	if (!(a_min_global == other.a_min_global)) return "a_min_global";
	if (!(a_max_global == other.a_max_global)) return "a_max_global";
	if (!(size_parameter_map == other.size_parameter_map)) return "size_parameter_map";
	if (!(dust_choices == other.dust_choices)) return "dust_choices";
	if (!(component_id_to_choice == other.component_id_to_choice)) return "component_id_to_choice";

	if (!(dust_paths == other.dust_paths)) return "dust_paths";
	if (!(size_keywords == other.size_keywords)) return "size_keywords";

	if (!(reset_dust_files == other.reset_dust_files)) return "reset_dust_files";

	// opiate parameters
	if (!(opiate_ray_detectors == other.opiate_ray_detectors)) return "opiate_ray_detectors";
	if (!(opiate_spec_ids == other.opiate_spec_ids)) return "opiate_spec_ids";
	if (!(opiata_path_emi == other.opiata_path_emi)) return "opiata_path_emi";
	if (!(opiata_path_abs == other.opiata_path_abs)) return "opiata_path_abs";

	return "equal";
    }

    bool operator==(const parameters& other) const {
	if (!(cmd == other.cmd)) return false;
	if (!(path_grid == other.path_grid)) return false;
	if (!(path_input == other.path_input)) return false;
	if (!(path_output == other.path_output)) return false;

	if (!(max_subpixel_lvl == other.max_subpixel_lvl)) return false;
	if (!(nr_ofThreads == other.nr_ofThreads)) return false;
	if (!(task_id == other.task_id)) return false;

	if (!(conv_l_in_SI == other.conv_l_in_SI)) return false;
	if (!(conv_dH_in_SI == other.conv_dH_in_SI)) return false;
	if (!(conv_B_in_SI == other.conv_B_in_SI)) return false;
	if (!(conv_V_in_SI == other.conv_V_in_SI)) return false;
	if (!(conv_mass_fraction == other.conv_mass_fraction)) return false;
	if (!(mu == other.mu)) return false;

	if (!(min_rot_angle_1 == other.min_rot_angle_1)) return false;
	if (!(max_rot_angle_1 == other.max_rot_angle_1)) return false;
	if (!(min_rot_angle_2 == other.min_rot_angle_2)) return false;
	if (!(max_rot_angle_2 == other.max_rot_angle_2)) return false;

	if (!(min_sidelength_x == other.min_sidelength_x)) return false;
	if (!(max_sidelength_x == other.max_sidelength_x)) return false;
	if (!(min_sidelength_y == other.min_sidelength_y)) return false;
	if (!(max_sidelength_y == other.max_sidelength_y)) return false;
	if (!(use_grid_sidelength_x == other.use_grid_sidelength_x)) return false;
	if (!(use_grid_sidelength_y == other.use_grid_sidelength_y)) return false;

	if (!(min_ray_map_shift_x == other.min_ray_map_shift_x)) return false;
	if (!(min_ray_map_shift_y == other.min_ray_map_shift_y)) return false;
	if (!(max_ray_map_shift_x == other.max_ray_map_shift_x)) return false;
	if (!(max_ray_map_shift_y == other.max_ray_map_shift_y)) return false;

	if (!(align == other.align)) return false;
	if (!(min_detector_pixel_x == other.min_detector_pixel_x)) return false;
	if (!(max_detector_pixel_x == other.max_detector_pixel_x)) return false;
	if (!(min_detector_pixel_y == other.min_detector_pixel_y)) return false;
	if (!(max_detector_pixel_y == other.max_detector_pixel_y)) return false;
	if (!(nr_ofInpAMIRAPoints == other.nr_ofInpAMIRAPoints)) return false;
	if (!(nr_ofOutAMIRAPoints == other.nr_ofOutAMIRAPoints)) return false;
	if (!(nr_ofInpMidDataPoints == other.nr_ofInpMidDataPoints)) return false;
	if (!(nr_ofOutMidDataPoints == other.nr_ofOutMidDataPoints)) return false;
	if (!(midplane_zoom == other.midplane_zoom)) return false;
	if (!(max_dust_component_choice == other.max_dust_component_choice)) return false;

	if (!(plot_inp_points == other.plot_inp_points)) return false;
	if (!(plot_out_points == other.plot_out_points)) return false;
	if (!(write_radiation_field == other.write_radiation_field)) return false;
	if (!(write_g_zero == other.write_g_zero)) return false;
	if (!(write_dust_files == other.write_dust_files)) return false;

	if (!(midplane_3d_param == other.midplane_3d_param)) return false;
	if (!(star_mass == other.star_mass)) return false;

	if (!(plot_list == other.plot_list)) return false;

	if (!(b_mrw == other.b_mrw)) return false;
	if (!(b_pda == other.b_pda)) return false;
	if (!(b_enforced == other.b_enforced)) return false;
	if (!(is_speed_of_sound == other.is_speed_of_sound)) return false;
	if (!(peel_off == other.peel_off)) return false;
	if (!(vel_maps == other.vel_maps)) return false;

	if (!(dust_offset == other.dust_offset)) return false;
	if (!(dust_gas_coupling == other.dust_gas_coupling)) return false;
	if (!(full_dust_temp == other.full_dust_temp)) return false;
	if (!(save_radiation_field == other.save_radiation_field)) return false;
	if (!(scattering_to_raytracing == other.scattering_to_raytracing)) return false;
	if (!(split_dust_emision == other.split_dust_emision)) return false;
	if (!(individual_dust_fractions == other.individual_dust_fractions)) return false;

	if (!(zeeman_catalog_path == other.zeeman_catalog_path)) return false;

	// if (!(phID == other.phID)) return false;
	if (!(phIDs == other.phIDs)) return false;

	if (!(min_obs_distance == other.min_obs_distance)) return false;
	if (!(max_obs_distance == other.max_obs_distance)) return false;
	if (!(kepler_star_mass == other.kepler_star_mass)) return false;
	if (!(turbulent_velocity == other.turbulent_velocity)) return false;
	if (!(stochastic_heating_max_size == other.stochastic_heating_max_size)) return false;
	if (!(delta0 == other.delta0)) return false;
	if (!(larm_f == other.larm_f)) return false;
	if (!(acceptance_angle == other.acceptance_angle)) return false;
	if (!(offset_min_gas_dens == other.offset_min_gas_dens)) return false;

	if (!(extinction_magnitude == other.extinction_magnitude)) return false;
	if (!(extinction_magnitude_wavelength == other.extinction_magnitude_wavelength)) return false;
	if (!(extinction_i_mixture == other.extinction_i_mixture)) return false;

	if (!(nrOfPlotPoints == other.nrOfPlotPoints)) return false;
	if (!(nrOfPlotVectors == other.nrOfPlotVectors)) return false;
	if (!(maxPlotLines == other.maxPlotLines)) return false;

	if (!(nr_of_mc_lvl_pop_photons == other.nr_of_mc_lvl_pop_photons)) return false;
	if (!(mc_lvl_pop_seed == other.mc_lvl_pop_seed)) return false;

	if (!(healpix_orientation == other.healpix_orientation)) return false;

	if (!(line_ray_detector_list == other.line_ray_detector_list)) return false;

	if (!(rt_grid_description == other.rt_grid_description)) return false;

	if (!(dust_mc_detectors == other.dust_mc_detectors)) return false;
	if (!(dust_ray_detectors == other.dust_ray_detectors)) return false;
	if (!(sync_ray_detectors == other.sync_ray_detectors)) return false;

	if (!(point_sources == other.point_sources)) return false;
	if (!(diffuse_sources == other.diffuse_sources)) return false;
	if (!(laser_sources == other.laser_sources)) return false;
	if (!(background_sources == other.background_sources)) return false;
	if (!(gas_species_abundance == other.gas_species_abundance)) return false;

	if (!(gas_species_level_pop_type == other.gas_species_level_pop_type)) return false;

	if (!(gas_species_cat_path == other.gas_species_cat_path)) return false;
	if (!(point_sources_str == other.point_sources_str)) return false;
	if (!(diffuse_sources_str == other.diffuse_sources_str)) return false;
	if (!(background_sources_path == other.background_sources_path)) return false;
	if (!(isrf_path == other.isrf_path)) return false;

	if (!(xymin == other.xymin)) return false;
	if (!(xymax == other.xymax)) return false;
	if (!(xysteps == other.xysteps)) return false;
	if (!(xy_bins == other.xy_bins)) return false;
	if (!(xylabel == other.xylabel)) return false;
	if (!(autoscale == other.autoscale)) return false;
	if (!(sublimate == other.sublimate)) return false;

	if (!(axis1 == other.axis1)) return false;
	if (!(axis2 == other.axis2)) return false;

	if (!(f_highJ == other.f_highJ)) return false;
	if (!(f_cor == other.f_cor)) return false;
	if (!(Q_ref == other.Q_ref)) return false;
	if (!(alpha_Q == other.alpha_Q)) return false;
	if (!(R_rayleigh == other.R_rayleigh)) return false;

	if (!(adjTgas == other.adjTgas)) return false;
	if (!(isrf_g_zero == other.isrf_g_zero)) return false;
	if (!(isrf_radius == other.isrf_radius)) return false;

	if (!(nr_ofISRFPhotons == other.nr_ofISRFPhotons)) return false;
	if (!(nr_ofDustPhotons == other.nr_ofDustPhotons)) return false;

	if (!(dust_fractions == other.dust_fractions)) return false;
	if (!(material_density == other.material_density)) return false;
	if (!(a_min_global == other.a_min_global)) return false;
	if (!(a_max_global == other.a_max_global)) return false;
	if (!(size_parameter_map == other.size_parameter_map)) return false;
	if (!(dust_choices == other.dust_choices)) return false;
	if (!(component_id_to_choice == other.component_id_to_choice)) return false;

	if (!(dust_paths == other.dust_paths)) return false;
	if (!(size_keywords == other.size_keywords)) return false;

	if (!(reset_dust_files == other.reset_dust_files)) return false;

	// opiate parameters
	if (!(opiate_ray_detectors == other.opiate_ray_detectors)) return false;
	if (!(opiate_spec_ids == other.opiate_spec_ids)) return false;
	if (!(opiata_path_emi == other.opiata_path_emi)) return false;
	if (!(opiata_path_abs == other.opiata_path_abs)) return false;

	return true;
    }

    class plot_parameter
    {
    public:
        plot_parameter()
        {
            label = "";
            abs_min_cut = double(MAX_UINT);
            abs_max_cut = double(MAX_UINT);
            rel_min_cut = double(MAX_UINT);
            rel_max_cut = double(MAX_UINT);
            int_cut = 0;
            log = false;
            plot = true;
            normalized = true;
            scale = 1;
            offset = 0;

            pixel_bins = MAX_UINT;
            vec_bins = 26;
            vec_color[0] = 255;
            vec_color[1] = 255;
            vec_color[2] = 255;
        }

        ~plot_parameter()
        {}

        void addColorBarColor(double pos, double R, double G, double B);

        void addContourLine(double val, double R, double G, double B);

        void setVectorColor(uchar R, uchar G, uchar B);

        string label;
        dlist cbar, cline;
        double abs_min_cut;
        double abs_max_cut;

        double rel_min_cut;
        double rel_max_cut;

        double int_cut;

        bool log;
        bool plot;
        bool normalized;

        double scale;
        double offset;

        uint vec_bins;
        uint pixel_bins;
        uchar vec_color[3];
    };

private:
    int cmd;
    string path_grid;
    string path_input;
    string path_output;

    uint max_subpixel_lvl;
    uint nr_ofThreads;
    uint task_id;

    double conv_l_in_SI;
    double conv_dH_in_SI;
    double conv_B_in_SI;
    double conv_V_in_SI;
    double conv_mass_fraction;
    double mu;

    double min_rot_angle_1, max_rot_angle_1;
    double min_rot_angle_2, max_rot_angle_2;

    double min_sidelength_x, max_sidelength_x;
    double min_sidelength_y, max_sidelength_y;
    bool use_grid_sidelength_x, use_grid_sidelength_y;

    double min_ray_map_shift_x, min_ray_map_shift_y;
    double max_ray_map_shift_x, max_ray_map_shift_y;

    uint align;
    uint min_detector_pixel_x, max_detector_pixel_x;
    uint min_detector_pixel_y, max_detector_pixel_y;
    uint nr_ofInpAMIRAPoints;
    uint nr_ofOutAMIRAPoints;
    uint nr_ofInpMidDataPoints;
    uint nr_ofOutMidDataPoints;
    uint midplane_zoom;
    uint max_dust_component_choice;

    bool plot_inp_points;
    bool plot_out_points;
    uint write_radiation_field;
    bool write_g_zero;
    bool write_dust_files;

    dlist midplane_3d_param;
    dlist star_mass;

    uilist plot_list;

    bool b_mrw;
    bool b_pda;
    bool b_enforced;
    bool is_speed_of_sound;
    bool peel_off;
    bool vel_maps;

    bool dust_offset, dust_gas_coupling;
    bool full_dust_temp, save_radiation_field;
    bool scattering_to_raytracing;
    bool split_dust_emision;
    bool individual_dust_fractions;

    strlist zeeman_catalog_path;

    // uint phID;
    uilist phIDs;

    double min_obs_distance, max_obs_distance;
    double kepler_star_mass, turbulent_velocity;
    double stochastic_heating_max_size;
    double delta0;
    double larm_f;
    double acceptance_angle;
    double offset_min_gas_dens;

    double extinction_magnitude;
    double extinction_magnitude_wavelength;
    uint extinction_i_mixture;

    uint nrOfPlotPoints;
    uint nrOfPlotVectors;
    uint maxPlotLines;

    uint nr_of_mc_lvl_pop_photons;
    uint mc_lvl_pop_seed;

    uint healpix_orientation;

    maplist line_ray_detector_list;

    string rt_grid_description;

    dlist dust_mc_detectors;
    dlist dust_ray_detectors;
    dlist sync_ray_detectors;

    dlist point_sources;
    dlist diffuse_sources;
    dlist laser_sources;
    dlist background_sources;
    dlist gas_species_abundance;

    uilist gas_species_level_pop_type;

    strlist gas_species_cat_path;
    strlist point_sources_str;
    strlist diffuse_sources_str;
    strlist background_sources_path;
    string isrf_path;

    double xymin, xymax, xysteps;
    uint xy_bins;
    string xylabel;
    bool autoscale;
    bool sublimate;

    Vector3D axis1, axis2;

    uint start;
    uint stop;

    double f_highJ;
    double f_cor;
    double Q_ref;
    double alpha_Q;
    double R_rayleigh;

    double adjTgas;
    double isrf_g_zero;
    double isrf_radius;

    ullong nr_ofISRFPhotons;
    ullong nr_ofDustPhotons;

    dlist dust_fractions;
    dlist material_density;
    dlist a_min_global;
    dlist a_max_global;
    maplist size_parameter_map;
    uilist dust_choices;
    uilist component_id_to_choice;

    strlist dust_paths;
    strlist size_keywords;

    bool reset_dust_files;

    // opiate parameters
    dlist opiate_ray_detectors;
    strlist opiate_spec_ids;
    string opiata_path_emi;
    string opiata_path_abs;
};

#endif /* PARAMETERS_H */
