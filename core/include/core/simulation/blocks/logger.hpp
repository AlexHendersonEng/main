#pragma once

#include <fstream>
#include <string>

#include "simulation/blocks/block.hpp"
#include "simulation/core/port.hpp"

template <typename T>
class Logger : public Block {
 public:
  // Constructor
  Logger(const std::string& path = "log.csv") : file_path(path) {}

  // Initialise block
  void init(double t) override {
    // Open file
    file.open(file_path);

    // Write CSV header
    file << "time,value\n";
  }

  // Step block
  void step(double t) override {
    // Get input value
    T value = inport.get();

    // Write to CSV
    if (file.is_open()) {
      file << t << "," << value << "\n";
    }

    // Pass through
    outport.set(value);
  }

  // Terminate block
  void term(double t) override {
    if (file.is_open()) {
      file.close();
    }
  }

  // Ports
  Inport<T> inport;
  Outport<T> outport;

 private:
  std::string file_path;
  std::ofstream file;
};
