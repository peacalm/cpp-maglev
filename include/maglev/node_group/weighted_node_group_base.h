#pragma once

#include <utility>

namespace maglev {
namespace node_group {

template <typename NodeGroupBaseType>
class WeightedNodeGroupBase: public NodeGroupBaseType {
  using base_t = NodeGroupBaseType;
public:

  template<typename ...Args>
  WeightedNodeGroupBase(float max_avg_rate_limit = 0, Args&&...args) :
      max_avg_rate_limit_(max_avg_rate_limit), base_t(std::forward<Args>(args)...){}

  void init() {
    max_weight_ = 0;
    weight_sum_ = 0;
    for (const auto& i : *this) {
      int w = i->weight();
      weight_sum_ += w;
      if (w > max_weight_) max_weight_ = w;
    }
    avg_weight_ = float(max_weight_) / float(size());
    int max_weight_limit = avg_weight_ * max_avg_rate_limit_;
    if (max_weight_ > max_weight_limit) max_weight_ = max_weight_limit;
  }

  int max_weight() const { return max_weight_; }

  int weight_sum() const { return weight_sum_; }

private:
  // user defined
  float max_avg_rate_limit_ = 0;
  // internal params
  int weight_sum_ = 0;
  int max_weight_ = 0;
  float avg_weight_ = 0;
};

}  // namespace node_group
}  // namespace maglev
