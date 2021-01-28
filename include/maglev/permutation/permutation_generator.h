#pragma once

#include <cassert>
#include <cstdlib>
#include <iostream>
#include <vector>

#include "maglev/util/prime.h"

namespace maglev {


/**
 * generate a permutation of number from 0 to n-1
 * NOTICE: n must be a prime number and n > 1
 */
class PermutationGenerator {
public:
  using hash64_t = unsigned long long;
  using num_t = unsigned int;

public:
  PermutationGenerator(size_t n) : n_(n), offset_(0), step_(1) {
    assert_n();
  }

  PermutationGenerator(size_t n, hash64_t hash64bit) : n_(n) {
    assert_n();
    hash_all(hash64bit);
  }

  PermutationGenerator& with_n(size_t n) {
    n_ = n;
    assert_n();
    return *this;
  }

  PermutationGenerator& with_offset(num_t offset = 0) {
    assert(offset >= 0);
    assert(offset < n_);
    offset_ = offset;
    return *this;
  }

  PermutationGenerator& with_step(num_t step = 1) {
    assert(step >= 1);
    assert(step < n_);
    step_ = step;
    return *this;
  }

  PermutationGenerator& hash_offset(hash64_t h4offset) {
    offset_ = h4offset % n_;
    return *this;
  }

  PermutationGenerator& hash_step(hash64_t h4step) {
    step_ = h4step % (n_ - 1) + 1;
    return *this;
  }

  PermutationGenerator& hash_all(hash64_t hash64bit) {
    offset_ = (hash64bit >> 32) % n_;
    step_ = (hash64bit & 0xffffffff) % (n_ - 1) + 1;
    return *this;
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

private:
  void assert_n() {
    assert(n_ > 1);
    assert(is_prime(n_));
  }

private:
  size_t n_;
  num_t offset_;
  num_t step_;
};


class PermutationGeneratorWithRand : public PermutationGenerator {
  using base_t = PermutationGenerator;

public:
  using seed_t = unsigned int;
  using rand_num_t = int;
  using hash64_t = typename base_t::hash64_t;

public:
  PermutationGeneratorWithRand(size_t n) : base_t(n), seed_(0) {}

  // use same hash value for seed as default
  PermutationGeneratorWithRand(size_t n, hash64_t h) : base_t(n, h), seed_(h) {}

  PermutationGeneratorWithRand(size_t n, hash64_t h, seed_t s) : base_t(n, h), seed_(s) {}

  PermutationGeneratorWithRand& with_rand_seed(seed_t s) {
    seed_ = s;
    return *this;
  }

  rand_num_t my_rand_max() const { return RAND_MAX; }

  rand_num_t my_rand() const { return rand_r(&seed_); }

  double my_rand_pure_decimal() const { return double(my_rand()) / double(my_rand_max()); }

private:
  mutable seed_t seed_;
};


}  // namespace maglev
