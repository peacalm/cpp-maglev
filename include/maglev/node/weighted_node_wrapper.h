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
class weighted_node_wrapper : public NodeBaseType {
  using base_t = NodeBaseType;

public:
  using weighted_t = void;  // for type trait

public:
  template <typename... Args>
  weighted_node_wrapper(Args&&... args)
      : weight_(0), base_t(std::forward<Args>(args)...) {}

  unsigned int weight() const { return weight_; }

  void set_weight(unsigned int w) { weight_ = w; }

  virtual std::string to_str() const override {
    return (std::ostringstream{} << *this).str();
  }

  template <typename Char, typename Traits>
  std::basic_ostream<Char, Traits>& output_members(
      std::basic_ostream<Char, Traits>& os) const {
    return base_t::output_members(os) << ",w:" << weight();
  }

private:
  unsigned int weight_ = 0;
};

template <typename Char, typename Traits, typename NodeBaseType>
std::basic_ostream<Char, Traits>& operator<<(
    std::basic_ostream<Char, Traits>&          os,
    const weighted_node_wrapper<NodeBaseType>& n) {
  os << "{";
  n.output_members(os);
  os << "}";
  return os;
}

}  // namespace maglev
