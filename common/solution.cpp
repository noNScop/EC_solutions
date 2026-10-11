#include "solution.hpp"

long long evaluate(const Instance& inst, const Solution& sol) {
    long long total = 0;
    const int m = static_cast<int>(sol.size());
    for (int i = 0; i < m; ++i) {
        total += inst.cost[sol[i]];
        total += inst.dist[sol[i]][sol[(i + 1) % m]];
    }
    return total;
}

bool validate(const Instance& inst, const Solution& sol, std::string* error) {
    auto fail = [&](const std::string& msg) {
        if (error) *error = msg;
        return false;
    };

    if (static_cast<int>(sol.size()) != inst.k())
        return fail("expected " + std::to_string(inst.k()) + " nodes, got " +
                    std::to_string(sol.size()));

    std::vector<char> seen(inst.n, 0);
    for (int v : sol) {
        if (v < 0 || v >= inst.n) return fail("node index out of range: " + std::to_string(v));
        if (seen[v]) return fail("duplicate node: " + std::to_string(v));
        seen[v] = 1;
    }
    return true;
}
