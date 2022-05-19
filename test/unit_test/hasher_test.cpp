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

#include <climits>
#include <numeric>

#include "unit_test.h"

TEST(hasher, slot_array) {
  maglev::slot_array<int, 7> slot_array;
  slot_array.fill(0);
  maglev_watch(slot_array);

  maglev::slot_vector<int> slot_vector;
  slot_vector.resize(5, -1);
  slot_vector[0] = 1;
  slot_vector[1] = 2;
  maglev_watch(slot_vector);
  slot_vector.resize(7);
  maglev_watch(slot_vector);
  slot_vector.resize(3, 3);

  maglev_watch(slot_vector);
}

TEST(hasher, maglev_hasher_consistent) {
  maglev::maglev_hasher<
      maglev::slot_counted_node_wrapper<maglev::node_base<int>>,
      maglev::slot_array<int, 65537>>
      h;
  for (int i = 0; i < 10; ++i) { h.node_manager().new_back(i); }
  h.build();
  std::map<int, int> hash_ret;
  for (int i = 0; i < 10000; ++i) {
    auto req_id      = i;
    auto ret         = h.pick_with_auto_hash(req_id);
    hash_ret[req_id] = ret.node->id();
    EXPECT_EQ(ret.node->id(), h.pick_with_auto_hash(req_id).node->id());
  }
  for (int i = 0; i < 10000; ++i) {
    auto req_id = i;
    auto ret    = h.pick_with_auto_hash(req_id);
    EXPECT_EQ(ret.node->id(), hash_ret[req_id]);
  }
}

TEST(hasher, maglev_hasher_consistent2) {
  maglev::maglev_hasher<
      maglev::slot_counted_node_wrapper<maglev::node_base<std::string>>,
      maglev::slot_array<int, 5003>>
      h;
  for (int i = 0; i < 10; ++i) { h.node_manager().new_back(std::to_string(i)); }
  h.build();
  std::map<std::string, std::string> hash_ret;
  for (int i = 0; i < 10000; ++i) {
    auto req_id      = std::string("req:") + std::to_string(i);
    auto ret         = h.pick_with_auto_hash(req_id);
    hash_ret[req_id] = ret.node->id();
    EXPECT_EQ(ret.node->id(), h.pick_with_auto_hash(req_id).node->id());
  }
  for (int i = 0; i < 10000; ++i) {
    auto req_id = std::string("req:") + std::to_string(i);
    auto ret    = h.pick_with_auto_hash(req_id);
    EXPECT_EQ(ret.node->id(), hash_ret[req_id]);
  }
}

TEST(hasher, maglev_hasher_slot_vector) {
  maglev::maglev_hasher<
      maglev::slot_counted_node_wrapper<maglev::node_base<std::string>>,
      maglev::slot_vector<>>
      h;
  h.slot_array().resize(65537);
  for (int i = 0; i < 10; ++i) { h.node_manager().new_back(std::to_string(i)); }
  h.build();
  std::map<std::string, std::string> hash_ret;
  for (int i = 0; i < 10000; ++i) {
    auto req_id      = std::string("req:") + std::to_string(i);
    auto ret         = h.pick_with_auto_hash(req_id);
    hash_ret[req_id] = ret.node->id();
    EXPECT_EQ(ret.node->id(), h.pick_with_auto_hash(req_id).node->id());
  }
  for (int i = 0; i < 10000; ++i) {
    auto req_id = std::string("req:") + std::to_string(i);
    auto ret    = h.pick_with_auto_hash(req_id);
    EXPECT_EQ(ret.node->id(), hash_ret[req_id]);
  }
}

TEST(hasher, maglev_hasher_all_zero_weight) {
  maglev::maglev_hasher<maglev::weighted_node_wrapper<
      maglev::slot_counted_node_wrapper<maglev::node_base<int>>>>
      h;
  for (int i = 0; i < 10; ++i) { h.node_manager().new_back(i); }

  h.build();

  std::map<int, int> hit_cnt;
  for (int i = 0; i < 10000; ++i) {
    auto ret = h.pick_with_auto_hash(i);
    hit_cnt[ret.node->id()]++;
  }

  for (auto i : hit_cnt) { EXPECT_NE(i.second, 0); }
}

TEST(hasher, maglev_hasher_some_zero_weight) {
  maglev::maglev_hasher<maglev::weighted_node_wrapper<
      maglev::slot_counted_node_wrapper<maglev::node_base<int>>>>
      h;
  for (int i = 0; i < 10; ++i) {
    h.node_manager().new_back(i)->set_weight(100 + rand() % 100);
  }

  h.node_manager()[1]->set_weight(0);
  h.node_manager()[5]->set_weight(0);

  h.build();

  std::map<int, int> hit_cnt;
  for (int i = 0; i < 10000; ++i) {
    auto ret = h.pick_with_auto_hash(i);
    hit_cnt[ret.node->id()]++;
  }

  for (int i = 0; i < 10; ++i) {
    if (i == 1 || i == 5) {
      EXPECT_EQ(hit_cnt[i], 0);
    } else {
      EXPECT_NE(hit_cnt[i], 0);
    }
  }
}

TEST(hasher, maglev_balancer) {
  maglev::maglev_balancer<> b;
  for (int i = 0; i < 10; ++i) { b.node_manager().new_back(std::to_string(i)); }
  b.maglev_hasher().build();
  for (int i = 0; i < 12345; ++i) {
    auto ret = b.pick_with_auto_hash(i);
    ret.node->incr_load();
    b.global_load().incr_load();

    if (i > 0 && i % 100 == 0) { b.heartbeat(); }
  }

  maglev_watch_with_std_cout(b.node_manager());
  maglev_watch_with_std_cout(b.global_load());
  maglev_watch_with_std_cout(b.heartbeat_cnt());
  maglev_watch(b.banned_cnt());
}

TEST(hasher, maglev_balancer_server_stats) {
  maglev::maglev_balancer<maglev::maglev_hasher<
      maglev::load_stats_wrapper<maglev::node_base<std::string>,
                                 maglev::server_load_stats_wrapper<>>>>
      b;
  for (int i = 0; i < 10; ++i) { b.node_manager().new_back(std::to_string(i)); }
  b.maglev_hasher().build();
  for (int i = 0; i < 12345; ++i) {
    auto ret = b.pick_with_auto_hash(i);

    ret.node->incr_load();
    b.global_load().incr_load();

    bool fatal   = rand() % 50 == 0;
    bool error   = fatal || rand() % 10 == 0;
    int  latency = 100 + rand() % 50;
    ret.node->incr_server_load(1, error, fatal, latency);
    b.global_load().incr_server_load(1, error, fatal, latency);

    if (i > 0 && i % 100 == 0) { b.heartbeat(); }
  }
  maglev_watch_with_std_cout(b.node_manager());
  maglev_watch_with_std_cout(b.global_load());
  maglev_watch_with_std_cout(b.heartbeat_cnt());
  maglev_watch(b.banned_cnt());
}

TEST(hasher, maglev_balancer_unweighted_server_stats) {
  maglev::maglev_balancer<maglev::maglev_hasher<
      maglev::load_stats_wrapper<maglev::node_base<std::string>,
                                 maglev::unweighted_server_load_stats<>>>>
      b;
  for (int i = 0; i < 10; ++i) { b.node_manager().new_back(std::to_string(i)); }
  b.maglev_hasher().build();

  int total_q      = 1234567;
  int consistent_q = 0;
  for (int i = 0; i < total_q; ++i) {
    auto ret = b.pick_with_auto_hash(i);
    consistent_q += ret.is_consistent;

    ret.node->incr_load();
    b.global_load().incr_load(ret.node->load_unit());

    bool fatal = rand() % 50 == 0;

    if (ret.node->id() == "3") { fatal = true; }

    bool error   = fatal || rand() % 10 == 0;
    int  latency = 100 + rand() % 50;

    ret.node->incr_server_load(1, error, fatal, latency);
    b.global_load().incr_server_load(1, error, fatal, latency);

    if (i > 0 && i % 300 == 0) { b.heartbeat(); }
  }
  maglev_watch_with_std_cout(b.node_manager());
  maglev_watch_with_std_cout(b.global_load());
  maglev_watch_with_std_cout(b.heartbeat_cnt());
  maglev_watch_with_std_cout(*b.node_manager().find_by_node_id("3"));  // banned
  maglev_watch(b.banned_cnt());
  maglev_watch(total_q, consistent_q, 1.0 * consistent_q / total_q);
}

TEST(hasher, maglev_balancer_weighted_server_stats) {
  maglev::maglev_balancer<maglev::maglev_hasher<maglev::load_stats_wrapper<
      maglev::slot_counted_node_wrapper<
          maglev::weighted_node_wrapper<maglev::node_base<std::string>>>,
      maglev::server_load_stats_wrapper<>>>>
      b;
  for (int i = 0; i < 10; ++i) {
    auto n = b.node_manager().new_back(std::to_string(i));
    n->set_weight(10 + rand() % 20);
  }
  b.maglev_hasher().build();
  //  b.maglev_hasher().node_manager().init_load_units(100);

  int total_q      = 1234567;
  int consistent_q = 0;
  for (int i = 0; i < total_q; ++i) {
    auto ret = b.pick_with_auto_hash(i);
    consistent_q += ret.is_consistent;

    if (ret.is_consistent) {
      EXPECT_EQ(ret.node_idx, ret.consistent_node_idx);
      EXPECT_EQ(ret.node, ret.consistent_node);
      EXPECT_EQ(ret.node->id(), ret.consistent_node->id());
    } else {
      EXPECT_NE(ret.node_idx, ret.consistent_node_idx);
      EXPECT_NE(ret.node, ret.consistent_node);
      EXPECT_NE(ret.node->id(), ret.consistent_node->id());
    }
    ret.node->incr_load();
    b.global_load().incr_load(ret.node->load_unit());

    bool fatal = rand() % 50 == 0;

    if (ret.node->id() == "3") { fatal = true; }

    bool error   = fatal || rand() % 10 == 0;
    int  latency = 100 + rand() % 50;

    ret.node->incr_server_load(1, error, fatal, latency);
    b.global_load().incr_server_load(1, error, fatal, latency);

    if (i > 0 && i % 300 == 0) { b.heartbeat(); }
  }
  maglev_watch_with_std_cout(b.node_manager());
  maglev_watch_with_std_cout(b.global_load());
  maglev_watch_with_std_cout(b.heartbeat_cnt());
  maglev_watch_with_std_cout(*b.node_manager().find_by_node_id("3"));
  maglev_watch(b.banned_cnt());
  maglev_watch(total_q, consistent_q, 1.0 * consistent_q / total_q);
}

TEST(hasher, default_balance_strategy) {
  maglev::default_balance_strategy b;
  std::vector<unsigned long long>  keys{0,
                                       1,
                                       5,
                                       INT_MAX,
                                       maglev::def_hash_t<size_t>()(0),
                                       maglev::def_hash_t<size_t>()(1),
                                       maglev::def_hash_t<size_t>()(5)};
  for (size_t key : keys) {
    EXPECT_EQ(b.rehash(key, 0, 5003), b.rehash(key + 5003 * 100, 0, 5003));
    EXPECT_EQ(b.rehash(key, 0, 5003),
              b.rehash(key + 5003 * std::time(0), 0, 5003));
    for (int t = 1; t < 10; ++t) {
      EXPECT_NE(b.rehash(key, 0, 5003), b.rehash(key, t, 5003));
    }
  }
}
