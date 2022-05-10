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

#include <iostream>
#include <map>
#include <string>
#include <unordered_map>

namespace maglev {

template <typename BaseType,
          typename MapType = std::map<std::string, std::string>>
class extra_wrapper : public BaseType {
  using base_t = BaseType;

public:
  using extra_t       = MapType;
  using extra_key_t   = typename extra_t::key_type;
  using extra_value_t = typename extra_t::mapped_type;

  template <typename... Args>
  extra_wrapper(Args&&... args) : base_t(std::forward<Args>(args)...) {}

  void extra_set(const extra_key_t& k, const extra_value_t& v) {
    extra_[k] = v;
  }

  void extra_set(const extra_key_t& k, extra_value_t&& v) {
    extra_[k] = std::move(v);
  }

  extra_value_t extra_get(const extra_key_t&   k,
                          const extra_value_t& def = extra_value_t{}) const {
    auto it = extra_.find(k);
    return it == extra_.end() ? def : it->second;
  }

  void extra_del(const extra_key_t& k) { extra_.erase(k); }

  bool extra_has(const extra_key_t& k) const {
    return extra_.find(k) != extra_.end();
  }

  bool extra_has(const extra_key_t& k, const extra_value_t& v) const {
    auto it = extra_.find(k);
    return it == extra_.end() ? false : it->second == v;
  }

  const extra_t& extra() const { return extra_; }

  extra_t& extra() { return extra_; }

  virtual std::string to_str() const override {
    return (std::ostringstream{} << *this).str();
  }

  template <typename Char, typename Traits>
  std::basic_ostream<Char, Traits>& output_members(
      std::basic_ostream<Char, Traits>& os) const {
    base_t::output_members(os);
    os << ",extra:{";
    for (auto it = extra_.begin(), b = extra_.begin(), e = extra_.end();
         it != e;
         ++it) {
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
std::basic_ostream<Char, Traits>& operator<<(
    std::basic_ostream<Char, Traits>&       os,
    const extra_wrapper<BaseType, MapType>& extra_wrapper) {
  os << "{";
  extra_wrapper.output_members(os);
  os << "}";
  return os;
}

}  // namespace maglev
