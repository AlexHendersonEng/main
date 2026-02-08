#pragma once

#include <string>
#include <vector>

int to_csv(const std::string& filename, const std::vector<double>& t,
           const std::vector<std::vector<double>>& y);
