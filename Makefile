CXX      ?= c++
CXXFLAGS ?= -std=c++17 -O2 -Wall -Wextra -pedantic

COMMON_SRC = common/instance.cpp common/solution.cpp
COMMON_HDR = common/instance.hpp common/solution.hpp

.PHONY: all task_1 run_task_1 plot_task_1 task_2 run_task_2 plot_task_2 task_3 run_task_3 plot_task_3 clean

all: task_1 task_2 task_3

task_1: build/task_1

build/task_1: task_1/main.cpp task_1/heuristics.cpp task_1/heuristics.hpp $(COMMON_SRC) $(COMMON_HDR)
	@mkdir -p build
	$(CXX) $(CXXFLAGS) -o $@ task_1/main.cpp task_1/heuristics.cpp $(COMMON_SRC)

run_task_1: build/task_1
	./build/task_1 results/task_1 data/TSPA.csv data/TSPB.csv

plot_task_1:
	python3 scripts/plot.py results/task_1 data/TSPA.csv data/TSPB.csv

task_2: build/task_2

build/task_2: task_2/main.cpp task_2/regret_heuristics.cpp task_2/regret_heuristics.hpp $(COMMON_SRC) $(COMMON_HDR)
	@mkdir -p build
	$(CXX) $(CXXFLAGS) -o $@ task_2/main.cpp task_2/regret_heuristics.cpp $(COMMON_SRC)

run_task_2: build/task_2
	./build/task_2 results/task_2 data/TSPA.csv data/TSPB.csv

plot_task_2:
	python3 scripts/plot.py results/task_2 data/TSPA.csv data/TSPB.csv

task_3: build/task_3

build/task_3: task_3/main.cpp task_3/local_search.cpp task_3/local_search.hpp task_1/heuristics.cpp task_1/heuristics.hpp $(COMMON_SRC) $(COMMON_HDR)
	@mkdir -p build
	$(CXX) $(CXXFLAGS) -o $@ task_3/main.cpp task_3/local_search.cpp task_1/heuristics.cpp $(COMMON_SRC)

run_task_3: build/task_3
	./build/task_3 results/task_3 data/TSPA.csv data/TSPB.csv

plot_task_3:
	python3 scripts/plot.py results/task_3 data/TSPA.csv data/TSPB.csv

clean:
	rm -rf build
