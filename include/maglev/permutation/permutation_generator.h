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

#include <cassert>
#include <cstdlib>
#include <iostream>
#include <vector>

#include "maglev/util/prime.h"

namespace maglev {

/**
 * Generate a permutation of number from 0 to n-1
 * NOTICE: n must be a prime number and n > 1
 */
class permutation_generator {
public:
  using hash64_t = unsigned long long;
  using num_t    = unsigned int;

public:
  permutation_generator(size_t n) : n_(n), offset_(0), step_(1) { assert_n(); }

  permutation_generator(size_t n, hash64_t hash64bit) : n_(n) {
    assert_n();
    hash_all(hash64bit);
  }

  void set_n(size_t n) {
    n_ = n;
    assert_n();
  }

  void set_offset(num_t offset = 0) {
    assert(offset >= 0);
    assert(offset < n_);
    offset_ = offset;
  }

  void set_step(num_t step = 1) {
    assert(step >= 1);
    assert(step < n_);
    step_ = step;
  }

  void hash_offset(hash64_t h4offset) { offset_ = h4offset % n_; }

  void hash_step(hash64_t h4step) { step_ = h4step % (n_ - 1) + 1; }

  void hash_all(hash64_t hash64bit) {
    offset_ = (hash64bit & 0X5555555555555555ULL) % n_;
    step_   = (hash64bit & 0XaaaaaaaaaaaaaaaaULL) % (n_ - 1) + 1;
  }

  num_t gen_one_num() {
    num_t ret = offset_;
    assert(ret >= 0);
    assert(ret < n_);
    offset_ = (offset_ + step_) % n_;
    return ret;
  }

  template <typename IntType = num_t>
  std::vector<IntType> gen_permutation() {
    std::vector<IntType> p(n_);
    for (size_t i = 0; i < n_; ++i) p[i] = gen_one_num();
    return p;
  }

  size_t n() const { return n_; }
  num_t  offset() const { return offset_; }
  num_t  step() const { return step_; }

private:
  void assert_n() {
    assert(n_ > 1);
    assert(is_prime(n_));
  }

private:
  size_t n_;
  num_t  offset_;
  num_t  step_;
};

class permutation_generator_with_rand : public permutation_generator {
  using base_t = permutation_generator;

public:
  using seed_t     = unsigned int;
  using rand_num_t = int;
  using hash64_t   = typename base_t::hash64_t;

public:
  permutation_generator_with_rand(size_t n) : base_t(n), seed_(0) {}

  // Use same hash value for seed as default.
  permutation_generator_with_rand(size_t n, hash64_t h)
      : base_t(n, h), seed_(h) {}

  permutation_generator_with_rand(size_t n, hash64_t h, seed_t s)
      : base_t(n, h), seed_(s) {}

  void set_seed(seed_t s) { seed_ = s; }

  rand_num_t my_rand_max() const { return RAND_MAX; }

  rand_num_t my_rand() const { return rand_r(&seed_); }

  double my_rand_pure_decimal() const {
    return double(my_rand()) / double(my_rand_max());
  }

private:
  mutable seed_t seed_;
};

}  // namespace maglev
