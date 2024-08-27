// Copyright (c) 2021-2022 Li Shuangquan. All Rights Reserved.
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

#include <sstream>
#include <string>
#include <type_traits>
#include <utility>

#include "maglev/util/hash.h"
#include "maglev/util/to_str.h"

namespace maglev {

// std::hash for integer got self, which may cause bad performance in maglev,
// so use maglev_int_hash as default. Or you can specify your own hash method.
template <typename IdType = std::string, typename HashType = def_hash_t<IdType>>
class node_base {
public:
  using node_id_t = IdType;
  using hash_t    = HashType;

public:
  template <typename... Args>
  node_base(Args&&... args) : id_(std::forward<Args>(args)...) {}

  node_base(const node_id_t& id) : id_(id) {}

  node_base(node_id_t&& id) : id_(std::move(id)) {}

  // delete copy and move
  node_base(const node_base&)            = delete;
  node_base(node_base&&)                 = delete;
  node_base& operator=(const node_base&) = delete;
  node_base& operator=(node_base&&)      = delete;

  node_id_t id() const { return id_; }

  size_t id_hash() const { return hash_t{}(id()); }

  bool operator<(const node_base& rhs) const { return id() < rhs.id(); }

  virtual std::string to_str() const { return maglev::to_str(*this); }

  template <typename Char, typename Traits>
  std::basic_ostream<Char, Traits>& output_members(
      std::basic_ostream<Char, Traits>& os) const {
    return os << "id:" << id();
  }

private:
  node_id_t id_;  // node id should be unique
};

template <typename Char, typename Traits, typename IdType, typename HashType>
std::basic_ostream<Char, Traits>& operator<<(
    std::basic_ostream<Char, Traits>&  os,
    const node_base<IdType, HashType>& n) {
  os << "{";
  n.output_members(os);
  os << "}";
  return os;
}

}  // namespace maglev
