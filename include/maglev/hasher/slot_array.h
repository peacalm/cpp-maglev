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

#include <array>
#include <type_traits>
#include <vector>

#include "maglev/util/prime.h"

namespace maglev {

template <typename IntType = int, size_t SlotNum = 65537>
class slot_array : public std::array<IntType, SlotNum> {
  using base_t = std::array<IntType, SlotNum>;

public:
  using int_t = typename std::enable_if<std::is_integral<IntType>::value &&
                                            is_prime(SlotNum),
                                        IntType>::type;
};

template <typename IntType = int, typename AllocType = std::allocator<IntType>>
class slot_vector : public std::vector<IntType, AllocType> {
  using base_t          = std::vector<IntType, AllocType>;
  using size_type       = typename base_t::size_type;
  using const_reference = typename base_t::const_reference;

public:
  using int_t =
      typename std::enable_if<std::is_integral<IntType>::value, IntType>::type;

public:
  slot_vector() {}

  slot_vector(size_type n) : base_t(n) { assert(is_prime(n)); }

  void resize(size_type n) {
    assert(is_prime(n));
    base_t::resize(n);
  }

  void resize(size_type n, const_reference v) {
    assert(is_prime(n));
    base_t::resize(n, v);
  }
};

}  // namespace maglev
