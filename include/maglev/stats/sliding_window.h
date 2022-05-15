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

namespace maglev {

/// Records a constant length timing sequence of integer points.
template <typename PointValueType   = unsigned long long,
          PointValueType Unit       = 1,
          size_t         SeqSize    = 64,
          typename CounterType      = atomic_counter<PointValueType, Unit>,
          typename PointSeqType     = cycle_array<PointValueType, SeqSize>,
          typename HeartbeatCntType = size_t>
class sliding_window {
public:
  using point_value_t   = PointValueType;
  using counter_t       = CounterType;
  using point_seq_t     = PointSeqType;
  using heartbeat_cnt_t = HeartbeatCntType;

public:
  sliding_window() : now_(0), sum_(0), heartbeat_cnt_(0) {}

  constexpr point_value_t unit() const { return Unit; }
  constexpr size_t        seq_size() const { return SeqSize; }

  // incr point of now by one unit.
  void incr() { ++now_; }
  // incr load by specific value.
  void incr(point_value_t delta) { now_ += delta; }

  // Push now to seq, reset now to zero, drop oldest one in seq.
  void heartbeat() {
    sum_ += now_ - seq_.curr_item();
    seq_.push(now_);
    now_.clear();
    ++heartbeat_cnt_;
  }

  // now is an incomplete load point
  point_value_t now() const { return now_; }
  // last is a complete load point
  point_value_t last() const { return seq_.prev_item(); }
  // sum of all complete load points
  point_value_t sum() const { return sum_; }

  double avg() const {
    return double(sum()) /
           double(heartbeat_cnt_ < seq_size() && heartbeat_cnt_ > 0
                      ? heartbeat_cnt_
                      : seq_size());
  }

  heartbeat_cnt_t heartbeat_cnt() const { return heartbeat_cnt_; }

private:
  counter_t       now_;  // realtime, point of now, incomplete point
  point_value_t   sum_;  // sum of points in seq_
  point_seq_t     seq_;  // sequence of history points, all complete points
  heartbeat_cnt_t heartbeat_cnt_;
};

}  // namespace maglev
