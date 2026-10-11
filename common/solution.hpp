#pragma once

#include <string>
#include <vector>

#include "instance.hpp"

// A solution is the cycle given as an ordered list of node indices.
// The edge from the last node back to the first one is implicit.
using Solution = std::vector<int>;

// Objective computed from scratch: cycle length + cost of the selected nodes.
long long evaluate(const Instance& inst, const Solution& sol);

// Checks that the solution has exactly k distinct nodes, all in range.
// On failure returns false and writes the reason to *error (if given).
bool validate(const Instance& inst, const Solution& sol, std::string* error = nullptr);
