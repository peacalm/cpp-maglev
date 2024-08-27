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

#include "unit_test.h"

TEST(permutation, permutation_generator) {
  int                           n = 31;
  maglev::permutation_generator g(n);
  for (int i = 0; i < n; ++i) { EXPECT_EQ(g.gen_one_num(), i); }

  g.hash_all(maglev::def_hash_t<int>{}(23596985));
  std::set<int> s1, s2;
  for (int i = 0; i < n; ++i) {
    auto t = g.gen_one_num();
    s1.insert(t);
    s2.insert(i);
  }
  EXPECT_EQ(s1, s2);

  s1.clear();
  s2.clear();
  auto p = g.gen_permutation();
  for (int i = 0; i < n; ++i) {
    s1.insert(p[i]);
    s2.insert(i);
  }
  EXPECT_EQ(s1, s2);
  maglev_watch(p);
}

TEST(permutation, permutation_generator_with_rand) {
  int                                     n = 53;
  maglev::permutation_generator_with_rand g1(
      n, maglev::node_base<>("id11").id_hash());
  maglev::permutation_generator_with_rand g2(
      n, maglev::node_base<>("id22").id_hash());

  std::set<int> s1, s2, s3;
  std::string   p1, p2;
  for (int i = 0; i < n; ++i) {
    auto t1 = g1.gen_one_num();
    auto t2 = g2.gen_one_num();
    s1.insert(t1);
    s2.insert(t2);
    s3.insert(i);
    p1 += std::to_string(t1);
    p2 += std::to_string(t2);
    EXPECT_NE(g1.my_rand(), g2.my_rand());
  }
  EXPECT_EQ(s1, s2);
  EXPECT_EQ(s1, s3);
  EXPECT_EQ(s2, s3);

  EXPECT_NE(p1, p2);
}