#include "instance.hpp"

#include <cmath>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <stdexcept>

Instance load_instance(const std::string& path) {
    std::ifstream in(path);
    if (!in) throw std::runtime_error("Cannot open instance file: " + path);

    std::vector<int> xs, ys;
    Instance inst;
    inst.name = std::filesystem::path(path).stem().string();

    std::string line;
    while (std::getline(in, line)) {
        if (line.empty() || line == "\r") continue;
        std::stringstream ss(line);
        std::string field;
        int values[3];
        for (int i = 0; i < 3; ++i) {
            if (!std::getline(ss, field, ';'))
                throw std::runtime_error("Malformed line in " + path + ": " + line);
            values[i] = std::stoi(field);
        }
        xs.push_back(values[0]);
        ys.push_back(values[1]);
        inst.cost.push_back(values[2]);
    }

    inst.n = static_cast<int>(xs.size());
    inst.dist.assign(inst.n, std::vector<int>(inst.n, 0));
    for (int i = 0; i < inst.n; ++i) {
        for (int j = i + 1; j < inst.n; ++j) {
            double dx = xs[i] - xs[j];
            double dy = ys[i] - ys[j];
            int d = static_cast<int>(std::lround(std::sqrt(dx * dx + dy * dy)));
            inst.dist[i][j] = inst.dist[j][i] = d;
        }
    }
    return inst;
}
