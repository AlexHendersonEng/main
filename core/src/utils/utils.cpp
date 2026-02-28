#include "utils/utils.h"

#include <fstream>
#include <vector>

int to_csv(const std::string& filename, const std::vector<double>& t,
           const std::vector<std::vector<double>>& y) {
  // Open output file
  std::ofstream file(filename);
  if (!file.is_open()) {
    return 1;
  }

  // Write CSV header: time column followed by solution components
  file << "t";
  for (size_t i = 0; i < y[0].size(); ++i) {
    file << ",y" << i;
  }
  file << '\n';

  // Write data rows
  for (size_t i = 0; i < y.size(); ++i) {
    file << t[i];
    for (double value : y[i]) {
      file << "," << value;
    }
    file << '\n';
  }

  return 0;
}

double vec_norm(const std::vector<double>& v) {
  double sum = 0.0;
  for (double val : v)
    sum += val * val;

  return std::sqrt(sum);
}