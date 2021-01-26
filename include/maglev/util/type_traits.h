#pragma once

#include <type_traits>

namespace maglev {


template <typename>
struct to_void { typedef void type; };

template <typename T>
using to_void_t = typename to_void<T>::type;


template <typename NodeT, typename = void>
struct is_slot_counted : std::false_type {};

template <class NodeT>
struct is_slot_counted<NodeT, to_void_t<typename NodeT::slot_counted_t>> : std::true_type {};

template <typename NodeT>
using is_slot_counted_t = typename is_slot_counted<NodeT>::type;



template <typename NodeT, typename = void>
struct is_weighted : std::false_type {};

template <class NodeT>
struct is_weighted<NodeT, to_void_t<typename NodeT::weighted_t>> : std::true_type {};

template <typename NodeT>
using is_weighted_t = typename is_weighted<NodeT>::type;


}  // namespace maglev
