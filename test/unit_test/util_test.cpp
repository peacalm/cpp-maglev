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

TEST(util, hash) {
  EXPECT_NE(maglev::maglev_int_hash<int>{}(0), 0);
  EXPECT_EQ(maglev::def_hash_t<int>{}(0), maglev::maglev_int_hash<int>{}(0));
  EXPECT_NE(maglev::def_hash_t<std::string>{}(""), 0);
}

TEST(util, prime) {
  EXPECT_FALSE(maglev::is_prime(0));
  EXPECT_FALSE(maglev::is_prime(1));
  EXPECT_FALSE(maglev::is_prime(4));
  EXPECT_FALSE(maglev::is_prime(9));
  EXPECT_FALSE(maglev::is_prime(10));
  EXPECT_FALSE(maglev::is_prime(50));

  EXPECT_TRUE(maglev::is_prime(2));
  EXPECT_TRUE(maglev::is_prime(3));
  EXPECT_TRUE(maglev::is_prime(5003));
  EXPECT_TRUE(maglev::is_prime(65537));
}

struct empty_class {};

struct slot_counted_void {
  using slot_counted_t = void;
};

struct slot_counted_bool {
  using slot_counted_t = bool;
};

struct weighted_void {
  using weighted_t = void;
};

struct weighted_bool {
  using weighted_t = bool;
};

TEST(util, type_traits) {
  EXPECT_FALSE(maglev::is_slot_counted_v<empty_class>);
  EXPECT_TRUE(maglev::is_slot_counted_v<slot_counted_void>);
  EXPECT_FALSE(maglev::is_slot_counted_v<slot_counted_bool>);

  EXPECT_FALSE(maglev::is_slot_counted_t<empty_class>::value);
  EXPECT_TRUE(maglev::is_slot_counted_t<slot_counted_void>::value);
  EXPECT_FALSE(maglev::is_slot_counted_t<slot_counted_bool>::value);

  EXPECT_FALSE(maglev::is_slot_counted<empty_class>::type::value);
  EXPECT_TRUE(maglev::is_slot_counted<slot_counted_void>::type::value);
  EXPECT_FALSE(maglev::is_slot_counted<slot_counted_bool>::type::value);


  EXPECT_FALSE(maglev::is_weighted_v<empty_class>);
  EXPECT_TRUE(maglev::is_weighted_v<weighted_void>);
  EXPECT_FALSE(maglev::is_weighted_v<weighted_bool>);
}