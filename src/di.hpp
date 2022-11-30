#pragma once

#include <di/ext.hpp>
#include <di/lazy.hpp>
#include <di/selection.hpp>
#include <di/util.hpp>

namespace di {

template <typename... Types>
using Services = Selection<std::shared_ptr, Types...>;

template <typename... Types>
using Deps = Selection<std::reference_wrapper, Types...>;

template <typename... Types>
using LazyServices = Selection<LazyHolder, Types...>;

} // namespace di