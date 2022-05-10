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

#include <type_traits>

namespace maglev {

template <typename IntType = long long>
struct maglev_int_hash {
  static_assert(std::is_integral<IntType>::value,
                "maglev_int_hash only support integral types");
  using type =
      typename std::enable_if<std::is_integral<IntType>::value, IntType>::type;

  size_t operator()(type n) const {
    size_t x = static_cast<size_t>(n);
    x        = (x ^ (x >> 30)) * 0xbf58476d1ce4e5b9ull;
    x        = (x ^ (x >> 27)) * 0x94d049bb133111ebull;
    x        = x ^ (x >> 31);
    // Here hack 0 to 0x9e3779b97f4a7c16ull to ensure final hash result is
    // always non-zero! Notice this will make a hash collision by input n = 0
    // and n = 4440575067278254172ull, but the latter is not a frequently used
    // number, so it's not a big problem.
    return x != 0 ? x : 0x9e3779b97f4a7c16ull;
  }
};

// std::hash for integer got self, which may cause bad performance in maglev,
// so use maglev_int_hash as default.
template <typename T>
using def_hash_t = typename std::conditional<
    std::is_integral<T>::value,
    maglev_int_hash<typename std::conditional<std::is_integral<T>::value,
                                              T,
                                              long long>::type>,
    std::hash<T>>::type;

}  // namespace maglev
