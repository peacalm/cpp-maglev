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
  EXPECT_EQ(nm.size(), 4);

  EXPECT_FALSE(nm.is_sorted());
  nm.sort();
  EXPECT_TRUE(nm.is_sorted());

  std::cout << "node_manager_base: " << nm << std::endl;
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

  std::cout << "weighted_node_manager_wrapper: " << nm << std::endl;

  nm.set_max_avg_rate_limit(1.5);
  nm.init_weight();
  EXPECT_EQ(nm.max_weight(), 7000);
  EXPECT_LT(nm.limited_max_weight(), 7000);

  std::cout << "weighted_node_manager_wrapper: " << nm << std::endl;
}
