#pragma once

#include <array>
#include <type_traits>
#include <vector>

#include "maglev/util/prime.h"

namespace maglev {


template <typename IntType = int, size_t SlotNum = 65537>
class SlotArray : public std::array<IntType, SlotNum> {
  using base_t = std::array<IntType, SlotNum>;

public:
  using int_t = typename std::enable_if<std::is_integral<IntType>::value && is_prime(SlotNum), IntType>::type;
};


template <typename IntType = int, typename AllocType = std::allocator<IntType>>
class SlotVector : public std::vector<IntType, AllocType> {
  using base_t = std::vector<IntType, AllocType>;
  using size_type = typename base_t::size_type;
  using const_reference = typename base_t::const_reference;

public:
  using int_t = typename std::enable_if<std::is_integral<IntType>::value, IntType>::type;

public:
  SlotVector() {}

  SlotVector(size_type n) : base_t(n) { assert(is_prime(n)); }

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
