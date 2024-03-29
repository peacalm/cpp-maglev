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
#include <memory>
#include <unordered_map>
#include <vector>

#include "maglev/util/to_str.h"

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
  using item_t     = typename base_t::value_type;

  static_assert((std::is_same<item_t, node_ptr_t>::value),
                "value in ContainerType must be same type as node_ptr_t");

public:
  // Call this after all nodes have pushed into node_manager.
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

  void push_back(const item_t& i) {
    assert(i);
    base_t::push_back(i);
  }

  // Returns node pointer if found, or nullptr if not.
  node_ptr_t find_by_node_id(const node_id_t& id) const {
    auto it = std::lower_bound(base_t::begin(),
                               base_t::end(),
                               id,
                               [](const item_t& item, const node_id_t& id) {
                                 return item->id() < id;
                               });
    return it != base_t::end() && (*it)->id() == id ? *it : nullptr;
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

  virtual std::string to_str() const { return maglev::to_str(*this); }
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
