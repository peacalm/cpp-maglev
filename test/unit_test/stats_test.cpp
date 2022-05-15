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
#include <ctime>

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
    ++ ++i;
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
    ++ ++i;
    EXPECT_EQ(i, 2);
  }
}

TEST(stats, cycle_index) {
  EXPECT_EQ(maglev::cycle_index<64>{}, 0);
  EXPECT_EQ(maglev::cycle_index<64>(64), 0);
  EXPECT_EQ(maglev::cycle_index<64>(65), 1);
  maglev::cycle_index<32> ci;
  for (int i = 0; i < 100; ++i) {
    EXPECT_EQ(ci, i % ci.size());
    ++ci;
  }
  ci = 0;
  for (int i = 0; i < 100; ++i) {
    EXPECT_EQ(ci, ci.mod(i));
    ++ci;
  }

  ci = 0;
  EXPECT_EQ(ci++, 0);
  EXPECT_EQ(++ci, 2);
  ci = 0;
  EXPECT_EQ(--ci, 31);
  EXPECT_EQ(ci--, 31);
  EXPECT_EQ(ci, 30);

  ci = 0;
  ci += 34;
  EXPECT_EQ(ci, 2);
  ci -= 3;
  EXPECT_EQ(ci, 31);

  EXPECT_EQ(ci.is_size_power_of_two(), true);

  maglev::cycle_index<29> j;

  EXPECT_EQ(j.is_size_power_of_two(), false);

  for (int i = 0; i < 100; ++i) {
    EXPECT_EQ(j, j.mod(i));
    ++j;
  }
}

TEST(stats, cycle_array) {
  maglev::cycle_array<int, 16> a;

  EXPECT_EQ(a.size(), 16);
  EXPECT_EQ(a.is_size_power_of_two(), true);

  for (int i = 0; i < 16; ++i) a.push(i);
  EXPECT_EQ(a[2], 2);
  EXPECT_EQ(a[12], 12);
  EXPECT_EQ(a[17], 1);

  EXPECT_EQ(a.index(), 0);
  EXPECT_EQ(a.curr_item(), 0);
  EXPECT_EQ(a.next_item(), 1);
  EXPECT_EQ(a.prev_item(), 15);

  a.index()++;
  EXPECT_EQ(a.index(), 1);
  EXPECT_EQ(a.curr_item(), 1);
  EXPECT_EQ(a.next_item(), 2);
  EXPECT_EQ(a.prev_item(), 0);

  a.index() += 10;
  EXPECT_EQ(a.index(), 11);
  EXPECT_EQ(a.curr_item(), 11);
  EXPECT_EQ(a.next_item(), 12);
  EXPECT_EQ(a.prev_item(), 10);

  --a.index();
  EXPECT_EQ(a.index(), 10);
  EXPECT_EQ(a.curr_item(), 10);
  EXPECT_EQ(a.next_item(), 11);
  EXPECT_EQ(a.prev_item(), 9);
}

TEST(stats, sliding_window) {
  maglev::sliding_window<> w;
  w.incr();
  EXPECT_EQ(w.now(), 1);
  EXPECT_EQ(w.last(), 0);
  EXPECT_EQ(w.sum(), 0);
  EXPECT_EQ(w.avg(), 0);
  EXPECT_EQ(w.heartbeat_cnt(), 0);

  w.heartbeat();
  w.incr(2);
  EXPECT_EQ(w.now(), 2);
  EXPECT_EQ(w.last(), 1);
  EXPECT_EQ(w.sum(), 1);
  EXPECT_EQ(w.heartbeat_cnt(), 1);
  EXPECT_EQ(w.avg(), 1);

  w.heartbeat();
  w.incr();
  w.incr(2);
  EXPECT_EQ(w.now(), 3);
  EXPECT_EQ(w.last(), 2);
  EXPECT_EQ(w.sum(), 3);
  EXPECT_EQ(w.avg(), 3. / 2.);
  EXPECT_EQ(w.heartbeat_cnt(), 2);

  maglev::sliding_window<int, 1, 4> s;
  s.incr(1);
  s.heartbeat();
  s.incr(2);
  s.heartbeat();
  s.incr(3);
  s.heartbeat();
  EXPECT_EQ(s.sum(), 6);
  s.incr(4);
  EXPECT_EQ(s.sum(), 6);
  s.heartbeat();
  EXPECT_EQ(s.sum(), 10);
  s.incr(5);
  s.heartbeat();
  EXPECT_EQ(s.sum(), 14);
  EXPECT_EQ(s.avg(), 14.0 / 4.0);
}

TEST(stats, load_stats) {
  maglev::load_stats<> a;
  EXPECT_EQ(a.load_unit(), 1);
  EXPECT_EQ(a.load_seq_size(), 64);
  EXPECT_EQ(decltype(a)::load_data_t ::point_seq_t ::is_size_power_of_two(),
            true);

  a.load().incr();
  EXPECT_EQ(a.load().now(), a.load_unit());

  a.heartbeat();
  EXPECT_EQ(a.load().now(), 0);
  EXPECT_EQ(a.load().last(), a.load_unit());

  EXPECT_EQ(a.load_rank(), 0);
  a.set_load_rank(3);
  EXPECT_EQ(a.load_rank(), 3);

  maglev::load_stats<int, 3, 33> b;
  EXPECT_EQ(b.load_unit(), 3);
  EXPECT_EQ(b.load_seq_size(), 33);
  EXPECT_EQ(decltype(b)::load_data_t ::point_seq_t ::is_size_power_of_two(),
            false);

  //
  maglev::ban_wrapper<maglev::load_stats<long>> c;
  EXPECT_EQ(c.consecutive_ban_cnt(), 0);
  EXPECT_EQ(c.last_ban_time(), 0);
  c.incr_consecutive_ban_cnt();
  EXPECT_EQ(c.consecutive_ban_cnt(), 1);
  auto ts = std::time(nullptr);
  c.set_last_ban_time(ts);
  EXPECT_EQ(c.last_ban_time(), ts);
  maglev_watch(ts, c.last_ban_time());
  c.set_consecutive_ban_cnt(0);
  c.set_last_ban_time(0);
  EXPECT_EQ(c.consecutive_ban_cnt(), 0);
  EXPECT_EQ(c.last_ban_time(), 0);

  //
  maglev::server_load_stats_wrapper<maglev::load_stats<>> s;
  EXPECT_EQ(s.query_rank(), 0);
  EXPECT_EQ(s.error_rank(), 0);
  EXPECT_EQ(s.fatal_rank(), 0);
  EXPECT_EQ(s.latency_rank(), 0);

  s.set_query_rank(1);
  s.set_error_rank(2);
  s.set_fatal_rank(3);
  s.set_latency_rank(4);

  EXPECT_EQ(s.query_rank(), 1);
  EXPECT_EQ(s.error_rank(), 2);
  EXPECT_EQ(s.fatal_rank(), 3);
  EXPECT_EQ(s.latency_rank(), 4);

  s.incr_server_load(100, 5, 1, 100 * 1000);
  EXPECT_EQ(s.error_rate_of_now(), 0.05);
  EXPECT_EQ(s.fatal_rate_of_now(), 0.01);
  s.heartbeat();
  EXPECT_EQ(s.error_rate_of_now(), 0.0);
  EXPECT_EQ(s.fatal_rate_of_now(), 0.0);
  EXPECT_EQ(s.error_rate_of_last(), 0.05);
  EXPECT_EQ(s.fatal_rate_of_last(), 0.01);
  EXPECT_EQ(s.error_rate_of_window(), 0.05);
  EXPECT_EQ(s.fatal_rate_of_window(), 0.01);

  maglev::server_load_stats_wrapper<maglev::fake_load_stats<int, 1, 4>> x;

  x.incr_server_load(10, 2, 1, 100 * 1000);
  x.heartbeat();
  x.incr_server_load(10, 1, 0, 80 * 1000);
  x.heartbeat();
  x.incr_server_load(10, 0, 1, 90 * 1000);
  x.heartbeat();
  x.incr_server_load(10, 4, 2, 60 * 1000);
  x.heartbeat();

  x.incr_server_load(1, 0, 0, 8 * 1000);

  EXPECT_EQ(x.query().sum(), 40);
  EXPECT_EQ(x.error().sum(), 7);
  EXPECT_EQ(x.fatal().sum(), 4);
  EXPECT_EQ(x.latency().sum(), 330 * 1000);

  EXPECT_EQ(x.query().last(), 10);
  EXPECT_EQ(x.error().last(), 4);
  EXPECT_EQ(x.fatal().last(), 2);
  EXPECT_EQ(x.latency().last(), 60 * 1000);

  EXPECT_EQ(x.error_rate_of_now(), 0.);
  EXPECT_EQ(x.error_rate_of_last(), 4. / 10.);
  EXPECT_EQ(x.error_rate_of_window(), 7. / 40.);

  EXPECT_EQ(x.fatal_rate_of_now(), 0.);
  EXPECT_EQ(x.fatal_rate_of_last(), 2. / 10.);
  EXPECT_EQ(x.fatal_rate_of_window(), 4. / 40.);

  EXPECT_EQ(x.avg_latency_of_now(), 8 * 1000. / 1.);
  EXPECT_EQ(x.avg_latency_of_last(), 60 * 1000. / 10.);
  EXPECT_EQ(x.avg_latency_of_window(), 330 * 1000. / 40.);

  x.heartbeat();

  EXPECT_EQ(x.query().sum(), 31);
  EXPECT_EQ(x.error().sum(), 5);
  EXPECT_EQ(x.fatal().sum(), 3);
  EXPECT_EQ(x.latency().sum(), 238 * 1000);
}