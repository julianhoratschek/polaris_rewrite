#ifndef RW_BACKGROUND_SRC_LOADER
#define RW_BACKGROUND_SRC_LOADER

#include "basic_loader.hpp"

#include <cstddef>
#include <filesystem>

namespace rewrite {

    struct BackgroundSourceFile {
	size_t			bins;
	std::vector<double>	tmp;
	std::vector<double>	f;
	std::vector<double>	q;
	std::vector<double>	u;
	std::vector<double>	v;
    };

    class BackgroundSourceLoader: public BasicLoader {
	BackgroundSourceFile		result;

    protected:
	void cleanup() override { }


    public:

	BackgroundSourceFile& get_result() { return result; }

	auto parse_file(const std::filesystem::path& filename)
	    -> std::expected<void, Message>
	{
	    file.open(filename);
	    if (file.fail())
		return safe_error( 
		    std::format( "Cannot open file: {}", filename.string() ) );

	    if (const auto num = rdline_number<unsigned int>("Bins");
		!num) return std::unexpected { num.error() };
	    else result.bins = num.value();

	    const size_t size = result.bins * result.bins;
	    result.tmp.resize(size);
	    result.f.resize(size);
	    result.q.resize(size);
	    result.u.resize(size);
	    result.v.resize(size);

	    size_t i = 0;

	    while (i < size) {
		if (const auto res = rdline_values(5, "f, temp, q, u, v");
		    !res) return res;
		result.f[i] = values[0];
		result.tmp[i] = values[1];
		result.q[i] = values[2];
		result.u[i] = values[3];
		result.v[i] = values[4];
		++i;
	    }

	    file.close();
	    return {};
	}
    };
}

#endif

