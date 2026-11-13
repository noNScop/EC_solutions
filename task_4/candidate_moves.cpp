#include "candidate_moves.hpp"
#include "../common/instance.hpp"
#include "../common/solution.hpp"

#include <algorithm>
#include <vector>

namespace {

enum class MoveType { Inter, EdgesExchange };

struct Move {
    MoveType type;
    int x, y;
};

class State {
public:
    State(const Instance& inst, const Solution& start)
        : d(inst.dist), c(inst.cost), k(static_cast<int>(start.size())), tour(start) {
        
        node_to_tour_idx.assign(inst.n, -1);
        node_to_unsel_idx.assign(inst.n, -1);

        std::vector<bool> selected(inst.n, false);
        for (int i = 0; i < k; ++i) {
            selected[tour[i]] = true;
            node_to_tour_idx[tour[i]] = i;
        }
        for (int v = 0; v < inst.n; ++v) {
            if (!selected[v]) {
                node_to_unsel_idx[v] = static_cast<int>(unselected.size());
                unselected.push_back(v);
            }
        }
    }

    int num_unselected() const { return static_cast<int>(unselected.size()); }
    const Solution& solution() const { return tour; }

    long long delta(const Move& m) const {
        if (m.type == MoveType::Inter) return delta_inter(m.x, m.y);
        return delta_edges(m.x, m.y);
    }

    void apply(const Move& m) {
        if (m.type == MoveType::Inter) apply_inter(m.x, m.y);
        else apply_edges(m.x, m.y);
    }

    // Public lookup tables for O(1) candidate checks
    std::vector<int> node_to_tour_idx;
    std::vector<int> node_to_unsel_idx;

private:
    const std::vector<std::vector<int>>& d;
    const std::vector<int>& c;
    int k;
    Solution tour;
    std::vector<int> unselected;

    int prev(int i) const { return tour[(i - 1 + k) % k]; }
    int next(int i) const { return tour[(i + 1) % k]; }

    // Inter-route: replace u = tour[i] with v = unselected[t]
    long long delta_inter(int i, int t) const {
        const int u = tour[i], v = unselected[t];
        const int a = prev(i), b = next(i);
        return d[a][v] + d[v][b] - d[a][u] - d[u][b] + c[v] - c[u];
    }

    void apply_inter(int i, int t) {
        const int u = tour[i], v = unselected[t];
        tour[i] = v;
        unselected[t] = u;
        // Keep fast lookup arrays updated!
        node_to_tour_idx[v] = i;
        node_to_tour_idx[u] = -1;
        node_to_unsel_idx[u] = t;
        node_to_unsel_idx[v] = -1;
    }

    // Intra-route (2-opt): reverse path between i+1 and j
    long long delta_edges(int i, int j) const {
        const int a = tour[i], a2 = tour[i + 1], b = tour[j], b2 = next(j);
        return d[a][b] + d[a2][b2] - d[a][a2] - d[b][b2];
    }

    void apply_edges(int i, int j) {
        std::reverse(tour.begin() + i + 1, tour.begin() + j + 1);
        // Keep fast lookup arrays updated for the reversed segment!
        for (int idx = i + 1; idx <= j; ++idx) {
            node_to_tour_idx[tour[idx]] = idx;
        }
    }
};

}  // namespace



// Candidate Moves Steepest Local Search

Result local_search_candidates(const Instance& inst, const Solution& start, int candidates_count) {
    State state(inst, start);
    long long objective = evaluate(inst, start);
    int k = static_cast<int>(start.size());
    
    int limit = std::min(candidates_count, inst.n - 1);

    // 1. Precompute Candidate Matrices
    std::vector<std::vector<int>> intra_candidates(inst.n, std::vector<int>(limit));
    std::vector<std::vector<int>> inter_candidates(inst.n, std::vector<int>(limit));

    for (int i = 0; i < inst.n; ++i) {
        std::vector<int> neighbors;
        neighbors.reserve(inst.n - 1);
        for (int j = 0; j < inst.n; ++j) {
            if (i != j) neighbors.push_back(j);
        }

        // Intra candidates: sort by Distance only
        std::partial_sort(neighbors.begin(), neighbors.begin() + limit, neighbors.end(),
            [&](int a, int b) {
                return inst.dist[i][a] < inst.dist[i][b];
            });
        for (int c = 0; c < limit; ++c) intra_candidates[i][c] = neighbors[c];

        // Inter candidates: sort by Distance + Cost
        std::partial_sort(neighbors.begin(), neighbors.begin() + limit, neighbors.end(),
            [&](int a, int b) {
                return inst.dist[i][a] + inst.cost[a] < inst.dist[i][b] + inst.cost[b];
            });
        for (int c = 0; c < limit; ++c) inter_candidates[i][c] = neighbors[c];
    }

    // 2. Steepest Search Loop
    while (true) {
        long long best_delta = 0;
        Move best_move = {MoveType::Inter, -1, -1};

        auto check = [&](const Move& m) {
            long long delta = state.delta(m);
            if (delta < best_delta) {
                best_delta = delta;
                best_move = m;
            }
        };

        for (int u = 0; u < inst.n; ++u) {
            bool u_selected = state.node_to_tour_idx[u] != -1;

            // --- INTRA-ROUTE (Edges Exchange) ---
            for (int v : intra_candidates[u]) {
                bool v_selected = state.node_to_tour_idx[v] != -1;
                
                if (u_selected && v_selected) {
                    int p1 = state.node_to_tour_idx[u];
                    int p2 = state.node_to_tour_idx[v];
                    if (p1 > p2) std::swap(p1, p2);

                    // Move 1: breaks edges (p1, p1+1) and (p2, p2+1)
                    if (p2 >= p1 + 2 && !(p1 == 0 && p2 == k - 1)) {
                        check({MoveType::EdgesExchange, p1, p2});
                    }

                    // Move 2: breaks edges (p1-1, p1) and (p2-1, p2)
                    int p3 = (p1 - 1 + k) % k;
                    int p4 = (p2 - 1 + k) % k;
                    if (p3 > p4) std::swap(p3, p4);
                    
                    if (p4 >= p3 + 2 && !(p3 == 0 && p4 == k - 1)) {
                        check({MoveType::EdgesExchange, p3, p4});
                    }
                }
            }

            // --- INTER-ROUTE (Nodes Exchange) ---
            for (int v : inter_candidates[u]) {
                bool v_selected = state.node_to_tour_idx[v] != -1;

                if (u_selected && !v_selected) {
                    int i = state.node_to_tour_idx[u];
                    int t = state.node_to_unsel_idx[v];
                    // Replace a neighbor of U with V to bring V next to U
                    check({MoveType::Inter, (i - 1 + k) % k, t});
                    check({MoveType::Inter, (i + 1) % k, t});
                } 
                else if (!u_selected && v_selected) {
                    int t = state.node_to_unsel_idx[u];
                    int i = state.node_to_tour_idx[v];
                    // Replace a neighbor of V with U to bring U next to V
                    check({MoveType::Inter, (i - 1 + k) % k, t});
                    check({MoveType::Inter, (i + 1) % k, t});
                }
            }
        }

        if (best_move.x == -1) break; // Local optimum reached
        state.apply(best_move);
        objective += best_delta;
    }

    return {state.solution(), objective};
}