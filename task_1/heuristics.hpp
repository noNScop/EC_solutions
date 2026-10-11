#pragma once

#include <random>

#include "../common/instance.hpp"
#include "../common/solution.hpp"

// Each heuristic returns the solution together with the objective value it
// tracked incrementally (sum of deltas). main() compares it with evaluate()
// to catch mistakes in the delta formulas.
struct Result {
    Solution tour;
    long long objective = 0;
};

// k nodes chosen uniformly at random, in random order.
Result random_solution(const Instance& inst, std::mt19937& rng);

// Nearest neighbor: the next node is appended only at the end of the path.
// Chooses the node minimizing d(last, j) + c_j. The cycle is closed at the end.
Result nn_end(const Instance& inst, int start);

// Nearest neighbor: the next node may be inserted at the beginning, at the end
// or between any two consecutive nodes of the (open) path. Chooses the
// (node, position) pair with the smallest increase of the objective.
Result nn_any(const Instance& inst, int start);

// Greedy cycle: keeps a closed cycle and inserts the (node, edge) pair with
// the smallest increase of the objective.
Result greedy_cycle(const Instance& inst, int start);
