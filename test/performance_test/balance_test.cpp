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

#include <numeric>

#include "performance_test.h"

TEST(hasher, maglev_hasher) {
  maglev::maglev_hasher<
      maglev::slot_counted_node_wrapper<maglev::node_base<int>>,
      maglev::slot_array<int, 65537>>
      h;
  for (int i = 0; i < 10; ++i) { h.node_manager().new_back(i); }
  h.build();

  maglev_watch_with_std_cout(h.node_manager());

  std::map<int, int> hit_cnt;
  for (int i = 0; i < 10000; ++i) {
    auto ret = h.pick_with_auto_hash(i);
    hit_cnt[ret.node->id()]++;
  }
  std::vector<int> hit_nums;
  for (auto& i : hit_cnt) hit_nums.push_back(i.second);
  int    maxhit = *std::max_element(hit_nums.begin(), hit_nums.end());
  int    minhit = *std::min_element(hit_nums.begin(), hit_nums.end());
  int    tothit = std::accumulate(hit_nums.begin(), hit_nums.end(), 0);
  double avghit = double(tothit) / double(hit_nums.size());
  maglev_watch(hit_nums,
               tothit,
               avghit,
               maxhit,
               maxhit / avghit,
               minhit,
               minhit / avghit);
}

TEST(hasher, maglev_hasher_zero_weight) {
  maglev::maglev_hasher<
      maglev::weighted_node_wrapper<
          maglev::slot_counted_node_wrapper<maglev::node_base<int>>>,
      maglev::slot_array<int, 65537>>
      h;
  for (int i = 0; i < 10; ++i) { h.node_manager().new_back(i); }
  h.build();

  maglev_watch_with_std_cout(h.node_manager());

  std::map<int, int> hit_cnt;
  for (int i = 0; i < 10000; ++i) {
    auto ret = h.pick_with_auto_hash(i);
    hit_cnt[ret.node->id()]++;
  }
  std::vector<int> hit_nums;
  for (auto& i : hit_cnt) hit_nums.push_back(i.second);
  int    maxhit = *std::max_element(hit_nums.begin(), hit_nums.end());
  int    minhit = *std::min_element(hit_nums.begin(), hit_nums.end());
  int    tothit = std::accumulate(hit_nums.begin(), hit_nums.end(), 0);
  double avghit = double(tothit) / double(hit_nums.size());
  maglev_watch(hit_nums,
               tothit,
               avghit,
               maxhit,
               maxhit / avghit,
               minhit,
               minhit / avghit);
}

TEST(hasher, maglev_hasher_weighted) {
  maglev::maglev_hasher<
      maglev::weighted_node_wrapper<
          maglev::slot_counted_node_wrapper<maglev::node_base<int>>>,
      maglev::slot_array<int, 65537>>
      h;
  for (int i = 0; i < 20; ++i) {
    h.node_manager().new_back(i)->set_weight(20 + rand() % 100);
  }
  h.build();

  maglev_watch_with_std_cout(h.node_manager());

  std::map<int, int> hit_cnt;
  for (int i = 0; i < 10000; ++i) {
    auto ret = h.pick_with_auto_hash(i);
    hit_cnt[ret.node->id()]++;
  }

  auto                node_map = h.node_manager().make_node_map();
  std::vector<int>    hit_nums;
  std::vector<double> norm_hit_nums;

  for (auto& i : hit_cnt) {
    hit_nums.push_back(i.second);
    norm_hit_nums.push_back(double(i.second) / node_map[i.first]->weight() *
                            h.node_manager().avg_weight());
  }

  auto   maxhit = *std::max_element(hit_nums.begin(), hit_nums.end());
  auto   minhit = *std::min_element(hit_nums.begin(), hit_nums.end());
  auto   tothit = std::accumulate(hit_nums.begin(), hit_nums.end(), 0);
  double avghit = double(tothit) / double(hit_nums.size());
  maglev_watch(hit_nums,
               tothit,
               avghit,
               maxhit,
               maxhit / avghit,
               minhit,
               minhit / avghit);

  {
    auto norm_maxhit =
        *std::max_element(norm_hit_nums.begin(), norm_hit_nums.end());
    auto norm_minhit =
        *std::min_element(norm_hit_nums.begin(), norm_hit_nums.end());
    auto norm_tothit =
        std::accumulate(norm_hit_nums.begin(), norm_hit_nums.end(), 0.0);
    double norm_avghit = double(norm_tothit) / double(norm_hit_nums.size());
    maglev_watch(norm_hit_nums,
                 norm_tothit,
                 norm_avghit,
                 norm_maxhit,
                 norm_maxhit / norm_avghit,
                 norm_minhit,
                 norm_minhit / norm_avghit);
  }

  int max_slot_cnt = 0;
  for (int i = 0; i < h.node_size(); ++i) {
    auto n       = node_map[i];
    max_slot_cnt = std::max(n->slot_cnt(), max_slot_cnt);

    maglev_watch(i,
                 double(n->slot_cnt()) / h.slot_size(),
                 double(n->weight()) / h.node_manager().weight_sum(),
                 double(hit_nums[i]) / tothit);
  }

  for (int i = 0; i < h.node_size(); ++i) {
    auto n = node_map[i];
    maglev_watch(i,
                 double(n->slot_cnt()) / max_slot_cnt,
                 double(n->weight()) / h.node_manager().max_weight(),
                 double(hit_nums[i]) / maxhit);
  }
}