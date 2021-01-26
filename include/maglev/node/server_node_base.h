#pragma once

#include <sstream>
#include <string>
#include <type_traits>
#include <utility>

#include "node_base.h"

namespace maglev {


template <typename NodeBaseType = NodeBase<std::string>>
class ServerNodeBase : public NodeBaseType {
  using base_t = NodeBaseType;

public:
  template <typename ...Args>
  ServerNodeBase(const std::string& ip, int port, Args&& ...args) :
      ip_(ip), port_(port), base_t(std::forward<Args>(args)...) {}

  ServerNodeBase(const std::string& ip, int port) : ip_(ip), port_(port), base_t(make_id(ip, port)) {}

  const std::string& ip() const { return ip_; }

  int port() const { return port_; }

public:
  static std::string make_id(const std::string& ip, int port) {
    return ip + ":" + std::to_string(port);
  }

private:
  std::string ip_;
  int port_;
};

template <typename Char, typename Traits, typename NodeBaseType>
std::basic_ostream<Char, Traits>& operator<<(std::basic_ostream<Char, Traits>& os,
                                             const ServerNodeBase<NodeBaseType>& n) {
  os << "{";
  n.output_members(os);
  os << "}";
  return os;
}


template <typename NodeBaseType>
class VirtualServerNodeBase : public ServerNodeBase<NodeBaseType> {
  using base_t = ServerNodeBase<NodeBaseType>;

public:
  template <typename ...Args>
  VirtualServerNodeBase(const std::string& ip, int port, int vid, Args&& ...args) :
      vid_(vid), base_t(ip, port, std::forward<Args>(args)...) {}

  VirtualServerNodeBase(const std::string& ip, int port, int vid) :
      vid_(vid), base_t(ip, port, make_id(ip, port, vid)) {}

public:
  static std::string make_id(const std::string& ip, int port, int vid) {
    return ip + ":" + std::to_string(port) + ":" + std::to_string(vid);
  }

private:
  int vid_;
};


}  // namespace maglev
