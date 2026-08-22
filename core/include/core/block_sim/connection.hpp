#ifndef CORE_BLOCK_SIM_CONNECTION_HPP_
#define CORE_BLOCK_SIM_CONNECTION_HPP_

#include <memory>

#include "block_sim/port.hpp"

namespace core::block_sim {

class ConnectionBase {
 public:
  ConnectionBase() = default;
  virtual ~ConnectionBase() = default;

  virtual void propagate() = 0;
};

template <typename T>
class Connection : public ConnectionBase {
 public:
  Connection(Port<T>& outport, Port<T>& inport)
      : outport_(outport), inport_(inport) {}

  void propagate() override { outport_.set(inport_.get()); }

 private:
  Port<T>& outport_;
  Port<T>& inport_;
};

template <typename T>
std::unique_ptr<ConnectionBase> make_connection(Port<T>& outport,
                                                Port<T>& inport) {
  return std::make_unique<Connection<T>>(inport, outport);
}

}  // namespace core::block_sim

#endif  // CORE_BLOCK_SIM_CONNECTION_HPP_
