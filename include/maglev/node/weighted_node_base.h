#pragma once

#include <sstream>
#include <string>
#include <type_traits>
#include <utility>

namespace maglev {
namespace node {

template <typename NodeBaseType>
class WeightedNodeBase: public NodeBaseType {
  using base_t = NodeBaseType;

public:
  template <typename ...Args>
  WeightedNodeBase(int weight, Args&& ...args) : base_t(std::forward<Args>(args)...), weight_(weight) {}

  int weight() const { return weight_; }

  virtual std::string to_str() const override;

  template <typename Char, typename Traits>
  std::basic_ostream<Char, Traits>& output_members(std::basic_ostream<Char, Traits>& os) const {
    return base_t::output_members(os) << ",w:" << weight();
  }
private:
  int weight_;
};

template <typename Char, typename Traits, typename NodeBaseType>
std::basic_ostream<Char, Traits>& operator<< (std::basic_ostream<Char, Traits>& os,
                                              const WeightedNodeBase<NodeBaseType>& n) {
  os << "{";
  n.output_members(os);
  os << "}";
  return os;
}

template <typename NodeBaseType>
std::string WeightedNodeBase<NodeBaseType>::to_str() const {
  std::ostringstream os;
  os << *this;
  return os.str();
}


}  // namespace node
}  // namespace maglev
