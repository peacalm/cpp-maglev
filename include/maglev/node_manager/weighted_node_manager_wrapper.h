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

#include <cassert>
#include <type_traits>
#include <utility>

#include "maglev/stats/load_stats.h"
#include "maglev/util/to_str.h"
#include "maglev/util/type_traits.h"

namespace maglev {

template <typename NodeManagerBaseType>
class weighted_node_manager_wrapper : public NodeManagerBaseType {
  using base_t = NodeManagerBaseType;

public:
  using node_t = typename base_t::node_t;

  static_assert(is_weighted_t<node_t>::value,
                "node should be wrapped by weighted_node_wrapper");
  using weighted_t = typename std::enable_if<is_weighted_t<node_t>::value,
                                             typename node_t::weighted_t>::type;

public:
  // Limit max_weight by max_weight <= avg_weight * max_avg_rate_limit, 0 means
  // no limit.
  template <typename... Args>
  weighted_node_manager_wrapper(Args&&... args)
      : max_avg_rate_limit_(0), base_t(std::forward<Args>(args)...) {}

  virtual void ready_go() override {
    base_t::ready_go();
    init_weight();
    __init_load_units(10000, has_stats_t<node_t>{});
  }

  void init_load_units(size_t factor = 10000) {
    for (const auto& n : *this) {
      if (n->weight() > 0) {
        n->set_load_unit(size_t(1.0 * factor * avg_weight() / n->weight()));
      }
    }
  }

  void set_max_avg_rate_limit(double r) { max_avg_rate_limit_ = r; }

  double max_avg_rate_limit() const { return max_avg_rate_limit_; }

  void init_weight() {
    if (base_t::empty()) return;
    weight_sum_ = 0;
    max_weight_ = 0;
    for (const auto& i : *this) {
      unsigned int w = i->weight();
      weight_sum_ += w;
      if (w > max_weight_) { max_weight_ = w; }
    }
    avg_weight_         = double(weight_sum_) / double(base_t::size());
    limited_max_weight_ = max_weight_;
    if (max_avg_rate_limit_ > 0) {
      unsigned int limit =
          (unsigned int)(max_avg_rate_limit_ * double(weight_sum_) /
                         double(base_t::size()));
      if (limited_max_weight_ > limit) { limited_max_weight_ = limit; }
    }
  }

  unsigned int max_weight() const { return max_weight_; }

  unsigned int limited_max_weight() const { return limited_max_weight_; }

  unsigned int weight_sum() const { return weight_sum_; }

  double avg_weight() const { return avg_weight_; }

  bool is_max_weight_limited() const {
    return limited_max_weight() < max_weight();
  }

  virtual std::string to_str() const override { return maglev::to_str(*this); }

private:
  void __init_load_units(size_t factor, std::false_type) {}

  void __init_load_units(size_t factor, std::true_type) {
    init_load_units(factor);
  }

private:
  // user defined
  double max_avg_rate_limit_ = 0;
  // internal params
  unsigned int weight_sum_         = 0;
  unsigned int max_weight_         = 0;
  unsigned int limited_max_weight_ = 0;
  double       avg_weight_         = 0;
};

template <typename Char, typename Traits, typename NodeGroupBaseType>
std::basic_ostream<Char, Traits>& operator<<(
    std::basic_ostream<Char, Traits>&                       os,
    const weighted_node_manager_wrapper<NodeGroupBaseType>& wnm) {
  os << "{" << static_cast<const NodeGroupBaseType&>(wnm)
     << ",sz:" << wnm.size() << ",w_sum:" << wnm.weight_sum()
     << ",avg_w:" << wnm.avg_weight() << ",max_w:" << wnm.max_weight();
  if (wnm.is_max_weight_limited()) {
    os << ">" << wnm.limited_max_weight();
  } else {
    os << "=";
  }
  os << ",w_lmt:" << wnm.max_avg_rate_limit() << "}";
  return os;
}

}  // namespace maglev
