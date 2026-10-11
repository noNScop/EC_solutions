// Assignment 1: greedy heuristics.
// Usage: task_1 [output_dir] instance1.csv [instance2.csv ...]
//   e.g. ./build/task_1 results/task_1 data/TSPA.csv data/TSPB.csv

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
#include "heuristics.hpp"

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
        {"random",
         [](const Instance& inst, int run) {
             std::mt19937 rng(run);  // seed = run index, for reproducibility
             return random_solution(inst, rng);
         }},
        {"nn_end", [](const Instance& inst, int run) { return nn_end(inst, run % inst.n); }},
        {"nn_any", [](const Instance& inst, int run) { return nn_any(inst, run % inst.n); }},
        {"greedy_cycle",
         [](const Instance& inst, int run) { return greedy_cycle(inst, run % inst.n); }},
    };

    std::ofstream summary(out_dir / "summary.csv");
    summary << "instance,method,min,max,avg\n";
    std::ofstream all(out_dir / "all_objectives.csv");
    all << "instance,method,run,objective\n";

    std::printf("%-8s %-14s %10s %10s %12s\n", "instance", "method", "min", "max", "avg");

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
            std::printf("%-8s %-14s %10lld %10lld %12.2f\n", inst.name.c_str(),
                        method.name.c_str(), min_obj, max_obj, avg);

            std::ofstream best_file(out_dir / "best" / (inst.name + "_" + method.name + ".txt"));
            for (size_t i = 0; i < best.size(); ++i) best_file << (i ? "," : "") << best[i];
            best_file << "\n";
        }
    }
    return 0;
}
