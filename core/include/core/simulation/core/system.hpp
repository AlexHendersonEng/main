#pragma once

#include <memory>
#include <vector>

#include "simulation/blocks/block.hpp"

class System {
 public:
  // Constructor
  System() = default;

  // Destructor
  ~System() = default;

  // Blocks
  template <typename T, typename... Args>
  T& add_block(Args&&... args) {
    // Create unique pointer to block
    auto block = std::make_unique<T>(std::forward<Args>(args)...);

    // Extract reference to block
    T& ref = *block;

    // Add block to block vector
    blocks.push_back(std::move(block));

    // Return reference
    return ref;
  }

  // Connections
  template <typename T>
  void connect(Outport<T>& out, Inport<T>& in) {
    // Check for connection
    if (in.signal != nullptr) {
      throw std::runtime_error("Inport already connected");
    }

    // Create connection
    in.signal = out.signal;
  }

  // Initialise system
  void init(double t);

  // Step system
  void step(double t);

  // Terminate system
  void term(double t);

  // States
  size_t num_states();
  std::vector<double>& get_states();
  void set_states(const std::vector<double>& states);

  // Derivatives
  std::vector<double>& get_derivatives();

 private:
  // Blocks
  std::vector<std::unique_ptr<Block>> blocks;

  // States
  std::vector<double> states;

  // Derivatives
  std::vector<double> derivatives;
};
