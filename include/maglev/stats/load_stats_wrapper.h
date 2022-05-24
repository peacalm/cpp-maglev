// Copyright (c) 2021-2022 Shuangquan Li. All Rights Reserved.
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

#include "maglev/node/node_base.h"
#include "maglev/node/server_node_base.h"
#include "maglev/node/slot_counted_node_wrapper.h"
#include "maglev/node/weighted_node_wrapper.h"
#include "maglev/stats/load_stats.h"
#include "maglev/util/to_str.h"

namespace maglev {

/// A wrapper to make a node type has stats, mainly used for maglev_balancer.
template <typename NodeMetaType, typename LoadStatsType>
class load_stats_wrapper : public NodeMetaType, public LoadStatsType {
public:
  using has_stats_t  = void;  // for type traits
  using node_meta_t  = NodeMetaType;
  using load_stats_t = LoadStatsType;

  template <typename... Args>
  load_stats_wrapper(Args&&... args)
      : node_meta_t(std::forward<Args>(args)...) {}

  node_meta_t&       node_meta() { return *static_cast<node_meta_t*>(this); }
  const node_meta_t& node_meta() const {
    return *static_cast<node_meta_t>(this);
  }

  load_stats_t&       load_stats() { return *static_cast<load_stats_t*>(this); }
  const load_stats_t& load_stats() const {
    return *static_cast<load_stats_t>(this);
  }

  virtual std::string to_str() const override { return maglev::to_str(*this); }
};

template <typename Char,
          typename Traits,
          typename NodeMetaType,
          typename NodeStatsType>
std::basic_ostream<Char, Traits>& operator<<(
    std::basic_ostream<Char, Traits>&                      os,
    const load_stats_wrapper<NodeMetaType, NodeStatsType>& n) {
  os << "{";
  n.output_members(os);
  os << ",[";
  n.output_stats(os);
  os << "]}";
  return os;
}

}  // namespace maglev