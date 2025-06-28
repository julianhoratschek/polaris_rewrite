#ifndef RW_UTIL_HPP
#define RW_UTIL_HPP

#include <string>
#include <sstream>
#include <utility>


namespace rewrite {
    /**
     * Helper function to create error strings
     * @tparam Args arguments to add to `msg`
     * @param msg Start of the error message
     * @param args... Arguments to add onto `msg`
     * @returns `msg` and `args` as a continuous string
     */
    template<typename... Args>
    std::string comp_error(const std::string& msg, Args... args) {
	return (std::ostringstream(msg) << ... << args).str();
    }

    template<typename E>
    consteval bool enable_enum_flag() { return false; }

    template<typename E>
    concept IsEnumFlag = std::is_scoped_enum_v<E> && enable_enum_flag<E>();

    template<IsEnumFlag E>
    E operator|(const E& lhs, const E& rhs) {
	return static_cast<E>(std::to_underlying(lhs) | std::to_underlying(rhs));
    }

    template<IsEnumFlag E>
    E operator&(const E& lhs, const E& rhs) {
	return static_cast<E>(std::to_underlying(lhs) & std::to_underlying(rhs));
    }

    template<IsEnumFlag E>
    E& operator|=(E& lhs, const E& rhs) {
	lhs = lhs | rhs;
	return lhs;
    }

    template<IsEnumFlag E>
    E& operator&=(E& lhs, const E& rhs) {
	lhs = lhs & rhs;
	return lhs;
    }

    template<IsEnumFlag E>
    bool flag_isset(const E& flags, const E& val) {
	const auto v = std::to_underlying(val);
	return (std::to_underlying(flags) & v) == v;
    }

    template<IsEnumFlag E>
    bool flag_anyset(const E& flags, const E& vals) {
	return (std::to_underlying(flags) & std::to_underlying(vals)) != 0;
    }

    template<IsEnumFlag E>
    E& flag_unset(E& flags, const E& which) {
	flags = static_cast<E>(std::to_underlying(flags) & ~std::to_underlying(which));
	return flags;
    }

}

#endif
