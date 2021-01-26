#pragma once

#include <type_traits>

namespace maglev {


template <typename NodeT, typename = void>
struct is_slot_counted : std::false_type {};

template <class NodeT>
struct is_slot_counted<NodeT, typename NodeT::slot_counted_t> : std::true_type {};

template <typename NodeT>
using is_slot_counted_t = typename is_slot_counted<NodeT>::type;



template <typename NodeT, typename = void>
struct is_weighted : std::false_type {};

template <class NodeT>
struct is_weighted<NodeT, typename NodeT::weighted_t> : std::true_type {};

template <typename NodeT>
using is_weighted_t = typename is_weighted<NodeT>::type;


}  // namespace maglev
