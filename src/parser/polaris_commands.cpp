#include "polaris_commands.hpp"
#include "Matrix2D.hpp"
#include "Vector3D.hpp"

#include <string>
#include <functional>

namespace rewrite {

    auto cmd_cmd(const ParsedLine& line, parameters& param)
	-> std::expected<bool, std::string> {

	constexpr auto commands = std::array {
	    "CMD_TEMP", "CMD_DUST_EMISSION", "CMD_DUST_SCATTERING",
	    "CMD_PROBING", "CMD_RAT", "CMD_TEMP_RAT",
	    "CMD_LINE_EMISSION", "CMD_FORCE", "CMD_OPIATE",
	    "CMD_SYNCHROTRON"
	};

	const auto e = line.get_id(0);

	if (!e.has_value())
	    return std::unexpected{ e.error() };

	auto res = std::ranges::find(commands, e.value());
	if (res == commands.end())
	    return std::unexpected{"Command cannot be recognized!"};

	param.setCommand(std::distance(commands.begin(), res));

	return true;
    }


    auto cmd_delta0(const ParsedLine& line, parameters& param)
	-> std::expected<bool, std::string> {
	
	if (const auto e = line.get_num(0); !e)
	    return std::unexpected{e.error()};
	else param.setDelta0(e.value());
	return true;
    }


    auto cmd_larm_f(const ParsedLine& line, parameters& param)
	-> std::expected<bool, std::string> {
	if (const auto e = line.get_num(0); !e)
	    return std::unexpected{e.error()};
	else param.setLarmF(e.value());
	return true;
    }


    auto cmd_plot_list(const ParsedLine& line, parameters& param)
	-> std::expected<bool, std::string> {

	if (line.num_params.empty())
	    return std::unexpected( "List of plot IDs is empty!\nOnly integer values are allowed");

	for (const auto& id: line.num_params) {
	    if (id < minGRID || id > maxGRID)
		return std::unexpected{
		    comp_error( "Unknown grid ID!\n",
			"A plot ID of ", id, " is not a valid POLARIS grid ID (see manual, Table 3.3)!\n")};
	    param.addToPlotList(id);
	}

        return true;
    }

    auto cmd_phase_function(ParsedLine& line, parameters& param)
	-> std::expected<bool, std::string> {

	constexpr auto phfn = std::array {
	    "PH_ISO", "PH_HG", "PH_DHG", "PH_TTHG", "PH_MIE" };

        uint dust_component_choice = 0;
	if (line.named_params.contains("id")) try {
	    dust_component_choice = std::stoul(std::string{line.named_params["id"]});
	}
	catch(...) {
	    return std::unexpected{ "ID parameter could not be converted to number" };
	}

	if (dust_component_choice < 0)
	    return std::unexpected{ comp_error( "ID ", dust_component_choice, " is not valid!") };

	const auto e = line.get_id(0);
	if (!e.has_value())
	    return std::unexpected { e.error() };

	const auto res = std::ranges::find(phfn, e.value());
	if (res == phfn.end())
	    return std::unexpected { "Phase function name could not be recognized!" };

	param.setPhaseFunctionID(std::distance(phfn.begin(), res), dust_component_choice);
	return true;
    }

    auto cmd_star_mass(ParsedLine& line, parameters& param)
	-> std::expected<bool, std::string> {

	for(const auto& mass: line.num_params)
	    param.addStarMass(mass * M_sun);
	return true;
    }

    auto cmd_opiata_path_emi(ParsedLine& line, parameters& param)
	-> std::expected<bool, std::string> {
	if (const auto e = line.get_str(0); !e)
	    return std::unexpected{ e.error() };
	else
	    param.setOpiatePathEmission(std::string{e.value()});
	return true;
    }

    auto cmd_opiata_path_abs(ParsedLine& line, parameters& param)
	-> std::expected<bool, std::string> {
	if (const auto e = line.get_str(0); !e)
	    return std::unexpected{ e.error() };
	else
	    param.setOpiatePathAbsorption(std::string{e.value()});
	return true;
    }

    auto cmd_gas_species(ParsedLine& line, parameters& param)
	-> std::expected<bool, std::string> {

	constexpr auto pop = std::array{
	    "POP_MC", "POP_LTE", "POP_FEP", "POP_LVG", "POP_DEGUCHI_LVG"
	};

	const auto p0 = line.get_str(0);
	if (!p0)
	    return std::unexpected{ p0.error() };

	const std::string gas_species_path{p0.value()};

	// Optional ID Parameter for POP index
	const auto p1 = line.get_id(1);
	if (p1.has_value()) {
	    const auto res_pop = std::ranges::find(pop, p1.value());
	    if (res_pop == pop.end())
		return std::unexpected{ "Unrecognised POP index" };
	    line.num_params.insert(
		line.num_params.begin(),
		std::distance(pop.begin(), res_pop));
	}

	// Optional zeeman path as last parameter
	std::string zeeman_path;
	const auto p3 = line.get_str(3);
	if (p3.has_value())
	    zeeman_path = std::string{p3.value()};

	if (line.num_params.size() != 2)
	    return std::unexpected{ "False amount of parameters for gas species line transfer!" };

	param.addGasSpecies(gas_species_path, zeeman_path, line.num_params);
        return true;
    }


    template<typename Fn>
	requires std::is_invocable_v<Fn, parameters, std::vector<double>&, std::string>
	    || std::is_invocable_v<Fn, parameters, std::vector<double>&>
    auto add_source(
	ParsedLine& line,
	parameters& param,
	Fn add_function,
	const std::string& source_name,
	const size_t nr_of_sources)
	-> std::expected<bool, std::string> {

	constexpr bool with_path = std::is_invocable_v<Fn, parameters, std::vector<double>&, std::string>;

	if (!line.named_params.contains("nr_photons"))
	    return std::unexpected{ "Expected parameter 'nr_photons'" };

	const ullong nr_of_photons = std::stoull(
	    std::string{line.named_params["nr_photons"]});

	if (nr_of_photons <= 0)
	    return std::unexpected{ 
		comp_error("Number of ", source_name, " photons could not be recognized!") };

	std::string ps_path;

	// TODO: consteval?
	if constexpr (with_path) {
	    const bool has_path = !line.str_params.empty();
	    if (has_path) {
		ps_path = line.str_params[0];
		if (line.num_params.size() != nr_of_sources - 5)
		    return std::unexpected{
			comp_error("False amount of parameters for source ", source_name) };
		line.num_params.resize(nr_of_sources - 1, 0);
	    }
	}

	if (line.num_params.size() == nr_of_sources - 3)
	    line.num_params.resize(nr_of_sources - 1, 0);

	if (line.num_params.size() != nr_of_sources - 1)
	    return std::unexpected{
		comp_error("False amount of parameters for source ", source_name)};

	const auto	q = line.num_params[nr_of_sources - 3],
			u = line.num_params[nr_of_sources - 2];
        const auto 	P_l = sqrt(q * q + u * u);

        if(P_l > 1.0)
	    return std::unexpected { "Chosen polarization of source star is larger than 1!" };
        else if(P_l < 0)
	    return std::unexpected{ "Chosen polarization of source is smaller than 0!" };

        line.num_params.push_back(static_cast<double>(nr_of_photons));
	if constexpr (with_path)
	    std::invoke(add_function, param, line.num_params, ps_path);
	else
	    std::invoke(add_function, param, line.num_params);
        return true;
    }
    

    auto cmd_source_star(ParsedLine& line, parameters& param)
	-> std::expected<bool, std::string> {
	return add_source(
	    line, param, &parameters::addPointSource, "star", NR_OF_POINT_SOURCES);
    }

    auto cmd_source_starfield(ParsedLine& line, parameters& param)
	-> std::expected<bool, std::string> {
	
	return add_source(
	    line, param, &parameters::addDiffuseSource, "starfield", NR_OF_DIFF_SOURCES);
    }

    // TODO very strange behavior: completely differs from other sources, params
    // seem in wrong order
    auto cmd_source_background(ParsedLine& line, parameters& param)
	-> std::expected<bool, std::string> {
	
	ullong	nr_of_photons = 0;
	if (line.named_params.contains("nr_photons"))
	    nr_of_photons = std::stoull(std::string{line.named_params["nr_photons"]});

	if (nr_of_photons <= 0)
	    return std::unexpected{ "Number of background source photons could not be recognized!" };

	std::string ps_path;
	const bool has_path = !line.str_params.empty();
	if (has_path) {
	    ps_path = line.str_params[0];
	    if (line.num_params.size() == NR_OF_BG_SOURCES - 5)
		param.addBackgroundSource(ps_path, line.num_params);
	    else if (line.num_params.size() < NR_OF_BG_SOURCES - 5)
		param.addBackgroundSource(ps_path);
	    else
		return std::unexpected{ "Wrong number of parameters for background source!" };
	    return true;
	}

	if (line.num_params.size() == NR_OF_BG_SOURCES - 3
	    || line.num_params.size() == NR_OF_BG_SOURCES - 1)
	    line.num_params.insert(line.num_params.begin(), -1);
	
	if (line.num_params.size() == NR_OF_BG_SOURCES - 2)
	    line.num_params.resize(NR_OF_BG_SOURCES, 0);

	if (line.num_params.size() != NR_OF_BG_SOURCES)
	    return std::unexpected{ "Wrong number of parameters for background source!" };

	param.addBackgroundSource(line.num_params);
	return true;
    }


    auto cmd_source_laser(ParsedLine& line, parameters& param)
	-> std::expected<bool, std::string> {
	return add_source(
	    line, param, &parameters::addLaserSource, "laser", NR_OF_LASER_SOURCES);
    }


    auto cmd_axis1(ParsedLine& line, parameters& param)
	-> std::expected<bool, std::string> {
	if (line.num_params.size() != 3)
	    return std::unexpected{ "Values for first axis are not a vector" };
	param.setAxis1(line.num_params[0], line.num_params[1], line.num_params[2]);
	return true;
    }

    auto cmd_axis2(ParsedLine& line, parameters& param)
	-> std::expected<bool, std::string> {
	if (line.num_params.size() != 3)
	    return std::unexpected{ "Values for second axis are not a vector" };
	param.setAxis2(line.num_params[0], line.num_params[1], line.num_params[2]);
	return true;
    }


    auto cmd_align(ParsedLine& line, parameters& param)
	-> std::expected<bool, std::string> {
	
	constexpr auto alignments = std::array{
	    "ALIG_INTERNAL", "ALIG_PA", "ALIG_IDG", "ALIG_RAT",
	    "ALIG_GOLD", "ALIG_KRAT", "ALIG_NONPA"
	};

	const auto e = line.get_id(0);
	if (!e.has_value())
	    return std::unexpected{ e.error() };

	const auto res = std::ranges::find(alignments, e.value());
	if (res == alignments.end())
	    return std::unexpected{ "Unknown alignment" };

	param.addAlignmentMechanism(1 << std::distance(alignments.begin(), res));
	return true;
    }


    auto cmd_mu(ParsedLine& line, parameters& param)
	-> std::expected<bool, std::string> {
	if (const auto e = line.get_num(0); !e)
	    return std::unexpected{ e.error() };
	else param.setMu(e.value());
	return true;
    }

    auto cmd_xy_min(ParsedLine& line, parameters& param)
	-> std::expected<bool, std::string> {
	if (const auto e = line.get_num(0); !e)
	    return std::unexpected{ e.error() };
	else {
	    param.setXYMin(e.value());
	    param.setAutoScale(false);
	}
	return true;
    }

    auto cmd_xy_max(ParsedLine& line, parameters& param)
	-> std::expected<bool, std::string> {
	if (const auto e = line.get_num(0); !e)
	    return std::unexpected{ e.error() };
	else {
	    param.setXYMax(e.value());
	    param.setAutoScale(false);
	}
	return true;
    }

    auto cmd_xy_steps(ParsedLine& line, parameters& param)
	-> std::expected<bool, std::string> {
	if (const auto e = line.get_num(0); !e)
	    return std::unexpected{ e.error() };
	else {
	    param.setXYSteps(static_cast<uint>(e.value()));
	    param.setAutoScale(false);
	}
	return true;
    }

    auto cmd_xy_bins(ParsedLine& line, parameters& param)
	-> std::expected<bool, std::string> {
	if (const auto e = line.get_num(0); !e)
	    return std::unexpected{ e.error() };
	else param.setXYBins(static_cast<uint>(e.value()));
	return true;
    }

    auto cmd_xy_label(ParsedLine& line, parameters& param)
	-> std::expected<bool, std::string> {
	if (const auto e = line.get_str(0); !e)
	    return std::unexpected{ e.error() };
	else {
	    param.setXYLabel(std::string{e.value()});
	    param.setAutoScale(false);
	}
	return true;
    }

    auto cmd_healpix_orientation(ParsedLine& line, parameters& param)
	-> std::expected<bool, std::string> {
	constexpr auto healpix = std::array{
	    "HEALPIX_FIXED", "HEALPIX_YAXIS", "HEALPIX_CENTER" };

	const auto e = line.get_id(0);
	if (!e)
	    return std::unexpected{ e.error() };
	const auto res = std::ranges::find(healpix, e.value());
	if (res == healpix.end())
	    return std::unexpected{ "Unknown healpix orientation" };
	param.setHealpixOrientation(std::distance(healpix.begin(), res));
	return true;
    }

    auto cmd_path_grid(ParsedLine& line, parameters& param)
	-> std::expected<bool, std::string> {
	if (const auto e = line.get_str(0); !e)
	    return std::unexpected{ e.error() };
	else param.setPathGrid(std::string{e.value()});
	return true;
    }

    auto cmd_path_grid_csg(ParsedLine& line, parameters& param)
	-> std::expected<bool, std::string> {
	if (const auto e = line.get_str(0); !e)
	    return std::unexpected{ e.error() };
	else param.setPathGrid(std::string{e.value()});

        param.updateSIConvDH(1e6);
        param.updateSIConvLength(1e-2);
        param.updateSIConvBField(1e-4);
        param.updateSIConvVField(1e-2);

        return true;
    }


    auto cmd_sub_dust(ParsedLine& line, parameters& param)
	-> std::expected<bool, std::string> {
	if (const auto e = line.get_num(0); !e)
	    return std::unexpected{ e.error() };
	else param.setSublimate(static_cast<bool>(e.value()));
	return true;
    }


    auto cmd_vel_maps(ParsedLine& line, parameters& param)
	-> std::expected<bool, std::string> {
	if (const auto e = line.get_num(0); !e)
	    return std::unexpected{ e.error() };
	else param.setVelMaps(static_cast<bool>(e.value()));
	return true;
    }

    auto cmd_max_subpixel_lvl(ParsedLine& line, parameters& param)
	-> std::expected<bool, std::string> {
	if (const auto e = line.get_num(0); !e)
	    return std::unexpected{ e.error() };
	else param.setMaxSubpixelLvl(static_cast<int>(e.value()))
	return true;
    }

    auto cmd_path_input(ParsedLine& line, parameters& param)
	-> std::expected<bool, std::string> {
	if (const auto e = line.get_str(0); !e)
	    return std::unexpected{ e.error() };
	else param.setPathInput(std::string{e.value()});
	return true;
    }

    auto cmd_dust_component(ParsedLine& line, parameters& param)
	-> std::expected<bool, std::string> {
	
	uint 		dust_component_choice = 0;

	if (line.named_params.contains("id")) try {
	    dust_component_choice = std::stoul(
		std::string{line.named_params["id"]});
	}
	catch(...) {
	    return std::unexpected{ "Invalid dust component ID"};
	}

	const auto 	path_param = line.get_str(0);

	if (!path_param.has_value())
	    return std::unexpected{ "Expected path as first parameter" };

	const std::string path{path_param.value()};

        param.AddDustComponentChoice(dust_component_choice);

	const auto 	sz_keyword_param = line.get_str(1);

	uint 		nr_size_parameter = 0;
	std::string 	size_keyword;

	if (!sz_keyword_param.has_value())
	    size_keyword = "plaw";

	else {
	    size_keyword = sz_keyword_param.value();

	    // TODO make this better
	    if (size_keyword == "plaw") {
		++nr_size_parameter;
		if (size_keyword.contains("-ed"))
		    nr_size_parameter += 3;
		if (size_keyword.contains("-cv"))
		    nr_size_parameter += 3;
	    }
	    else if (size_keyword == "logn")
		nr_size_parameter += 2;
	    else if (size_keyword == "zda")
		nr_size_parameter += 14;
	    else
		return std::unexpected{ "Unknown size distribution keyword" };
	}

	std::vector<double>	size_parameter(NR_OF_SIZE_DIST_PARAM, 0);

	if (nr_size_parameter > 0 && line.num_params.size() == nr_size_parameter + 4) {
	    std::copy(line.num_params.begin() + 4, line.num_params.end(), size_parameter.begin());
	    param.addDustComponent(path, size_keyword,
		line.num_params[0], line.num_params[1], line.num_params[2], line.num_params[3],
		size_parameter);
	    return true;
	}

	if (nr_size_parameter == 0) {
	    double	fr = line.num_params[0], a_min = 0, a_max = 0;

	    switch (line.num_params.size()) {
		case 4:
		    size_parameter[0] = line.num_params[1];
		    a_min = line.num_params[2];
		    a_max = line.num_params[3];
		    break;

		case 2:
		    size_parameter[0] = line.num_params[1];
		    break;

		case 0:
		    fr = 1.0;
		    break;

		default:
		    return std::unexpected{ "Wrong number of parameters" };
	    }

	    param.addDustComponent(path, size_keyword, fr, 0, a_min, a_max, size_parameter);
	    return true;
	}

        return std::unexpected{ "Wrong number of size parameters" };
    }


    auto cmd_path_out(ParsedLine& line, parameters& param)
	-> std::expected<bool, std::string> {

	if (const auto e = line.get_str(0); !e)
	    return std::unexpected{ e.error() };
	else param.setPathOutput(std::string{e.value()});
	return true;
    }


    auto cmd_nr_plot_points(ParsedLine& line, parameters& param)
	-> std::expected<bool, std::string> {

	if (const auto e = line.get_num(0); !e)
	    return std::unexpected{ e.error() };
	else param.setNrOfPlotPoints(static_cast<uint>(e.value()));
	return true;
    }


    auto cmd_nr_plot_vectors(ParsedLine& line, parameters& param)
	-> std::expected<bool, std::string> {

	if (const auto e = line.get_num(0); !e)
	    return std::unexpected{ e.error() };
	else param.setnrOfPlotVectors(static_cast<uint>(e.value()));
	return true;
    }


    auto cmd_f_highJ(ParsedLine& line, parameters& param)
	-> std::expected<bool, std::string> {

	if (const auto e = line.get_num(0); !e)
	    return std::unexpected{ e.error() };
	else param.setFhighJ(e.value());
	return true;
    }


    auto cmd_Q_ref(ParsedLine& line, parameters& param)
	-> std::expected<bool, std::string> {

	if (const auto e = line.get_num(0); !e)
	    return std::unexpected{ e.error() };
	else param.setQref(e.value());
	return true;
    }


    auto cmd_alpha_Q(ParsedLine& line, parameters& param)
	-> std::expected<bool, std::string> {

	if (const auto e = line.get_num(0); !e)
	    return std::unexpected{ e.error() };
	else param.setAlphaQ(e.value());
	return true;
    }


    auto cmd_R_rayleigh(ParsedLine& line, parameters& param)
	-> std::expected<bool, std::string> {

	if (const auto e = line.get_num(0); !e)
	    return std::unexpected{ e.error() };
	else param.setRayleighReductionFactor(e.value());
	return true;
    }


    auto cmd_f_c(ParsedLine& line, parameters& param)
	-> std::expected<bool, std::string> {

	if (const auto e = line.get_num(0); !e)
	    return std::unexpected{ e.error() };
	else param.setFcorr(e.value());
	return true;
    }


    auto cmd_adj_tgs(ParsedLine& line, parameters& param)
	-> std::expected<bool, std::string> {

	if (const auto e = line.get_num(0); !e)
	    return std::unexpected{ e.error() };
	else param.setAdjTgas(e.value());
	return true;
    }


    auto cmd_max_plot_lines(ParsedLine& line, parameters& param)
	-> std::expected<bool, std::string> {

	if (const auto e = line.get_num(0); !e)
	    return std::unexpected{ e.error() };
	else param.setMaxPlotLines(static_cast<uint>(e.value()));
	return true;
    }

    auto cmd_start(ParsedLine& line, parameters& param)
	-> std::expected<bool, std::string> {

	if (const auto e = line.get_num(0); !e)
	    return std::unexpected{ e.error() };
	else param.setStart(static_cast<uint>(e.value()));
	return true;
    }

    auto cmd_stop(ParsedLine& line, parameters& param)
	-> std::expected<bool, std::string> {

	if (const auto e = line.get_num(0); !e)
	    return std::unexpected{ e.error() };
	else param.setStop(static_cast<uint>(e.value()));
	return true;
    }


    auto cmd_cons_dens(ParsedLine& line, parameters& param)
	-> std::expected<bool, std::string> {

	const auto e = line.get_num(0);

	if (!e.has_value())
	    return std::unexpected{ e.error() };

	auto value = e.value();
	if (value < 0) {
	    // TODO we don't like this
	    std::cout << WARNING_LINE << "Negative conversion factors are no longer supported!\n"
	    << "\tGrid must always contain number densities" << endl;
	    value = -value;
	}
	param.updateSIConvDH(value);

	return true;
    }


    auto cmd_conv_len(ParsedLine& line, parameters& param)
	-> std::expected<bool, std::string> {

	if (const auto e = line.get_num(0); !e)
	    return std::unexpected{ e.error() };
	else param.updateSIConvLength(e.value());
	return true;
    }


    auto cmd_conv_mag(ParsedLine& line, parameters& param)
	-> std::expected<bool, std::string> {

	if (const auto e = line.get_num(0); !e)
	    return std::unexpected{ e.error() };
	else param.updateSIConvBField(e.value());
	return true;
    }


    auto cmd_conv_vel(ParsedLine& line, parameters& param)
	-> std::expected<bool, std::string> {

	const auto e = line.get_num(0);

	if (!e.has_value())
	    return std::unexpected{ e.error() };

	double conv = e.value();
	if (conv < 0) {
            cout << WARNING_LINE << "Negative conversion factor are no longer allowed!\n"
		<< "\tThe grid can only contain number densities.\n\n";
	    conv = std::abs(conv);
	}

        param.updateSIConvVField(conv);
	return true;
    }


    auto cmd_mass_fraction(ParsedLine& line, parameters& param)
	-> std::expected<bool, std::string> {
	const auto e = line.get_num(0);

	if (!e.has_value())
	    return std::unexpected{ e.error() };

	const double value = e.value();

	if (value == 0) {
            param.setDustMassFraction(1.0);
            param.setIndividualDustMassFractions(true);
	}
	else
            param.setDustMassFraction(value);

        return true;
    }

    auto cmd_mrw(ParsedLine& line, parameters& param)
	-> std::expected<bool, std::string> {
	const auto e = line.get_num(0);

	if (!e.has_value())
	    return std::unexpected{ e.error() };
	param.setMRW(static_cast<bool>(e.value()));
	std::cout << WARNING_LINE << "MRW currently unavailable!\n";
	return true;
    }


    auto cmd_pda(ParsedLine& line, parameters& param)
	-> std::expected<bool, std::string> {
	const auto e = line.get_num(0);

	if (!e.has_value())
	    return std::unexpected{ e.error() };
	param.setPDA(static_cast<bool>(e.value()));
	std::cout << WARNING_LINE << "PDA currently unavailable!\n";
	return true;
    }


    auto cmd_dust_offset(ParsedLine& line, parameters& param)
	-> std::expected<bool, std::string> {
	const auto e = line.get_num(0);

	if (!e.has_value())
	    return std::unexpected{ e.error() };

	if (e.value() == 0)
	    return true;

	if (!line.named_params.contains("min_gas_density"))
	    param.setDustOffset(true);
	else try {
	    param.setDustOffset(
		std::stod(std::string{line.named_params["min_gas_density"]}));
	}
	catch(...) {
	    return std::unexpected{ "Invalid number for min_gas_density" };
	}

	return true;
    }
    

    auto cmd_gas_coupling(ParsedLine& line, parameters& param)
	-> std::expected<bool, std::string> {

	const auto e = line.get_num(0);

	if (!e.has_value())
	    return std::unexpected{ e.error() };

	if (e.value() == 0)
	    return true;

	if (!line.named_params.contains("min_gas_density"))
	    param.setDustGasCoupling(true);
	else try {
	    param.setDustGasCoupling(
		std::stod(std::string{line.named_params["min_gas_density"]}));
	}
	catch(...) {
	    return std::unexpected{ "Invalid number for min_gas_density" };
	}

	return true;
    }


    auto cmd_radiation_field(ParsedLine& line, parameters& param)
	-> std::expected<bool, std::string> {
	const auto e = line.get_num(0);

	if (!e.has_value())
	    return std::unexpected{ e.error() };
	param.setSaveRadiationField(static_cast<bool>(e.value()));
	return true;
    }


    auto cmd_rt_scattering(ParsedLine& line, parameters& param)
	-> std::expected<bool, std::string> {
	const auto e = line.get_num(0);

	if (!e.has_value())
	    return std::unexpected{ e.error() };
	param.setScatteringToRay(static_cast<bool>(e.value()));
	return true;
    }


    auto cmd_split_dust_emission(ParsedLine& line, parameters& param)
	-> std::expected<bool, std::string> {
	const auto e = line.get_num(0);

	if (!e.has_value())
	    return std::unexpected{ e.error() };
	param.setSplitDustEmission(static_cast<bool>(e.value()));
	return true;
    }


    auto cmd_full_dust_temp(ParsedLine& line, parameters& param)
	-> std::expected<bool, std::string> {
	const auto e = line.get_num(0);

	if (!e.has_value())
	    return std::unexpected{ e.error() };
	param.setFullDustTemp(static_cast<bool>(e.value()));
	return true;
    }


    auto cmd_stochastic_heating(ParsedLine& line, parameters& param)
	-> std::expected<bool, std::string> {
	if (line.num_params.size() != 1 || line.num_params[0] < 0)
            return std::unexpected { "For stochastic heating, a non-negative dust grain size limit needs to be chosen!" };

	param.setStochasticHeatingMaxSize(line.num_params[0]);
	return true;
    }
}
