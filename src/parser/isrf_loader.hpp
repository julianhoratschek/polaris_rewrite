
#ifndef RW_ISRF_LOADER
#define RW_ISRF_LOADER

#include "basic_loader.hpp"

#include <filesystem>

namespace rewrite {

    struct ISRFFile {
	double			c_q, c_u, c_v;
	double			w_min, w_max;
	std::vector<double>	x;
	std::vector<double>	sp_ext_wl;
    };

    class ISRFLoader: public BasicLoader {
	ISRFFile		result;

    protected:
	void cleanup() override {

	}

    public:
	ISRFFile& get_result() { return result; }


	std::expected<void, Message> parse_file(
	    const std::filesystem::path& path)
	{
	    file.open(path);

	    if (file.fail())
		return safe_error(
		    std::format(
			"Could not open spectrum file: {}", path.string() ));

	    if (const auto res = rdline_values(3, "Q, U, V"); !res)
		return res;

	    result.c_q = values[0];
	    result.c_u = values[1];
	    result.c_v = values[2];

	    result.w_min = 1e300;
	    result.w_max = 0;

	    while (next_line(file)) {
		if (const auto res = read_values(); !res)
		    return safe_error( res.error().message );

		if (values.size() != 2)
		    return safe_error( "Wrong amount of values" );

		const auto x = values[0];

		result.x.push_back(x);
		result.sp_ext_wl.push_back(values[1]);

		result.w_min = std::min(result.w_min, x);
		result.w_max = std::max(result.w_max, x);
	    }

	    file.close();
	    return {};
	}
    };
}

#endif
