#pragma once

#include <sstream>
#include <string>
#include <type_traits>
#include <utility>

namespace maglev {
namespace node {

template <typename NodeBaseType>
class SlotCountedNodeBase: public NodeBaseType {
  using base_t = NodeBaseType;

public:
  template <typename ...Args>
  SlotCountedNodeBase(Args&& ...id) : base_t(std::forward<Args>(id)...), slot_cnt_(0) {}

  void incr_slot_cnt(int d = 1) { slot_cnt_ += d; }

  int slot_cnt() const { return slot_cnt_; }

  virtual std::string to_str() const override;

  template <typename Char, typename Traits>
  std::basic_ostream<Char, Traits>& output_members(std::basic_ostream<Char, Traits>& os) const {
    return base_t::output_members(os) << ",s:" << slot_cnt();
  }

private:
  int slot_cnt_ = 0;  // slot num obtained from maglev-hasher
};

template <typename Char, typename Traits, typename NodeBaseType>
std::basic_ostream<Char, Traits>& operator << (std::basic_ostream<Char, Traits>& os,
                                               const SlotCountedNodeBase<NodeBaseType>& n) {
  os << "{";
  n.output_members(os);
  os << "}";
  return os;
}

template <typename NodeBaseType>
std::string SlotCountedNodeBase<NodeBaseType>::to_str() const {
  std::ostringstream os;
  os << *this;
  return os.str();
}


}  // namespace node
}  // namespace maglev
