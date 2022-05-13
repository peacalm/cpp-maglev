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

#include <array>

namespace maglev {

/// Index for a cycle array, integral value in [0, Size-1].
template <size_t Size = 64>
class cycle_index {
  static constexpr bool is_power_of_two = ((Size & (Size - 1)) == 0);

public:
  using index_value_t = size_t;

  cycle_index(index_value_t v = 0) : i_(mod(v)) {}
  cycle_index(const cycle_index& r) : i_(r.i_) {}
  cycle_index(cycle_index&& r) : i_(r.i_) {}

  constexpr size_t size() const { return Size; }

  index_value_t get() const { return i_; }

  operator index_value_t() { return i_; }

  cycle_index& operator=(cycle_index r) {
    i_ = r.i_;
    return *this;
  }

  cycle_index& operator+=(cycle_index r) {
    i_ = next(r.i_);
    return *this;
  }

  cycle_index& operator-=(cycle_index r) {
    i_ = next(size() - r.i_);
    return *this;
  }

  // Suffix increment and decrement
  cycle_index& operator++() {
    i_ = next(1);
    return *this;
  }
  cycle_index& operator--() {
    i_ = next(size() - 1);
    return *this;
  }

  // Prefix increment and decrement
  cycle_index operator++(int) {
    cycle_index ret(*this);
    i_ = next(1);
    return ret;
  }
  cycle_index operator--(int) {
    cycle_index ret(*this);
    i_ = next(size() - 1);
    return ret;
  }

  cycle_index operator+(cycle_index r) const { return cycle_index(next(r.i_)); }
  cycle_index operator-(cycle_index r) const {
    return cycle_index(next(size() - r.i_));
  }

  index_value_t next(index_value_t delta) const { return mod(i_ + delta); }

  index_value_t mod(index_value_t v) const { return __mod<is_power_of_two>(v); }

  bool operator==(const cycle_index& r) const { return i_ == r.i_; }
  bool operator!=(const cycle_index& r) const { return i_ != r.i_; }
  bool operator<(const cycle_index& r) const { return i_ < r.i_; }
  bool operator<=(const cycle_index& r) const { return i_ <= r.i_; }
  bool operator>(const cycle_index& r) const { return i_ > r.i_; }
  bool operator>=(const cycle_index& r) const { return i_ >= r.i_; }

  bool operator==(index_value_t r) const { return i_ == r; }
  bool operator!=(index_value_t r) const { return i_ != r; }
  bool operator<(index_value_t r) const { return i_ < r; }
  bool operator<=(index_value_t r) const { return i_ <= r; }
  bool operator>(index_value_t r) const { return i_ > r; }
  bool operator>=(index_value_t r) const { return i_ >= r; }

private:
  template <bool>
  index_value_t __mod(index_value_t v) const;

  template <>
  index_value_t __mod<false>(index_value_t v) const {
    return v % size();
  }
  template <>
  index_value_t __mod<true>(index_value_t v) const {
    return v & (size() - 1);
  }

private:
  index_value_t i_ = 0;
};

template <typename Char, typename Traits, size_t Size>
std::basic_ostream<Char, Traits>& operator<<(
    std::basic_ostream<Char, Traits>& os, const cycle_index<Size> i) {
  os << i.get();
  return os;
}

/// Cycle array with constant size. Prefer the size number to be power of 2.
template <typename T, size_t Size = 64>
class cycle_array {
  static constexpr bool is_power_of_two = ((Size & (Size - 1)) == 0);

public:
  using item_t        = T;
  using index_t       = cycle_index<Size>;
  using index_value_t = typename index_t::index_value_t;

public:
  constexpr size_t size() const { return Size; }

  const index_t& index() const { return i_; }
  index_t&       index() { return i_; }

  const item_t& curr_item() const { return a_[i_.get()]; }
  item_t&       curr_item() { return a_[i_.get()]; }
  const item_t& next_item() const { return a_[i_.next(1)]; }
  item_t&       next_item() { return a_[i_.next(1)]; }
  const item_t& prev_item() const { return a_[i_.next(size() - 1)]; }
  item_t&       prev_item() { return a_[i_.next(size() - 1)]; }

  const item_t& operator[](index_t i) const { return a_[i.get()]; }
  item_t&       operator[](index_t i) { return a_[i.get()]; }

  void push(const item_t& i) { a_[i_++] = i; }
  void push(item_t&& i) { a_[i_++] = std::move(i); }

private:
  std::array<item_t, Size> a_;
  index_t                  i_ = 0;
};

}  // namespace maglev
