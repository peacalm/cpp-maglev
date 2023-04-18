# Maglev: a Consistent Hasher & Dynamic Load Balancer

[![Build](https://github.com/peacalm/cpp-maglev/actions/workflows/cmake.yml/badge.svg)](https://github.com/peacalm/cpp-maglev/actions)
[![License: MIT](https://img.shields.io/badge/License-MIT-blue.svg)](LICENSE)


**Maglev consistent hash algorithm** is published by Google, here is a C++ 
implementation library based on it. But this is not only a consistent hasher, 
it is also a dynamic load balancer based on consistent hash. In particular, 
the dynamic load balancer can deal with hot key problem, which a pure Magle 
hasher can not.
This library is very useful as a Service Proxy to build a Stateful Service. 

Google's Maglev consistent hash algorithm doesn’t support consistent hash on 
weighted candidate nodes, here this library has supported this feature with 
an original idea. (By probability based on pseudo-random number sequence) 

This library is implemented by pluggable template parameters, it's very 
flexible to adapt to different application scenarios. 
Even its components can be used separately.

* C++ standard requirement: >= C++14

## Intruduction

### maglev_hasher: only a consistent hasher
The class to do consistent hash:
```c++
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
NodeType: A node is a candidate, which must have a unique id, and the id must 
hashable. If the node has a weight, then the weight must be a **non-negative** 
number.
Here is a `maglev::weighted_node_wrapper` which can support weight for a node.

SlotArrayType: `maglev::slot_array` or `maglev::slot_vector`, 
slot number must be a **prime** number, and suggest it should be at least 10 times 
larger than number of candidate nodes.

NodeManagerType and PermutationGeneratorType will be auto deduced.

### maglev_balancer: not only a consistent hasher, but also a dynamic load balancer
```c++
template <typename MaglevHasherType =
              maglev_hasher<load_stats_wrapper<node_base<>, load_stats<>>>,
          typename BalanceStrategyType = default_balance_strategy>
class maglev_balancer;
```
MaglevHasherType: a maglev_hasher, its node type must have stats.

BalanceStrategyType: a struct contains dynamic balance parameters and methods.

### Node Types
Node type is a primary template parameter for `maglev::maglev_hasher`. 
To make this lib flexible for different scenarios, node types are designed into 
node-base-types and node-wrapper-types. 
e.g. if you want to use weighted node, you can use a 
`maglev::weighted_node_wrapper` wrapped on a `maglev::node_base`.

```c++
// Basic node type with an immutable member id
template <typename IdType = std::string, typename HashType = def_hash_t<IdType>>
class node_base;

// A server node with member ip and port.
template <typename NodeBaseType = node_base<std::string>>
class server_node_base : public NodeBaseType;

// A virtual server node with member ip, port and virtual-id.
template <typename NodeBaseType = node_base<std::string>>
class virtual_server_node_base : public server_node_base<NodeBaseType>;

// A node wrapper to make a weighted node type.
template <typename NodeBaseType>
class weighted_node_wrapper : public NodeBaseType;

// A node wrapper to record how many slots obtained from maglev hasher.
// Useful for debug.
template <typename NodeBaseType>
class slot_counted_node_wrapper : public NodeBaseType;
```

Except for the above node types, which is called node-meta-type, 
nodes to be used for `maglev::maglev_balancer` must have a load-stats-type 
member. So here is a wrapper:

```c++
// A wrapper to make a node type have stats, mainly used for maglev_balancer.
template <typename NodeMetaType, typename LoadStatsType>
class load_stats_wrapper : public NodeMetaType, public LoadStatsType;
```

A load-stats-type is a timing sequence container with a constant length sliding 
window. Once a heartbeat called, it will generate a new point and drop out the 
oldest point. So it's useful to describe a node's load status during a short 
period of time.
```c++
// Basic load-stats-type.
template <typename PointValueType = unsigned long long, size_t LoadSeqSize = 64>
class load_stats;

// To describe a server's load in RPC scene. 
// Record info about query, error, fatal and latency.
template <typename LoadStatsBase  = load_stats<>,
          typename QueryCntType   = unsigned int,
          typename LatencyCntType = unsigned long long,
          size_t SeqSize          = LoadStatsBase::load_seq_size()>
class server_load_stats_wrapper : public ban_wrapper<LoadStatsBase>;


// To describe a unweighted server's load in RPC scene. 
// Use query count to describe load.
template <typename QueryCntType   = unsigned int,
          typename LatencyCntType = unsigned long long,
          size_t SeqSize          = 64>
class unweighted_server_load_stats
    : public server_load_stats_wrapper<fake_load_stats<QueryCntType>,
                                       QueryCntType,
                                       LatencyCntType,
                                       SeqSize>;
```

## Usage Examples

### maglev_hasher: a pure Maglev consistent hasher

Candidates without weights:
```c++
// Use default template parameters
maglev::maglev_hasher<> h;
// Init candidate nodes, which has string type node id
for (int i = 0; i < 10; ++i) {
  std::string node_id = std::string("node-") + std::to_string(i);
  // Add node. An alternate writing is: h.node_manager().new_back(node_id);
  auto n = h.node_manager().new_node(node_id);
  h.node_manager().push_back(n);
}
// Init maglev_hasher
h.build();

for (int i = 0; i < 100; ++i) {
  auto req = i;
  // Pick with int type req
  auto node_ptr = h.pick_with_auto_hash(req).node;
  std::cout << "maglev hash: " << req << " -> " << node_ptr->id() << std::endl;
}
for (int i = 0; i < 100; ++i) {
  auto req = std::to_string(i);
  // Pick with string type req
  auto node_ptr = h.pick_with_auto_hash(req).node;
  std::cout << "maglev hash: " << req << " -> " << node_ptr->id() << std::endl;
}
```

Candidates with weights, Weighted Maglev Hasher:
```c++
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

### maglev balancer: a dynamic load balancer based on Maglev consistent hasher

With unweighted nodes:
```c++
maglev::maglev_balancer<> b;
for (int i = 0; i < 10; ++i) { b.node_manager().new_back(std::to_string(i)); }
b.maglev_hasher().build();
for (int i = 0; i < 12345; ++i) {
  auto ret = b.pick_with_auto_hash(i);
  ret.node->incr_load();
  b.global_load().incr_load();
  // Heartbeat should better be called in another thread as a cron job in real application
  if (i > 0 && i % 100 == 0) { b.heartbeat(); }
}
```

With unweighted server nodes:
```c++
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

  // Heartbeat should better be called in another thread as a cron job in real application
  if (i > 0 && i % 300 == 0) { b.heartbeat(); }
}
```

With weighted server nodes:
```c++
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

  // Heartbeat should better be called in another thread as a cron job in real application
  if (i > 0 && i % 300 == 0) { b.heartbeat(); }
}
```

## Build, Test, Install
Test cases are built using [GoogleTest](https://github.com/google/googletest), 
you need to install it first.
Then run:
```Bash
git clone https://github.com/peacalm/cpp-maglev.git
cd cpp-maglev
mkdir build
cd build
cmake .. -DBUILD_TEST=TRUE
make
# Test
make test  # or ctest
# Install
sudo make install
```
