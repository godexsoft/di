#pragma once

#include <di/util.hpp>

#include <functional>
#include <memory>
#include <tuple>
#include <variant>

namespace di {

/**
 * @brief A requirement for T to not be stored as a const in Types
 * 
 * @tparam T 
 * @tparam Types 
 */
template <typename T, typename... Types>
concept NonConstServiceStored = any_type_match<std::decay_t<T>, Types...>::value;

/**
 * @brief A requirement for T to be stored as a const in Types
 * 
 * @tparam T 
 * @tparam Types 
 */
template <typename T, typename... Types>
concept ConstServiceStored = any_type_match<std::add_const_t<T>, Types...>::value;

/**
 * @brief A requirement that type T is actually available in some way in Types
 * 
 * @tparam T 
 * @tparam Types 
 */
template <typename T, typename... Types>
concept ServiceIsStored = any_type_match<std::decay_t<T>, std::decay_t<Types>...>::value;

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
 * @brief Represents a selection of Selection that can be passed around cheaply
 * 
 * This template enables us to narrow the selection by simply specifying 
 * only the types (or Selection) we need.
 * 
 * @tparam Types List of types of the selection (required to be unique)
 */
template <template <typename> typename HolderType, typename... Types>
requires EachIsUnique<Types...> class Selection {
    using data_t = std::tuple<HolderType<Types>...>;
    data_t data_;

public:
    /**
     * @brief Default-constructs each service and stores it as a shared_ptr
     */
    constexpr Selection() requires std::is_same_v<HolderType<void>, std::shared_ptr<void>>
        : data_{ std::make_shared<Types>()... } {}

    /**
     * @brief Construct a selection directly from data to be stored
     * 
     * @param ts The data to store
     */
    constexpr Selection(HolderType<Types>... ts)
        : data_{ ts... } {}

    /**
     * @brief Constructs a new (possibly narrower) selection by copying relevant Selection
     * 
     * @tparam SenderTypes (required for each type in Types to also be in SenderTypes)
     * @param other The (possibly wider) Selection selection
     */
    template <typename... SenderTypes>
    constexpr Selection(Selection<HolderType, SenderTypes...> const &other) requires(ServiceIsStored<Types, SenderTypes...> &&...)
        : data_(other.template get<Types...>()) {
    }

    /**
     * @brief Get a service by its type
     * 
     * If T is const (e.g. explicit const requested) the non-const service stored in the selection
     * is promoted to a HolderType<const T> automatically.
     * 
     * @tparam T The type of service (required to be stored as non-const)
     * @return HolderType<T> 
     */
    template <typename T>
    constexpr HolderType<T> get() const requires NonConstServiceStored<T, Types...> {
        return std::get<HolderType<std::decay_t<T>>>(data_);
    }

    /**
     * @brief Get a const service by its type
     * 
     * @tparam T The type of service (required to be stored as const)
     * @return std::shared_ptr<const T> 
     */
    template <typename T>
    constexpr HolderType<const T> get() const requires ConstServiceStored<T, Types...> {
        return std::get<HolderType<const T>>(data_);
    }

    /**
     * @brief Get multiple Selection at once
     * 
     * This is useful in combination with structured bindings:
     * @code  
     *   auto [a, b] = selection.get<A, B>();
     * @endcode
     * 
     * @tparam Ts
     * @return auto Roughly std::tuple<std::shared_ptr<Ts>...>
     */
    template <typename... Ts>
    constexpr std::tuple<HolderType<Ts>...> get() const requires TwoOrMoreInPack<Ts...> {
        return std::make_tuple<HolderType<Ts>...>(get<Ts>()...);
    }

private:
    template <typename... Ts>
    friend constexpr decltype(auto) get(auto const &selection);

    template <template <typename> typename HType, typename... SenderTypes, typename... OtherTypes>
    friend constexpr Selection<HType, SenderTypes..., OtherTypes...> extend(
        Selection<HType, SenderTypes...> const &selection,
        HType<OtherTypes>... others);

    template <template <typename> typename HType, typename... LTypes, typename... RTypes>
    friend constexpr Selection<HType, LTypes..., RTypes...> combine(
        Selection<HType, LTypes...> const &lhs,
        Selection<HType, RTypes...> const &rhs);

    template <typename LService, typename MService, typename... Others>
    friend constexpr auto combine(
        LService const &lhs,
        MService const &mid,
        Others const &... others) -> decltype(auto);
};

} // namespace di