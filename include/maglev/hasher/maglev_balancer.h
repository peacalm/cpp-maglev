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

#pragma once

#include <algorithm>

#include "maglev/stats/load_stats.h"
#include "maglev/stats/load_stats_wrapper.h"
#include "maglev_hasher.h"

namespace maglev {

struct default_balance_strategy {
  // ========== parameters ====================================================

  // balance parameters
  double eps_of_load_to_balance       = 1.2;
  int    min_heartbeat_cnt_to_balance = 5;
  int    min_load_to_balance          = 10;

  // server balance parameters
  int    min_query_to_balance                 = 10;
  double min_error_rate_to_balance            = 0;
  int    min_avg_latency_to_balance           = 0;
  double eps_of_latency_to_balance            = 1.5;
  double max_pct_of_balance_by_latency        = 0.03;
  double min_error_rate_to_balance_by_latency = 0.01;
  int    latency_th_to_force_balance          = INT_MAX;

  double max_pct_of_balance_by_error        = 0.03;
  double min_error_rate_to_balance_by_error = 0.5;

  // ban parameters only for server_load_stats
  int    max_fatal_rank_to_ban   = 3;
  double max_pct_of_ban_by_fatal = 0.03;
  int    min_query_to_ban        = 10;
  double min_fatal_ratio_to_ban  = 0.9;
  // recover from ban
  int recover_delay_s     = 5;
  int max_recover_delay_s = 600;

  // ========== methods =======================================================

  // should_balance

  template <typename StatsTypeA, typename StatsTypeB>
  bool should_balance(const StatsTypeA& n,
                      const StatsTypeB& g,
                      size_t            node_size) const {
    return false;
  }

  template <typename PointValueType, size_t LoadSeqSize>
  bool should_balance(const load_stats<PointValueType, LoadSeqSize>& n,
                      const load_stats<PointValueType, LoadSeqSize>& g,
                      size_t node_size) const {
    if (g.heartbeat_cnt() <= min_heartbeat_cnt_to_balance) { return false; }
    if (n.load().now() <= min_load_to_balance) { return false; }
    // g_load = max of now and last, or, maybe add max of sum/node_size as well
    auto g_load = std::max(g.load().now(), g.load().last());
    if (n.load().now() * node_size > g_load * eps_of_load_to_balance) {
      return true;
    }
    return false;
  }

  template <typename LoadStatsBase,
            typename QueryCntType,
            typename LatencyCntType,
            size_t SeqSize>
  bool should_balance(const server_load_stats_wrapper<LoadStatsBase,
                                                      QueryCntType,
                                                      LatencyCntType,
                                                      SeqSize>& n,
                      const server_load_stats_wrapper<LoadStatsBase,
                                                      QueryCntType,
                                                      LatencyCntType,
                                                      SeqSize>& g,
                      size_t node_size) const {
    if (g.heartbeat_cnt() <= min_heartbeat_cnt_to_balance) { return false; }
    if (n.load().now() <= min_load_to_balance) { return false; }
    if (n.query().now() <= min_query_to_balance) { return false; }
    if (n.error_rate_of_window() <= min_error_rate_to_balance) { return false; }
    if (n.avg_latency_of_window() <= min_avg_latency_to_balance) {
      return false;
    }

    // g_load = max of now and last, or, maybe add max of sum/node_size as well
    auto g_load = std::max(g.load().now(), g.load().last());
    if (n.load().now() * node_size > g_load * eps_of_load_to_balance) {
      return true;
    }
    // balance by latency
    if (n.error_rate_of_window() > min_error_rate_to_balance_by_latency &&
        n.latency_rank() <=
            std::ceil(node_size * max_pct_of_balance_by_latency)) {
      if (n.avg_latency_of_window() >
          g.avg_latency_of_window() * eps_of_latency_to_balance) {
        return true;
      }
      if (n.avg_latency_of_window() > latency_th_to_force_balance) {
        return true;
      }
    }
    // balance by error
    if (n.error().sum() > 0 &&
        n.error_rate_of_window() > min_error_rate_to_balance_by_error &&
        n.error_rank() <= std::ceil(node_size * max_pct_of_balance_by_error)) {
      return true;
    }
    return false;
  }

  // should_ban

  template <typename StatsType>
  bool should_ban(const StatsType& n,
                  const StatsType& g,
                  size_t           node_size) const {
    return false;
  }

  template <typename PointValueType, size_t LoadSeqSize>
  bool should_ban(const load_stats<PointValueType, LoadSeqSize>& n,
                  const load_stats<PointValueType, LoadSeqSize>& g,
                  size_t node_size) const {
    return false;
  }

  template <typename LoadStatsBase,
            typename QueryCntType,
            typename LatencyCntType,
            size_t SeqSize>
  bool should_ban(const server_load_stats_wrapper<LoadStatsBase,
                                                  QueryCntType,
                                                  LatencyCntType,
                                                  SeqSize>& n,
                  const server_load_stats_wrapper<LoadStatsBase,
                                                  QueryCntType,
                                                  LatencyCntType,
                                                  SeqSize>& g,
                  size_t                                    node_size) const {
    return should_ban_server(n, g, node_size);
  }

  template <typename QueryCntType, typename LatencyCntType, size_t SeqSize>
  bool should_ban(
      const unweighted_server_load_stats<QueryCntType, LatencyCntType, SeqSize>&
          n,
      const unweighted_server_load_stats<QueryCntType, LatencyCntType, SeqSize>&
             g,
      size_t node_size) const {
    return should_ban_server(n, g, node_size);
  }

  template <typename ServerLoadStatsType>
  bool should_ban_server(const ServerLoadStatsType& n,
                         const ServerLoadStatsType& g,
                         size_t                     node_size) const {
    if (n.fatal_rank() > max_fatal_rank_to_ban ||
        n.fatal_rank() > std::ceil(node_size * max_pct_of_ban_by_fatal) ||
        n.query().now() < min_query_to_ban) {
      return false;
    }

    return should_ban_by_fatal(n, g, node_size) ||
           should_ban_by_delay_recover(n, g, node_size);
  }

  template <typename ServerLoadStatsType>
  bool should_ban_by_fatal(const ServerLoadStatsType& n,
                           const ServerLoadStatsType& g,
                           size_t                     node_size) const {
    if (n.fatal_rate_of_now() > min_fatal_ratio_to_ban &&
        n.fatal_rate_of_last() > min_fatal_ratio_to_ban) {
      return true;
    }
    return false;
  }

  template <typename ServerLoadStatsType>
  bool should_ban_by_delay_recover(const ServerLoadStatsType& n,
                                   const ServerLoadStatsType& g,
                                   size_t                     node_size) const {
    if (n.consecutive_ban_cnt() <= 0) { return false; }
    auto delay_s = std::min(recover_delay_s << n.consecutive_ban_cnt(),
                            max_recover_delay_s);
    if (std::time(nullptr) <= n.last_ban_time() + delay_s) { return true; }
    return false;
  }

  // heartbeat
  // return banned node count

  template <typename LoadStatsType, typename NodeManagerType>
  int heartbeat(const LoadStatsType& g, NodeManagerType& nm) const {
    return 0;
  }

  template <typename PointValueType,
            size_t LoadSeqSize,
            typename NodeManagerType>
  int heartbeat(const load_stats<PointValueType, LoadSeqSize>& g,
                NodeManagerType&                               n) const {
    using node_ptr_t = typename NodeManagerType::node_ptr_t;

    // load rank
    std::sort(n.begin(), n.end(), [](const node_ptr_t& l, const node_ptr_t& r) {
      return l->load().sum() > r->load().sum();
    });
    for (size_t i = 0; i < n.size(); ++i) { n[i]->set_load_rank(i + 1); }

    return 0;
  }

  template <typename LoadStatsBase,
            typename QueryCntType,
            typename LatencyCntType,
            size_t SeqSize,
            typename NodeManagerType>
  int heartbeat(const server_load_stats_wrapper<LoadStatsBase,
                                                QueryCntType,
                                                LatencyCntType,
                                                SeqSize>& g,
                NodeManagerType&                          n) const {
    return server_heartbeat(g, n);
  }

  template <typename QueryCntType,
            typename LatencyCntType,
            size_t SeqSize,
            typename NodeManagerType>
  int heartbeat(
      const unweighted_server_load_stats<QueryCntType, LatencyCntType, SeqSize>&
                       g,
      NodeManagerType& n) const {
    return server_heartbeat(g, n);
  }

  template <typename ServerLoadStatsType, typename NodeManagerType>
  int server_heartbeat(const ServerLoadStatsType& g, NodeManagerType& n) const {
    using node_ptr_t = typename NodeManagerType::node_ptr_t;

    // load rank
    std::sort(n.begin(), n.end(), [](const node_ptr_t& l, const node_ptr_t& r) {
      return l->load().sum() > r->load().sum();
    });
    for (size_t i = 0; i < n.size(); ++i) { n[i]->set_load_rank(i + 1); }

    // query rank
    std::sort(n.begin(), n.end(), [](const node_ptr_t& l, const node_ptr_t& r) {
      return l->query().sum() > r->query().sum();
    });
    for (size_t i = 0; i < n.size(); ++i) { n[i]->set_query_rank(i + 1); }

    // error rank
    std::sort(n.begin(), n.end(), [](const node_ptr_t& l, const node_ptr_t& r) {
      return l->error_rate_of_window() > r->error_rate_of_window();
    });
    for (size_t i = 0; i < n.size(); ++i) { n[i]->set_error_rank(i + 1); }

    // fatal rank
    std::sort(n.begin(), n.end(), [](const node_ptr_t& l, const node_ptr_t& r) {
      return l->fatal_rate_of_window() > r->fatal_rate_of_window();
    });
    for (size_t i = 0; i < n.size(); ++i) { n[i]->set_fatal_rank(i + 1); }

    // latency rank
    std::sort(n.begin(), n.end(), [](const node_ptr_t& l, const node_ptr_t& r) {
      return l->avg_latency_of_window() > r->avg_latency_of_window();
    });
    for (size_t i = 0; i < n.size(); ++i) { n[i]->set_latency_rank(i + 1); }

    // ban
    int banned_cnt = 0;
    for (const auto& i : n) {
      if (should_ban_by_delay_recover(i->load_stats(), g, n.size())) {
        ++banned_cnt;
      } else if (should_ban_by_fatal(i->load_stats(), g, n.size())) {
        i->incr_consecutive_ban_cnt();
        i->set_last_ban_time(std::time(nullptr));
        ++banned_cnt;
      } else if (i->query().now() > 0 && i->fatal().now() == 0 &&
                 i->query().last() > 0 && i->fatal().last() == 0) {
        i->set_consecutive_ban_cnt(0);
      }
    }
    return banned_cnt;
  }

  // rehash, get slot index from (key, try_cnt, slot_size)
  size_t rehash(size_t key, size_t try_cnt, size_t slot_size) const {
    return (key + (key % 997 + 1) * try_cnt) % slot_size;
  }
};

template <typename MaglevHasherType =
              maglev_hasher<load_stats_wrapper<node_base<>, load_stats<>>>,
          typename BalanceStrategyType = default_balance_strategy>
class maglev_balancer {
public:
  using maglev_hasher_t     = MaglevHasherType;
  using maglev_hasher_ptr_t = maglev_hasher_t*;
  using node_manager_t      = typename maglev_hasher_t::node_manager_t;
  using slot_array_t        = typename maglev_hasher_t::slot_array_t;
  using node_t              = typename maglev_hasher_t::node_t;
  using node_ptr_t          = typename maglev_hasher_t::node_ptr_t;
  using node_meta_t         = typename node_t::node_meta_t;
  using load_stats_t        = typename node_t::load_stats_t;
  using balance_strategy_t  = BalanceStrategyType;

  struct pick_ret_t : public maglev_hasher_t::pick_ret_t {
    bool is_consistent = true;
  };

public:
  maglev_balancer(maglev_hasher_ptr_t h = nullptr) {
    if (!h) { h = new maglev_hasher_t{}; }
    maglev_hasher_.store(h, std::memory_order_relaxed);
  }

  ~maglev_balancer() {
    set_maglev_hasher(nullptr);
    if (old_maglev_hasher_) {
      delete old_maglev_hasher_;
      old_maglev_hasher_ = nullptr;
    }
  }

  void set_maglev_hasher(maglev_hasher_ptr_t h) {
    maglev_hasher_ptr_t old =
        maglev_hasher_.exchange(h, std::memory_order_relaxed);
    if (old_maglev_hasher_) { delete old_maglev_hasher_; }
    old_maglev_hasher_ = old;
  }

  const maglev_hasher_t& maglev_hasher() const {
    maglev_hasher_ptr_t h = maglev_hasher_.load(std::memory_order_relaxed);
    assert(h);
    return *h;
  }
  maglev_hasher_t& maglev_hasher() {
    maglev_hasher_ptr_t h = maglev_hasher_.load(std::memory_order_relaxed);
    assert(h);
    return *h;
  }

  // methods from maglev_hasher

  void build() { maglev_hasher().build(); }

  slot_array_t& slot_array() { return maglev_hasher().slot_array(); }

  const slot_array_t& slot_array() const {
    return maglev_hasher().slot_array();
  }

  size_t slot_size() const { return slot_array().size(); }

  node_manager_t& node_manager() { return maglev_hasher().node_manager(); }

  const node_manager_t& node_manager() const {
    return maglev_hasher().node_manager();
  }

  size_t node_size() const { return maglev_hasher().node_size(); }

  // balance strategies

  balance_strategy_t&       balance_strategy() { return balance_strategy_; }
  const balance_strategy_t& balance_strategy() const {
    return balance_strategy_;
  }

  pick_ret_t pick(size_t hashed_key) const {
    pick_ret_t ret;
    for (size_t try_cnt = 0; try_cnt < node_size(); ++try_cnt) {
      size_t slot_idx =
          balance_strategy().rehash(hashed_key, try_cnt, slot_size());
      ret.node_idx      = slot_array()[slot_idx];
      ret.node          = node_manager()[ret.node_idx];
      ret.is_consistent = try_cnt == 0;
      if (balance_strategy().should_balance(
              ret.node->load_stats(), global_load(), node_size())) {
        continue;
      }
      if (balance_strategy().should_ban(
              ret.node->load_stats(), global_load(), node_size())) {
        continue;
      }
      break;
    }
    return ret;
  }

  template <typename KeyType, typename HashType = def_hash_t<KeyType>>
  pick_ret_t pick_with_auto_hash(const KeyType& key) const {
    static auto h = HashType{};
    return pick(h(key));
  }

  void heartbeat() {
    auto nm_copy = node_manager();
    banned_cnt_  = balance_strategy().heartbeat(global_load(), nm_copy);

    for (auto i : node_manager()) { i->heartbeat(); }
    global_load_.heartbeat();
  }

  load_stats_t&       global_load() { return global_load_; }
  const load_stats_t& global_load() const { return global_load_; }

  size_t heartbeat_cnt() const { return global_load().heartbeat_cnt(); }

  int banned_cnt() const { return banned_cnt_; }

protected:
  std::atomic<maglev_hasher_ptr_t> maglev_hasher_{nullptr};
  maglev_hasher_ptr_t              old_maglev_hasher_{nullptr};

  load_stats_t global_load_;

  balance_strategy_t balance_strategy_;
  int                banned_cnt_ = 0;
};

}  // namespace maglev
