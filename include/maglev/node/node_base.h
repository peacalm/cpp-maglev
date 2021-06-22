#pragma once

#include <sstream>
#include <string>
#include <type_traits>
#include <utility>

#include "maglev/util/hash.h"

namespace maglev {


// std::hash for integer got self, which may cause bad performance in maglev,
// so use MaglevIntHash as default. Or you can specify your own hash method.
template <typename IdType = std::string, typename HashType = def_hash_t<IdType>>
class NodeBase {
public:
  using node_id_t = IdType;
  using hash_t = HashType;

public:
  template <typename ...Args>
  NodeBase(Args&& ...args) : id_(std::forward<Args>(args)...) {}

  NodeBase(const node_id_t& id) : id_(id) {}

  NodeBase(node_id_t&& id) : id_(std::move(id)) {}

  node_id_t id() const { return id_; }

  size_t id_hash() const { return hash_t{}(id()); }

  bool operator<(const NodeBase& rhs) const {
    return id() < rhs.id();
  }

  virtual std::string to_str() const {
    return (std::ostringstream{} << *this).str();
  }

  template <typename Char, typename Traits>
  std::basic_ostream<Char, Traits>& output_members(std::basic_ostream<Char, Traits>& os) const {
    return os << "id:" << id();
  }

private:
  node_id_t id_;  // node id should be unique
};

template <typename Char, typename Traits, typename IdType, typename HashType>
std::basic_ostream<Char, Traits>& operator<<(std::basic_ostream<Char, Traits>& os,
                                             const NodeBase<IdType, HashType>& n) {
  os << "{";
  n.output_members(os);
  os << "}";
  return os;
}


}  // namespace maglev
