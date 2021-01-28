#pragma once

#include <cassert>

#include <algorithm>
#include <map>
#include <unordered_map>
#include <vector>

namespace maglev {


template <typename NodeType, typename ContainerType = std::vector<std::shared_ptr<NodeType>>>
class NodeGroupBase : public ContainerType {
  using base_t = ContainerType;
public:
  using node_t = NodeType;
  using node_ptr_t = std::shared_ptr<node_t>;
  using node_id_t = typename node_t::node_id_t;
  using node_map_t = std::unordered_map<node_id_t, const node_ptr_t>;
  using item_t = node_ptr_t;

public:

  virtual void ready_go() { if (!is_sorted()) sort(); }

  bool is_sorted() const { return std::is_sorted(base_t::begin(), base_t::end(), item_cmp); }

  void sort() { std::sort(base_t::begin(), base_t::end(), item_cmp); }

  template <typename ...Args>
  static node_ptr_t new_node(Args&& ...args) {
    return std::make_shared<node_t>(std::forward<Args>(args)...);
  }

  template <typename ...Args>
  void new_back(Args&& ...args) { base_t::push_back(this->new_node(std::forward<Args>(args)...)); }

  node_map_t make_node_map() const {
    node_map_t ret;
    for (auto& i : *this) {
      ret[i->id()] = i;
    }
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


template <typename Char, typename Traits, typename NodeType, typename ContainerType>
std::basic_ostream<Char, Traits>& operator<<(std::basic_ostream<Char, Traits>& os,
                                             const NodeGroupBase<NodeType, ContainerType>& ng) {
  os << "[";
  for (size_t i = 0; i < ng.size(); ++i) {
    if (i > 0) os << ",";
    os << *ng[i];
  }
  os << "]";
  return os;
}


}  // namespace maglev
