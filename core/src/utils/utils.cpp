#include "utils/utils.h"

#include <fstream>

int to_csv(const std::string& filename, const std::vector<double>& t,
           const std::vector<std::vector<double>>& y) {
  // Open file
  std::ofstream file(filename);
  if (!file.is_open()) {
    return 1;
  }

  // Write header
  file << "t";
  for (size_t i = 0; i < y[0].size(); ++i) {
    file << ",y" << i;
  }
  file << '\n';

  // Write data
  for (size_t i = 0; i < y.size(); ++i) {
    file << t[i];
    for (double value : y[i]) {
      file << "," << value;
    }
    file << '\n';
  }

  return 0;
}
