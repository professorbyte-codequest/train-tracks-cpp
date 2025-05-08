// Unit tests for the Grid class
#include <gtest/gtest.h>
#include <sstream>
#include "PathSolver.h"
#include "Grid.h"
#include "Puzzle.h"
#include "Piece.h"
#include "Point.h"

using namespace TrainTracks;

static Puzzle makeSimpleSolvablePuzzle() {
    Puzzle p;
    p.data.rowConstraints = {1, 1, 1};
    p.data.colConstraints = {0, 3, 0};
    p.gridWidth = 3;
    p.gridHeight = 3;
    // Two vertical pieces at (1,0) and (1,2) as exits
    p.data.startingGrid.assign(9, Piece::Empty);
    p.data.startingGrid[Point{1, 0}.transpose(3)] = Piece::Vertical;
    p.data.startingGrid[Point{1, 2}.transpose(3)] = Piece::Vertical;
    return p;
}

static Puzzle makeSimpleUnsolvablePuzzle() {
    Puzzle p;
    p.data.rowConstraints = {1, 0, 1};
    p.data.colConstraints = {0, 2, 0};
    p.gridWidth = 3;
    p.gridHeight = 3;
    // Two vertical pieces at (1,0) and (1,2) as exits, but they can't connect
    p.data.startingGrid.assign(9, Piece::Empty);
    p.data.startingGrid[Point{1, 0}.transpose(3)] = Piece::Vertical;
    p.data.startingGrid[Point{1, 2}.transpose(3)] = Piece::Vertical;
    return p;
}

TEST(PathSolverTest, SolvesSimplePuzzle) {
    const auto p = makeSimpleSolvablePuzzle();
    Grid g(p);

    PathSolver ps;
    EXPECT_TRUE(ps.Solve(g));
    // Three steps (Entry, middle, Exit)
    EXPECT_EQ(ps.Steps(), 3);
}

TEST(PathSolverTest, DoesntSolvesimplePuzzle) {
    const auto p = makeSimpleUnsolvablePuzzle();
    Grid g(p);

    PathSolver ps;
    EXPECT_FALSE(ps.Solve(g));
    // Entry, then no where to go
    EXPECT_EQ(ps.Steps(), 2);
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
