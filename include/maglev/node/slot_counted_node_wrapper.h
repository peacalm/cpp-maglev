#pragma once

#include <sstream>
#include <string>
#include <type_traits>
#include <utility>

namespace maglev {


template <typename NodeBaseType>
class SlotCountedNodeWrapper : public NodeBaseType {
  using base_t = NodeBaseType;

public:
  using slot_counted_t = void;  // for type_trait

public:
  template <typename ...Args>
  SlotCountedNodeWrapper(Args&& ...id) : slot_cnt_(0), base_t(std::forward<Args>(id)...) {}

  void incr_slot_cnt(int d = 1) { slot_cnt_ += d; }

  int slot_cnt() const { return slot_cnt_; }

  void set_slot_cnt(int s) { slot_cnt_ = s; }

  virtual std::string to_str() const override;

  template <typename Char, typename Traits>
  std::basic_ostream<Char, Traits>& output_members(std::basic_ostream<Char, Traits>& os) const {
    return base_t::output_members(os) << ",s:" << slot_cnt();
  }

private:
  int slot_cnt_ = 0;  // slot num obtained from maglev-hasher
};

template <typename Char, typename Traits, typename NodeBaseType>
std::basic_ostream<Char, Traits>& operator<<(std::basic_ostream<Char, Traits>& os,
                                             const SlotCountedNodeWrapper<NodeBaseType>& n) {
  os << "{";
  n.output_members(os);
  os << "}";
  return os;
}

template <typename NodeBaseType>
std::string SlotCountedNodeWrapper<NodeBaseType>::to_str() const {
  std::ostringstream os;
  os << *this;
  return os.str();
}


}  // namespace maglev
