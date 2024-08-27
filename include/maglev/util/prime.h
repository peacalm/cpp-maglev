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

namespace maglev {

template <typename T, typename V>
inline constexpr T power(T x, unsigned long long n, V mod) {
  x %= mod;
  T ret = 1;
  while (n > 0) {
    if (n & 1) ret = ret * x % mod;
    x = x * x % mod;
    n >>= 1;
  }
  return ret;
}

// MillerRabin prime test for n < 4,759,123,141 (4,759,123,141 > 4,294,967,295)
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
    auto x = power((unsigned long long)(a[i]), d, n);
    if (x == 1 || x == n - 1) continue;
    // repeat r-1 times
    for (int k = 1; k < r; ++k) {
      x = 1ULL * x * x % n;
      if (x == 1) return false;
      if (x == n - 1) break;
    }
    if (x != n - 1) return false;  // a^( (n-1)/2 )%n != 1 or n-1
  }
  return true;
}

}  // namespace maglev
