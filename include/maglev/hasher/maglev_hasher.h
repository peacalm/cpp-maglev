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

#include <array>
#include <cstring>
#include <type_traits>
#include <vector>

#include "maglev/hasher/slot_array.h"
#include "maglev/node/node_base.h"
#include "maglev/node_manager/node_manager_base.h"
#include "maglev/node_manager/weighted_node_manager_wrapper.h"
#include "maglev/permutation/permutation_generator.h"
#include "maglev/util/hash.h"
#include "maglev/util/type_traits.h"

namespace maglev {

template <typename NodeType        = node_base<std::string>,
          typename SlotArrayType   = slot_array<int>,
          typename NodeManagerType = typename std::conditional<
              is_weighted_t<NodeType>::value,
              weighted_node_manager_wrapper<node_manager_base<NodeType>>,
              node_manager_base<NodeType>>::type,
          typename PermutationGeneratorType =
              typename std::conditional<is_weighted_t<NodeManagerType>::value,
                                        permutation_generator_with_rand,
                                        permutation_generator>::type>
class maglev_hasher {
public:
  using slot_array_t        = SlotArrayType;
  using slot_int_t          = typename slot_array_t::int_t;
  using perm_gen_t          = PermutationGeneratorType;
  using perm_gen_array_t    = std::vector<perm_gen_t>;
  using node_manager_t      = NodeManagerType;
  using node_t              = typename node_manager_t::node_t;
  using node_ptr_t          = typename node_manager_t::node_ptr_t;
  using node_manager_item_t = typename node_manager_t::item_t;

  struct pick_ret_t {
    node_manager_item_t node     = nullptr;  // node pointer
    size_t              node_idx = 0;        // index in node_manager
  };

protected:
  using is_slot_counted_node_t     = is_slot_counted_t<node_t>;
  using is_weighted_node_manager_t = is_weighted_t<node_manager_t>;

public:
  maglev_hasher() {}

  template <typename NodeManagerT, typename... Args>
  maglev_hasher(NodeManagerT&& nm, Args&&... args)
      : node_manager_(std::forward<NodeManagerT>(nm)),
        slot_array_(std::forward<Args>(args)...) {}

  slot_array_t& slot_array() { return slot_array_; }

  const slot_array_t& slot_array() const { return slot_array_; }

  size_t slot_size() const { return slot_array_.size(); }

  node_manager_t& node_manager() { return node_manager_; }

  const node_manager_t& node_manager() const { return node_manager_; }

  size_t node_size() const { return node_manager_.size(); }

  pick_ret_t pick(size_t hashed_key) const {
    pick_ret_t ret;
    ret.node_idx = slot_array_[hashed_key % slot_size()];
    ret.node     = node_manager_[ret.node_idx];
    return ret;
  }

  template <typename KeyType, typename HashType = def_hash_t<KeyType>>
  pick_ret_t pick_with_auto_hash(const KeyType& key) const {
    static auto h = HashType{};
    return pick(h(key));
  }

  void build() {
    init_slot_array();
    init_node_manager();
    auto       p = make_perm_gen_array();
    const auto n = node_size();
    for (size_t node_idx = 0, slot_distributed_cnt = 0;
         slot_distributed_cnt < slot_size();) {
      select_once(p[node_idx],
                  node_idx,
                  slot_distributed_cnt,
                  is_weighted_node_manager_t{});
      if (++node_idx >= n) node_idx = 0;
    }
  }

protected:
  void init_node_manager() { node_manager_.ready_go(); }

  constexpr slot_int_t slot_initial_value() const { return slot_int_t(-1); }

  void init_slot_array() {
    memset(
        (void*)slot_array_.data(), -1, sizeof(slot_int_t) * slot_array_.size());
  }

  bool is_slot_distributed(size_t idx) const {
    return slot_array_[idx] != slot_initial_value();
  }

  void distribut_slot(size_t slot_idx, size_t node_idx, std::true_type) {
    slot_array_[slot_idx] = (slot_int_t)node_idx;
    node_manager_[node_idx]->incr_slot_cnt();
  }

  void distribut_slot(size_t slot_idx, size_t node_idx, std::false_type) {
    slot_array_[slot_idx] = (slot_int_t)node_idx;
  }

  void select_once(perm_gen_t& perm_gen,
                   size_t&     node_idx,
                   size_t&     slot_distributed_cnt,
                   std::true_type) {
    auto& node = node_manager_[node_idx];
    bool  slected =
        1ULL * perm_gen.my_rand() * node_manager_.limited_max_weight() <=
        1ULL * node->weight() * perm_gen.my_rand_max();
    if (slected) {
      select_once(perm_gen, node_idx, slot_distributed_cnt, std::false_type{});
    }
  }

  void select_once(perm_gen_t& perm_gen,
                   size_t&     node_idx,
                   size_t&     slot_distributed_cnt,
                   std::false_type) {
    while (true) {
      auto t = perm_gen.gen_one_num();
      if (!is_slot_distributed(t)) {
        distribut_slot(t, node_idx, is_slot_counted_node_t{});
        ++slot_distributed_cnt;
        break;
      }
    }
  }

  perm_gen_t make_a_perm_gen(const node_ptr_t& i) const {
    return perm_gen_t(slot_size(), i->id_hash());
  }

  perm_gen_array_t make_perm_gen_array() const {
    perm_gen_array_t p;
    p.reserve(node_size());
    for (const auto& i : node_manager_) { p.push_back(make_a_perm_gen(i)); }
    return p;
  }

private:
  slot_array_t   slot_array_;
  node_manager_t node_manager_;
};

}  // namespace maglev
