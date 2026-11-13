#pragma once

#include "../common/instance.hpp"
#include "../common/solution.hpp"
#include "../task_1/heuristics.hpp"

// Baseline steepest local search: evaluates all possible moves in the neighborhood.
// Neighborhood: Inter-route (node exchange) + Intra-route (edge exchange).
Result local_search_baseline(const Instance& inst, const Solution& start);

// Optimized steepest local search: uses 10-nearest-neighbor candidate edges.
// Intra-route candidate edges are chosen by distance.
// Inter-route candidate edges are chosen by distance + node cost.
Result local_search_candidates(const Instance& inst, const Solution& start, int candidates_count = 10);