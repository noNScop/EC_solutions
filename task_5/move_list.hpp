#pragma once

#include "../common/instance.hpp"
#include "../common/solution.hpp"
#include "../task_1/heuristics.hpp"

// Steepest local search with a list of improving moves (LM), reusing move
// evaluations (deltas) from previous iterations. Neighborhood as in the best
// variant of Assignment 3: inter-route node exchange + two-edges exchange.
// When no move in LM is applicable, all moves are evaluated again; the search
// stops when a fresh evaluation gives no applicable improving move.
Result local_search_move_list(const Instance& inst, const Solution& start);
