#pragma once

/**
 * @file utils.h
 * @brief Utility functions for input/output operations.
 *
 * This header declares helper functions used for exporting numerical results,
 * such as writing time-dependent solution data to CSV files.
 */

#include <string>
#include <vector>

/**
 * @brief Write time-series solution data to a CSV file.
 *
 * This function writes a set of time points and corresponding solution vectors
 * to a comma-separated values (CSV) file. The first column contains the time
 * values, followed by one column per component of the solution vector.
 *
 * The CSV file format is:
 * @code
 * t,y0,y1,y2,...
 * t0,...
 * t1,...
 * @endcode
 *
 * @param filename Path to the output CSV file.
 * @param t        Vector of time points.
 * @param y        Vector of solution vectors corresponding to each time point.
 *
 * @return Zero on success, non-zero on failure (e.g. file could not be opened).
 *
 * @note The size of @p t must match the size of @p y, and all entries in
 *       @p y must have the same length.
 */
int to_csv(const std::string& filename, const std::vector<double>& t,
           const std::vector<std::vector<double>>& y);