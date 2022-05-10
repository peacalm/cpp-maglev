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

TEST(node, node_base) {
  maglev::node_base<> sn("string-type-node-id");
  EXPECT_EQ(sn.id(), "string-type-node-id");
  EXPECT_NE(sn.id_hash(), 0);
  EXPECT_EQ(maglev::node_base<>("").id(), maglev::node_base<>().id());
  EXPECT_NE(maglev::node_base<>("").id_hash(), 0);

  maglev::node_base<int> in(123);
  EXPECT_EQ(in.id(), 123);
  EXPECT_NE(in.id_hash(), 0);
  EXPECT_NE(maglev::node_base<int>(0).id_hash(), 0);
  EXPECT_NE(maglev::node_base<int>().id_hash(), 0);

  std::cout << "node_base: " << sn << in << std::endl;
}

TEST(node, server_node_base) {
  maglev::server_node_base<> n("1.2.3.4", 88);
  EXPECT_EQ(n.ip(), "1.2.3.4");
  EXPECT_EQ(n.port(), 88);
  EXPECT_EQ(n.id(), "1.2.3.4:88");
  EXPECT_NE(n.id_hash(), 0);

  maglev::server_node_base<> n2("1.2.3.4", 88, "str-id");
  EXPECT_EQ(n2.id(), "str-id");

  std::cout << "server_node_base: " << n << n2 << std::endl;
}

TEST(node, slot_counted_node_wrapper) {
  maglev::slot_counted_node_wrapper<maglev::node_base<int>> n(123);
  EXPECT_EQ(n.id(), 123);
  EXPECT_EQ(n.slot_cnt(), 0);
  n.incr_slot_cnt();
  EXPECT_EQ(n.slot_cnt(), 1);
  n.incr_slot_cnt(10);
  EXPECT_EQ(n.slot_cnt(), 11);
  n.set_slot_cnt(13);
  EXPECT_EQ(n.slot_cnt(), 13);

  std::cout << "slot_counted_node_wrapper: " << n << std::endl;
}

TEST(node, weighted_node_wrapper) {
  maglev::weighted_node_wrapper<maglev::node_base<int>> n(123);
  EXPECT_EQ(n.id(), 123);
  EXPECT_EQ(n.weight(), 0);
  n.set_weight(90);
  EXPECT_EQ(n.weight(), 90);

  std::cout << "weighted_node_wrapper: " << n << std::endl;
}

TEST(node, complex) {
  maglev::slot_counted_node_wrapper<
      maglev::weighted_node_wrapper<maglev::node_base<int>>>
      n1(123);
  n1.set_slot_cnt(13);
  n1.set_weight(90);

  maglev::weighted_node_wrapper<
      maglev::slot_counted_node_wrapper<maglev::node_base<int>>>
      n2(123);
  n2.set_slot_cnt(13);
  n2.set_weight(90);

  EXPECT_EQ(n1.weight(), n2.weight());
  EXPECT_EQ(n1.slot_cnt(), n2.slot_cnt());
  EXPECT_EQ(n1.id(), n2.id());

  maglev::weighted_node_wrapper<
      maglev::slot_counted_node_wrapper<maglev::server_node_base<>>>
      n3("10.0.0.1", 88);
  n3.set_slot_cnt(13);
  n3.set_weight(20);

  std::cout << "complex node: " << n1 << n2 << n3 << std::endl;
}