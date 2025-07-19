#ifndef RW_STAR_SOURCE_LOADER
#define RW_STAR_SOURCE_LOADER

#include "basic_loader.hpp"

#include <filesystem>

namespace rewrite {

    struct StarSourceFile {
	// w_min and w_max were not used in original code, commented here
	// double			w_min, w_max;
	std::vector<double>	x;
	std::vector<double>	sp_ext;
	std::vector<double>	sp_ext_q;
	std::vector<double>	sp_ext_u;
    };

    class StarSourceLoader: public BasicLoader {
	StarSourceFile		result;

    protected:
	void cleanup() override
	{

	}

    public:
	StarSourceFile& get_result() { return result; }

	std::expected<void, Message> parse_file(
	    const std::filesystem::path& path)
	{
	    file.open(path);

	    if (file.fail())
		return safe_error(
		    std::format(
			"Could not open spectrum file: {}", path.string() ));

	    // result.w_min = 1e300;
	    // result.w_max = 0;
	    while (next_line(file)) {
		if (const auto res = read_values(); !res)
		    return safe_error( res.error().message );

		switch (values.size()) {
		    case 2:
			result.sp_ext_q.push_back(0);
			result.sp_ext_u.push_back(0);
			break;

		    case 4:
			result.sp_ext_q.push_back(values[2]);
			result.sp_ext_u.push_back(values[3]);
			break;

		    default:
			return safe_error(
			    std::format(
				"In spectrum file: {}\n"
				    "Wrong amount of values in line {}\n",
				    path.string(), line_nr));
		}

		result.x.push_back(values[0]);
		result.sp_ext.push_back(values[1]);
		// result.w_min = std::min(result.w_min, values.front());
		// result.w_max = std::max(result.w_max, values.front());
	    }

	    file.close();
	    return {};
	}
    };
}

#endif
