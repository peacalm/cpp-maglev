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

#include "unit_test.h"

TEST(node_manager, node_manager_base) {
  maglev::node_manager_base<maglev::node_base<int>> nm;
  EXPECT_TRUE(nm.empty());
  nm.new_back(3);
  nm.new_back(9);
  nm.push_back(nm.new_node(2));
  nm.new_back(7);
  nm.new_back(10);
  nm.new_back(18);
  EXPECT_EQ(nm.size(), 6);

  EXPECT_FALSE(nm.is_sorted());
  nm.sort();
  EXPECT_TRUE(nm.is_sorted());

  auto i2 = nm.find_by_node_id(2);
  EXPECT_TRUE(i2 != nullptr);
  EXPECT_EQ(i2->id(), 2);

  auto i3 = nm.find_by_node_id(3);
  EXPECT_TRUE(i3 != nullptr);
  EXPECT_EQ(i3->id(), 3);

  auto i9 = nm.find_by_node_id(9);
  EXPECT_TRUE(i9 != nullptr);
  EXPECT_EQ(i9->id(), 9);

  auto i1 = nm.find_by_node_id(1);
  EXPECT_TRUE(i1 == nullptr);
  auto i8 = nm.find_by_node_id(8);
  EXPECT_TRUE(i8 == nullptr);
  auto i100 = nm.find_by_node_id(100);
  EXPECT_TRUE(i100 == nullptr);

  maglev_watch_with_std_cout(nm);
}

TEST(node_manager, weighted_node_manager_wrapper) {
  maglev::weighted_node_manager_wrapper<maglev::node_manager_base<
      maglev::weighted_node_wrapper<maglev::node_base<int>>>>
      nm;
  EXPECT_TRUE(nm.empty());
  nm.new_back(3)->set_weight(30);
  nm.new_back(9)->set_weight(90);
  nm.push_back(nm.new_node(2));
  nm.new_back(7)->set_weight(7000);
  EXPECT_EQ(nm.size(), 4);

  EXPECT_FALSE(nm.is_sorted());
  nm.sort();
  EXPECT_TRUE(nm.is_sorted());

  nm.init_weight();
  EXPECT_EQ(nm.max_weight(), 7000);
  EXPECT_EQ(nm.limited_max_weight(), 7000);

  maglev_watch_with_std_cout(nm);

  nm.set_max_avg_rate_limit(1.5);
  nm.init_weight();
  EXPECT_EQ(nm.max_weight(), 7000);
  EXPECT_LT(nm.limited_max_weight(), 7000);

  maglev_watch_with_std_cout(nm);
}

TEST(node_manager, weighted_node_manager_same_weights) {
  using node_type = maglev::load_stats_wrapper<
      maglev::weighted_node_wrapper<maglev::node_base<int>>,
      maglev::load_stats<>>;
  maglev::weighted_node_manager_wrapper<maglev::node_manager_base<node_type>>
      nm;
  EXPECT_TRUE(nm.empty());
  nm.new_back(1)->set_weight(10);
  nm.new_back(2)->set_weight(10);
  nm.new_back(3)->set_weight(10);
  nm.new_back(4)->set_weight(10);
  EXPECT_EQ(nm.size(), 4);

  nm.ready_go();
  nm.find_by_node_id(1)->incr_load();
  nm.find_by_node_id(2)->incr_load();
  nm.init_load_units(1);
  nm.find_by_node_id(3)->incr_load();
  nm.find_by_node_id(4)->incr_load();
  maglev_watch_with_std_cout(nm);
}
