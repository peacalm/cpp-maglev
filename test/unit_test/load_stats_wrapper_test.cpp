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

TEST(stats, node_stats_wrapper) {
  using node_t = maglev::load_stats_wrapper<maglev::node_base<int>,
                                            maglev::load_stats<int, 1, 64>>;
  node_t a(1);
  EXPECT_EQ(a.id(), 1);
  EXPECT_EQ(a.load().now(), 0);
  a.load().incr(1);
  EXPECT_EQ(a.load().now(), 1);
  a.heartbeat();
  EXPECT_EQ(a.load().now(), 0);
  EXPECT_EQ(a.load().last(), 1);
  EXPECT_EQ(a.load().sum(), 1);
}
