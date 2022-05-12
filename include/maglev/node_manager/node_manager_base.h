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

#include <algorithm>
#include <cassert>
#include <map>
#include <unordered_map>
#include <vector>

namespace maglev {

template <typename NodeType,
          typename ContainerType = std::vector<std::shared_ptr<NodeType>>>
class node_manager_base : public ContainerType {
  using base_t = ContainerType;

public:
  using node_t     = NodeType;
  using node_ptr_t = std::shared_ptr<node_t>;
  using node_id_t  = typename node_t::node_id_t;
  using node_map_t = std::unordered_map<node_id_t, node_ptr_t>;
  using item_t     = node_ptr_t;

public:
  virtual void ready_go() {
    if (!is_sorted()) sort();
  }

  bool is_sorted() const {
    return std::is_sorted(base_t::begin(), base_t::end(), item_cmp);
  }

  void sort() { std::sort(base_t::begin(), base_t::end(), item_cmp); }

  template <typename... Args>
  static node_ptr_t new_node(Args&&... args) {
    return std::make_shared<node_t>(std::forward<Args>(args)...);
  }

  template <typename... Args>
  node_ptr_t new_back(Args&&... args) {
    auto new_node_ptr = this->new_node(std::forward<Args>(args)...);
    base_t::push_back(new_node_ptr);
    return new_node_ptr;
  }

  node_map_t make_node_map() const {
    node_map_t ret;
    for (auto& i : *this) { ret.emplace(i->id(), i); }
    return ret;
  }

  static bool item_cmp(const item_t& l, const item_t& r) {
    assert(l && r);
    return *l < *r;
  }

  template <typename Function>
  void for_each(Function f) {
    for (auto& i : *this) f(i);
  }
};

template <typename Char,
          typename Traits,
          typename NodeType,
          typename ContainerType>
std::basic_ostream<Char, Traits>& operator<<(
    std::basic_ostream<Char, Traits>&                 os,
    const node_manager_base<NodeType, ContainerType>& nm) {
  os << "[";
  for (size_t i = 0; i < nm.size(); ++i) {
    if (i > 0) os << ",";
    os << *(nm[i]);
  }
  os << "]";
  return os;
}

}  // namespace maglev
