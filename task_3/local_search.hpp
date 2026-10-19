#pragma once

#include <random>

#include "../common/instance.hpp"
#include "../common/solution.hpp"
#include "../task_1/heuristics.hpp"

enum class SearchType { Steepest, Greedy };
enum class IntraMove { NodesExchange, EdgesExchange };

// Local search starting from `start` until no improving move exists.
// The neighborhood always contains inter-route moves (exchange a selected node
// with a not selected one) plus one type of intra-route moves (`intra`).
// Steepest: evaluate the whole neighborhood and apply the best move.
// Greedy: browse moves in random order and apply the first improving one.
// Returned objective = objective of start + sum of applied deltas.
Result local_search(const Instance& inst, const Solution& start, SearchType search,
                    IntraMove intra, std::mt19937& rng);
