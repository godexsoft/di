#pragma once

#include <type_traits>

namespace di {

/**
 * @brief Checks that T is listed in Types
 * 
 * @tparam T Type to match
 * @tparam Types List of types to match against
 */
template <typename T, typename... Types>
struct any_type_match {
    constexpr static bool value = (std::is_same_v<T, Types> || ...);
};

/**
 * @brief Counts how many times T is listed in Types
 * 
 * @tparam T Type to match
 * @tparam Types List of types to match against
 */
template <typename T, typename... Types>
struct type_match_count {
    constexpr static std::size_t value = (std::is_same_v<T, Types> + ...);
};

/**
 * @brief Verifies that all types in the list are unique
 * 
 * @tparam Types List of types to verify
 */
template <typename... Types>
struct check_unique {
    static constexpr bool value = ((type_match_count<Types, Types...>::value == 1) && ...);
};

} // namespace di