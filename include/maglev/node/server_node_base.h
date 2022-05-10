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

#include <sstream>
#include <string>
#include <type_traits>
#include <utility>

#include "node_base.h"

namespace maglev {

template <typename NodeBaseType = node_base<std::string>>
class server_node_base : public NodeBaseType {
  using base_t = NodeBaseType;

public:
  template <typename... Args>
  server_node_base(const std::string& ip, int port, Args&&... args)
      : ip_(ip), port_(port), base_t(std::forward<Args>(args)...) {}

  server_node_base(const std::string& ip, int port)
      : ip_(ip), port_(port), base_t(make_id(ip, port)) {}

  const std::string& ip() const { return ip_; }

  int port() const { return port_; }

  static std::string make_id(const std::string& ip, int port) {
    return ip + ":" + std::to_string(port);
  }

  virtual std::string to_str() const override {
    return (std::ostringstream{} << *this).str();
  }

  template <typename Char, typename Traits>
  std::basic_ostream<Char, Traits>& output_members(
      std::basic_ostream<Char, Traits>& os) const {
    return base_t::output_members(os) << ",ip:" << ip() << ",port:" << port_;
  }

private:
  std::string ip_;
  int         port_;
};

template <typename Char, typename Traits, typename NodeBaseType>
std::basic_ostream<Char, Traits>& operator<<(
    std::basic_ostream<Char, Traits>&     os,
    const server_node_base<NodeBaseType>& n) {
  os << "{";
  n.output_members(os);
  os << "}";
  return os;
}

template <typename NodeBaseType>
class virtual_server_node_base : public server_node_base<NodeBaseType> {
  using base_t = server_node_base<NodeBaseType>;

public:
  template <typename... Args>
  virtual_server_node_base(const std::string& ip,
                           int                port,
                           int                vid,
                           Args&&... args)
      : vid_(vid), base_t(ip, port, std::forward<Args>(args)...) {}

  virtual_server_node_base(const std::string& ip, int port, int vid)
      : vid_(vid), base_t(ip, port, make_id(ip, port, vid)) {}

  int vid() const { return vid_; }

  static std::string make_id(const std::string& ip, int port, int vid) {
    return ip + ":" + std::to_string(port) + ":" + std::to_string(vid);
  }

  virtual std::string to_str() const override {
    return (std::ostringstream{} << *this).str();
  }

  template <typename Char, typename Traits>
  std::basic_ostream<Char, Traits>& output_members(
      std::basic_ostream<Char, Traits>& os) const {
    return base_t::output_members(os) << ",vid:" << vid();
  }

private:
  int vid_;
};

template <typename Char, typename Traits, typename NodeBaseType>
std::basic_ostream<Char, Traits>& operator<<(
    std::basic_ostream<Char, Traits>&             os,
    const virtual_server_node_base<NodeBaseType>& n) {
  os << "{";
  n.output_members(os);
  os << "}";
  return os;
}

}  // namespace maglev
