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
}
