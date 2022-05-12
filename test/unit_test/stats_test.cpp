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

#include <atomic>

#include "unit_test.h"

TEST(stats, atomic_counter) {
  maglev::atomic_counter<> a;
  EXPECT_EQ(a, 0);

  a = 100;
  EXPECT_EQ(a, 100);
  a += 100;
  EXPECT_EQ(a, 200);
  a -= 50;
  EXPECT_EQ(a, 150);

  a     = 0;
  int v = a++;
  EXPECT_EQ(v, 0);
  v = a++;
  EXPECT_EQ(v, 1);
  v = a;
  EXPECT_EQ(v, 2);
  v = ++a;
  EXPECT_EQ(v, 3);

  EXPECT_EQ(a.unit(), 1);

  maglev::atomic_counter<int, 100> b;
  EXPECT_EQ(b.unit(), 100);

  EXPECT_EQ(b++, 0);
  EXPECT_EQ(b, 100);
  EXPECT_EQ(++b, 200);
  EXPECT_EQ(b++, 200);
  EXPECT_EQ(b, 300);
  EXPECT_EQ(b--, 300);
  EXPECT_EQ(b, 200);
  EXPECT_EQ(--b, 100);
  b = 1234;
  EXPECT_EQ(--b, 1134);
}