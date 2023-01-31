#pragma once

#include <di/selection.hpp>

namespace di {

/**
 * @brief Free function similar to `std::get` but for @ref Selection.
 * 
 * @tparam Ts The types to get out of the selection 
 * @param selection The selection to query
 * @return decltype(auto) The resulting selection as tuple or single wrapped object
 */
template <typename... Ts>
constexpr decltype(auto) get(auto const &selection) {
    return selection.template get<Ts...>();
}

} // namespace di