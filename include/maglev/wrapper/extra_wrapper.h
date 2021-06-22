#pragma once

#include <iostream>
#include <map>
#include <string>
#include <unordered_map>

namespace maglev {

template <typename BaseType, typename MapType = std::map<std::string, std::string>>
class ExtraWrapper : public BaseType {
  using base_t = BaseType;

public:
  using extra_t = MapType;
  using extra_key_t = typename extra_t::key_type;
  using extra_value_t = typename extra_t::mapped_type;

  template <typename ...Args>
  ExtraWrapper(Args&& ... args) : base_t(std::forward<Args>(args)...) {}

  void extra_set(const extra_key_t& k, const extra_value_t& v) {
    extra_.emplace(std::make_pair(k, v));
  }

  void extra_set(const extra_key_t& k, extra_value_t&& v) {
    extra_.emplace(std::make_pair(k, std::move(v)));
  }

  bool extra_has(const extra_key_t& k) const {
    return extra_.find(k) != extra_.end();
  }

  extra_value_t extra_get(const extra_key_t& k, const extra_value_t& def = extra_value_t{}) const {
    auto it = extra_.find(k);
    if (it == extra_.end()) {
      return def;
    }
    return it->second;
  }

  const extra_t& extra() const { return extra_; }

  extra_t& extra() { return extra_; }

  virtual std::string to_str() const override;

  template <typename Char, typename Traits>
  std::basic_ostream<Char, Traits>& output_members(std::basic_ostream<Char, Traits>& os) const {
    base_t::output_members(os);
    os << ",extra:{";
    for (auto it = extra_.begin(), b = extra_.begin(), e = extra_.end(); it != e; ++it) {
      if (it != b) os << ",";
      os << it->first << ":" << it->second;
    }
    os << "}";
    return os;
  }

private:
  extra_t extra_;
};

template <typename Char, typename Traits, typename BaseType, typename MapType>
std::basic_ostream<Char, Traits>& operator<<(std::basic_ostream<Char, Traits>& os,
                                             const ExtraWrapper<BaseType, MapType>& extra_wrapper) {
  os << "{";
  extra_wrapper.output_members(os);
  os << "}";
  return os;
}

template <typename BaseType, typename MapType>
std::string ExtraWrapper<BaseType, MapType>::to_str() const {
  std::ostringstream os;
  os << *this;
  return os.str();
}

}  // namespace maglev
