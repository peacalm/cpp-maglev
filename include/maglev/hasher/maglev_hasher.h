#pragma once

#include <cstring>

#include <array>
#include <type_traits>
#include <vector>

#include "maglev/hasher/slot_array.h"
#include "maglev/node/node_base.h"
#include "maglev/node_group/node_group_base.h"
#include "maglev/node_group/weighted_node_group_wrapper.h"
#include "maglev/permutation/permutation_generator.h"
#include "maglev/util/hash.h"
#include "maglev/util/type_traits.h"

namespace maglev {


template <
    typename SlotArrayType = SlotArray<int>,
    typename NodeType = NodeBase<>,
    typename NodeGroupType = typename std::conditional<
        is_weighted<NodeType>::value,
        WeightedNodeGroupWrapper<NodeGroupBase<NodeType>>,
        NodeGroupBase<NodeType>
    >::type,
    typename PermutationGeneratorType = typename std::conditional<
        is_weighted_t<NodeGroupType>::value,
        PermutationGeneratorWithRand,
        PermutationGenerator
    >::type
>
class MaglevHasher {
public:
  using slot_array_t = SlotArrayType;
  using slot_int_t = typename slot_array_t::int_t;
  using perm_gen_t = PermutationGeneratorType;
  using perm_gen_array_t = std::vector<perm_gen_t>;
  using node_group_t = NodeGroupType;
  using node_t = typename node_group_t::node_t;
  using node_ptr_t = typename node_group_t::node_ptr_t;
  using node_group_item_t = typename node_group_t::item_t;

public:
  struct PickRet {
    node_group_item_t node;
    size_t node_idx = 0;
  };
  using pick_ret_t = PickRet;

protected:
  using is_slot_counted_node_t = is_slot_counted_t<node_t>;
  using is_weighted_node_group_t = is_weighted_t<node_group_t>;

public:

  MaglevHasher() {}

  template <typename NodeGroupT, typename ...Args>
  MaglevHasher(NodeGroupT&& ng, Args&& ...args) :
      node_group_(std::forward<NodeGroupT>(ng)),
      slot_array_(std::forward<Args>(args)...) {}

  slot_array_t& mutable_slot_array() { return slot_array_; }

  const slot_array_t& slot_array() const { return slot_array_; }

  size_t slot_size() const { return slot_array_.size(); }

  node_group_t& mutable_node_group() { return node_group_; }

  const node_group_t& node_group() const { return node_group_; }

  size_t node_size() const { return node_group_.size(); }

  pick_ret_t pick(size_t hashed_key) const {
    pick_ret_t ret;
    ret.node_idx = slot_array_[hashed_key % slot_size()];
    ret.node = node_group_[ret.node_idx];
    return ret;
  }

  template <typename KeyType, typename HashType = def_hash_t<KeyType>>
  pick_ret_t pick_with_auto_hash(const KeyType& key) {
    static auto h = HashType{};
    return pick(h(key));
  }

  void build() {
    init_slot_array();
    init_node_group();
    auto p = make_perm_gen_array();
    const auto n = node_size();
    for (size_t node_idx = 0, slot_distributed_cnt = 0; slot_distributed_cnt < slot_size();) {
      select_once(p[node_idx], node_idx, slot_distributed_cnt, is_weighted_node_group_t{});
      if (++node_idx >= n) node_idx = 0;
    }
  }

protected:
  void init_node_group() { node_group_.ready_go(); }

  constexpr slot_int_t slot_initial_value() const { return slot_int_t(-1); }

  void init_slot_array() {
    memset((void *) slot_array_.data(), -1, sizeof(slot_int_t) * slot_array_.size());
  }

  bool is_slot_distributed(size_t idx) const {
    return slot_array_[idx] != slot_initial_value();
  }

  void distribut_slot(size_t slot_idx, size_t node_idx, std::true_type) {
    slot_array_[slot_idx] = (slot_int_t) node_idx;
    node_group_[node_idx]->incr_slot_cnt();
  }

  void distribut_slot(size_t slot_idx, size_t node_idx, std::false_type) {
    slot_array_[slot_idx] = (slot_int_t) node_idx;
  }

  void select_once(perm_gen_t& perm_gen, size_t& node_idx, size_t& slot_distributed_cnt, std::true_type) {
    auto& node = node_group_[node_idx];
    bool slected = 1ULL * perm_gen.my_rand() * node_group_.max_weight() <
                   1ULL * node->weight() * perm_gen.my_rand_max();
    if (slected) {
      select_once(perm_gen, node_idx, slot_distributed_cnt, std::false_type{});
    }
  }

  void select_once(perm_gen_t& perm_gen, size_t& node_idx, size_t& slot_distributed_cnt, std::false_type) {
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
    for (const auto& i: node_group_) {
      p.push_back(make_a_perm_gen(i));
    }
    return p;
  }

private:
  slot_array_t slot_array_;
  node_group_t node_group_;
};


}  // namespace maglev
