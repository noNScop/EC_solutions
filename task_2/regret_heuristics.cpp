#include "regret_heuristics.hpp"

#include <algorithm>
#include <climits>
#include <numeric>

#include "../task_1/heuristics.hpp"

Result regret_2_nn_any(const Instance& inst, int start, float regret_weight) {
    const auto& d = inst.dist;
    const auto& c = inst.cost;
    std::vector<char> used(inst.n, 0);

    Result r;
    r.tour.push_back(start);
    used[start] = 1;
    r.objective = c[start];

    while (static_cast<int>(r.tour.size()) < inst.k()) {
        const auto& path = r.tour;
        const int m = static_cast<int>(path.size());
        int best_node = -1, best_pos = -1;
        long long best_delta = LLONG_MAX;
        double best_value = 1e18;

        for (int j = 0; j < inst.n; ++j) {  
            if (used[j]) continue;
            long long delta1 = LLONG_MAX, delta2 = LLONG_MAX;
            int best_pos_for_j = -1;
            // pos = index at which j is inserted (before path[pos]); pos == m means at the end.
            for (int pos = 0; pos <= m; ++pos) {
                long long delta;
                if (pos == 0)
                    delta = d[j][path[0]];
                else if (pos == m)
                    delta = d[path[m - 1]][j];
                else
                    delta = d[path[pos - 1]][j] + d[j][path[pos]] - d[path[pos - 1]][path[pos]];
                delta += c[j];
                if (delta < delta1) {
                    delta2 = delta1;
                    delta1 = delta;
                    best_pos_for_j = pos;
                }
                else if (delta < delta2) {
                    delta2 = delta;
                }
            }
            long long regret = delta2 - delta1;
            // we want to maximize regret and minimize the best insertion cost
            double value = -(regret_weight * regret) + (1.0 - regret_weight) * delta1;
            if (value < best_value) {
                best_value = value;
                best_node = j;
                best_pos = best_pos_for_j;
                best_delta = delta1;
            }
        }
        r.tour.insert(r.tour.begin() + best_pos, best_node);
        used[best_node] = 1;
        r.objective += best_delta;
    }

    // Close the cycle.
    r.objective += d[r.tour.back()][r.tour.front()];
    return r;
}

Result regret_2_greedy_cycle(const Instance& inst, int start, float regret_weight) {
    const auto& d = inst.dist;
    const auto& c = inst.cost;
    std::vector<char> used(inst.n, 0);

    Result r;
    r.tour.push_back(start);
    used[start] = 1;
    r.objective = c[start];
    if (inst.k() == 1) return r;

    // Second node: the cycle start -> j -> start costs 2 * d(start, j) + c_j.
    {
        int best_node = -1;
        long long best_delta = LLONG_MAX;
        for (int j = 0; j < inst.n; ++j) {
            if (used[j]) continue;
            long long delta = 2LL * d[start][j] + c[j];
            if (delta < best_delta) {
                best_delta = delta;
                best_node = j;
            }
        }
        r.tour.push_back(best_node);
        used[best_node] = 1;
        r.objective += best_delta;
    }

    while (static_cast<int>(r.tour.size()) < inst.k()) {
        const auto& cycle = r.tour;
        const int m = static_cast<int>(cycle.size());
        int best_node = -1, best_edge = -1;
        long long best_delta = LLONG_MAX;
        double best_value = 1e18;

        for (int j = 0; j < inst.n; ++j) {
            if (used[j]) continue;
            long long delta1 = LLONG_MAX, delta2 = LLONG_MAX;
            int best_edge_for_j = -1;
            // Edge i connects cycle[i] and cycle[i + 1] (wrapping around).
            for (int i = 0; i < m; ++i) {
                
                const int a = cycle[i];
                const int b = cycle[(i + 1) % m];
                long long delta = d[a][j] + d[j][b] - d[a][b] + c[j];
                if (delta < delta1) {
                    delta2 = delta1;
                    delta1 = delta;
                    best_edge_for_j = i;
                }
                else if (delta < delta2) {
                    delta2 = delta;
                }
            }

            long long regret = delta2 - delta1;
            // we want to maximize regret and minimize the best insertion cost
            double value = -(regret_weight * regret) + (1.0 - regret_weight) * delta1;
            if (value < best_value) {
                best_value = value;
                best_node = j;
                best_edge = best_edge_for_j;
                best_delta = delta1;
            }
        }
        r.tour.insert(r.tour.begin() + best_edge + 1, best_node);
        used[best_node] = 1;
        r.objective += best_delta;
    }
    return r;
}