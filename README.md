# Maglev: a Consistent Hasher & Load Balancer

[![Build](https://github.com/peacalm/cpp-maglev/actions/workflows/cmake.yml/badge.svg)](actions)
[![License: MIT](https://img.shields.io/badge/License-MIT-blue.svg)](LICENSE)


**Maglev consistent hash algorithm** is published by Google, here is a C++ 
implementation library based on it. But this is not only a consistent hasher, 
it is also a dynamic load balancer based on consistent hash. 
This library is very useful as a Service Proxy to build a Stateful Service. 

Google's Maglev consistent hash algorithm doesn’t support consistent hash on 
weighted candidate nodes, here this library has supported this feature with 
an original idea. (By probability based on pseudo-random number sequence) 

This library is implemented by pluggable template parameters, it's very 
flexible to adapt to different application scenarios. 
Even its components can be used separately.

C++ standard requirement: >= C++14

## Intruduction

### maglev_hasher: only a consistent hasher
The class to do consistent hash:
```C++
template <typename NodeType        = node_base<std::string>,
          typename SlotArrayType   = slot_array<int>,
          typename NodeManagerType = typename std::conditional<
              is_weighted_t<NodeType>::value,
              weighted_node_manager_wrapper<node_manager_base<NodeType>>,
              node_manager_base<NodeType>>::type,
          typename PermutationGeneratorType =
              typename std::conditional<is_weighted_t<NodeManagerType>::value,
                                        permutation_generator_with_rand,
                                        permutation_generator>::type>
class maglev_hasher;
```
NodeType: A node is a candidate, which must a unique id, and the id must 
hashable. If the node has a weight, then the weight must be a **non-negative** 
number.
Here is a `maglev::weighted_node_wrapper` which can support weight for a node.

SlotArrayType: `maglev::slot_array` or `maglev::slot_vector`, 
slot number must be a **prime** number, and suggest it should be at least 10 times 
larger than number of candidate nodes.

NodeManagerType and NodeManagerType will be auto deduced.

### maglev_balancer: not only a consistent hasher, but a dynamic load balancer
```C++
template <typename MaglevHasherType =
              maglev_hasher<load_stats_wrapper<node_base<>, load_stats<>>>,
          typename BalanceStrategyType = default_balance_strategy>
class maglev_balancer;
```
MaglevHasherType: a maglev_hasher, its node type must have stats.

BalanceStrategyType: a struct contains dynamic balance parameters and methods.

## Usage Examples

### maglev_hasher

Candidates without weights:
```C++
// Use default template parameters
maglev::maglev_hasher<> h;
// Init candidate nodes, which has string type node id
for (int i = 0; i < 10; ++i) {
  std::string node_id = std::string("node-") + std::to_string(i);
  auto        n       = h.node_manager().new_node(node_id);
  h.node_manager().push_back(n);
}
// Init maglev_hasher
h.build();

for (int i = 0; i < 100; ++i) {
  auto req = i;
  // Pick with int type req
  auto node_ptr = h.pick_with_auto_hash(req).node;
  std::cout << "" << req << " -> " << node_ptr->id() << std::endl;
}
for (int i = 0; i < 100; ++i) {
  auto req = std::to_string(i);
  // Pick with string type req
  auto node_ptr = h.pick_with_auto_hash(req).node;
  std::cout << "maglev hash: " << req << " -> " << node_ptr->id() << std::endl;
}
```

Candidates with weights, Weighted Maglev Hasher:
```C++
// Nodes with int type id
maglev::maglev_hasher<maglev::weighted_node_wrapper<maglev::node_base<int>>> h;
for (int i = 0; i < 10; ++i) {
  h.node_manager().new_back(i)->set_weight(20 + rand() % 100);
}
h.build();
for (int i = 0; i < 100; ++i) {
  auto req      = i;
  auto node_ptr = h.pick_with_auto_hash(req).node;
  std::cout << "maglev hash: " << req << " -> " << node_ptr->id() << std::endl;
}
```

### maglev balancer

With unweighted nodes:
```C++
maglev::maglev_balancer<> b;
for (int i = 0; i < 10; ++i) { b.node_manager().new_back(std::to_string(i)); }
b.maglev_hasher().build();
for (int i = 0; i < 12345; ++i) {
  auto ret = b.pick_with_auto_hash(i);
  ret.node->incr_load();
  b.global_load().incr_load();
  // Heartbeat should better be called in another thread in real application
  if (i > 0 && i % 100 == 0) { b.heartbeat(); }
}
```

With unweighted server nodes:
```C++
maglev::maglev_balancer<maglev::maglev_hasher<
    maglev::load_stats_wrapper<maglev::node_base<std::string>,
                               maglev::unweighted_server_load_stats<>>>>
    b;
for (int i = 0; i < 10; ++i) { b.node_manager().new_back(std::to_string(i)); }
b.maglev_hasher().build();

for (int i = 0; i < 10000; ++i) {
  auto ret = b.pick_with_auto_hash(i);

  ret.node->incr_load();
  b.global_load().incr_load(ret.node->load_unit());

  // Mock RPC result
  bool fatal   = rand() % 50 == 0;
  bool error   = fatal || rand() % 10 == 0;
  int  latency = 100 + rand() % 50;

  ret.node->incr_server_load(1, error, fatal, latency);
  b.global_load().incr_server_load(1, error, fatal, latency);

  // Heartbeat should better be called in another thread in real application
  if (i > 0 && i % 300 == 0) { b.heartbeat(); }
}
```

With weighted server nodes:
```C++
maglev::maglev_balancer<maglev::maglev_hasher<maglev::load_stats_wrapper<
    maglev::slot_counted_node_wrapper<
        maglev::weighted_node_wrapper<maglev::server_node_base<>>>,
    maglev::server_load_stats_wrapper<>>>>
    b;
for (int i = 0; i < 10; ++i) {
  auto n = b.node_manager().new_back("10.0.0." + std::to_string(i), 88);
  n->set_weight(10 + rand() % 20);
}
b.maglev_hasher().build();

for (int i = 0; i < 10000; ++i) {
  auto ret = b.pick_with_auto_hash(i);

  ret.node->incr_load();
  b.global_load().incr_load(ret.node->load_unit());

  // mock RPC result
  bool fatal   = rand() % 50 == 0;
  bool error   = fatal || rand() % 10 == 0;
  int  latency = 100 + rand() % 50;

  ret.node->incr_server_load(1, error, fatal, latency);
  b.global_load().incr_server_load(1, error, fatal, latency);

  // heartbeat should be called in another thread
  if (i > 0 && i % 300 == 0) { b.heartbeat(); }
}
```
