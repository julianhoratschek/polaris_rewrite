#include "polaris_commands.hpp"

#include <array>
#include <cstddef>
#include <sstream>
#include <string>
#include <string_view>
#include <functional>
#include <algorithm>
#include <iterator>

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
		    std::format(
			"Too few parameters, expected at least {}", reg.param.min_cnt) } };
	    
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
		    std::format(
			"Expected {} named parameter", pixel_name ) } };
	    
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

	    if (line.num_params.size() != reg.param.check_cnt) 
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
		    std::format(
			"Number of {} photons could not be recognized!", source_name ) } };

	    std::string ps_path;

	    if constexpr (with_path) {
		if (!line.str_params.empty()) {
		    ps_path = line.str_params[0];
		    if (line.num_params.size() != nr_of_sources - 5)
			return std::unexpected{ Message {
			    std::format(
				"False amount of parameters for source {} (expected {})",
				source_name, nr_of_sources - 5 ) } };
		    line.num_params.resize(nr_of_sources - 1, 0);
		}
	    }

	    if (line.num_params.size() == nr_of_sources - 3)
		line.num_params.resize(nr_of_sources - 1, 0);

	    if (line.num_params.size() != nr_of_sources - 1)
		return std::unexpected{ Message {
		    std::format(
			"False amount of parameters for source {} (expected {} or {})",
			source_name, nr_of_sources - 3, nr_of_sources - 1 ) } };

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

	// TODO: constriction to chartypes
	template<typename T, size_t N>
	std::string from_array(const std::array<T, N>& arr) {
	    if (arr.empty())
		return "[]";

	    if (arr.size() == 1)
		return arr.back();

	    std::ostringstream		oss;
	    oss << "[";
	    std::copy(arr.begin(), arr.end() - 1,
		std::ostream_iterator<std::string>(oss, ", "));
	    oss << arr.back() << "]";
	    return oss.str();
	}
    }

    //---------------------------------------------------------------------

    DEFINE_COMMAND(cmd) {

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
	    return std::unexpected{ Message { 
		std::format( 
		    "Unknown command {}"
		    "\n\tPossible values are {}",
		    e.value(), from_array(commands)) } };

	param.setCommand(std::distance(commands.begin(), res));

	return {};
    }


    DEFINE_COMMAND(delta0) {
	return param_set_number(line, param, &parameters::setDelta0);
    }


    DEFINE_COMMAND(larm_f) {
	return param_set_number(line, param, &parameters::setLarmF);
    }


    DEFINE_COMMAND(plot_list) {

	if (line.num_params.empty())
	    return std::unexpected{ Message{
		"List of plot IDs is empty!\nOnly integer values are allowed"} };

	for (const auto& id: line.num_params) {
	    if (id < minGRID || id > maxGRID)
		return std::unexpected{ Message{
		    std::format(
			"Invalid grid ID {} (see manual, Table 3.3)!"
			"\n\tValues should be integers between {} and {}",
			id, minGRID, maxGRID ) } };
	    param.addToPlotList(static_cast<unsigned int>(id));
	}

        return {};
    }


    DEFINE_COMMAND(phase_function) {
	constexpr auto phfn = std::array {
	    "PH_ISO", "PH_HG", "PH_DHG", "PH_TTHG", "PH_MIE" };

	using namespace std::literals;

        uint dust_component_choice = 0;
	if (line.named_params.contains("id"sv))
	    dust_component_choice = line.named_params["id"sv].at(0);

	if (dust_component_choice < 0)
	    return std::unexpected{ Message {
		std::format( 
		    "ID {} is invalid, expected value greater than 0!",
		     dust_component_choice ) } };

	const auto e = line.get_id(0);
	if (!e.has_value())
	    return std::unexpected { e.error() };

	const auto res = std::ranges::find(phfn, e.value());
	if (res == phfn.end())
	    return std::unexpected { Message {
		std::format(
		    "Phase function name {} is not valid"
		    "\n\tPossible values are{}",
		    e.value(), from_array(phfn)) } };

	param.setPhaseFunctionID(
	    std::distance(phfn.begin(), res),
	    dust_component_choice);

	return {};
    }


    DEFINE_COMMAND(star_mass) {
	for(const auto& mass: line.num_params)
	    param.addStarMass(mass * M_sun);
	return {};
    }


    // TODO: Is "opiata" correct? Not "opiate"?
    DEFINE_COMMAND(opiata_path_emi) {
	if (const auto e = line.get_str(0); !e)
	    return std::unexpected{ e.error() };
	else
	    param.setOpiatePathEmission(std::string{e.value()});
	return {};
    }


    DEFINE_COMMAND(opiata_path_abs) {
	if (const auto e = line.get_str(0); !e)
	    return std::unexpected{ e.error() };
	else
	    param.setOpiatePathAbsorption(std::string{e.value()});
	return {};
    }


    DEFINE_COMMAND(gas_species) {
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
		    std::format(
			"Unrecognised POP index"
			"\n\tPossible values are {}",
			from_array(pop)) } };

	    // Replace found ID with number in num_params
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


    DEFINE_COMMAND(source_star) {
	return add_source(
	    line, param, &parameters::addPointSource, "star", NR_OF_POINT_SOURCES);
    }


    DEFINE_COMMAND(source_starfield) {
	return add_source(
	    line, param, &parameters::addDiffuseSource, "starfield", NR_OF_DIFF_SOURCES);
    }


    // TODO very strange behavior: completely differs from other sources, params
    // seem in wrong order
    DEFINE_COMMAND(source_background) {

	using namespace std::literals;
	
	ullong	nr_of_photons = 0;
	if (line.named_params.contains("nr_photons"sv))
	    nr_of_photons = line.named_params["nr_photons"sv].at(0);

	if (nr_of_photons <= 0)
	    return std::unexpected{ Message{
		"Number of background source photons could not be recognized!"
		"\n\tExpected it to be greater than 0"} };

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
		    std::format(
			"Wrong number of parameters for background source!"
			"\n\tMust be smaller than or equal to {}", NR_OF_BG_SOURCES - 5) } };
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


    DEFINE_COMMAND(source_laser) {
	return add_source(
	    line, param, &parameters::addLaserSource, "laser", NR_OF_LASER_SOURCES);
    }


    DEFINE_COMMAND(axis1) {
	if (line.num_params.size() != 3)
	    return std::unexpected{ Message {
		"Values for first axis are not a vector (expected 3 parameters)" } };
	param.setAxis1(line.num_params[0], line.num_params[1], line.num_params[2]);
	return {};
    }


    DEFINE_COMMAND(axis2) {
	if (line.num_params.size() != 3)
	    return std::unexpected{ Message {
		"Values for second axis are not a vector (expected 3 parameters)" } };
	param.setAxis2(line.num_params[0], line.num_params[1], line.num_params[2]);
	return {};
    }


    DEFINE_COMMAND(align) {
	constexpr auto alignments = std::array{
	    "ALIG_INTERNAL", "ALIG_PA", "ALIG_IDG", "ALIG_RAT",
	    "ALIG_GOLD", "ALIG_KRAT", "ALIG_NONPA"
	};

	const auto e = line.get_id(0);
	if (!e.has_value())
	    return std::unexpected{ e.error() };

	const auto res = std::ranges::find(alignments, e.value());
	if (res == alignments.end())
	    return std::unexpected{ Message { 
		std::format(
		    "Unknown alignment"
		    "\n\tPossible values are {}",
		    from_array(alignments)) } };

	param.addAlignmentMechanism(1 << std::distance(alignments.begin(), res));
	return {};
    }


    DEFINE_COMMAND(mu) {
	return param_set_number(line, param, &parameters::setMu);
    }


    DEFINE_COMMAND(xy_min) {
	if (const auto e = line.get_num(0); !e)
	    return std::unexpected{ e.error() };
	else {
	    param.setXYMin(e.value());
	    param.setAutoScale(false);
	}
	return {};
    }


    DEFINE_COMMAND(xy_max) {
	if (const auto e = line.get_num(0); !e)
	    return std::unexpected{ e.error() };
	else {
	    param.setXYMax(e.value());
	    param.setAutoScale(false);
	}
	return {};
    }


    DEFINE_COMMAND(xy_steps) {
	if (const auto e = line.get_num(0); !e)
	    return std::unexpected{ e.error() };
	else {
	    param.setXYSteps(static_cast<uint>(e.value()));
	    param.setAutoScale(false);
	}
	return {};
    }


    DEFINE_COMMAND(xy_bins) {
	return param_set_number(line, param, &parameters::setXYBins);
    }


    DEFINE_COMMAND(xy_label) {
	if (const auto e = line.get_str(0); !e)
	    return std::unexpected{ e.error() };
	else {
	    param.setXYLabel(std::string{e.value()});
	    param.setAutoScale(false);
	}
	return {};
    }


    DEFINE_COMMAND(healpix_orientation) {
	constexpr auto healpix = std::array{
	    "HEALPIX_FIXED", "HEALPIX_YAXIS", "HEALPIX_CENTER" };

	const auto e = line.get_id(0);
	if (!e)
	    return std::unexpected{ e.error() };

	const auto res = std::ranges::find(healpix, e.value());
	if (res == healpix.end())
	    return std::unexpected{ Message {
		std::format(
		    "Unknown healpix orientation"
		    "\n\tPossible values are {}",
		    from_array(healpix)) } };
	param.setHealpixOrientation(std::distance(healpix.begin(), res));
	return {};
    }


    DEFINE_COMMAND(path_grid) {
	if (const auto e = line.get_str(0); !e)
	    return std::unexpected{ e.error() };
	else param.setPathGrid(std::string{e.value()});
	return {};
    }


    DEFINE_COMMAND(path_grid_cgs) {
	if (const auto e = line.get_str(0); !e)
	    return std::unexpected{ e.error() };
	else param.setPathGrid(std::string{e.value()});

        param.updateSIConvDH(1e6);
        param.updateSIConvLength(1e-2);
        param.updateSIConvBField(1e-4);
        param.updateSIConvVField(1e-2);

        return {};
    }


    DEFINE_COMMAND(sub_dust) {
	return param_set_number(line, param, &parameters::setSublimate);
    }


    DEFINE_COMMAND(vel_maps) {
	return param_set_number(line, param, &parameters::setVelMaps);
    }


    DEFINE_COMMAND(max_subpixel_lvl) {
	return param_set_number(line, param, &parameters::setMaxSubpixelLvl);
    }


    DEFINE_COMMAND(path_input) {
	if (const auto e = line.get_str(0); !e)
	    return std::unexpected{ e.error() };
	else param.setPathInput(std::string{e.value()});
	return {};
    }


    DEFINE_COMMAND(dust_component) {
	uint 		dust_component_choice = 0;

	using namespace std::literals;

	if (line.named_params.contains("id"sv)) {
	    if (line.named_params["id"sv].empty())
		return std::unexpected { Message { "Invalid dust component ID" } };
	    dust_component_choice = line.named_params["id"sv][0];
	}

	const auto 	path_param = line.get_str(0);

	if (!path_param.has_value())
	    return std::unexpected{ Message{ 
		"Expected path as first parameter" } };

	const std::string 	path{ path_param.value() };

	uint 			nr_size_parameter = 0;
	std::string 		size_keyword = "plaw";

	if (const auto res = line.get_str(1); res.has_value()) {
	    size_keyword = res.value();

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
	    std::format(
		"Wrong number of size parameters (expected {})", nr_size_parameter ) } };
    }


    DEFINE_COMMAND(path_out) {
	if (const auto e = line.get_str(0); !e)
	    return std::unexpected{ e.error() };
	else param.setPathOutput(std::string{e.value()});
	return {};
    }


    DEFINE_COMMAND(nr_plot_points) {
	return param_set_number(line, param, &parameters::setNrOfPlotPoints);
    }


    DEFINE_COMMAND(nr_plot_vectors) {
	return param_set_number(line, param, &parameters::setnrOfPlotVectors);
    }


    DEFINE_COMMAND(f_highJ) {
	return param_set_number(line, param, &parameters::setFhighJ);
    }


    DEFINE_COMMAND(Q_ref) {
	return param_set_number(line, param, &parameters::setQref);
    }


    DEFINE_COMMAND(alpha_Q) {
	return param_set_number(line, param, &parameters::setAlphaQ);
    }


    DEFINE_COMMAND(R_rayleigh) {
	return param_set_number(line, param, &parameters::setRayleighReductionFactor);
    }


    DEFINE_COMMAND(f_c) {
	return param_set_number(line, param, &parameters::setFcorr);
    }


    DEFINE_COMMAND(adj_tgas) {
	return param_set_number(line, param, &parameters::setAdjTgas);
    }


    DEFINE_COMMAND(max_plot_lines) {
	return param_set_number(line, param, &parameters::setMaxPlotLines);
    }

    DEFINE_COMMAND(start) {
	if (const auto e = line.get_num(0); !e)
	    return std::unexpected{ e.error() };
	else param.setStart(e.value() - 1);
	return {};
    }

    DEFINE_COMMAND(stop) {
	if (const auto e = line.get_num(0); !e)
	    return std::unexpected{ e.error() };
	else param.setStop(e.value() - 1);
	return {};
    }

    DEFINE_COMMAND(conv_dens) {

	const auto e = line.get_num(0);

	if (!e)
	    return std::unexpected{ e.error() };

	const auto value = e.value();
	if (value < 0) {
	    param.updateSIConvDH(-value);
	    return std::unexpected { Message{
		"Negative conversion factors are no longer supported!"
		"\n\tGrid must always contain number densities",
		Message::Type::Warning } };
	}
	param.updateSIConvDH(value);

	return {};
    }


    DEFINE_COMMAND(conv_len) {
	return param_set_number(line, param, &parameters::updateSIConvLength);
    }


    DEFINE_COMMAND(conv_mag) {
	return param_set_number(line, param, &parameters::updateSIConvBField);
    }


    DEFINE_COMMAND(conv_vel) {

	const auto e = line.get_num(0);

	if (!e.has_value())
	    return std::unexpected{ e.error() };

	const double conv = e.value();
	if (conv < 0) {
	    param.updateSIConvVField(std::abs(conv));
            return std::unexpected{ Message{
		"Negative conversion factor are no longer allowed!"
		"\n\tThe grid can only contain number densities.",
		Message::Type::Warning } };
	}

        param.updateSIConvVField(conv);
	return {};
    }


    DEFINE_COMMAND(mass_fraction) {
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


    DEFINE_COMMAND(mrw) {
	const auto res = param_set_number(line, param, &parameters::setMRW);
	return std::unexpected{ Message{
	    "MRW currently unavailable", Message::Type::Warning } };
    }


    DEFINE_COMMAND(pda) {
	const auto res = param_set_number(line, param, &parameters::setPDA);
	return std::unexpected{ Message{
	    "PDA currently unavailable", Message::Type::Warning } };
    }


    DEFINE_COMMAND(dust_offset) {
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
    

    DEFINE_COMMAND(dust_gas_coupling) {

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


    DEFINE_COMMAND(radiation_field) {
	return param_set_number(line, param, &parameters::setSaveRadiationField);
    }


    DEFINE_COMMAND(rt_scattering) {
	return param_set_number(line, param, &parameters::setScatteringToRay);
    }


    DEFINE_COMMAND(split_dust_emission) {
	return param_set_number(line, param, &parameters::setSplitDustEmission);
    }


    DEFINE_COMMAND(full_dust_temp) {
	return param_set_number(line, param, &parameters::setFullDustTemp);
    }


    DEFINE_COMMAND(stochastic_heating) {
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


    DEFINE_COMMAND(source_dust) {
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
    DEFINE_COMMAND(source_isrf) {
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
    

    DEFINE_COMMAND(foreground_extinction) {

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


    DEFINE_COMMAND(enfsca) {
	return param_set_number(line, param, &parameters::setEnfScattering);
    }


    DEFINE_COMMAND(peel_off) {
	return param_set_number(line, param, &parameters::setPeelOff);
    }


    DEFINE_COMMAND(acceptance_angle) {
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


    DEFINE_COMMAND(nr_threads) {
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
		std::format("Max. nr. of threads is:  {}", max_t), 
		Message::Type::Warning } };
        }

        if(tr <= 0) [[unlikely]] {
	// TODO: This will not be reached
	    param.setNrOfThreads(1);
	    return std::unexpected{ Message {
		"Max. nr. of threads is: 1",
		Message::Type::Warning } };
        }

        param.setNrOfThreads(tr);
        return {};
    }


    DEFINE_COMMAND(vel_is_speed_of_sound) {
	return param_set_number(line, param, &parameters::setIsSpeedOfSound);
    }


    DEFINE_COMMAND(amira_inp_points) {
	return param_set_number(line, param, &parameters::setInpAMIRAPoints);
    }


    DEFINE_COMMAND(amira_out_points) {
	return param_set_number(line, param, &parameters::setOutAMIRAPoints);
    }


    DEFINE_COMMAND(plot_inp_midplanes) {
	return param_set_number(line, param, &parameters::setInpMidPlot);
    }


    DEFINE_COMMAND(plot_out_midplanes) {
	return param_set_number(line, param, &parameters::setOutMidPlot);
    }


    DEFINE_COMMAND(write_3d_midplanes) {

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


    DEFINE_COMMAND(write_inp_midplanes) {
	return param_set_number(line, param, &parameters::setInpMidDataPoints);
    }


    DEFINE_COMMAND(write_out_midplanes) {
	return param_set_number(line, param, &parameters::setOutMidDataPoints);
    }


    DEFINE_COMMAND(write_radiation_field) {
	const auto e = line.get_num(0);

	if (!e.has_value())
	    return std::unexpected{ e.error() };

	const auto val = e.value();
        if (val < 0 || val > 3) {
	    param.setWriteRadiationField(0);
            return std::unexpected{ Message{
		"Command \"<write_radiation_field>\" accepts only parameters between 0 to 3!" } };
	}

	param.setWriteRadiationField(val);
        return {};
    }


    DEFINE_COMMAND(write_full_radiation_field) {
        return std::unexpected{ Message{
	    "Command <write_full_radiation_field> is no longer available!",
	    Message::Type::Warning } };
    }


    DEFINE_COMMAND(write_g_zero) {
	return param_set_number(line, param, &parameters::setWriteGZero);
    }


    DEFINE_COMMAND(write_dust_files) {
	return param_set_number(line, param, &parameters::setWriteDustFiles);
    }


    DEFINE_COMMAND(midplane_zoom) {
	return param_set_number(line, param, &parameters::setMidplaneZoom);
    }


    DEFINE_COMMAND(kepler_star_mass) {
	return param_set_number(line, param, &parameters::setKeplerStarMass);
    }


    DEFINE_COMMAND(turbulent_velocity) {
	return param_set_number(line, param, &parameters::setTurbulentVelocity);
    }


    DEFINE_COMMAND(mc_lvl_pop_photons) {
	return param_set_number(line, param, &parameters::setMCLvlPopNrOfPhotons);
    }


    DEFINE_COMMAND(mc_lvl_pop_seed) {
	return param_set_number(line, param, &parameters::setMCLvlPopSeed);
    }


    DEFINE_COMMAND(detector_opiate) {
	
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


    DEFINE_COMMAND(detector_opiate_healpix) {

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

    DEFINE_COMMAND(detector_line) {

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


    DEFINE_COMMAND(detector_line_healpix) {

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

    DEFINE_COMMAND(detector_line_polar) {

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

    DEFINE_COMMAND(detector_line_slice) {

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


    DEFINE_COMMAND(detector_dust) {

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


    DEFINE_COMMAND(detector_dust_healpix) {

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

    DEFINE_COMMAND(detector_dust_polar) {

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

    DEFINE_COMMAND(detector_dust_slice) {

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

    DEFINE_COMMAND(detector_dust_mc) {

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


    DEFINE_COMMAND(detector_sync) {

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


    DEFINE_COMMAND(detector_sync_slice) {

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


    DEFINE_COMMAND(detector_sync_healpix) {

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
