#pragma once

#include "../common/instance.hpp"
#include "../common/solution.hpp"
#include "../task_1/heuristics.hpp"


Result regret_2_nn_any(const Instance& inst, int start, float regret_weight = 0.5);

Result regret_2_greedy_cycle(const Instance& inst, int start, float regret_weight = 0.5);
