#ifndef RW_AGN_SOURCE_LOADER
#define RW_AGN_SOURCE_LOADER

// TODO: This is exactly star source: replace??

#include "basic_loader.hpp"

#include <filesystem>

namespace rewrite {

    struct AGNSourceFile {
	std::vector<double>	x;
	std::vector<double>	sp_ext;
	std::vector<double>	sp_ext_q;
	std::vector<double>	sp_ext_u;
    };

    class AGNSourceLoader: public BasicLoader {
	AGNSourceFile		result;

    protected:
	void cleanup() override { }

    public:
	AGNSourceFile& get_result() { return result; }

	std::expected<void, Message> parse_file(
	    const std::filesystem::path& path)
	{
	    file.open(path);

	    if (file.fail())
		return safe_error(
		    std::format(
			"Could not open spectrum file: {}", path.string() ));

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
	    }

	    file.close();
	    return {};
	}
    };
}

#endif
