#include "polaris_commands.hpp"

#include <array>
#include <cstddef>
#include <string>
#include <string_view>
#include <functional>
#include <algorithm>

namespace rewrite {

    // Local to this file
    namespace {
	/**
	 * 
	 */
	struct DetectorRegistration {
	    struct Flags {
		bool		is_healpix;
		bool		with_vel_channels;
		bool		check_wavelength;
	    } flags;

	    struct Param {
		size_t		min_cnt;
		size_t		fill_cnt;
		size_t		check_cnt;
		size_t		add_360_begin;
	    } param;

	    uint		det_type;

	    constexpr DetectorRegistration(Flags _flags, Param _param, uint _det)
		: flags(_flags), param(_param), det_type(_det) {}
	};

	/**
	 *
	 */
	template<DetectorRegistration reg, size_t N>
	t_ret register_detector(
	    ParsedLine& line,
	    std::array<double, N> defaults)
	{
	    using namespace std::literals;
	    
	    const size_t 	sz = line.num_params.size();
	    if (sz < reg.param.min_cnt)
		return std::unexpected{ Message { 
		    comp_error("Too few parameters, expected at least ", reg.param.min_cnt) } };
	    
	    if constexpr (reg.flags.check_wavelength) {
		if (sz < 4) [[unlikely]]
		    return std::unexpected{
			Message { "Expected wavelength parameter at position 3" } };

		if (line.num_params[2] < 1)
		    return std::unexpected {
			Message { "Number of wavelengths needs to be at least 1" } };

		if (line.num_params[2] > 1 && line.num_params[0] == line.num_params[1])
		    return std::unexpected {
			Message { "Minimum and maximum wavelength cannot be the same if the number of wavelengths is larger than 1" } };
	    }

	    const std::string_view pixel_name = reg.flags.is_healpix ?
		"nr_sides"sv : "nr_pixel"sv;

	    if (!line.named_params.contains(pixel_name))
		return std::unexpected{ Message {
		    comp_error("Expected ", pixel_name, " named parameter") } };
	    
	    std::vector<double>	nr_of_channels;
	    std::vector<double>	nr_of_pixel = std::move(line.named_params[pixel_name]);

	    if (nr_of_pixel.empty()
		|| nr_of_pixel.size() > 2
		|| std::ranges::any_of(nr_of_pixel, [](double d) { return d <= 0; }))
		return std::unexpected { Message {
		    std::format("Could not recognize {}", pixel_name) } };

	    // Test if nr_of_pixel is power of 2 for healpix detectors
	    if constexpr (reg.flags.is_healpix) {
		const auto n = static_cast<uint>(nr_of_pixel[0]);
		if((n & (n - 1)) != 0) 
		    return std::unexpected{ Message {
			"Number of sides must be a power of two" } };
	    }

	    if constexpr (reg.flags.with_vel_channels) {
		if (!line.named_params.contains("vel_channels"sv))
		    return std::unexpected{ Message {
			"Expected vel_channels named parameter" } };

		nr_of_channels = std::move(line.named_params["vel_channels"sv]);

		if(nr_of_channels.size() != 1
		    || nr_of_channels[0] <= 0)
		    return std::unexpected { Message {
			"Number of velocity channels could not be recognized!" } };
	    }

	    // TODO: rather "not is_healpix"?
	    if constexpr (reg.param.add_360_begin != 0) {
		constexpr auto a = reg.param.add_360_begin;
		constexpr auto b = a + 1;
		while(line.num_params[a] < 0)
		    line.num_params[a] += 360;
		while(line.num_params[b] < 0)
		    line.num_params[b] += 360;
	    }

	    line.num_params.resize(reg.param.fill_cnt, 0.0);
	    std::copy(
		defaults.begin() + sz - reg.param.min_cnt,
		defaults.end(),
		line.num_params.begin() + sz);

	    if (reg.det_type != DET_MC)
		line.num_params.push_back(reg.det_type);

	    if constexpr (reg.flags.is_healpix)  {
		const uint a = nr_of_pixel[0];
		line.num_params.push_back(a);
		line.num_params.push_back(a);
	    }

	    else {
		const uint a = nr_of_pixel[0];
		const uint b = nr_of_pixel.size() == 2 ? nr_of_pixel[1] : a;

		line.num_params.push_back(a);
		line.num_params.push_back(b);
	    }

	    if constexpr (reg.flags.with_vel_channels)
		line.num_params.push_back(static_cast<uint>(nr_of_channels[0]));

	    if(line.num_params.size() != reg.param.check_cnt) 
		return std::unexpected { Message {
		    "Number of parameters could not be recognized" } };

	    return {};
	}


	/**
	 * Helper function to set one singular numerical parameter with a
	 * setter function of parameters
	 */
	template<typename SetterFn>
	    requires std::is_invocable_v<SetterFn, parameters, double>
	t_ret param_set_number(ParsedLine& line, parameters& param, SetterFn setter)
	{
	    if (const auto e = line.get_num(0); !e)
		return std::unexpected{ e.error() };
	    else std::invoke(setter, param, e.value());
	    return {};
	}


	/**
	 * Helper function to set sources
	 */
	template<typename Fn>
	    requires std::is_invocable_v<Fn, parameters, std::vector<double>&, std::string>
		  || std::is_invocable_v<Fn, parameters, std::vector<double>&>
	t_ret add_source(
	    ParsedLine& line, parameters& param,
	    Fn add_function, const std::string& source_name, const size_t nr_of_sources)
	{
	    using namespace std::literals;

	    constexpr bool with_path = std::is_invocable_v<Fn, parameters, std::vector<double>&, std::string>;

	    if (!line.named_params.contains("nr_photons"sv))
		return std::unexpected{ Message {
		    "Expected parameter 'nr_photons'" } };

	    if (line.named_params["nr_photons"sv].empty())
		return std::unexpected{ Message{ "Named parameter nr_photons not defined" } };

	    const ullong nr_of_photons = line.named_params["nr_photons"sv].at(0);

	    if (nr_of_photons <= 0)
		return std::unexpected{ Message {
		    comp_error("Number of ", source_name, " photons could not be recognized!") } };

	    std::string ps_path;

	    // TODO: consteval?
	    if constexpr (with_path) {
		if (!line.str_params.empty()) {
		    ps_path = line.str_params[0];
		    if (line.num_params.size() != nr_of_sources - 5)
			return std::unexpected{ Message {
			    comp_error("False amount of parameters for source ", source_name, " (expected ", nr_of_sources - 5, ')') } };
		    line.num_params.resize(nr_of_sources - 1, 0);
		}
	    }

	    if (line.num_params.size() == nr_of_sources - 3)
		line.num_params.resize(nr_of_sources - 1, 0);

	    if (line.num_params.size() != nr_of_sources - 1)
		return std::unexpected{ Message {
		    comp_error("False amount of parameters for source ", source_name, " (expected ", nr_of_sources - 3, " or ", nr_of_sources - 1, ')')} };

	    const auto	q = line.num_params[nr_of_sources - 3],
			    u = line.num_params[nr_of_sources - 2];
	    const auto 	P_l = sqrt(q * q + u * u);

	    if (P_l > 1.0)
		return std::unexpected { Message {
		    "Chosen polarization of source star is larger than 1!" } };
	    else if (P_l < 0)
		return std::unexpected { Message {
		    "Chosen polarization of source is smaller than 0!" } };

	    line.num_params.push_back(static_cast<double>(nr_of_photons));

	    if constexpr (with_path)
		std::invoke(add_function, param, line.num_params, ps_path);
	    else
		std::invoke(add_function, param, line.num_params);

	    return {};
	}
    }

    //---------------------------------------------------------------------

    t_ret cmd_cmd(ParsedLine& line, parameters& param) {

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
	    return std::unexpected{ Message { "Command cannot be recognized!" } };

	param.setCommand(std::distance(commands.begin(), res));

	return {};
    }


    t_ret cmd_delta0(ParsedLine& line, parameters& param) {
	return param_set_number(line, param, &parameters::setDelta0);
    }


    t_ret cmd_larm_f(ParsedLine& line, parameters& param) {
	return param_set_number(line, param, &parameters::setLarmF);
    }


    t_ret cmd_plot_list(ParsedLine& line, parameters& param) {

	if (line.num_params.empty())
	    return std::unexpected{ Message{
		"List of plot IDs is empty!\nOnly integer values are allowed"} };

	for (const auto& id: line.num_params) {
	    if (id < minGRID || id > maxGRID)
		return std::unexpected{ Message{
		    std::format(
			"Unknown grid ID!\nA plot Id of {} if not a valid POLARIS grid ID (see manual, Table 3.3)!\n", id ) } };
	    param.addToPlotList(id);
	}

        return {};
    }


    t_ret cmd_phase_function(ParsedLine& line, parameters& param) {
	constexpr auto phfn = std::array {
	    "PH_ISO", "PH_HG", "PH_DHG", "PH_TTHG", "PH_MIE" };

	using namespace std::literals;

        uint dust_component_choice = 0;
	if (line.named_params.contains("id"sv))
	    dust_component_choice = line.named_params["id"sv].at(0);

	if (dust_component_choice < 0)
	    return std::unexpected{ Message {
		std::format( "ID {} is not valid!", dust_component_choice ) } };

	const auto e = line.get_id(0);
	if (!e.has_value())
	    return std::unexpected { e.error() };

	const auto res = std::ranges::find(phfn, e.value());
	if (res == phfn.end())
	    return std::unexpected { Message {
		"Phase function name could not be recognized!" } };

	param.setPhaseFunctionID(std::distance(phfn.begin(), res), dust_component_choice);
	return {};
    }


    t_ret cmd_star_mass(ParsedLine& line, parameters& param) {
	for(const auto& mass: line.num_params)
	    param.addStarMass(mass * M_sun);
	return {};
    }


    t_ret cmd_opiata_path_emi(ParsedLine& line, parameters& param) {
	if (const auto e = line.get_str(0); !e)
	    return std::unexpected{ e.error() };
	else
	    param.setOpiatePathEmission(std::string{e.value()});
	return {};
    }


    t_ret cmd_opiata_path_abs(ParsedLine& line, parameters& param) {
	if (const auto e = line.get_str(0); !e)
	    return std::unexpected{ e.error() };
	else
	    param.setOpiatePathAbsorption(std::string{e.value()});
	return {};
    }


    t_ret cmd_gas_species(ParsedLine& line, parameters& param) {

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
		return std::unexpected{ Message{
		    "Unrecognised POP index" } };

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
	    return std::unexpected{ Message{
		"False amount of parameters for gas species line transfer (expected 2)" } };

	param.addGasSpecies(gas_species_path, zeeman_path, line.num_params);
        return {};
    }


    t_ret cmd_source_star(ParsedLine& line, parameters& param) {
	return add_source(
	    line, param, &parameters::addPointSource, "star", NR_OF_POINT_SOURCES);
    }


    t_ret cmd_source_starfield(ParsedLine& line, parameters& param) {
	return add_source(
	    line, param, &parameters::addDiffuseSource, "starfield", NR_OF_DIFF_SOURCES);
    }


    // TODO very strange behavior: completely differs from other sources, params
    // seem in wrong order
    t_ret cmd_source_background(ParsedLine& line, parameters& param) {

	using namespace std::literals;
	
	ullong	nr_of_photons = 0;
	if (line.named_params.contains("nr_photons"sv))
	    nr_of_photons = line.named_params["nr_photons"sv].at(0);

	if (nr_of_photons <= 0)
	    return std::unexpected{ Message{
		"Number of background source photons could not be recognized!" } };

	std::string ps_path;
	const bool has_path = !line.str_params.empty();
	if (has_path) {
	    ps_path = line.str_params[0];
	    if (line.num_params.size() == NR_OF_BG_SOURCES - 5)
		param.addBackgroundSource(ps_path, line.num_params);
	    else if (line.num_params.size() < NR_OF_BG_SOURCES - 5)
		param.addBackgroundSource(ps_path);
	    else
		return std::unexpected{ Message {
		    "Wrong number of parameters for background source!" } };
	    return {};
	}

	if (line.num_params.size() == NR_OF_BG_SOURCES - 3
	    || line.num_params.size() == NR_OF_BG_SOURCES - 1)
	    line.num_params.insert(line.num_params.begin(), -1);
	
	if (line.num_params.size() == NR_OF_BG_SOURCES - 2)
	    line.num_params.resize(NR_OF_BG_SOURCES, 0);

	if (line.num_params.size() != NR_OF_BG_SOURCES)
	    return std::unexpected{ Message {
		"Wrong number of parameters for background source!" } };

	param.addBackgroundSource(line.num_params);
	return {};
    }


    t_ret cmd_source_laser(ParsedLine& line, parameters& param) {
	return add_source(
	    line, param, &parameters::addLaserSource, "laser", NR_OF_LASER_SOURCES);
    }


    t_ret cmd_axis1(ParsedLine& line, parameters& param) {
	if (line.num_params.size() != 3)
	    return std::unexpected{ Message {
		"Values for first axis are not a vector" } };
	param.setAxis1(line.num_params[0], line.num_params[1], line.num_params[2]);
	return {};
    }


    t_ret cmd_axis2(ParsedLine& line, parameters& param) {
	if (line.num_params.size() != 3)
	    return std::unexpected{ Message {
		"Values for second axis are not a vector" } };
	param.setAxis2(line.num_params[0], line.num_params[1], line.num_params[2]);
	return {};
    }


    t_ret cmd_align(ParsedLine& line, parameters& param) {
	constexpr auto alignments = std::array{
	    "ALIG_INTERNAL", "ALIG_PA", "ALIG_IDG", "ALIG_RAT",
	    "ALIG_GOLD", "ALIG_KRAT", "ALIG_NONPA"
	};

	const auto e = line.get_id(0);
	if (!e.has_value())
	    return std::unexpected{ e.error() };

	const auto res = std::ranges::find(alignments, e.value());
	if (res == alignments.end())
	    return std::unexpected{ Message { "Unknown alignment" } };

	param.addAlignmentMechanism(1 << std::distance(alignments.begin(), res));
	return {};
    }


    t_ret cmd_mu(ParsedLine& line, parameters& param) {
	    return param_set_number(line, param, &parameters::setMu);
    }


    t_ret cmd_xy_min(ParsedLine& line, parameters& param) {
	if (const auto e = line.get_num(0); !e)
	    return std::unexpected{ e.error() };
	else {
	    param.setXYMin(e.value());
	    param.setAutoScale(false);
	}
	return {};
    }


    t_ret cmd_xy_max(ParsedLine& line, parameters& param) {
	if (const auto e = line.get_num(0); !e)
	    return std::unexpected{ e.error() };
	else {
	    param.setXYMax(e.value());
	    param.setAutoScale(false);
	}
	return {};
    }


    t_ret cmd_xy_steps(ParsedLine& line, parameters& param) {
	if (const auto e = line.get_num(0); !e)
	    return std::unexpected{ e.error() };
	else {
	    param.setXYSteps(static_cast<uint>(e.value()));
	    param.setAutoScale(false);
	}
	return {};
    }


    t_ret cmd_xy_bins(ParsedLine& line, parameters& param) {
	return param_set_number(line, param, &parameters::setXYBins);
    }


    t_ret cmd_xy_label(ParsedLine& line, parameters& param) {
	if (const auto e = line.get_str(0); !e)
	    return std::unexpected{ e.error() };
	else {
	    param.setXYLabel(std::string{e.value()});
	    param.setAutoScale(false);
	}
	return {};
    }


    t_ret cmd_healpix_orientation(ParsedLine& line, parameters& param) {
	constexpr auto healpix = std::array{
	    "HEALPIX_FIXED", "HEALPIX_YAXIS", "HEALPIX_CENTER" };

	const auto e = line.get_id(0);
	if (!e)
	    return std::unexpected{ e.error() };
	const auto res = std::ranges::find(healpix, e.value());
	if (res == healpix.end())
	    return std::unexpected{ Message {
		"Unknown healpix orientation" } };
	param.setHealpixOrientation(std::distance(healpix.begin(), res));
	return {};
    }


    t_ret cmd_path_grid(ParsedLine& line, parameters& param) {
	if (const auto e = line.get_str(0); !e)
	    return std::unexpected{ e.error() };
	else param.setPathGrid(std::string{e.value()});
	return {};
    }


    t_ret cmd_path_grid_cgs(ParsedLine& line, parameters& param) {
	if (const auto e = line.get_str(0); !e)
	    return std::unexpected{ e.error() };
	else param.setPathGrid(std::string{e.value()});

        param.updateSIConvDH(1e6);
        param.updateSIConvLength(1e-2);
        param.updateSIConvBField(1e-4);
        param.updateSIConvVField(1e-2);

        return {};
    }


    t_ret cmd_sub_dust(ParsedLine& line, parameters& param) {
	return param_set_number(line, param, &parameters::setSublimate);
    }


    t_ret cmd_vel_maps(ParsedLine& line, parameters& param) {
	return param_set_number(line, param, &parameters::setVelMaps);
    }


    t_ret cmd_max_subpixel_lvl(ParsedLine& line, parameters& param) {
	return param_set_number(line, param, &parameters::setMaxSubpixelLvl);
    }


    t_ret cmd_path_input(ParsedLine& line, parameters& param) {
	if (const auto e = line.get_str(0); !e)
	    return std::unexpected{ e.error() };
	else param.setPathInput(std::string{e.value()});
	return {};
    }


    t_ret cmd_dust_component(ParsedLine& line, parameters& param) {
	uint 		dust_component_choice = 0;

	using namespace std::literals;

	if (line.named_params.contains("id"sv)) try {
	    dust_component_choice = line.named_params["id"sv].at(0);
	}
	catch(...) {
	    return std::unexpected{ Message{ "Invalid dust component ID"} };
	}

	const auto 	path_param = line.get_str(0);

	if (!path_param.has_value())
	    return std::unexpected{ Message{ 
		"Expected path as first parameter" } };

	const std::string 	path{ path_param.value() };
	const auto 		sz_keyword_param = line.get_str(1);

	uint 			nr_size_parameter = 0;
	std::string 		size_keyword;

	if (!sz_keyword_param.has_value())
	    size_keyword = "plaw";

	else {
	    size_keyword = sz_keyword_param.value();

	    // TODO make this better
	    if (size_keyword.contains("plaw")) {
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
		return std::unexpected{ Message{
		    "Unknown size distribution keyword" } };
	}

	std::vector<double>	size_parameter(NR_OF_SIZE_DIST_PARAM, 0);

        param.AddDustComponentChoice(dust_component_choice);

	if (nr_size_parameter > 0 && line.num_params.size() == nr_size_parameter + 4) {
	    std::copy(
		line.num_params.begin() + 4,
		line.num_params.end(),
		size_parameter.begin());

	    param.addDustComponent(path, size_keyword,
		line.num_params[0], line.num_params[1], line.num_params[2], line.num_params[3],
		size_parameter);

	    return {};
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
		    return std::unexpected{ Message{
			"Wrong number of parameters" } };
	    }

	    param.addDustComponent(path, size_keyword, fr, 0, a_min, a_max, size_parameter);
	    return {};
	}

        return std::unexpected{ Message{
	    comp_error("Wrong number of size parameters (expected ", nr_size_parameter, ')') } };
    }


    t_ret cmd_path_out(ParsedLine& line, parameters& param) {
	if (const auto e = line.get_str(0); !e)
	    return std::unexpected{ e.error() };
	else param.setPathOutput(std::string{e.value()});
	return {};
    }


    t_ret cmd_nr_plot_points(ParsedLine& line, parameters& param) {
	return param_set_number(line, param, &parameters::setNrOfPlotPoints);
    }


    t_ret cmd_nr_plot_vectors(ParsedLine& line, parameters& param) {
	return param_set_number(line, param, &parameters::setnrOfPlotVectors);
    }


    t_ret cmd_f_highJ(ParsedLine& line, parameters& param) {
	return param_set_number(line, param, &parameters::setFhighJ);
    }


    t_ret cmd_Q_ref(ParsedLine& line, parameters& param) {
	return param_set_number(line, param, &parameters::setQref);
    }


    t_ret cmd_alpha_Q(ParsedLine& line, parameters& param) {
	return param_set_number(line, param, &parameters::setAlphaQ);
    }


    t_ret cmd_R_rayleigh(ParsedLine& line, parameters& param) {
	return param_set_number(line, param, &parameters::setRayleighReductionFactor);
    }


    t_ret cmd_f_c(ParsedLine& line, parameters& param) {
	return param_set_number(line, param, &parameters::setFcorr);
    }


    t_ret cmd_adj_tgas(ParsedLine& line, parameters& param) {
	return param_set_number(line, param, &parameters::setAdjTgas);
    }


    t_ret cmd_max_plot_lines(ParsedLine& line, parameters& param) {
	return param_set_number(line, param, &parameters::setMaxPlotLines);
    }

    t_ret cmd_start(ParsedLine& line, parameters& param) {
	if (const auto e = line.get_num(0); !e)
	    return std::unexpected{ e.error() };
	else param.setStart(e.value() - 1);
	return {};
    }

    t_ret cmd_stop(ParsedLine& line, parameters& param) {
	if (const auto e = line.get_num(0); !e)
	    return std::unexpected{ e.error() };
	else param.setStop(e.value() - 1);
	return {};
    }

    t_ret cmd_conv_dens(ParsedLine& line, parameters& param) {

	const auto e = line.get_num(0);

	if (!e.has_value())
	    return std::unexpected{ e.error() };

	const auto value = e.value();
	if (value < 0) {
	    param.updateSIConvDH(-value);
	    return std::unexpected { Message{
		"Negative conversion factors are no longer supported!\n\tGrid must always contain number densities",
		Message::Type::Warning } };
	}
	param.updateSIConvDH(value);

	return {};
    }


    t_ret cmd_conv_len(ParsedLine& line, parameters& param) {
	return param_set_number(line, param, &parameters::updateSIConvLength);
    }


    t_ret cmd_conv_mag(ParsedLine& line, parameters& param) {
	return param_set_number(line, param, &parameters::updateSIConvBField);
    }


    t_ret cmd_conv_vel(ParsedLine& line, parameters& param) {

	const auto e = line.get_num(0);

	if (!e.has_value())
	    return std::unexpected{ e.error() };

	const double conv = e.value();
	if (conv < 0) {
	    param.updateSIConvVField(std::abs(conv));
            return std::unexpected{ Message{
		"Negative conversion factor are no longer allowed!\n\tThe grid can only contain number densities.",
		Message::Type::Warning } };
	}

        param.updateSIConvVField(conv);
	return {};
    }


    t_ret cmd_mass_fraction(ParsedLine& line, parameters& param) {
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

        return {};
    }


    t_ret cmd_mrw(ParsedLine& line, parameters& param) {
	const auto res = param_set_number(line, param, &parameters::setMRW);
	return std::unexpected{ Message{
	    "MRW currently unavailable", Message::Type::Warning } };
    }


    t_ret cmd_pda(ParsedLine& line, parameters& param) {
	const auto res = param_set_number(line, param, &parameters::setPDA);
	return std::unexpected{ Message{
	    "PDA currently unavailable", Message::Type::Warning } };
    }


    t_ret cmd_dust_offset(ParsedLine& line, parameters& param) {
	const auto e = line.get_num(0);

	using namespace std::literals;

	if (!e.has_value())
	    return std::unexpected{ e.error() };

	if (e.value() == 0)
	    return {};

	if (!line.named_params.contains("min_gas_density"sv))
	    param.setDustOffset(true);
	else try {
	    param.setDustOffset(line.named_params["min_gas_density"sv].at(0));
	}
	catch(...) {
	    return std::unexpected{ Message{ "Invalid number for min_gas_density" } };
	}

	return {};
    }
    

    t_ret cmd_dust_gas_coupling(ParsedLine& line, parameters& param) {

	using namespace std::literals;

	const auto e = line.get_num(0);

	if (!e.has_value())
	    return std::unexpected{ e.error() };

	if (e.value() == 0)
	    return {};

	if (!line.named_params.contains("min_gas_density"sv))
	    param.setDustGasCoupling(true);
	else try {
	    param.setDustGasCoupling(line.named_params["min_gas_density"sv].at(0));
	}
	catch(...) {
	    return std::unexpected{ Message{ "Invalid number for min_gas_density" } };
	}

	return {};
    }


    t_ret cmd_radiation_field(ParsedLine& line, parameters& param) {
	return param_set_number(line, param, &parameters::setSaveRadiationField);
    }


    t_ret cmd_rt_scattering(ParsedLine& line, parameters& param) {
	return param_set_number(line, param, &parameters::setScatteringToRay);
    }


    t_ret cmd_split_dust_emission(ParsedLine& line, parameters& param) {
	return param_set_number(line, param, &parameters::setSplitDustEmission);
    }


    t_ret cmd_full_dust_temp(ParsedLine& line, parameters& param) {
	return param_set_number(line, param, &parameters::setFullDustTemp);
    }


    t_ret cmd_stochastic_heating(ParsedLine& line, parameters& param) {
	const auto e = line.get_num(0);
	if (!e.has_value())
	    return std::unexpected{ e.error() };

	const auto i = e.value();
	if (i < 0)
            return std::unexpected { Message{
		"For stochastic heating, a non-negative dust grain size limit needs to be chosen!" } };

	param.setStochasticHeatingMaxSize(i);
	return {};
    }


    t_ret cmd_source_dust(ParsedLine& line, parameters& param) {
	using namespace std::literals;

	if (line.named_params.contains("nr_photons"sv)) try {
	    param.setNrOfDustPhotons(line.named_params["nr_photons"sv].at(0));
	    return {};
	}
	catch(...) {
	    return std::unexpected{ Message {
		"Ill formed number of photons" } };
	}

	return std::unexpected{ Message{
	    "Number of photons could not be recognized!" } };
    }


    // TODO: this behaves differently to old method
    t_ret cmd_source_isrf(ParsedLine& line, parameters& param) {
	using namespace std::literals;

	if (!line.named_params.contains("nr_photons"sv))
	    return std::unexpected{ Message {
		"Parameter nr_photons required" } };

	try {
	    param.setNrOfISRFPhotons(line.named_params["nr_photons"sv].at(0));
	}
	catch(...) {
	    return std::unexpected{ Message {
		"Invalid number for nr_photons" } };
	}
	
	const std::string	path{line.get_str(0).value_or("")};
	std::vector<double>	values = {0, 2};
	const size_t		sz = line.num_params.size();

	if (!path.empty())
	    values[1] = sz > 0 ? line.num_params[0] : 2;
	else {
	    std::copy(
		line.num_params.begin(),
		std::min(line.num_params.begin() + 2, line.num_params.end()),
		values.begin());
	}

	if (values[0] < 0 || values[1] < 1)
	    return std::unexpected{ Message {
		"ISRF parameters could not be recognized" } };

	param.setISRF(path, values[0], values[1]);
	return {};
    }
    

    t_ret cmd_foreground_extinction(ParsedLine& line, parameters& param) {

	if (line.num_params.size() < 1 || line.num_params.size() > 3)
	    return std::unexpected{ Message {
		"Wrong number of parameters (expected between 1 and 3)" } };

	std::vector<double>	values{0, 0.55e-6, MAX_UINT};
	std::copy(
	    line.num_params.begin(),
	    std::min(
		line.num_params.begin() + 3,
		line.num_params.end()),
	    values.begin());

	param.setForegroundExtinction(values[0], values[1], values[2]);

	return {};
    }


    t_ret cmd_enfsca(ParsedLine& line, parameters& param) {
	return param_set_number(line, param, &parameters::setEnfScattering);
    }


    t_ret cmd_peel_off(ParsedLine& line, parameters& param) {
	return param_set_number(line, param, &parameters::setPeelOff);
    }


    t_ret cmd_acceptance_angle(ParsedLine& line, parameters& param) {
	const auto e = line.get_num(0);
	if (!e.has_value())
	    return std::unexpected{ e.error() };

	const auto i = e.value();
	if (i <= 0)
	    return std::unexpected{ Message{
		"Acceptance angle must be greater than 0" } };
	param.setAcceptanceAngle(i);
	return {};
    }


    t_ret cmd_nr_threads(ParsedLine& line, parameters& param) {
	const auto e = line.get_num(0);

	if (!e.has_value())
	    return std::unexpected{ Message{ "Expected parameter" } };

        const int max_t = omp_get_max_threads();
	int tr = e.value();

        if(tr == -1)
            tr = max_t;

        if(tr > max_t) {
	    param.setNrOfThreads(max_t);
	    return std::unexpected{ Message {
		comp_error("Max. nr. of threads is:  ", max_t),
		Message::Type::Warning } };
        }

        if(tr <= 0) {
	    param.setNrOfThreads(1);
	    return std::unexpected{ Message {
		"Max. nr. of threads is: 1",
		Message::Type::Warning } };
        }

        param.setNrOfThreads(tr);
        return {};
    }


    t_ret cmd_vel_is_speed_of_sound(ParsedLine& line, parameters& param) {
	return param_set_number(line, param, &parameters::setIsSpeedOfSound);
    }


    t_ret cmd_amira_inp_points(ParsedLine& line, parameters& param) {
	return param_set_number(line, param, &parameters::setInpAMIRAPoints);
    }


    t_ret cmd_amira_out_points(ParsedLine& line, parameters& param) {
	return param_set_number(line, param, &parameters::setOutAMIRAPoints);
    }


    t_ret cmd_plot_inp_midplanes(ParsedLine& line, parameters& param) {
	return param_set_number(line, param, &parameters::setInpMidPlot);
    }


    t_ret cmd_plot_out_midplanes(ParsedLine& line, parameters& param) {
	return param_set_number(line, param, &parameters::setOutMidPlot);
    }


    t_ret cmd_write_3d_midplanes(ParsedLine& line, parameters& param) {

	if (line.num_params.size() < 1 || line.num_params.size() > 4)
	    return std::unexpected{ Message{
		"Wrong number of parameters for 3D midplane files (must be between 1 and 4)" } };

	// values{plane, nr_of_slices, z_min, z_max}
	std::vector<double>	values{0, 0, 0, 0};
	std::copy(
	    line.num_params.begin(),
	    std::min(
		line.num_params.begin() + 4,
		line.num_params.end()),
	    values.begin());

	if (values[0] < 1 || values[0] > 3) // PROJ_XY, PROJ_XZ, PROJ_YZ
	    return std::unexpected{ Message{
		"Param 1 must be larger than 1 and smaller than 3" } };

	if (values[2] > values[3])
	    return std::unexpected{ Message{
		"z_min (param 3) is larger than z_max (param 4)" } };

	param.set3dMidplane(values[0], values[1], values[2], values[3]);
        return {};
    }


    t_ret cmd_write_inp_midplanes(ParsedLine& line, parameters& param) {
	return param_set_number(line, param, &parameters::setInpMidDataPoints);
    }


    t_ret cmd_write_out_midplanes(ParsedLine& line, parameters& param) {
	return param_set_number(line, param, &parameters::setOutMidDataPoints);
    }


    t_ret cmd_write_radiation_field(ParsedLine& line, parameters& param) {
	const auto e = line.get_num(0);

	if (!e.has_value())
	    return std::unexpected{ e.error() };

	const auto val = e.value();
        if(val > 3) {
	    param.setWriteRadiationField(0);
            return std::unexpected{ Message{
		"Command \"<write_radiation_field>\" accepts only parameters between 0 to 3!" } };
	}

	param.setWriteRadiationField(val);
        return {};
    }


    t_ret cmd_write_full_radiation_field(ParsedLine& line, parameters& param) {
        return std::unexpected{ Message{
	    "Command <write_full_radiation_field> is no longer available!",
	    Message::Type::Warning } };
    }


    t_ret cmd_write_g_zero(ParsedLine& line, parameters& param) {
	return param_set_number(line, param, &parameters::setWriteGZero);
    }


    t_ret cmd_write_dust_files(ParsedLine& line, parameters& param) {
	return param_set_number(line, param, &parameters::setWriteDustFiles);
    }


    t_ret cmd_midplane_zoom(ParsedLine& line, parameters& param) {
	return param_set_number(line, param, &parameters::setMidplaneZoom);
    }


    t_ret cmd_kepler_star_mass(ParsedLine& line, parameters& param) {
	return param_set_number(line, param, &parameters::setKeplerStarMass);
    }


    t_ret cmd_turbulent_velocity(ParsedLine& line, parameters& param) {
	return param_set_number(line, param, &parameters::setTurbulentVelocity);
    }


    t_ret cmd_mc_lvl_pop_photons(ParsedLine& line, parameters& param) {
	return param_set_number(line, param, &parameters::setMCLvlPopNrOfPhotons);
    }


    t_ret cmd_mc_lvl_pop_seed(ParsedLine& line, parameters& param) {
	return param_set_number(line, param, &parameters::setMCLvlPopSeed);
    }


    t_ret cmd_detector_opiate(ParsedLine& line, parameters& param) {
	
	constexpr DetectorRegistration	reg{
	    {
		.is_healpix = false,
		.with_vel_channels = true,
		.check_wavelength = false
	    },

	    {
		.min_cnt = NR_OF_OPIATE_DET - 13,
		.fill_cnt = NR_OF_OPIATE_DET - 4,
		.check_cnt = NR_OF_OPIATE_DET,
		.add_360_begin = 2
	    },

	    // DetectorRegistration::PixelName::NrPixel,
	    DET_PLANE
	};

	if (line.str_params.empty())
	    return std::unexpected{ Message{
		"String ID needed" } };
	const std::string str_id{line.str_params[0]};

	if (const auto e = register_detector<reg, 3>(line, {1.0, -1.0, -1.0});
	    not e) return e;

        param.addOpiateRayDetector(line.num_params);
        param.addOpiateSpec(str_id);

        param.updateDetectorAngles(line.num_params[2], line.num_params[3]);
        param.updateObserverDistance(line.num_params[4]);
        param.updateMapSidelength(line.num_params[5], line.num_params[6]);
        param.updateRayGridShift(line.num_params[7], line.num_params[8]);
        param.updateDetectorPixel(
	    static_cast<uint>(line.num_params[NR_OF_OPIATE_DET - 3]), 
	    static_cast<uint>(line.num_params[NR_OF_OPIATE_DET - 2]));

        return {};
    }


    t_ret cmd_detector_opiate_healpix(ParsedLine& line, parameters& param) {

	constexpr DetectorRegistration	reg{
	    {
		.is_healpix = true,
		.with_vel_channels = true,
		.check_wavelength = false
	    },

	    {
		.min_cnt = NR_OF_OPIATE_DET - 12,
		.fill_cnt = NR_OF_OPIATE_DET - 4,
		.check_cnt = NR_OF_OPIATE_DET,
		.add_360_begin = 0
	    },

	    // DetectorRegistration::PixelName::NrPixel,
	    DET_SPHER
	};

	if (line.str_params.empty())
	    return std::unexpected{ Message{
		"String ID needed" } };
	const std::string str_id{line.str_params[0]};

	if (const auto e = register_detector<reg, 4>(line, {-180.0, 180.0, -90.0, 90.0});
	    not e) return e;

        param.addOpiateRayDetector(line.num_params);
        param.addOpiateSpec(str_id);

        param.updateDetectorPixel(static_cast<uint>(line.num_params[line.num_params.size() - 2]), 0);

        const double distance = sqrt(line.num_params[2] * line.num_params[2] + line.num_params[3] * line.num_params[3] + line.num_params[4] * line.num_params[4]);
        param.updateObserverDistance(distance);

        // Showing full sphere coverage
        param.updateDetectorAngles(-90, -180);
        param.updateDetectorAngles(90, 180);

	return {};
    }

    t_ret cmd_detector_line(ParsedLine& line, parameters& param) {

	constexpr DetectorRegistration	reg{
	    {
		.is_healpix = false,
		.with_vel_channels = true,
		.check_wavelength = false
	    },

	    {
		.min_cnt = NR_OF_LINE_DET - 11,
		.fill_cnt = NR_OF_LINE_DET - 3,
		.check_cnt = NR_OF_LINE_DET + 1,
		.add_360_begin = 3
	    },

	    DET_PLANE
	};

	const double dupl = line.num_params.size() > NR_OF_LINE_DET - 10 ?
	    line.num_params[NR_OF_LINE_DET - 10] : -1;
	if (const auto e = register_detector<reg, 3>(line, {1.0, -1.0, dupl});
	    not e) return e;

        param.addLineRayDetector(line.num_params);
        param.updateDetectorAngles(
		line.num_params[4], line.num_params[5]);
        param.updateObserverDistance(line.num_params[6]);
        param.updateMapSidelength(
		line.num_params[7], line.num_params[8]);
        param.updateRayGridShift(
		line.num_params[9], line.num_params[10]);
        param.updateDetectorPixel(
	    static_cast<uint>(line.num_params[NR_OF_LINE_DET - 2]),
	    static_cast<uint>(line.num_params[NR_OF_LINE_DET - 1]));

        return {};
    }


    t_ret cmd_detector_line_healpix(ParsedLine& line, parameters& param) {

	constexpr DetectorRegistration	reg{
	    {
		.is_healpix = true,
		.with_vel_channels = true,
		.check_wavelength = false
	    },

	    {
		.min_cnt = NR_OF_LINE_DET - 10,
		.fill_cnt = NR_OF_LINE_DET - 3,
		.check_cnt = NR_OF_LINE_DET + 1,
		.add_360_begin = 0
	    },

	    DET_SPHER
	};

	if (const auto e = register_detector<reg, 4>(line, { -180.0, 180.0, -90.0, 90.0});
	    not e) return e;

        param.addLineRayDetector(line.num_params);
        param.updateDetectorPixel(static_cast<uint>(line.num_params[line.num_params.size() - 2]), 0);

        double distance = sqrt(line.num_params[4] * line.num_params[4] + line.num_params[5] * line.num_params[5] + line.num_params[6] * line.num_params[6]);
        param.updateObserverDistance(distance);

        // Showing full sphere coverage
        param.updateDetectorAngles(-90, -180);
        param.updateDetectorAngles(90, 180);

        return {};
    }

    t_ret cmd_detector_line_polar(ParsedLine& line, parameters& param) {

	constexpr DetectorRegistration	reg{
	    {
		.is_healpix = false,
		.with_vel_channels = true,
		.check_wavelength = false
	    },

	    {
		.min_cnt = NR_OF_LINE_DET - 11,
		.fill_cnt = NR_OF_LINE_DET - 3,
		.check_cnt = NR_OF_LINE_DET + 1,
		.add_360_begin = 3
	    },

	    DET_POLAR
	};

	const double dupl = line.num_params.size() > NR_OF_LINE_DET - 10 ?
	    line.num_params[NR_OF_LINE_DET - 10] : -1;
	if (const auto e = register_detector<reg, 3>(line, {1.0, -1.0, dupl});
	    not e) return e;

        param.addLineRayDetector(line.num_params);
        param.updateDetectorAngles(line.num_params[4], line.num_params[5]);
        param.updateObserverDistance(line.num_params[6]);
        param.updateMapSidelength(line.num_params[7], line.num_params[8]);
        param.updateDetectorPixel(static_cast<uint>(line.num_params[NR_OF_LINE_DET - 2]), static_cast<uint>(line.num_params[NR_OF_LINE_DET - 1]));

        return {};
    }

    t_ret cmd_detector_line_slice(ParsedLine& line, parameters& param) {

	constexpr DetectorRegistration	reg{
	    {
		.is_healpix = false,
		.with_vel_channels = true,
		.check_wavelength = false
	    },

	    {
		.min_cnt = NR_OF_LINE_DET - 11,
		.fill_cnt = NR_OF_LINE_DET - 3,
		.check_cnt = NR_OF_LINE_DET + 1,
		.add_360_begin = 3
	    },

	    DET_SLICE
	};

	const double dupl = line.num_params.size() > NR_OF_LINE_DET - 10 ?
	    line.num_params[NR_OF_LINE_DET - 10] : -1;
	if (const auto e = register_detector<reg, 3>(line, {1.0, -1.0, dupl});
	    not e) return e;

        param.addLineRayDetector(line.num_params);
        param.updateDetectorAngles(line.num_params[4], line.num_params[5]);
        param.updateObserverDistance(line.num_params[6]);
        param.updateMapSidelength(line.num_params[7], line.num_params[8]);
        param.updateRayGridShift(line.num_params[9], line.num_params[10]);
        param.updateDetectorPixel(static_cast<uint>(line.num_params[NR_OF_LINE_DET - 2]), static_cast<uint>(line.num_params[NR_OF_LINE_DET - 1]));

        return {};
    }


    t_ret cmd_detector_dust(ParsedLine& line, parameters& param) {

	constexpr DetectorRegistration	reg{
	    {
		.is_healpix = false,
		.with_vel_channels = false,
		.check_wavelength = true
	    },

	    {
		.min_cnt = NR_OF_RAY_DET - 9,
		.fill_cnt = NR_OF_RAY_DET - 3,
		.check_cnt = NR_OF_RAY_DET,
		.add_360_begin = 4
	    },

	    DET_PLANE
	};

	const double dupl = line.num_params.size() > NR_OF_RAY_DET - 8 ?
	    line.num_params[NR_OF_RAY_DET - 8] : -1;
	if (const auto e = register_detector<reg, 3>(line, {1.0, -1.0, dupl});
	    not e) return e;

        param.addDustRayDetector(line.num_params);
        param.updateDetectorAngles(line.num_params[4], line.num_params[5]);
        param.updateObserverDistance(line.num_params[6]);
        param.updateMapSidelength(line.num_params[7], line.num_params[8]);
        param.updateRayGridShift(line.num_params[9], line.num_params[10]);
        param.updateDetectorPixel(static_cast<uint>(line.num_params[NR_OF_RAY_DET - 2]), static_cast<uint>(line.num_params[NR_OF_RAY_DET - 1]));

        return {};
    }


    t_ret cmd_detector_dust_healpix(ParsedLine& line, parameters& param) {

	constexpr DetectorRegistration	reg{
	    {
		.is_healpix = true,
		.with_vel_channels = false,
		.check_wavelength = true
	    },

	    {
		.min_cnt = NR_OF_RAY_DET - 8,
		.fill_cnt = NR_OF_RAY_DET - 3,
		.check_cnt = NR_OF_RAY_DET,
		.add_360_begin = 0
	    },

	    DET_SPHER
	};

	if (const auto e = register_detector<reg, 4>(line, { -180.0, 180.0, -90.0, 90.0 });
	    not e) return e;

        param.addDustRayDetector(line.num_params);
        param.updateDetectorPixel(static_cast<uint>(line.num_params[line.num_params.size() - 2]), 0);

        double distance = sqrt(line.num_params[3] * line.num_params[3] + line.num_params[4] * line.num_params[4] + line.num_params[5] * line.num_params[5]);
        param.updateObserverDistance(distance);

        // Showing full sphere coverage
        param.updateDetectorAngles(0, 0);
        param.updateDetectorAngles(180, 360);

        return {};
    }

    t_ret cmd_detector_dust_polar(ParsedLine& line, parameters& param) {

	constexpr DetectorRegistration	reg{
	    {
		.is_healpix = false,
		.with_vel_channels = false,
		.check_wavelength = true
	    },

	    {
		.min_cnt = NR_OF_RAY_DET - 9,
		.fill_cnt = NR_OF_RAY_DET - 3,
		.check_cnt = NR_OF_RAY_DET,
		.add_360_begin = 4
	    },

	    DET_POLAR
	};

	const double dupl = line.num_params.size() > NR_OF_RAY_DET - 8 ?
	    line.num_params[NR_OF_RAY_DET - 8] : -1;
	if (const auto e = register_detector<reg, 3>(line, { 1.0, -1.0, dupl });
	    not e) return e;

        param.addDustRayDetector(line.num_params);
        param.updateDetectorAngles(line.num_params[4], line.num_params[5]);
        param.updateObserverDistance(line.num_params[6]);
        param.updateMapSidelength(line.num_params[7], line.num_params[8]);
        param.updateDetectorPixel(static_cast<uint>(line.num_params[NR_OF_RAY_DET - 2]), static_cast<uint>(line.num_params[NR_OF_RAY_DET - 1]));

        return {};
    }

    t_ret cmd_detector_dust_slice(ParsedLine& line, parameters& param) {

	constexpr DetectorRegistration	reg{
	    {
		.is_healpix = false,
		.with_vel_channels = false,
		.check_wavelength = true
	    },

	    {
		.min_cnt = NR_OF_RAY_DET - 9,
		.fill_cnt = NR_OF_RAY_DET - 3,
		.check_cnt = NR_OF_RAY_DET,
		.add_360_begin = 4
	    },

	    DET_SLICE
	};

	const double dupl = line.num_params.size() > NR_OF_RAY_DET - 8 ?
	    line.num_params[NR_OF_RAY_DET - 8] : -1;
	if (const auto e = register_detector<reg, 3>(line, { 1.0, -1.0, dupl });
	    not e) return e;

        param.addDustRayDetector(line.num_params);
        param.updateDetectorAngles(line.num_params[4], line.num_params[5]);
        param.updateObserverDistance(line.num_params[6]);
        param.updateMapSidelength(line.num_params[7], line.num_params[8]);
        param.updateRayGridShift(line.num_params[9], line.num_params[10]);
        param.updateDetectorPixel(static_cast<uint>(line.num_params[NR_OF_RAY_DET - 2]), static_cast<uint>(line.num_params[NR_OF_RAY_DET - 1]));

	return {};
    }

    t_ret cmd_detector_dust_mc(ParsedLine& line, parameters& param) {

	constexpr DetectorRegistration	reg{
	    {
		.is_healpix = false,
		.with_vel_channels = false,
		.check_wavelength = true
	    },

	    {
		.min_cnt = NR_OF_MC_DET - 6,
		.fill_cnt = NR_OF_MC_DET - 2,
		.check_cnt = NR_OF_MC_DET,
		.add_360_begin = 0
	    },

	    DET_MC
	};

	if (const auto e = register_detector<reg, 2>(line, { -1.0, -1.0 });
	    not e) return e;

        param.addDustMCDetector(line.num_params);
        param.updateDetectorAngles(line.num_params[3], line.num_params[4]);
        param.updateObserverDistance(line.num_params[5]);
        param.updateMapSidelength(line.num_params[6], line.num_params[7]);
        param.updateDetectorPixel(static_cast<uint>(line.num_params[NR_OF_MC_DET - 2]), static_cast<uint>(line.num_params[NR_OF_MC_DET - 1]));

        return {};
    }


    t_ret cmd_detector_sync(ParsedLine& line, parameters& param) {

	constexpr DetectorRegistration	reg{
	    {
		.is_healpix = false,
		.with_vel_channels = false,
		.check_wavelength = true
	    },

	    {
		.min_cnt = NR_OF_RAY_DET - 9,
		.fill_cnt = NR_OF_RAY_DET - 3,
		.check_cnt = NR_OF_RAY_DET,
		.add_360_begin = 4
	    },

	    DET_PLANE
	};

	const double dupl = line.num_params.size() > NR_OF_RAY_DET - 8 ?
	    line.num_params[NR_OF_RAY_DET - 8] : -1;
	if (const auto e = register_detector<reg, 3>(line, { 1.0, -1.0, dupl });
	    not e) return e;

        param.addSyncRayDetector(line.num_params);
        param.updateDetectorAngles(line.num_params[4], line.num_params[5]);
        param.updateObserverDistance(line.num_params[6]);
        param.updateMapSidelength(line.num_params[7], line.num_params[8]);
        param.updateRayGridShift(line.num_params[9], line.num_params[10]);
        param.updateDetectorPixel(static_cast<uint>(line.num_params[NR_OF_RAY_DET - 2]), static_cast<uint>(line.num_params[NR_OF_RAY_DET - 1]));

        return {};
    }


    t_ret cmd_detector_sync_slice(ParsedLine& line, parameters& param) {

	constexpr DetectorRegistration	reg{
	    {
		.is_healpix = false,
		.with_vel_channels = false,
		.check_wavelength = true
	    },

	    {
		.min_cnt = NR_OF_RAY_DET - 9,
		.fill_cnt = NR_OF_RAY_DET - 3,
		.check_cnt = NR_OF_RAY_DET,
		.add_360_begin = 4
	    },

	    DET_SLICE
	};

	const double dupl = line.num_params.size() > NR_OF_RAY_DET - 8 ?
	    line.num_params[NR_OF_RAY_DET - 8] : -1;
	if (const auto e = register_detector<reg, 3>(line, { 1.0, -1.0, dupl });
	    not e) return e;

        param.addSyncRayDetector(line.num_params);
        param.updateDetectorAngles(line.num_params[4], line.num_params[5]);
        param.updateObserverDistance(line.num_params[6]);
        param.updateMapSidelength(line.num_params[7], line.num_params[8]);
        param.updateRayGridShift(line.num_params[9], line.num_params[10]);
        param.updateDetectorPixel(static_cast<uint>(line.num_params[NR_OF_RAY_DET - 2]), static_cast<uint>(line.num_params[NR_OF_RAY_DET - 1]));

        return {};
    }


    t_ret cmd_detector_sync_healpix(ParsedLine& line, parameters& param) {

	constexpr DetectorRegistration	reg{
	    {
		.is_healpix = true,
		.with_vel_channels = false,
		.check_wavelength = true
	    },

	    {
		.min_cnt = NR_OF_RAY_DET - 8,
		.fill_cnt = NR_OF_RAY_DET - 3,
		.check_cnt = NR_OF_RAY_DET,
		.add_360_begin = 0
	    },

	    DET_SPHER
	};

	if (const auto e = register_detector<reg, 4>(line, { -180.0, 180.0, -90.0, 90.0 });
	    not e) return e;

        param.addSyncRayDetector(line.num_params);
	const uint det_pix = line.num_params[line.num_params.size() - 2];
        param.updateDetectorPixel(12 * det_pix * det_pix, 0);

        double distance = sqrt(line.num_params[3] * line.num_params[3] + line.num_params[4] * line.num_params[4] + line.num_params[5] * line.num_params[5]);
        param.updateObserverDistance(distance);

        // Showing full sphere coverage
        param.updateDetectorAngles(0, 0);
        param.updateDetectorAngles(180, 360);

        return {};
    }
}
