#include "heuristics.hpp"

#include <algorithm>
#include <climits>
#include <numeric>

Result random_solution(const Instance& inst, std::mt19937& rng) {
    std::vector<int> nodes(inst.n);
    std::iota(nodes.begin(), nodes.end(), 0);
    std::shuffle(nodes.begin(), nodes.end(), rng);
    nodes.resize(inst.k());

    Result r;
    r.tour = nodes;
    r.objective = evaluate(inst, r.tour);
    return r;
}

Result nn_end(const Instance& inst, int start) {
    const auto& d = inst.dist;
    const auto& c = inst.cost;
    std::vector<char> used(inst.n, 0);

    Result r;
    r.tour.push_back(start);
    used[start] = 1;
    r.objective = c[start];

    while (static_cast<int>(r.tour.size()) < inst.k()) {
        const int last = r.tour.back();
        int best_node = -1;
        long long best_delta = LLONG_MAX;
        for (int j = 0; j < inst.n; ++j) {
            if (used[j]) continue;
            long long delta = d[last][j] + c[j];
            if (delta < best_delta) {
                best_delta = delta;
                best_node = j;
            }
        }
        r.tour.push_back(best_node);
        used[best_node] = 1;
        r.objective += best_delta;
    }

    // Close the cycle.
    r.objective += d[r.tour.back()][r.tour.front()];
    return r;
}

Result nn_any(const Instance& inst, int start) {
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

        for (int j = 0; j < inst.n; ++j) {
            if (used[j]) continue;
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

                if (delta < best_delta) {
                    best_delta = delta;
                    best_node = j;
                    best_pos = pos;
                }
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

Result greedy_cycle(const Instance& inst, int start) {
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

        for (int j = 0; j < inst.n; ++j) {
            if (used[j]) continue;
            // Edge i connects cycle[i] and cycle[i + 1] (wrapping around).
            for (int i = 0; i < m; ++i) {
                const int a = cycle[i];
                const int b = cycle[(i + 1) % m];
                long long delta = d[a][j] + d[j][b] - d[a][b] + c[j];
                if (delta < best_delta) {
                    best_delta = delta;
                    best_node = j;
                    best_edge = i;
                }
            }
        }
        r.tour.insert(r.tour.begin() + best_edge + 1, best_node);
        used[best_node] = 1;
        r.objective += best_delta;
    }
    return r;
}
