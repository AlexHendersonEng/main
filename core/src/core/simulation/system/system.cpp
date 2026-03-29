#include "simulation/system/system.hpp"

void System::init() {
  // Loop through all blocks and initialise them
  for (auto& block : blocks) block->init(time);
}

void System::step() {
  // Loop through all blocks and step them
  for (auto& block : blocks) block->step(time);
}

void System::term() {
  // Loop through all blocks and terminate them
  for (auto& block : blocks) block->term(time);
}
