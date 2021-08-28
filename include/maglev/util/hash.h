#pragma once

#include <type_traits>

namespace maglev {


template <typename IntType = long long>
struct MaglevIntHash {
  static_assert(std::is_integral<IntType>::value, "MaglevIntHash only support integral types");
  using type = typename std::enable_if<std::is_integral<IntType>::value, IntType>::type;

  size_t operator()(type n) const {
    size_t x = static_cast<size_t>(n);
    x = (x ^ (x >> 30)) * 0xbf58476d1ce4e5b9ull;
    x = (x ^ (x >> 27)) * 0x94d049bb133111ebull;
    x = x ^ (x >> 31);
    // Ensure result is non-zero! Notice this will make input n = 0 conflict with
    // n = 4440575067278254172ull, but the latter is not a frequently used number,
    // so it's not a big problem.
    return x != 0 ? x : 0x9e3779b97f4a7c16ull;
  }
};


// std::hash for integer got self, which may cause bad performance in maglev,
// so use MaglevIntHash as default. Or you can specify your own hash method.
template <typename T>
using def_hash_t = typename std::conditional<std::is_integral<T>::value,
    MaglevIntHash<typename std::conditional<std::is_integral<T>::value, T, long long>::type>,
    std::hash<T>>::type;


}  // namespace maglev
