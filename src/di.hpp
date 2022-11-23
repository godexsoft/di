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
 * @brief A requirement for T to not be stored as a const in Types
 * 
 * @tparam T 
 * @tparam Types 
 */
template <typename T, typename... Types>
concept NonConstServiceStored = not any_type_match<std::add_const_t<T>, Types...>::value;

/**
 * @brief A requirement for T to be stored as a const in Types
 * 
 * @tparam T 
 * @tparam Types 
 */
template <typename T, typename... Types>
concept ConstServiceStored = any_type_match<std::add_const_t<T>, Types...>::value;

/**
 * @brief A requirement for Types to be a pack of at least two types
 * 
 * @tparam Types 
 */
template <typename... Types>
concept TwoOrMoreInPack = sizeof...(Types) >= 2;

/**
 * @brief A requirement for each type in Types to be a unique type
 * 
 * @tparam Types 
 */
template <typename... Types>
concept EachIsUnique = check_unique<std::decay_t<Types>...>::value;

/**
 * @brief Represents a selection of services that can be passed around cheaply
 * 
 * This template enables us to narrow the selection by simply specifying 
 * only the types (or services) we need.
 * 
 * @tparam Types List of types of the selection (required to be unique)
 */
template <typename... Types>
requires EachIsUnique<Types...> class Services {
    using data_t = std::tuple<std::shared_ptr<Types>...>;
    data_t data_;

public:
    /**
     * @brief Default-constructs each service and stores it as a shared_ptr
     */
    Services()
        : data_{ std::make_shared<Types>()... } {}
    Services(std::shared_ptr<Types>... ts)
        : data_{ ts... } {}

    /**
     * @brief Constructs a new (possibly narrower) selection by copying relevant services
     * 
     * @tparam SenderTypes (required for each type in Types to also be in SenderTypes)
     * @param other The (possibly wider) services selection
     */
    template <typename... SenderTypes>
    Services(Services<SenderTypes...> const &other) {
        static_assert((any_type_match<std::decay_t<Types>,
                           std::decay_t<SenderTypes>...>::value
                          && ...),
            "Required type is not in typelist");
        (set<decltype(other), Types>(other), ...);
    }

    /**
     * @brief Get a service by its type
     * 
     * If T is const (e.g. explicit const requested) the non-const service stored in the selection
     * is promoted to a shared_ptr<const T> automatically.
     * 
     * @tparam T The type of service (required to be stored as non-const)
     * @return std::shared_ptr<T> 
     */
    template <typename T>
    std::shared_ptr<T> get() const requires NonConstServiceStored<T, Types...> {
        static_assert(any_type_match<std::decay_t<T>, Types...>::value,
            "Required type is not in typelist");
        return std::get<std::shared_ptr<std::decay_t<T>>>(data_);
    }

    /**
     * @brief Get a const service by its type
     * 
     * @tparam T The type of service (required to be stored as const)
     * @return std::shared_ptr<const T> 
     */
    template <typename T>
    std::shared_ptr<const T> get() const requires ConstServiceStored<T, Types...> {
        static_assert(any_type_match<std::add_const_t<T>, Types...>::value,
            "Required type is not in typelist");
        return std::get<std::shared_ptr<const T>>(data_);
    }

    /**
     * @brief Get multiple services at once
     * 
     * This is useful in combination with structured bindings:
     * @code  
     *   auto [a, b] = services.get<A, B>();
     * @endcode
     * 
     * @tparam Ts
     * @return auto Roughly std::tuple<std::shared_ptr<Ts>...>
     */
    template <typename... Ts>
    auto get() requires TwoOrMoreInPack<Ts...> {
        return std::make_tuple(get<Ts>()...);
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