#pragma once

#include <sstream>
#include <string>
#include <type_traits>
#include <utility>

namespace maglev {


template <typename NodeBaseType>
class WeightedNodeWrapper : public NodeBaseType {
  using base_t = NodeBaseType;

public:
  using weighted_t = void;  // for type trait

public:
  template <typename ...Args>
  WeightedNodeWrapper(int weight, Args&& ...args) : weight_(weight), base_t(std::forward<Args>(args)...) {
    assert(weight > 0);
  }

  int weight() const { return weight_; }

  void set_weight(int w) {
    assert(w > 0);
    weight_ = w;
  }

  virtual std::string to_str() const override {
    return (std::ostringstream{} << *this).str();
  }

  template <typename Char, typename Traits>
  std::basic_ostream<Char, Traits>& output_members(std::basic_ostream<Char, Traits>& os) const {
    return base_t::output_members(os) << ",w:" << weight();
  }

private:
  int weight_;
};

template <typename Char, typename Traits, typename NodeBaseType>
std::basic_ostream<Char, Traits>& operator<<(std::basic_ostream<Char, Traits>& os,
                                             const WeightedNodeWrapper<NodeBaseType>& n) {
  os << "{";
  n.output_members(os);
  os << "}";
  return os;
}


}  // namespace maglev
