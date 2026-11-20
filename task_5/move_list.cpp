#include "move_list.hpp"

#include <algorithm>
#include <tuple>
#include <vector>

namespace {

enum class MoveType { Inter, EdgesExchange };

// Moves are saved by node ids, not positions, so a saved move can be checked
// against the current solution after other moves have been applied.
//   Inter:         n1 = p, n2 = u, n3 = n, n4 = v
//                  replace selected u (neighbors p and n) with not selected v
//   EdgesExchange: n1 = a, n2 = a2, n3 = b, n4 = b2
//                  remove edges a->a2 and b->b2 (in this direction), add (a, b) and (a2, b2)
struct Move {
    long long delta;
    MoveType type;
    int n1, n2, n3, n4;

    bool operator<(const Move& o) const {
        return std::tie(delta, type, n1, n2, n3, n4) <
               std::tie(o.delta, o.type, o.n1, o.n2, o.n3, o.n4);
    }
};

// What to do with a saved move when browsing LM.
enum class Status { Remove, Skip, Apply };

class State {
public:
    State(const Instance& inst, const Solution& start)
        : d(inst.dist), c(inst.cost), k(static_cast<int>(start.size())), tour(start),
          pos(inst.n, -1), unsel_pos(inst.n, -1) {
        for (int i = 0; i < k; ++i) pos[tour[i]] = i;
        for (int v = 0; v < inst.n; ++v)
            if (pos[v] == -1) {
                unsel_pos[v] = static_cast<int>(unselected.size());
                unselected.push_back(v);
            }
    }

    const Solution& solution() const { return tour; }

    // Evaluates all moves applicable to the current solution, plus two-edges
    // exchanges with relatively inverted edges, and returns the improving ones
    // sorted from the best to the worst.
    std::vector<Move> evaluate_all() const {
        std::vector<Move> lm;
        for (int i = 0; i < k; ++i) {
            const int p = prev(tour[i]), u = tour[i], n = next(tour[i]);
            for (int v : unselected) {
                const long long delta = d[p][v] + d[v][n] - d[p][u] - d[u][n] + c[v] - c[u];
                if (delta < 0) lm.push_back({delta, MoveType::Inter, p, u, n, v});
            }
        }
        for (int i = 0; i < k; ++i) {
            for (int j = i + 2; j < k; ++j) {
                if (i == 0 && j == k - 1) continue;  // edges (k-1, 0) and (0, 1) are adjacent
                const int a = tour[i], a2 = tour[i + 1], b = tour[j], b2 = tour[(j + 1) % k];
                const long long removed = d[a][a2] + d[b][b2];
                // a->a2 and b->b2 in the same direction: add (a, b) and (a2, b2)
                const long long delta = d[a][b] + d[a2][b2] - removed;
                if (delta < 0) lm.push_back({delta, MoveType::EdgesExchange, a, a2, b, b2});
                // Relatively inverted: a->a2 and b2->b, add (a, b2) and (a2, b). Not
                // applicable now, but it can become applicable after a reversal.
                const long long delta_inv = d[a][b2] + d[a2][b] - removed;
                if (delta_inv < 0) lm.push_back({delta_inv, MoveType::EdgesExchange, a, a2, b2, b});
            }
        }
        std::sort(lm.begin(), lm.end());
        return lm;
    }

    Status check(const Move& m) const {
        if (m.type == MoveType::Inter) {
            const int p = m.n1, u = m.n2, n = m.n3, v = m.n4;
            if (pos[u] == -1 || pos[v] != -1) return Status::Remove;
            // Both edges (p, u) and (u, n) must still exist, in any direction.
            const int pu = prev(u), nu = next(u);
            const bool same = (pu == p && nu == n) || (pu == n && nu == p);
            return same ? Status::Apply : Status::Remove;
        }
        const int dir1 = edge_direction(m.n1, m.n2), dir2 = edge_direction(m.n3, m.n4);
        if (dir1 == 0 || dir2 == 0) return Status::Remove;  // a removed edge no longer exists
        if (dir1 != dir2) return Status::Skip;              // different relative direction
        return Status::Apply;                               // same direction, or both reversed
    }

    void apply(const Move& m) {
        if (m.type == MoveType::Inter) {
            const int u = m.n2, v = m.n4;
            const int i = pos[u], t = unsel_pos[v];
            tour[i] = v;
            pos[v] = i;
            pos[u] = -1;
            unselected[t] = u;
            unsel_pos[u] = t;
            unsel_pos[v] = -1;
            return;
        }
        int a = m.n1, a2 = m.n2, b = m.n3, b2 = m.n4;
        if (edge_direction(a, a2) == -1) {  // both edges reversed: a2->a and b2->b
            std::swap(a, a2);
            std::swap(b, b2);
        }
        // Now a->a2 and b->b2 are edges in tour order; reversing the part between
        // them adds (a, b) and (a2, b2).
        int i = pos[a], j = pos[b];
        if (i > j) std::swap(i, j);
        std::reverse(tour.begin() + i + 1, tour.begin() + j + 1);
        for (int p = i + 1; p <= j; ++p) pos[tour[p]] = p;
    }

private:
    const std::vector<std::vector<int>>& d;
    const std::vector<int>& c;
    int k;
    Solution tour;                // selected nodes in cycle order
    std::vector<int> pos;         // pos[v] = position of v in tour, -1 if not selected
    std::vector<int> unselected;  // nodes not in tour (order does not matter)
    std::vector<int> unsel_pos;   // unsel_pos[v] = index of v in unselected, -1 if selected

    int prev(int v) const { return tour[(pos[v] - 1 + k) % k]; }
    int next(int v) const { return tour[(pos[v] + 1) % k]; }

    // +1 if the tour contains x->y, -1 if it contains y->x, 0 if there is no edge (x, y).
    int edge_direction(int x, int y) const {
        if (pos[x] == -1 || pos[y] == -1) return 0;
        if (next(x) == y) return 1;
        if (prev(x) == y) return -1;
        return 0;
    }
};

}  // namespace

Result local_search_move_list(const Instance& inst, const Solution& start) {
    State state(inst, start);
    long long objective = evaluate(inst, start);

    std::vector<Move> lm = state.evaluate_all();
    std::vector<bool> removed(lm.size(), false);
    size_t head = 0;     // all moves before head are removed
    bool fresh = true;   // lm was just evaluated from scratch

    while (true) {
        // Browse LM from the best move until an applicable one is found.
        bool applied = false;
        for (size_t idx = head; idx < lm.size(); ++idx) {
            if (removed[idx]) continue;
            const Status status = state.check(lm[idx]);
            if (status == Status::Skip) continue;  // keep it, it may be applicable later
            removed[idx] = true;
            if (status == Status::Apply) {
                state.apply(lm[idx]);
                objective += lm[idx].delta;
                applied = true;
                break;
            }
        }
        while (head < lm.size() && removed[head]) ++head;

        if (applied) {
            fresh = false;
            continue;
        }
        // No applicable move in LM. If LM was just evaluated from scratch, no
        // applicable improving move exists: local optimum.
        if (fresh) break;
        lm = state.evaluate_all();
        removed.assign(lm.size(), false);
        head = 0;
        fresh = true;
    }

    return {state.solution(), objective};
}
