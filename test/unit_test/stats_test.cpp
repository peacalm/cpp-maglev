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
  // default unit, 1
  maglev::atomic_counter<> a;
  EXPECT_EQ(a, 0);

  a = 100;
  EXPECT_EQ(a, 100);
  a += 100;
  EXPECT_EQ(a, 200);
  a -= 50;
  EXPECT_EQ(a, 150);

  a = 0;
  EXPECT_EQ(++a, 1);
  int v = ++a;
  EXPECT_EQ(v, 2);

  EXPECT_EQ(a.unit(), 1);

  // unit 100
  maglev::atomic_counter<int, 100> b;
  EXPECT_EQ(b.unit(), 100);

  EXPECT_EQ(++b, 100);
  EXPECT_EQ(++b, 200);
  EXPECT_EQ(--b, 100);
  EXPECT_EQ(--b, 0);
  EXPECT_EQ(--b, -100);
  b = 102;
  EXPECT_EQ(--b, 2);
  EXPECT_EQ(--b, -98);

  EXPECT_EQ(b.get(), b);
  b.clear();
  EXPECT_EQ(b.get(), 0);
  EXPECT_EQ(b, 0);

  // assignment
  maglev::atomic_counter<> x, y;
  EXPECT_EQ(x, y);
  x = 100;
  y = x;
  EXPECT_EQ(x, y);
  y = std::move(x);
  EXPECT_EQ(x, y);
  EXPECT_EQ(++x, ++y);
  x = y = 100;
  EXPECT_EQ(x, y);
  maglev::atomic_counter<> z1(x), z2{y};
  EXPECT_EQ(z1, z2);

  z1 = x + y;
  EXPECT_EQ(z1, 200);
  EXPECT_EQ(z1, 2 * x);

  z1 = z2 = -1;
  EXPECT_EQ(z1, z2);

  (x = 1) = 2;
  EXPECT_EQ(x, 2);

  maglev::atomic_counter<> z3(maglev::atomic_counter<>{x.get()}),
      z4(std::move(x));
  EXPECT_EQ(z3, x);
  EXPECT_EQ(z4, x);

  {
    int i, j;
    (i = 1) = 2;
    j       = i;
    EXPECT_EQ(i, 2);

    i = j = 100;
    EXPECT_EQ(i, 100);
    EXPECT_EQ(j, 100);
    i = j = 0;
    i += j += 100;
    EXPECT_EQ(i, 100);
    EXPECT_EQ(j, 100);

    i = 0;
    (i += 1) += 2;
    EXPECT_EQ(i, 3);

    i = 0;
    // i++ ++; // invalid
    ++++i;
    EXPECT_EQ(i, 2);
  }
  {
    std::atomic_int i, j;
    // (i = 1) = 2;  // invalid
    // j = i;  // invalid

    i = j = 100;
    EXPECT_EQ(i, 100);
    EXPECT_EQ(j, 100);
    i = j = 0;
    i += j += 100;
    EXPECT_EQ(i, 100);
    EXPECT_EQ(j, 100);

    i = 0;
    // (i += 1) += 2;  // invalid
    // EXPECT_EQ(i, 3);

    i = 0;
    // i++ ++; // invalid
    // ++ ++i; // invalid
    // EXPECT_EQ(i, 2);
  }
  {
    maglev::atomic_counter<> i, j;
    (i = 1) = 2;
    j       = i;

    i = j = 100;
    EXPECT_EQ(i, 100);
    EXPECT_EQ(j, 100);
    i = j = 0;
    i += j += 100;
    EXPECT_EQ(i, 100);
    EXPECT_EQ(j, 100);

    i = 0;
    (i += 1) += 2;
    EXPECT_EQ(i, 3);

    i = 0;
    // i++ ++; // invalid
    ++++i;
    EXPECT_EQ(i, 2);
  }
}