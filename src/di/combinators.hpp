#pragma once

#include <di/selection.hpp>

namespace di {

/**
 * @brief Allows to extend a selection of Selection by any amount of extra objects
 * 
 * @tparam SenderTypes Types of selection being extended
 * @tparam OtherTypes Types to add to the selection
 * @param Selection The selection being extended
 * @param others Objects to add to the selection
 * @return constexpr Selection<SenderTypes..., OtherTypes...> Extended selection
 */
template <template <typename> typename HType, typename... SenderTypes, typename... OtherTypes>
constexpr Selection<HType, SenderTypes..., OtherTypes...> extend(
    Selection<HType, SenderTypes...> const &selection,
    HType<OtherTypes>... others) {
    static_assert((not any_type_match<SenderTypes, OtherTypes...>::value && ...),
        "Additional types should not match any types from extended service");
    return Selection<HType, SenderTypes..., OtherTypes...>{
        selection.template get<SenderTypes>()...,
        others...
    };
}

/**
 * @brief Allows to combine two selections into one
 * 
 * @tparam LTypes Left selection types
 * @tparam RTypes Right selection types
 * @param lhs Left selection
 * @param rhs Right selection
 * @return constexpr Selection<LTypes..., RTypes...> Combined selection
 */
template <template <typename> typename HType, typename... LTypes, typename... RTypes>
constexpr Selection<HType, LTypes..., RTypes...> combine(
    Selection<HType, LTypes...> const &lhs,
    Selection<HType, RTypes...> const &rhs) {
    return Selection<HType, LTypes..., RTypes...>{
        lhs.template get<LTypes>()...,
        rhs.template get<RTypes>()...
    };
}

/**
 * @brief Allows to combine multiple selections into one
 * 
 * @tparam LType Leftmost selection types
 * @tparam MType Next selection types
 * @tparam Others All other selections
 * @param lhs Leftmost selection
 * @param mid Next selection
 * @param others All other selections
 * @return decltype(auto) Combined selection
 */
template <typename LType, typename MType, typename... Others>
constexpr auto combine(
    LType const &lhs,
    MType const &mid,
    Others const &... others) -> decltype(auto) {
    return combine(combine(lhs, mid), others...);
}

/**
 * @brief Extension helper for Deps automatically adds std::ref
 */
template <typename... SenderTypes, typename... OtherTypes>
constexpr Selection<std::reference_wrapper, SenderTypes..., OtherTypes...> extend(
    Selection<std::reference_wrapper, SenderTypes...> const &selection,
    OtherTypes &... others) {
    return extend(selection, std::ref(others)...);
}

} // namespace di