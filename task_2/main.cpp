// Assignment 2: greedy heuristics.
// Usage: task_2 [output_dir] instance1.csv [instance2.csv ...]
//   e.g. ./build/task_2 results/task_2 data/TSPA.csv data/TSPB.csv

#include <algorithm>
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
#include "regret_heuristics.hpp"

namespace fs = std::filesystem;

constexpr int RUNS = 200;

struct Method {
    std::string name;
    // run(instance, run index) -> result. For greedy methods the run index is the start node.
    std::function<Result(const Instance&, int)> run;
};

int main(int argc, char** argv) {
    if (argc < 3) {
        std::cerr << "Usage: " << argv[0] << " output_dir instance.csv [instance.csv ...]\n";
        return 1;
    }
    const fs::path out_dir = argv[1];
    fs::create_directories(out_dir / "best");

    const std::vector<Method> methods = {
        {"regret_2_nn",
         [](const Instance& inst, int run) { return regret_2_nn_any(inst, run % inst.n, 1.0); }},
        {"regret_2_greedy_cycle",
         [](const Instance& inst, int run) { return regret_2_greedy_cycle(inst, run % inst.n, 1.0); }},

        {"regret_2_nn_w01",
         [](const Instance& inst, int run) { return regret_2_nn_any(inst, run % inst.n, 0.1); }},
        {"regret_2_greedy_cycle_w01",
         [](const Instance& inst, int run) { return regret_2_greedy_cycle(inst, run % inst.n, 0.1); }},

        {"regret_2_nn_w02",
         [](const Instance& inst, int run) { return regret_2_nn_any(inst, run % inst.n, 0.2); }},
        {"regret_2_greedy_cycle_w02",
         [](const Instance& inst, int run) { return regret_2_greedy_cycle(inst, run % inst.n, 0.2); }},

        {"regret_2_nn_w03",
         [](const Instance& inst, int run) { return regret_2_nn_any(inst, run % inst.n, 0.3); }},
        {"regret_2_greedy_cycle_w03",
         [](const Instance& inst, int run) { return regret_2_greedy_cycle(inst, run % inst.n, 0.3); }},

        {"regret_2_nn_w04",
         [](const Instance& inst, int run) { return regret_2_nn_any(inst, run % inst.n, 0.4); }},
        {"regret_2_greedy_cycle_w04",
         [](const Instance& inst, int run) { return regret_2_greedy_cycle(inst, run % inst.n, 0.4); }},

        {"regret_2_nn_w05",
         [](const Instance& inst, int run) { return regret_2_nn_any(inst, run % inst.n, 0.5); }},
        {"regret_2_greedy_cycle_w05",
         [](const Instance& inst, int run) { return regret_2_greedy_cycle(inst, run % inst.n, 0.5); }},

        {"regret_2_nn_w06",
         [](const Instance& inst, int run) { return regret_2_nn_any(inst, run % inst.n, 0.6); }},
        {"regret_2_greedy_cycle_w06",
         [](const Instance& inst, int run) { return regret_2_greedy_cycle(inst, run % inst.n, 0.6); }},

        {"regret_2_nn_w07",
         [](const Instance& inst, int run) { return regret_2_nn_any(inst, run % inst.n, 0.7); }},
        {"regret_2_greedy_cycle_w07",
         [](const Instance& inst, int run) { return regret_2_greedy_cycle(inst, run % inst.n, 0.7); }},

        {"regret_2_nn_w08",
         [](const Instance& inst, int run) { return regret_2_nn_any(inst, run % inst.n, 0.8); }},
        {"regret_2_greedy_cycle_w08",
         [](const Instance& inst, int run) { return regret_2_greedy_cycle(inst, run % inst.n, 0.8); }},

        {"regret_2_nn_w09",
         [](const Instance& inst, int run) { return regret_2_nn_any(inst, run % inst.n, 0.9); }},   
        {"regret_2_greedy_cycle_w09",
         [](const Instance& inst, int run) { return regret_2_greedy_cycle(inst, run % inst.n, 0.9); }},
    };

    std::ofstream summary(out_dir / "summary.csv");
    summary << "instance,method,min,max,avg\n";
    std::ofstream all(out_dir / "all_objectives.csv");
    all << "instance,method,run,objective\n";

    std::printf("%-8s %-20s %10s %10s %12s\n", "instance", "method", "min", "max", "avg");

    for (int a = 2; a < argc; ++a) {
        const Instance inst = load_instance(argv[a]);

        for (const auto& method : methods) {
            long long min_obj = LLONG_MAX, max_obj = LLONG_MIN, sum = 0;
            Solution best;

            for (int run = 0; run < RUNS; ++run) {
                const Result r = method.run(inst, run);

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

                all << inst.name << "," << method.name << "," << run << "," << obj << "\n";
                sum += obj;
                max_obj = std::max(max_obj, obj);
                if (obj < min_obj) {
                    min_obj = obj;
                    best = r.tour;
                }
            }

            const double avg = static_cast<double>(sum) / RUNS;
            summary << inst.name << "," << method.name << "," << min_obj << "," << max_obj << ","
                    << avg << "\n";
            std::printf("%-8s %-30s %10lld %10lld %12.2f\n", inst.name.c_str(),
                        method.name.c_str(), min_obj, max_obj, avg);

            std::ofstream best_file(out_dir / "best" / (inst.name + "_" + method.name + ".txt"));
            for (size_t i = 0; i < best.size(); ++i) best_file << (i ? "," : "") << best[i];
            best_file << "\n";
        }
    }
    return 0;
}