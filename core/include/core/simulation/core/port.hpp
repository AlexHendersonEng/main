#pragma once

#include <memory>
#include <stdexcept>
#include <typeinfo>

#include "simulation/blocks/block.hpp"
#include "simulation/core/signal.hpp"

class Port {
 public:
  // Constructor
  Port() = default;

  // Destructor
  ~Port() = default;

  // Type
  virtual const std::type_info& type() const = 0;
};

template <typename T>
class Inport : public Port {
 public:
  // Type
  const std::type_info& type() const override { return typeid(T); }

  // Signal
  std::shared_ptr<Signal<T>> signal = nullptr;

  // Getter
  T get() const {
    if (!signal) {
      throw std::runtime_error("Inport not connected");
    }
    return signal->get();
  }

  // Connections
  bool is_connected() const { return signal != nullptr; }
};

template <typename T>
class Outport : public Port {
 public:
  // Type
  const std::type_info& type() const override { return typeid(T); }

  // Signal
  std::shared_ptr<Signal<T>> signal = std::make_shared<Signal<T>>();

  // Setter
  void set(const T& value) { signal->set(value); }
};
