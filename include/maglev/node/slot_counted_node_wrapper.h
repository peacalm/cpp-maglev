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

#include <sstream>
#include <string>
#include <type_traits>
#include <utility>

namespace maglev {

template <typename NodeBaseType>
class slot_counted_node_wrapper : public NodeBaseType {
  using base_t = NodeBaseType;

public:
  using slot_counted_t = void;  // for type_trait

public:
  template <typename... Args>
  slot_counted_node_wrapper(Args&&... args)
      : slot_cnt_(0), base_t(std::forward<Args>(args)...) {}

  void incr_slot_cnt(int d = 1) { slot_cnt_ += d; }

  int slot_cnt() const { return slot_cnt_; }

  void set_slot_cnt(int s) { slot_cnt_ = s; }

  virtual std::string to_str() const override {
    return (std::ostringstream{} << *this).str();
  }

  template <typename Char, typename Traits>
  std::basic_ostream<Char, Traits>& output_members(
      std::basic_ostream<Char, Traits>& os) const {
    return base_t::output_members(os) << ",s:" << slot_cnt();
  }

private:
  int slot_cnt_ = 0;  // slot num obtained from maglev-hasher
};

template <typename Char, typename Traits, typename NodeBaseType>
std::basic_ostream<Char, Traits>& operator<<(
    std::basic_ostream<Char, Traits>&              os,
    const slot_counted_node_wrapper<NodeBaseType>& n) {
  os << "{";
  n.output_members(os);
  os << "}";
  return os;
}

}  // namespace maglev
