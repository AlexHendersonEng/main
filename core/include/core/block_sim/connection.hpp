#ifndef CORE_CONNECTION_HPP_
#define CORE_CONNECTION_HPP_

namespace core::block_sim {

struct Connection {
  int from_block;
  int from_port;
  int to_block;
  int to_port;
};

}  // namespace core::block_sim

#endif  // CORE_CONNECTION_HPP_
