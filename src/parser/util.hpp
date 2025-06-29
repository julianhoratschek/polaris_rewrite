#ifndef RW_UTIL_HPP
#define RW_UTIL_HPP

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
	std::ostringstream os;
	os << msg;

	((os << args), ...);
	return os.str();
    }


    /**
     * Trigger-consteval method to enable flag-processing templates for an
     * enum class. Must be defined as specialized template for the desired
     * class:
     *
     * ```
     * enum class MyFlagEnum: unsigned char { SomeValue, SomeOtherValue };
     * template<>
     * consteval bool enable_enum_vlag<MyFlagEnum>() { return true; }
     * ```
     */
    template<typename E>
    consteval bool enable_enum_flag() { return false; }


    template<typename E>
    concept IsEnumFlag = std::is_scoped_enum_v<E> && enable_enum_flag<E>();


    /**
     * OR-Operator for enum flags
     */
    template<IsEnumFlag E>
    E operator|(const E& lhs, const E& rhs) {
	return static_cast<E>(std::to_underlying(lhs) | std::to_underlying(rhs));
    }


    /**
     * AND-Operator for enum flags
     */
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


    /**
     * Returns `true` if val is contained in flags.
     * @tparam E enum class with flags enabled
     * @param flags Flags with different bits set
     * @param val Bits to test if set in `flags`
     * @returns `true` if all bits in `val` are set in `flags`
     */
    template<IsEnumFlag E>
    bool flag_isset(const E& flags, const E& val) {
	const auto v = std::to_underlying(val);
	return (std::to_underlying(flags) & v) == v;
    }


    /**
     * Unsets all bits in `which` in `flags`
     * @tparam E enum class with flags enabled
     * @param flags Flags to unset bits in
     * @param which Bits to unset in `flags`
     * @returns processed version of `flags`
     */
    template<IsEnumFlag E>
    E& flag_unset(E& flags, const E& which) {
	flags = static_cast<E>(std::to_underlying(flags) & ~std::to_underlying(which));
	return flags;
    }
}

#endif
