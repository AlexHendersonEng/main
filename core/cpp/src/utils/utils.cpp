/**
 * @file utils.cpp
 * @brief Implementation of utility functions for CSV output.
 *
 * This file provides the implementation of functions used to export numerical
 * solution data to CSV files.
 */

#include "utils/utils.h"

#include <fstream>

/**
 * @brief Write time-series solution data to a CSV file.
 *
 * This function outputs the time vector and associated solution vectors to a
 * CSV file. Each row corresponds to a single time step.
 *
 * @param filename Path to the output CSV file.
 * @param t        Vector of time points.
 * @param y        Vector of solution vectors corresponding to each time point.
 *
 * @return Zero on success, non-zero if the file could not be opened.
 *
 * @note This function does not perform explicit bounds or consistency checks
 *       on the input vectors. It assumes that @p t and @p y have the same size
 *       and that @p y is non-empty.
 */
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