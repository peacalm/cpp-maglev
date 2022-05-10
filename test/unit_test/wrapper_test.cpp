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

TEST(wrapper, extra_wrapper) {
  maglev::extra_wrapper<maglev::node_base<>> n("string-type-node-id");
  n.extra_set("ip", "1.1.1.1");
  n.extra_set("port", "22");
  std::cout << "extra wrapped node base: " << n << std::endl;

  EXPECT_TRUE(n.extra_has("ip"));
  EXPECT_TRUE(n.extra_has("port"));
  EXPECT_TRUE(n.extra_has("port", "22"));
  EXPECT_FALSE(n.extra_has("port", "33"));

  n.extra_set("port", "33");
  std::cout << "extra wrapped node base: " << n << std::endl;

  EXPECT_TRUE(n.extra_has("port"));
  EXPECT_FALSE(n.extra_has("port", "22"));
  EXPECT_TRUE(n.extra_has("port", "33"));
  EXPECT_EQ(n.extra_get("port", ""), "33");
  EXPECT_EQ(n.extra_get("nx", "xx"), "xx");

  n.extra_del("port");
  std::cout << "extra wrapped node base: " << n << std::endl;

  EXPECT_FALSE(n.extra_has("port"));
  EXPECT_FALSE(n.extra_has("port", "22"));
  EXPECT_FALSE(n.extra_has("port", "33"));
}