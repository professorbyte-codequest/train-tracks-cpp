#include <iostream>
#include "Puzzle.h"
#include "PathSolver.h"
#include "Utils.h"
#include "Grid.h"
#include "ConsoleReporter.h"

int main(int argc, char** argv) {
    const auto puzzle = TrainTracks::Puzzle::loadFromFile(argv[1]);
    auto grid = TrainTracks::Grid(puzzle);
    TrainTracks::ConsoleReporter r(grid, 1);

    TrainTracks::PathSolver ps;
    ps.Reporter(&r);

    std::cout << TrainTracks::cls;
    const auto solved = ps.Solve(grid);
    std::cout << (solved ? "Solved" : "Unable to solve") << std::endl;
    
    return 0;
}