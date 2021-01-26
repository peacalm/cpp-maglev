#pragma once
#include <algorithm>
#include <utility>

namespace maglev {
namespace node_group {

template <typename NodeGroupBaseType>
class WeightedNodeGroupWrapper: public NodeGroupBaseType {
  using base_t = NodeGroupBaseType;

public:

  /// limit max_weight by max_weight <= avg_weight * max_avg_rate_limit, 0 means no limit
  template<typename ...Args>
  WeightedNodeGroupWrapper(double max_avg_rate_limit, Args&&...args) :
      max_avg_rate_limit_(max_avg_rate_limit), base_t(std::forward<Args>(args)...){}

  virtual void ready_go() override { base_t::ready_go(); init(); }

  void init() {
    weight_sum_ = 0;
    real_max_weight_ = 0;
    for (const auto& i : *this) {
      int w = i->weight();
      weight_sum_ += w;
      real_max_weight_ = std::max(real_max_weight_, w);
    }
    avg_weight_ = double(weight_sum_) / double(size());
    if (max_avg_rate_limit_ > 0) {
      int limit = avg_weight_ * max_avg_rate_limit_;
      max_weight_ = std::min(real_max_weight_, limit);
    }
  }

  void set_max_avg_rate_limit(double r) const { max_avg_rate_limit_ = r; }

  double max_avg_rate_limit() const { return max_avg_rate_limit_; }

  int weight_sum() const { return weight_sum_; }

  double avg_weight() const { return avg_weight_; }

  // limited max weight, which is really used in maglev
  int max_weight() const { return max_weight_; }

  int real_max_weight() const { return real_max_weight_; }

private:
  // user defined
  double max_avg_rate_limit_ = 0;
  // internal params
  int weight_sum_ = 0;
  int max_weight_ = 0;
  int real_max_weight_ = 0;
  double avg_weight_ = 0;
};

}  // namespace node_group
}  // namespace maglev
