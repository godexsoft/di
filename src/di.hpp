#pragma once

#include <memory>
#include <tuple>

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

/**
 * @brief Represents a selection of services that can be passed around
 * 
 * @tparam Types List of types of the selection
 */
template <typename... Types>
class Services {
    using data_t = std::tuple<std::shared_ptr<Types>...>;
    std::enable_if_t<
        check_unique<std::decay_t<Types>...>::value, data_t>
        data_; // whole class is sfinaed away if typelist contains duplicate types

public:
    Services()
        : data_{ std::make_shared<Types>()... } {}
    Services(std::shared_ptr<Types>... ts)
        : data_{ ts... } {}

    template <typename... SenderTypes>
    Services(Services<SenderTypes...> const &other) {
        static_assert((any_type_match<std::decay_t<Types>,
                           std::decay_t<SenderTypes>...>::value
                          && ...),
            "Required type is not in typelist");
        (set<decltype(other), Types>(other), ...);
    }

    template <typename T, typename = std::enable_if_t<not any_type_match<std::add_const_t<T>, Types...>::value or std::is_const_v<T>>>
    std::shared_ptr<T> get() const {
        static_assert(any_type_match<std::decay_t<T>, Types...>::value,
            "Required type is not in typelist");
        return std::get<std::shared_ptr<std::decay_t<T>>>(data_);
    }

    template <typename T, typename = std::enable_if_t<any_type_match<std::add_const_t<T>, Types...>::value>>
    std::shared_ptr<const T> get() const {
        static_assert(any_type_match<std::add_const_t<T>, Types...>::value,
            "Required type is not in typelist");
        return std::get<std::shared_ptr<const T>>(data_);
    }

    template <typename... T, typename = std::enable_if_t<sizeof...(T) >= 2>>
    auto get() {
        return std::make_tuple(get<T>()...);
    }

private:
    template <typename... SenderTypes, typename... OtherTypes>
    friend constexpr Services<SenderTypes..., OtherTypes...> extend(
        Services<SenderTypes...> const &services,
        std::shared_ptr<OtherTypes>... others);

    template <typename... LTypes, typename... RTypes>
    friend constexpr Services<LTypes..., RTypes...> combine(
        Services<LTypes...> const &lhs,
        Services<RTypes...> const &rhs);

    template <typename LService, typename MService, typename... Others>
    friend constexpr auto combine(
        LService const &lhs,
        MService const &mid,
        Others const &... others) -> decltype(auto);

    template <typename S, typename T>
    void set(S const &other) {
        std::get<std::shared_ptr<T>>(data_) = other.template get<std::decay_t<T>>();
    }

    template <typename T>
    void set(std::shared_ptr<T> other) {
        std::get<std::shared_ptr<T>>(data_) = other;
    }
};

/**
 * @brief Allows to extend a selection of services by any amount of extra objects
 * 
 * @tparam SenderTypes Types of selection being extended
 * @tparam OtherTypes Types to add to the selection
 * @param services The selection being extended
 * @param others Objects to add to the selection
 * @return constexpr Services<SenderTypes..., OtherTypes...> Extended selection
 */
template <typename... SenderTypes, typename... OtherTypes>
constexpr Services<SenderTypes..., OtherTypes...> extend(
    Services<SenderTypes...> const &services,
    std::shared_ptr<OtherTypes>... others) {
    static_assert((not any_type_match<SenderTypes, OtherTypes...>::value && ...),
        "Additional types should not match any types from extended service");

    // TODO: ideally this would work without default construction
    Services<SenderTypes..., OtherTypes...> out;
    (out.template set<decltype(services), SenderTypes>(services), ...);
    (out.template set<OtherTypes>(others), ...);
    return out;
}

/**
 * @brief Allows to combine two selections into one
 * 
 * @tparam LTypes Left selection types
 * @tparam RTypes Right selection types
 * @param lhs Left selection
 * @param rhs Right selection
 * @return constexpr Services<LTypes..., RTypes...> Combined selection
 */
template <typename... LTypes, typename... RTypes>
constexpr Services<LTypes..., RTypes...> combine(
    Services<LTypes...> const &lhs,
    Services<RTypes...> const &rhs) {
    Services<LTypes..., RTypes...> out;
    (out.template set<decltype(lhs), LTypes>(lhs), ...);
    (out.template set<decltype(rhs), RTypes>(rhs), ...);
    return out;
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

} // namespace di