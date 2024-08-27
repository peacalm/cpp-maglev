// Copyright (c) 2021-2022 Li Shuangquan. All Rights Reserved.
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

#include <sstream>

#include "maglev/stats/atomic_counter.h"
#include "maglev/stats/cycle_array.h"
#include "maglev/stats/sliding_window.h"
#include "maglev/util/to_str.h"

namespace maglev {

/// Fake load stats.
template <typename PointValueType = unsigned long long, size_t LoadSeqSize = 64>
class fake_load_stats {
public:
  using load_data_t     = sliding_window<PointValueType, LoadSeqSize>;
  using load_value_t    = typename load_data_t::point_value_t;
  using heartbeat_cnt_t = typename load_data_t::heartbeat_cnt_t;

  static constexpr size_t load_seq_size() { return load_data_t::seq_size(); }

  // A load_stats must have a heartbeat() method.
  void heartbeat() {}

  /*
  // should mock these methonds in derived class
  load_value_t load_unit() const;
  void         set_load_unit(load_value_t u);
  heartbeat_cnt_t heartbeat_cnt() const;
  load_data_t&       load();
  const load_data_t& load() const;
  int load_rank() const;
  void set_load_rank(int r);
  void incr_load();
  void incr_load(load_value_t d);
  */

  virtual std::string to_str() const { return maglev::to_str(*this); }

  template <typename Char, typename Traits>
  std::basic_ostream<Char, Traits>& output_stats(
      std::basic_ostream<Char, Traits>& os) const {
    os << "<>";
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
  using load_data_t     = sliding_window<PointValueType, LoadSeqSize>;
  using load_value_t    = typename load_data_t::point_value_t;
  using heartbeat_cnt_t = typename load_data_t::heartbeat_cnt_t;

  static constexpr size_t load_seq_size() { return load_data_t::seq_size(); }

  // A load_stats must have a heartbeat() method.
  void heartbeat() { load_.heartbeat(); }

  load_value_t load_unit() const { return load_.unit(); }
  void         set_load_unit(load_value_t u) { load_.set_unit(u); }

  heartbeat_cnt_t heartbeat_cnt() const { return load().heartbeat_cnt(); }

  load_data_t&       load() { return load_; }
  const load_data_t& load() const { return load_; }

  int load_rank() const { return load_rank_; }

  void set_load_rank(int r) { load_rank_ = r; }

  void incr_load() { load_.incr(load_unit()); }
  void incr_load(load_value_t d) { load_.incr(d); }

  virtual std::string to_str() const { return maglev::to_str(*this); }

  template <typename Char, typename Traits>
  std::basic_ostream<Char, Traits>& output_stats(
      std::basic_ostream<Char, Traits>& os) const {
    os << "<n:" << load().now() << ",l:" << load().last()
       << ",s:" << load().sum();
    os << ",avg:" << load().avg();
    if (load_rank()) os << ",rnk:" << load_rank();
    os << ",c:" << load().heartbeat_cnt() << ">";
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

  virtual std::string to_str() const override { return maglev::to_str(*this); }

  template <typename Char, typename Traits>
  std::basic_ostream<Char, Traits>& output_stats(
      std::basic_ostream<Char, Traits>& os) const {
    base_t::output_stats(os);
    if (last_ban_time() != 0 || consecutive_ban_cnt() != 0) {
      os << ",ban:(" << last_ban_time() << "," << consecutive_ban_cnt() << ")";
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
template <typename LoadStatsBase  = load_stats<>,
          typename QueryCntType   = unsigned int,
          typename LatencyCntType = unsigned long long,
          size_t SeqSize          = LoadStatsBase::load_seq_size()>
class server_load_stats_wrapper : public ban_wrapper<LoadStatsBase> {
  using base_t = ban_wrapper<LoadStatsBase>;

public:
  using query_cnt_t   = QueryCntType;
  using error_cnt_t   = QueryCntType;
  using fatal_cnt_t   = QueryCntType;
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

  int query_rank() const { return query_rank_; }
  int error_rank() const { return error_rank_; }
  int fatal_rank() const { return fatal_rank_; }
  int latency_rank() const { return latency_rank_; }

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

  virtual std::string to_str() const override { return maglev::to_str(*this); }

  template <typename Char, typename Traits>
  std::basic_ostream<Char, Traits>& output_stats(
      std::basic_ostream<Char, Traits>& os) const {
    base_t::output_stats(os);
    os << ",<n:(" << query().now() << "," << error().now() << ","
       << fatal().now() << "," << avg_latency_of_now() << ")";
    os << ",l:(" << query().last() << "," << error().last() << ","
       << fatal().last() << "," << avg_latency_of_last() << ")";
    os << ",s:(" << query().sum() << "," << error().sum() << ","
       << fatal().sum() << "," << avg_latency_of_window() << ")";
    // avg query per heartbeat, avg error rate, avg fatal rate
    os << ",avg:(q:" << query().avg() << ",er:" << error_rate_of_window()
       << ",fr:" << fatal_rate_of_window() << ")";
    // ranks
    if (query_rank() || error_rank() || fatal_rank() || latency_rank()) {
      os << ",rnk:(" << query_rank() << "," << error_rank() << ","
         << fatal_rank() << "," << latency_rank() << ")";
    }
    os << ">";
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
          typename LatencyCntType,
          size_t SeqSize>
std::basic_ostream<Char, Traits>& operator<<(
    std::basic_ostream<Char, Traits>&         os,
    const server_load_stats_wrapper<LoadStatsBase,
                                    QueryCntType,
                                    LatencyCntType,
                                    SeqSize>& s) {
  os << "[";
  s.output_stats(os);
  os << "]";
  return os;
}

/// To describe a unweighted server's load in RPC scene.
template <typename QueryCntType   = unsigned int,
          typename LatencyCntType = unsigned long long,
          size_t SeqSize          = 64>
class unweighted_server_load_stats
    : public server_load_stats_wrapper<fake_load_stats<QueryCntType>,
                                       QueryCntType,
                                       LatencyCntType,
                                       SeqSize> {
  using base_t = server_load_stats_wrapper<fake_load_stats<QueryCntType>,
                                           QueryCntType,
                                           LatencyCntType,
                                           SeqSize>;

public:
  using heartbeat_cnt_t = typename base_t::heartbeat_cnt_t;
  using load_data_t     = typename base_t::query_data_t;
  using load_value_t    = typename base_t::load_value_t;

  heartbeat_cnt_t heartbeat_cnt() const {
    return base_t::query().heartbeat_cnt();
  }

  load_value_t load_unit() const { return 1; }
  void         set_load_unit(load_value_t u) {}

  load_data_t&       load() { return base_t::query(); }
  const load_data_t& load() const { return base_t::query(); }
  int                load_rank() const { return base_t::query_rank(); }

  void set_load_rank(int r) {}
  void incr_load() {}
  void incr_load(load_value_t d) {}

  virtual std::string to_str() const override { return maglev::to_str(*this); }

  template <typename Char, typename Traits>
  std::basic_ostream<Char, Traits>& output_stats(
      std::basic_ostream<Char, Traits>& os) const {
    base_t::output_stats(os);
    os << ",c:" << heartbeat_cnt();
    return os;
  }
};

template <typename Char,
          typename Traits,
          typename QueryCntType,
          typename LatencyCntType,
          size_t SeqSize>
std::basic_ostream<Char, Traits>& operator<<(
    std::basic_ostream<Char, Traits>& os,
    const unweighted_server_load_stats<QueryCntType, LatencyCntType, SeqSize>&
        s) {
  os << "[";
  s.output_stats(os);
  os << "]";
  return os;
}

}  // namespace maglev
