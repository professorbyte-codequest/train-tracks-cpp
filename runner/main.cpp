#include <iostream>
#include "Puzzle.h"
#include "PathSolver.h"
#include "Utils.h"
#include "Grid.h"
#include "ConsoleReporter.h"

int main(int argc, char** argv) {
    //const auto puzzle = TrainTracks::Puzzle::loadFromFile(argv[1]);
    
    // Arrange: create puzzle from JSON
    TrainTracks::Puzzle puzzle;
    puzzle.gridWidth  = 12;
    puzzle.gridHeight = 12;

    puzzle.data.rowConstraints = {
        5, 1, 2, 3, 9, 4, 6, 7, 7, 10, 7, 4
    };
    puzzle.data.colConstraints = {
        5, 10, 5, 4, 5, 8, 6, 6, 4, 3, 4, 5
    };

    // startingGrid values map directly to Piece enum underlying ints:
    // 0=Empty, 3=Horizontal, 4=Vertical, 5=CornerNE, 6=CornerSE, 7=CornerSW, 8=CornerNW
    std::vector<int> flat = {
        0, 0, 0, 0, 0, 8, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 7, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 6, 8,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 4, 0, 0, 0, 8, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 3, 0, 0,
        6, 0, 0, 3, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 5,
        0, 0, 0, 0, 5, 0, 0, 0, 0, 0, 0, 0
    };
    // convert to Pieces
    puzzle.data.startingGrid.reserve(flat.size());
    for (int v : flat) {
        puzzle.data.startingGrid.push_back(static_cast<TrainTracks::Piece>(v));
    }
    
    auto grid = TrainTracks::Grid(puzzle);
    TrainTracks::ConsoleReporter r(grid, 1000000);

    grid.displayConstraints(true);

    std::cout << TrainTracks::cls;
    std::cout << "Initial grid:" << std::endl;
    std::cout << grid << std::endl;
    std::cout << "Entry: " << grid.entry() << std::endl;
    std::cout << "Exit: " << grid.exit() << std::endl;
    std::cout << "Total pieces: " << grid.target() << std::endl;
    std::cout << "Placed pieces: " << grid.placed() << std::endl;

    std::cout << "Press Enter to continue..." << std::endl;
    std::cin.get();
    std::cout << TrainTracks::cls;

    grid.displayConstraints(false);

    TrainTracks::PathSolver ps;
    ps.Reporter(&r);

    const auto solved = ps.Solve(grid);
    std::cout << (solved ? "Solved" : "Unable to solve") << std::endl;
    
    return 0;
}