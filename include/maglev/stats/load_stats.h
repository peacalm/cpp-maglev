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

#include "atomic_counter.h"
#include "cycle_array.h"
#include "sliding_window.h"

namespace maglev {

/// Fake load stats.
template <typename PointValueType = unsigned long long, size_t LoadSeqSize = 64>
class fake_load_stats {
public:
  using load_data_t  = sliding_window<PointValueType, LoadSeqSize>;
  using load_value_t = typename load_data_t::point_value_t;

  static constexpr size_t load_seq_size() { return load_data_t::seq_size(); }

  // A load_stats must have a heartbeat() method.
  void heartbeat() {}

  template <typename Char, typename Traits>
  std::basic_ostream<Char, Traits>& output_stats(
      std::basic_ostream<Char, Traits>& os) const {
    return os;
  }
};

template <typename Char,
          typename Traits,
          typename PointValueType,
          size_t LoadSeqSize>
std::basic_ostream<Char, Traits>& operator<<(
    std::basic_ostream<Char, Traits>&                   os,
    const fake_load_stats<PointValueType, LoadSeqSize>& s) {
  return os;
}

/// To record a node's load.
template <typename PointValueType = unsigned long long, size_t LoadSeqSize = 64>
class load_stats {
public:
  using load_data_t  = sliding_window<PointValueType, LoadSeqSize>;
  using load_value_t = typename load_data_t::point_value_t;

  static constexpr size_t load_seq_size() { return load_data_t::seq_size(); }
  load_value_t            load_unit() const { return load_.unit(); }
  void                    set_load_unit(load_value_t u) { load_.set_unit(u); }

  // A load_stats must have a heartbeat() method.
  void heartbeat() { load_.heartbeat(); }

  load_data_t&       load() { return load_; }
  const load_data_t& load() const { return load_; }

  int load_rank() const { return load_rank_; }

  void set_load_rank(int r) { load_rank_ = r; }

  void incr_load() { load_.incr(load_unit()); }
  void incr_load(load_value_t d) { load_.incr(d); }

  template <typename Char, typename Traits>
  std::basic_ostream<Char, Traits>& output_stats(
      std::basic_ostream<Char, Traits>& os) const {
    os << "n:" << load().now() << ",l:" << load().last()
       << ",s:" << load().sum();
    if (load_rank()) os << ",r:" << load_rank();
    os << ",c:" << load().heartbeat_cnt();
    return os;
  }

private:
  load_data_t load_;
  int         load_rank_ = 0;
};

template <typename Char,
          typename Traits,
          typename PointValueType,
          size_t LoadSeqSize>
std::basic_ostream<Char, Traits>& operator<<(
    std::basic_ostream<Char, Traits>&              os,
    const load_stats<PointValueType, LoadSeqSize>& s) {
  os << "[";
  s.output_stats(os);
  os << "]";
  return os;
}

/// To record info about whether a node is banned.
template <typename LoadStatsBase>
class ban_wrapper : public LoadStatsBase {
  using base_t     = LoadStatsBase;
  using ban_cnt_t  = int;
  using ban_time_t = long long;

public:
  ban_cnt_t consecutive_ban_cnt() const { return consecutive_ban_cnt_; }
  void      set_consecutive_ban_cnt(ban_cnt_t v) { consecutive_ban_cnt_ = v; }
  void incr_consecutive_ban_cnt(ban_cnt_t v = 1) { consecutive_ban_cnt_ += v; }

  ban_time_t last_ban_time() const { return last_ban_time_; }
  void       set_last_ban_time(ban_time_t t) { last_ban_time_ = t; }

  template <typename Char, typename Traits>
  std::basic_ostream<Char, Traits>& output_stats(
      std::basic_ostream<Char, Traits>& os) const {
    base_t::output_stats(os);
    if (last_ban_time() != 0 || consecutive_ban_cnt() != 0) {
      os << "ban:(" << last_ban_time() << "," << consecutive_ban_cnt() << ")";
    }
    return os;
  }

private:
  ban_cnt_t  consecutive_ban_cnt_ = 0;
  ban_time_t last_ban_time_       = 0;
};

template <typename Char, typename Traits, typename LoadStatsBase>
std::basic_ostream<Char, Traits>& operator<<(
    std::basic_ostream<Char, Traits>& os, const ban_wrapper<LoadStatsBase>& s) {
  os << "[";
  s.output_stats(os);
  os << "]";
  return os;
}

/// To describe a server's load in RPC scene.
template <typename LoadStatsBase,
          typename QueryCntType   = unsigned long long,
          typename ErrorCntType   = unsigned long long,
          typename FatalCntType   = unsigned long long,
          typename LatencyCntType = unsigned long long,
          size_t SeqSize          = LoadStatsBase::load_seq_size()>
class server_load_stats_wrapper : public LoadStatsBase {
  using base_t = LoadStatsBase;

public:
  using query_cnt_t   = QueryCntType;
  using error_cnt_t   = ErrorCntType;
  using fatal_cnt_t   = FatalCntType;
  using latency_cnt_t = LatencyCntType;

  using query_data_t   = sliding_window<query_cnt_t, SeqSize>;
  using error_data_t   = sliding_window<error_cnt_t, SeqSize>;
  using fatal_data_t   = sliding_window<fatal_cnt_t, SeqSize>;
  using latency_data_t = sliding_window<latency_cnt_t, SeqSize>;

  // A load_stats must have a heartbeat() method.
  void heartbeat() {
    base_t::heartbeat();
    query_.heartbeat();
    error_.heartbeat();
    fatal_.heartbeat();
    latency_.heartbeat();
  }

  query_data_t&   query() { return query_; }
  error_data_t&   error() { return error_; }
  fatal_data_t&   fatal() { return fatal_; }
  latency_data_t& latency() { return latency_; }

  const query_data_t&   query() const { return query_; }
  const error_data_t&   error() const { return error_; }
  const fatal_data_t&   fatal() const { return fatal_; }
  const latency_data_t& latency() const { return latency_; }

  int query_rank() { return query_rank_; }
  int error_rank() { return error_rank_; }
  int fatal_rank() { return fatal_rank_; }
  int latency_rank() { return latency_rank_; }

  void set_query_rank(int r) { query_rank_ = r; }
  void set_error_rank(int r) { error_rank_ = r; }
  void set_fatal_rank(int r) { fatal_rank_ = r; }
  void set_latency_rank(int r) { latency_rank_ = r; }

  // Error rate of point now, last point, sum of all points in sliding window.
  double error_rate_of_now() const {
    return query_.now() > 0 ? double(error_.now()) / double(query_.now()) : 0;
  }
  double error_rate_of_last() const {
    return query_.last() > 0 ? double(error_.last()) / double(query_.last())
                             : 0;
  }
  double error_rate_of_window() const {
    return query_.sum() > 0 ? double(error_.sum()) / double(query_.sum()) : 0;
  }

  // Fatal rate of point now, last point, sum of all points in sliding window.
  double fatal_rate_of_now() const {
    return query_.now() > 0 ? double(fatal_.now()) / double(query_.now()) : 0;
  }
  double fatal_rate_of_last() const {
    return query_.last() > 0 ? double(fatal_.last()) / double(query_.last())
                             : 0;
  }
  double fatal_rate_of_window() const {
    return query_.sum() > 0 ? double(fatal_.sum()) / double(query_.sum()) : 0;
  }

  // Avg latency of point now, last point, sum of all points in sliding window.
  double avg_latency_of_now() const {
    return query_.now() > 0 ? double(latency_.now()) / double(query_.now()) : 0;
  }
  double avg_latency_of_last() const {
    return query_.last() > 0 ? double(latency_.last()) / double(query_.last())
                             : 0;
  }
  double avg_latency_of_window() const {
    return query_.sum() > 0 ? double(latency_.sum()) / double(query_.sum()) : 0;
  }

  void incr_server_load(query_cnt_t   q,
                        error_cnt_t   e,
                        fatal_cnt_t   f,
                        latency_cnt_t l) {
    query_.incr(q);
    if (e) error_.incr(e);
    if (f) fatal_.incr(f);
    latency_.incr(l);
  }

  template <typename Char, typename Traits>
  std::basic_ostream<Char, Traits>& output_stats(
      std::basic_ostream<Char, Traits>& os) const {
    base_t::output_stats(os);
    os << "n:(" << query().now() << "," << error().now() << "," << fatal().now()
       << "," << avg_latency_of_now() << ")";
    os << ",l:(" << query().last() << "," << error().last() << ","
       << fatal().last() << "," << avg_latency_of_last() << ")";
    os << ",s:(" << query().sum() << "," << error().sum() << ","
       << fatal().sum() << "," << avg_latency_of_window() << ")";
    if (query_rank() || error_rank() || fatal_rank() || latency_rank()) {
      os << ",r:(" << query_rank() << "," << error_rank() << "," << fatal_rank()
         << "," << latency_rank() << ")";
    }
    return os;
  }

private:
  query_data_t   query_;
  error_data_t   error_;
  fatal_data_t   fatal_;
  latency_data_t latency_;

  int query_rank_   = 0;
  int error_rank_   = 0;
  int fatal_rank_   = 0;
  int latency_rank_ = 0;
};

template <typename Char,
          typename Traits,
          typename LoadStatsBase,
          typename QueryCntType,
          typename ErrorCntType,
          typename FatalCntType,
          typename LatencyCntType,
          size_t SeqSize>
std::basic_ostream<Char, Traits>& operator<<(
    std::basic_ostream<Char, Traits>&         os,
    const server_load_stats_wrapper<LoadStatsBase,
                                    QueryCntType,
                                    ErrorCntType,
                                    FatalCntType,
                                    LatencyCntType,
                                    SeqSize>& s) {
  os << "[";
  s.output_stats(os);
  os << "]";
  return os;
}

}  // namespace maglev
