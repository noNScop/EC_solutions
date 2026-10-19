#include "local_search.hpp"

#include <algorithm>

namespace {

enum class MoveType { Inter, NodesExchange, EdgesExchange };

// A move is described by indices, not node ids, so the list of all moves
// stays valid while the solution changes.
//   Inter:         x = position in tour, y = index in the unselected list
//   NodesExchange: x < y are positions in tour (swap the nodes at them)
//   EdgesExchange: x < y are positions in tour; removes edges (x, x+1) and (y, y+1)
struct Move {
    MoveType type;
    int x, y;
};

class State {
public:
    State(const Instance& inst, const Solution& start)
        : d(inst.dist), c(inst.cost), k(static_cast<int>(start.size())), tour(start),
          pos(inst.n, -1) {
        for (int i = 0; i < k; ++i) pos[tour[i]] = i;
        for (int v = 0; v < inst.n; ++v)
            if (pos[v] == -1) unselected.push_back(v);
    }

    int num_unselected() const { return static_cast<int>(unselected.size()); }
    const Solution& solution() const { return tour; }

    long long delta(const Move& m) const {
        switch (m.type) {
            case MoveType::Inter: return delta_inter(m.x, m.y);
            case MoveType::NodesExchange: return delta_nodes(m.x, m.y);
            case MoveType::EdgesExchange: return delta_edges(m.x, m.y);
        }
        return 0;
    }

    void apply(const Move& m) {
        switch (m.type) {
            case MoveType::Inter: apply_inter(m.x, m.y); break;
            case MoveType::NodesExchange: apply_nodes(m.x, m.y); break;
            case MoveType::EdgesExchange: apply_edges(m.x, m.y); break;
        }
    }

private:
    const std::vector<std::vector<int>>& d;
    const std::vector<int>& c;
    int k;
    Solution tour;                // selected nodes in cycle order
    std::vector<int> pos;         // pos[v] = position of v in tour, -1 if not selected
    std::vector<int> unselected;  // nodes not in tour (order does not matter)

    int prev(int i) const { return tour[(i - 1 + k) % k]; }
    int next(int i) const { return tour[(i + 1) % k]; }

    // Replace u = tour[i] with v = unselected[t]: ... a u b ... -> ... a v b ...
    long long delta_inter(int i, int t) const {
        const int u = tour[i], v = unselected[t];
        const int a = prev(i), b = next(i);
        return d[a][v] + d[v][b] - d[a][u] - d[u][b] + c[v] - c[u];
    }

    void apply_inter(int i, int t) {
        const int u = tour[i], v = unselected[t];
        tour[i] = v;
        pos[v] = i;
        pos[u] = -1;
        unselected[t] = u;
    }

    // Swap u = tour[i] and v = tour[j], i < j. Node costs do not change.
    long long delta_nodes(int i, int j) const {
        const int u = tour[i], v = tour[j];
        if (j == i + 1) {  // neighbors: ... a u v b ... -> ... a v u b ...
            const int a = prev(i), b = next(j);
            return d[a][v] + d[u][b] - d[a][u] - d[v][b];
        }
        if (i == 0 && j == k - 1) {  // neighbors through the closing edge: ... a v | u b ...
            const int a = prev(j), b = next(i);
            return d[a][u] + d[v][b] - d[a][v] - d[u][b];
        }
        const int pi = prev(i), ni = next(i), pj = prev(j), nj = next(j);
        return d[pi][v] + d[v][ni] + d[pj][u] + d[u][nj]
             - d[pi][u] - d[u][ni] - d[pj][v] - d[v][nj];
    }

    void apply_nodes(int i, int j) {
        std::swap(tour[i], tour[j]);
        pos[tour[i]] = i;
        pos[tour[j]] = j;
    }

    // Remove edges (a, a2) and (b, b2), add (a, b) and (a2, b2):
    // a a2 ... b b2 -> a b ... a2 b2 (the part between them is reversed).
    long long delta_edges(int i, int j) const {
        const int a = tour[i], a2 = tour[i + 1], b = tour[j], b2 = next(j);
        return d[a][b] + d[a2][b2] - d[a][a2] - d[b][b2];
    }

    void apply_edges(int i, int j) {
        std::reverse(tour.begin() + i + 1, tour.begin() + j + 1);
        for (int p = i + 1; p <= j; ++p) pos[tour[p]] = p;
    }
};

std::vector<Move> all_moves(int k, int num_unselected, IntraMove intra) {
    std::vector<Move> moves;
    for (int i = 0; i < k; ++i)
        for (int t = 0; t < num_unselected; ++t) moves.push_back({MoveType::Inter, i, t});

    if (intra == IntraMove::NodesExchange) {
        for (int i = 0; i < k; ++i)
            for (int j = i + 1; j < k; ++j) moves.push_back({MoveType::NodesExchange, i, j});
    } else {
        // Edges must not be adjacent: j >= i + 2, and edge (k-1, 0) is adjacent to edge (0, 1).
        for (int i = 0; i < k; ++i)
            for (int j = i + 2; j < k; ++j)
                if (!(i == 0 && j == k - 1)) moves.push_back({MoveType::EdgesExchange, i, j});
    }
    return moves;
}

}  // namespace

Result local_search(const Instance& inst, const Solution& start, SearchType search,
                    IntraMove intra, std::mt19937& rng) {
    State state(inst, start);
    std::vector<Move> moves =
        all_moves(static_cast<int>(start.size()), state.num_unselected(), intra);
    long long objective = evaluate(inst, start);

    if (search == SearchType::Steepest) {
        while (true) {
            long long best_delta = 0;
            const Move* best = nullptr;
            for (const Move& m : moves) {
                const long long delta = state.delta(m);
                if (delta < best_delta) {
                    best_delta = delta;
                    best = &m;
                }
            }
            if (!best) break;  // no improving move: local optimum
            state.apply(*best);
            objective += best_delta;
        }
    } else {
        // Moves are visited in uniformly random order using a lazy Fisher-Yates
        // shuffle: before evaluating the move at index idx, swap a random move from
        // [idx, M) into that place. Only moves that are actually evaluated get
        // shuffled, and inter- and intra-route moves are mixed in one list.
        const size_t M = moves.size();
        while (true) {
            bool improved = false;
            for (size_t idx = 0; idx < M; ++idx) {
                std::uniform_int_distribution<size_t> pick(idx, M - 1);
                std::swap(moves[idx], moves[pick(rng)]);
                const long long delta = state.delta(moves[idx]);
                if (delta < 0) {
                    state.apply(moves[idx]);
                    objective += delta;
                    improved = true;
                    break;
                }
            }
            if (!improved) break;  // all moves checked, none improving: local optimum
        }
    }

    return {state.solution(), objective};
}
