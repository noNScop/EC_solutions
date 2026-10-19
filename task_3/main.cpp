// Assignment 3: local search (+ timed re-run of the previous methods).
// Usage: task_3 output_dir instance1.csv [instance2.csv ...]
//   e.g. ./build/task_3 results/task_3 data/TSPA.csv data/TSPB.csv

#include <algorithm>
#include <chrono>
#include <climits>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iostream>
#include <random>
#include <string>
#include <vector>

#include "../common/instance.hpp"
#include "../common/solution.hpp"
#include "../task_1/heuristics.hpp"
#include "local_search.hpp"

namespace fs = std::filesystem;

constexpr int RUNS = 200;

struct Method {
    std::string name;
    // run(instance, run index) -> result. The run index is the seed for random
    // solutions and the starting node for greedy heuristics.
    std::function<Result(const Instance&, int)> run;
};

struct Stats {
    std::string instance, method;
    long long obj_min, obj_max;
    double obj_avg, time_min, time_max, time_avg;  // times in ms
};

// Starting solution for "greedy start" local search.
// TODO: replace greedy_cycle with the best heuristic from Assignment 2 (regret).
Solution greedy_start(const Instance& inst, int run) {
    return greedy_cycle(inst, run % inst.n).tour;
}

std::vector<Method> build_methods() {
    std::vector<Method> methods = {
        {"random",
         [](const Instance& inst, int run) {
             std::mt19937 rng(run);
             return random_solution(inst, rng);
         }},
        {"nn_end", [](const Instance& inst, int run) { return nn_end(inst, run % inst.n); }},
        {"nn_any", [](const Instance& inst, int run) { return nn_any(inst, run % inst.n); }},
        {"greedy_cycle",
         [](const Instance& inst, int run) { return greedy_cycle(inst, run % inst.n); }},
    };

    // The 8 local search variants: search type x intra-route move x starting solution.
    for (SearchType search : {SearchType::Steepest, SearchType::Greedy}) {
        for (IntraMove intra : {IntraMove::NodesExchange, IntraMove::EdgesExchange}) {
            for (bool random_start : {true, false}) {
                std::string name = std::string("ls_") +
                                   (search == SearchType::Steepest ? "steepest" : "greedy") +
                                   (intra == IntraMove::NodesExchange ? "_nodes" : "_edges") +
                                   (random_start ? "_random" : "_greedy");
                methods.push_back(
                    {name, [=](const Instance& inst, int run) {
                         std::mt19937 rng(run);  // seed = run index, for reproducibility
                         Solution start = random_start ? random_solution(inst, rng).tour
                                                       : greedy_start(inst, run);
                         return local_search(inst, start, search, intra, rng);
                     }});
            }
        }
    }
    return methods;
}

// Writes a table in the report format: rows = methods, columns = "av (min – max)" per instance.
void write_table(std::ostream& out, const std::vector<Stats>& stats,
                 const std::vector<std::string>& instances, bool times) {
    out << "| Method |";
    for (const auto& name : instances) out << " " << name << " |";
    out << "\n|---|";
    for (size_t i = 0; i < instances.size(); ++i) out << "---|";
    out << "\n";

    std::vector<std::string> methods;
    for (const auto& s : stats)
        if (std::find(methods.begin(), methods.end(), s.method) == methods.end())
            methods.push_back(s.method);

    char cell[128];
    for (const auto& method : methods) {
        out << "| " << method << " |";
        for (const auto& name : instances) {
            for (const auto& s : stats) {
                if (s.instance != name || s.method != method) continue;
                if (times)
                    std::snprintf(cell, sizeof cell, "%.3f (%.3f – %.3f)", s.time_avg, s.time_min,
                                  s.time_max);
                else
                    std::snprintf(cell, sizeof cell, "%.2f (%lld – %lld)", s.obj_avg, s.obj_min,
                                  s.obj_max);
                out << " " << cell << " |";
            }
        }
        out << "\n";
    }
}

int main(int argc, char** argv) {
    if (argc < 3) {
        std::cerr << "Usage: " << argv[0] << " output_dir instance.csv [instance.csv ...]\n";
        return 1;
    }
    const fs::path out_dir = argv[1];
    fs::create_directories(out_dir / "best");

    const std::vector<Method> methods = build_methods();

    std::ofstream all(out_dir / "all_runs.csv");
    all << "instance,method,run,objective,time_ms\n";

    std::vector<Stats> stats;
    std::vector<std::string> instances;

    for (int a = 2; a < argc; ++a) {
        const Instance inst = load_instance(argv[a]);
        instances.push_back(inst.name);

        for (const auto& method : methods) {
            Stats s{inst.name, method.name, LLONG_MAX, LLONG_MIN, 0, 1e300, 0, 0};
            Solution best;

            for (int run = 0; run < RUNS; ++run) {
                const auto t0 = std::chrono::steady_clock::now();
                const Result r = method.run(inst, run);
                const auto t1 = std::chrono::steady_clock::now();
                const double ms = std::chrono::duration<double, std::milli>(t1 - t0).count();

                std::string error;
                if (!validate(inst, r.tour, &error)) {
                    std::cerr << "Invalid solution: " << inst.name << " " << method.name
                              << " run " << run << ": " << error << "\n";
                    return 1;
                }
                const long long obj = evaluate(inst, r.tour);
                if (obj != r.objective) {
                    std::cerr << "Objective mismatch: " << inst.name << " " << method.name
                              << " run " << run << ": tracked " << r.objective
                              << ", evaluated " << obj << "\n";
                    return 1;
                }

                all << inst.name << "," << method.name << "," << run << "," << obj << "," << ms
                    << "\n";
                s.obj_avg += obj;
                s.time_avg += ms;
                s.obj_max = std::max(s.obj_max, obj);
                s.time_min = std::min(s.time_min, ms);
                s.time_max = std::max(s.time_max, ms);
                if (obj < s.obj_min) {
                    s.obj_min = obj;
                    best = r.tour;
                }
            }
            s.obj_avg /= RUNS;
            s.time_avg /= RUNS;
            stats.push_back(s);
            std::printf("%-6s %-26s obj %10.2f (%lld - %lld)   time %9.3f ms\n",
                        inst.name.c_str(), method.name.c_str(), s.obj_avg, s.obj_min, s.obj_max,
                        s.time_avg);
            std::fflush(stdout);

            std::ofstream best_file(out_dir / "best" / (inst.name + "_" + method.name + ".txt"));
            for (size_t i = 0; i < best.size(); ++i) best_file << (i ? "," : "") << best[i];
            best_file << "\n";
        }
    }

    std::ofstream summary(out_dir / "summary.csv");
    summary << "instance,method,obj_min,obj_max,obj_avg,time_min_ms,time_max_ms,time_avg_ms\n";
    for (const auto& s : stats)
        summary << s.instance << "," << s.method << "," << s.obj_min << "," << s.obj_max << ","
                << s.obj_avg << "," << s.time_min << "," << s.time_max << "," << s.time_avg
                << "\n";

    std::ofstream tables(out_dir / "tables.md");
    tables << "## Objective function: av (min – max)\n\n";
    write_table(tables, stats, instances, false);
    tables << "\n## Running time [ms]: av (min – max)\n\n";
    write_table(tables, stats, instances, true);
    return 0;
}
