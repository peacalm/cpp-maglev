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

#include <maglev/maglev.h>

int main() {
  maglev::maglev_balancer<maglev::maglev_hasher<maglev::load_stats_wrapper<
      maglev::slot_counted_node_wrapper<
          maglev::weighted_node_wrapper<maglev::node_base<std::string>>>,
      maglev::server_load_stats_wrapper<>>>>
      b;
  for (int i = 0; i < 30; ++i) {
    auto n = b.node_manager().new_back(std::to_string(i));
    n->set_weight(20 + rand() % 100);
  }
  b.maglev_hasher().build();

  int total_q      = 1234567;
  int consistent_q = 0;
  for (int i = 0; i < total_q; ++i) {
    auto ret = b.pick_with_auto_hash(i);
    consistent_q += ret.is_consistent;

    ret.node->incr_load();
    b.global_load().incr_load(ret.node->load_unit());

    bool fatal   = rand() % 50 == 0;
    bool error   = fatal || rand() % 10 == 0;
    int  latency = 100 + rand() % 50;

    ret.node->incr_server_load(1, error, fatal, latency);
    b.global_load().incr_server_load(1, error, fatal, latency);

    if (i > 0 && i % 300 == 0) { b.heartbeat(); }
  }
  return 0;
}
