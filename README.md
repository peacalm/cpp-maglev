# Maglev: a Consistent Hasher & Load Balancer

C++ standard requirement: >= C++14

## Intruduction
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


## Usage Examples
Candidates without weights:
```C++
// use default template parameters
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
  // pick with int type req
  auto node_ptr = h.pick_with_auto_hash(req).node;
  std::cout << "" << req << " -> " << node_ptr->id() << std::endl;
}
for (int i = 0; i < 100; ++i) {
  auto req = std::to_string(i);
  // pick with string type req
  auto node_ptr = h.pick_with_auto_hash(req).node;
  std::cout << "maglev hash: " << req << " -> " << node_ptr->id() << std::endl;
}
```

Candidates with weights, Weighted Maglev Hasher:
```C++
// nodes with int type id
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


