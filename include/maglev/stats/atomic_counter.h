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

#include <atomic>
#include <type_traits>

namespace maglev {

/// Atomic integral counter, `Unit` used as the operand for operator ++ and --.
template <typename IntType = int, IntType Unit = 1>
class atomic_counter {
  static_assert(std::is_integral<IntType>::value &&
                    !std::is_same<IntType, bool>::value,
                "atomic_counter should contain a integral type");

public:
  using value_t   = IntType;
  using counter_t = typename std::atomic<value_t>;

  value_t unit() const { return Unit; }

public:
  atomic_counter(value_t v = 0) : cnt_(v) {}
  atomic_counter(const atomic_counter& r) : cnt_(r) {}

  operator value_t() const noexcept { return get(); }

  value_t operator=(value_t v) noexcept {
    set(v);
    return v;
  }

  value_t operator+(value_t v) const noexcept { return get() + v; }
  value_t operator-(value_t v) const noexcept { return get() - v; }
  value_t operator+=(value_t v) noexcept { return incr(v); }
  value_t operator-=(value_t v) noexcept { return decr(v); }

  value_t operator++() noexcept { return incr(unit()); }
  value_t operator--() noexcept { return decr(unit()); }
  value_t operator++(int) noexcept { return fetch_add(unit()); }
  value_t operator--(int) noexcept { return fetch_sub(unit()); }

protected:
  value_t get() const noexcept { return cnt_.load(std::memory_order_relaxed); }

  void set(value_t v) noexcept { cnt_.store(v, std::memory_order_relaxed); }

  value_t fetch_add(value_t delta) noexcept {
    return cnt_.fetch_add(delta, std::memory_order_relaxed);
  }

  value_t fetch_sub(value_t delta) noexcept {
    return cnt_.fetch_sub(delta, std::memory_order_relaxed);
  }

  void clear() noexcept { set(value_t{0}); }

  value_t incr(value_t delta) noexcept { return fetch_add(delta) + delta; }

  value_t decr(value_t delta) noexcept { return fetch_sub(delta) - delta; }

private:
  counter_t cnt_{0};
};

}  // namespace maglev
