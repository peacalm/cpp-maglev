#pragma once

#include <type_traits>

namespace maglev {


template <typename>
struct to_void { typedef void type; };

template <typename T>
using to_void_t = typename to_void<T>::type;


template <typename NodeT, typename = void>
struct node_has_slot_counted : std::false_type {};

template <class NodeT>
struct node_has_slot_counted<NodeT, to_void_t<typename NodeT::slot_counted_t>> : std::true_type {};

template <typename NodeT>
using node_has_slot_counted_t = typename node_has_slot_counted<NodeT>::type;


}  // namespace maglev
