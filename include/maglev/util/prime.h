#pragma once


namespace maglev {


template <typename T, typename V>
inline constexpr T power(T x, unsigned long long n, V mod) {
  x %= mod;
  T ret = 1;
  while (n > 0) {
    if (n & 1) ret = ret * x % mod;;
    x = x * x % mod;
    n >>= 1;
  }
  return ret;
}

// MillerRabin prime test for n < 4,759,123,141, (4,759,123,141 > 4 294 967 295)
inline constexpr bool is_prime(unsigned int n) {
  if (n == 2) return true;
  if ((~n & 1) || n == 1) return false;

  // form n - 1 = d * 2 ^ r//
  unsigned int d = (n - 1) >> 1;
  unsigned int r = 1;
  while (~d & 1) {
    d >>= 1;
    ++r;
  }

  constexpr unsigned int a[3] = {2, 7, 61};
  for (int i = 0; i < 3; ++i) {
    if (a[i] == n) return true;
    auto x = power((unsigned long long) (a[i]), d, n);
    if (x == 1 || x == n - 1) continue;
    // repeat r-1 times
    for (int k = 1; k < r; ++k) {
      x = 1ULL * x * x % n;
      if (x == 1) return false;
      if (x == n - 1) break;
    }
    if (x != n - 1) return false; // a^( (n-1)/2 )%n != 1 or n-1
  }
  return true;
}


}  // namespace maglev
