#pragma once

#include <di/selection.hpp>

namespace di {

/**
 * @brief A simple Selection holder type that allows lazy loading. 
 * 
 * You can instantiate the holder with a factory function 
 * or an instance (std::shared_ptr<T>).
 * 
 * @tparam T
 */
template <typename T>
class LazyHolder {
    template <class... Ts>
    struct overloaded : Ts... { using Ts::operator()...; };
    template <class... Ts>
    overloaded(Ts...)->overloaded<Ts...>;
    using ptr_t     = std::shared_ptr<T>;
    using factory_t = std::function<ptr_t()>;
    using variant_t = std::variant<ptr_t, factory_t>;
    using data_t    = std::shared_ptr<variant_t>;

    data_t data_;

public:
    /**
     * @brief Store a factory function for lazy loading.
     * 
     * @tparam Fn Any compatible function or lambda
     * @param factory Expected to be compatible with `shared_ptr<T>()`
     */
    template <typename Fn>
    LazyHolder(Fn factory)
        : data_{ std::make_shared<variant_t>(factory) } {
    }

    /**
     * @brief Store an instance eagerly.
     * 
     * @param ptr The instance
     */
    LazyHolder(ptr_t ptr)
        : data_{ std::make_shared<variant_t>(ptr) } {
    }

    /**
     * @brief Get a shared instance possibly invoking lazy loading.
     * 
     * @return ptr_t
     */
    ptr_t get() {
        // clang-format off
        return std::visit(
            overloaded{
                [](ptr_t ptr) {
                    return ptr;
                },
                [this](factory_t factory) mutable {
                    // todo: maybe lock here or smth
                    return data_->template emplace<ptr_t>(factory());
                } 
            }, *data_);
        // clang-format on
    }

    ptr_t operator->() {
        return get();
    }

    ptr_t operator*() {
        return get();
    }
};

}; // namespace di