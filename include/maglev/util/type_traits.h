// Copyright (c) 2021-2022 Li Shuangquan. All Rights Reserved.
//
// Licensed under the MIT License (the "License"); you may not use this file
// except in compliance with the License. You may obtain a copy of the License
// at
//
//   http://opensource.org/licenses/MIT
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
// WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
// License for the specific language governing permissions and limitations under
// the License.

#pragma once

#include <type_traits>

namespace maglev {

/* ***** is slot counted ***** */

template <typename NodeT, typename = void>
struct is_slot_counted : std::false_type {};

template <class NodeT>
struct is_slot_counted<NodeT, typename NodeT::slot_counted_t> : std::true_type {
};

template <typename NodeT>
using is_slot_counted_t = typename is_slot_counted<NodeT>::type;

// variable template, since C++14
template <typename NodeT>
constexpr bool is_slot_counted_v = is_slot_counted_t<NodeT>::value;

/* ***** is weighted ***** */

template <typename NodeT, typename = void>
struct is_weighted : std::false_type {};

template <class NodeT>
struct is_weighted<NodeT, typename NodeT::weighted_t> : std::true_type {};

template <typename NodeT>
using is_weighted_t = typename is_weighted<NodeT>::type;

// variable template, since C++14
template <typename NodeT>
constexpr bool is_weighted_v = is_weighted_t<NodeT>::value;

/* ***** has stats ***** */

template <typename NodeT, typename = void>
struct has_stats : std::false_type {};

template <class NodeT>
struct has_stats<NodeT, typename NodeT::has_stats_t> : std::true_type {};

template <typename NodeT>
using has_stats_t = typename has_stats<NodeT>::type;

// variable template, since C++14
template <typename NodeT>
constexpr bool has_stats_v = has_stats_t<NodeT>::value;

}  // namespace maglev
