#pragma once

#include <string>
#include <vector>

// Problem instance. Coordinates are used only to build the distance matrix
// and are not stored, so optimization methods can only access dist and cost.
struct Instance {
    std::string name;
    int n = 0;
    std::vector<int> cost;
    std::vector<std::vector<int>> dist;

    // Number of nodes to select: 50%, rounded up.
    int k() const { return (n + 1) / 2; }
};

// Reads a file with lines "x;y;cost" and builds the distance matrix
// (Euclidean distance rounded to the nearest integer).
Instance load_instance(const std::string& path);
