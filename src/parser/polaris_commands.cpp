#include "polaris_commands.hpp"

namespace rewrite {

    auto cmd_cmd(ParsedLine& line, parameters& param)
	-> std::expected<bool, std::string> {

	constexpr auto commands = std::array {
	    "CMD_TEMP", "CMD_DUST_EMISSION", "CMD_DUST_SCATTERING",
	    "CMD_PROBING", "CMD_RAT", "CMD_TEMP_RAT",
	    "CMD_LINE_EMISSION", "CMD_FORCE", "CMD_OPIATE",
	    "CMD_SYNCHROTRON"
	};

	auto res = std::ranges::find(commands, line[0].value);
	if (res == commands.end())
	    return std::unexpected{"Command cannot be recognized!"};

	param.setCommand(std::distance(commands.begin(), res));
	return true;

    }


    auto cmd_delta0(ParsedLine& line, parameters& param)
	-> std::expected<bool, std::string> {
	if (const auto e = line[0].as_number();
	    not e) return std::unexpected{e.error()};
	else param.setDelta0(e.value());
	return true;
    }


    auto cmd_larm_f(ParsedLine& line, parameters& param)
	-> std::expected<bool, std::string> {
	if (const auto e = line[0].as_number();
	    not e) return std::unexpected{e.error()};
	else param.setLarmF(e.value());
	return true;
    }


    auto cmd_plot_list(ParsedLine& line, parameters& param)
	-> std::expected<bool, std::string> {
	
	if (line.size() == 0)
	    return std::unexpected{ "List of IDs is empty" };

	for (size_t i = 0; i < line.size(); i++) {
	    if (const auto num = line[i].as_number();
		not num) return std::unexpected(num.error());
	    else if (const auto id = num.value();
		    minGRID <= id && id <= maxGRID) {
		param.addToPlotList(id);
	    } else return std::unexpected{ "Unknown grid ID (see manual, Table 3.3)!"};
	}

        return true;
    }

    auto cmd_phase_function(ParsedLine& line, parameters& param)
	-> std::expected<bool, std::string> {

	const uint dust_component_choice = 0;
	if (const auto res = line.get_named("id");
	    res.has_value()) {
	    if (const auto num = res.value().as_number();
		not num) return std::unexpected{num.error()};
	    else
		dust_component_choice = num.value();
	}

	if (dust_component_choice < 0)
	    return std::unexpected{"Dust component choice ID from phase_function is not valid!"};

	constexpr auto functions = std::array{
	    "PH_ISO", "PH_HG", "PH_DHG", "PH_TTHG", "PH_MIE"
	};

	const auto res = std::ranges::find(functions, line[0].value);
	if (res == functions.end())
	    return std::unexpected{"Phase function cannot be recognized!"};
	param.setPhaseFunctionID(
		std::distance(functions.begin(), res),
		dust_component_choice);
	return true;
    }

    auto cmd_star_mass(ParsedLine& line, parameters& param)
	-> std::expected<bool, std::string> {
	
	for(auto i=0; i < line.size(); i++) {
	    if (const auto num = line[i].as_number();
		not num) return std::unexpected{num.error()};
	    else
		param.addStarMass(num.value() * M_sun);
	}
	return true;
    }

    auto cmd_opiata_path_emi(ParsedLine& line, parameters& param)
	-> std::expected<bool, std::string> {
	param.setOpiatePathEmission(std::string{line[0].value});
    }

    auto cmd_opiata_path_abs(ParsedLine& line, parameters& param)
	-> std::expected<bool, std::string> {
	param.setOpiatePathAbsorption(std::string{line[0].value});
    }

    auto cmd_gas_species(ParsedLine& line, parameters& param)
	-> std::expected<bool, std::string> {

	constexpr auto pop = std::array{
	    "POP_MC", "POP_LTE", "POP_FEP", "POP_LVG", "POP_DEGUCHI_LVG"
	};

	std::vector<double>	values;
	std::string 		gas_species_path{line[0].value};
	const auto 		pos = std::ranges::find(pop, line[1].value);

	if (pos != pop.end())
	    values.push_back(std::distance(pop.begin(), pos));
	else {
	    if (const auto num = line[1].as_number();
		not num) return std::unexpected{num.error()};
	    else values.push_back(num.value());
	}

	if (const auto num = line[2].as_number();
	    not num) return std::unexpected{num.error()};
	else values.push_back(num.value());

	auto zeeman_path = line.get_optional(3)
	    .transform([](auto p) { return p.value; });

	param.addGasSpecies(gas_species_path, std::string{zeeman_path.value_or("")}, values);
        return true;
    }


    auto cmd_source_star(ParsedLine& line, parameters& param)
	-> std::expected<bool, std::string> {

	ullong nr_of_photons;
	std::string ps_path;

	if (const auto nr_photons = line.get_named("nr_photons");
	    not nr_photons) return std::unexpected{"Number of star photons could not be recognized"};
	else {
	    if (const auto num = nr_photons.value().as_number();
		not num) return std::unexpected{num.error()};
	    else nr_of_photons = static_cast<ullong>(num.value());
	}

	if (line[3].type == ParsedParameter::Type::String) {
	    ps_path = line[3].value;

            if(values.size() == NR_OF_POINT_SOURCES - 5)
            {
                values.push_back(0);
                values.push_back(0);
                values.push_back(0);
                values.push_back(0);
            }
            else
            {
                cout << ERROR_LINE << "False amount of parameters for source star in line " << line_counter << "!"
                     << endl;
                return false;
            }

	}

        string str = seperateString(data);
        string ps_path = seperateString(data);

        dlist values = parseValues(data);

        if(ps_path.size() != 0)
        {
            if(values.size() == NR_OF_POINT_SOURCES - 5)
            {
                values.push_back(0);
                values.push_back(0);
                values.push_back(0);
                values.push_back(0);
            }
            else
            {
                cout << ERROR_LINE << "False amount of parameters for source star in line " << line_counter << "!"
                     << endl;
                return false;
            }
        }
        else
        {
            if(values.size() == NR_OF_POINT_SOURCES - 3)
            {
                values.push_back(0);
                values.push_back(0);
            }
            else if(values.size() != NR_OF_POINT_SOURCES - 1)
            {
                cout << ERROR_LINE << "False amount of parameters for source star in line " << line_counter << "!"
                     << endl;
                return false;
            }
        }

        double P_l = sqrt(pow(values[5], 2) + pow(values[6], 2));
        if(P_l > 1.0)
        {
            cout << ERROR_LINE << "Chosen polarization of source star is larger than 1!" << endl;
            return false;
        }
        else if(P_l < 0)
        {
            cout << INFO_LINE << "Chosen polarization of source star is less than 0 (now set to 0)!" << endl;
            values[5] = 0;
            values[6] = 0;
        }

        values.push_back(double(nr_of_photons));
        param->addPointSource(values, ps_path);
        return true;
    }

}
