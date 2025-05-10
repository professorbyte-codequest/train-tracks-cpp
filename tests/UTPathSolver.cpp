// Unit tests for the Grid class
#include <gtest/gtest.h>
#include <sstream>
#include "PathSolver.h"
#include "Grid.h"
#include "Puzzle.h"
#include "Piece.h"
#include "Point.h"
#include "ConsoleReporter.h"

using namespace TrainTracks;

static Puzzle makeSimpleSolvablePuzzle() {
    Puzzle p;
    p.data.rowConstraints = {1, 1, 1};
    p.data.colConstraints = {0, 3, 0};
    p.gridWidth = 3;
    p.gridHeight = 3;
    // Two vertical pieces at (1,0) and (1,2) as exits
    p.data.startingGrid.assign(9, Piece::Empty);
    p.data.startingGrid[Point{1, 0}.project(3)] = Piece::Vertical;
    p.data.startingGrid[Point{1, 2}.project(3)] = Piece::Vertical;
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
    p.data.startingGrid[Point{1, 0}.project(3)] = Piece::Vertical;
    p.data.startingGrid[Point{1, 2}.project(3)] = Piece::Vertical;
    return p;
}

static Puzzle makeLargerSolvablePuzzle() {
    Puzzle p;
    p.data.rowConstraints = {2, 2, 2, 2, 2, 2, 2, 2, 2,};
    p.data.colConstraints = {1, 2, 2, 2, 2, 2, 2, 2, 2, 1};
    p.gridWidth = p.data.colConstraints.size();
    p.gridHeight = p.data.rowConstraints.size();
    p.data.startingGrid.assign(p.gridWidth * p.gridHeight, Piece::Empty);
    p.data.startingGrid[Point{0, 0}.project(p.gridWidth)] = Piece::Horizontal;
    p.data.startingGrid[Point{p.gridWidth - 1, p.gridHeight - 1}.project(p.gridWidth)] = Piece::Horizontal;
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

TEST(PathSolverTest, SolvesLargerPuzzle)
{
    const auto p = makeLargerSolvablePuzzle();
    Grid g(p);
    
    const int totalSteps = 20;
    const int autoPlaced = g.placed() - g.fixedCount();
    const int expectedSteps = totalSteps - autoPlaced;

    EXPECT_EQ(g.fixedCount(), 2);

    PathSolver ps;
    EXPECT_TRUE(ps.Solve(g));

    std::cout << g;
    EXPECT_EQ(ps.Steps(), expectedSteps);
}

TEST(PathSolverTest, LargeJsonPuzzle) {

    // Arrange: create puzzle from JSON
    Puzzle p;
    p.gridWidth  = 12;
    p.gridHeight = 12;

    p.data.rowConstraints = {
        5, 1, 2, 3, 9, 4, 6, 7, 7, 10, 7, 4
    };
    p.data.colConstraints = {
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
    p.data.startingGrid.reserve(flat.size());
    for (int v : flat) {
        p.data.startingGrid.push_back(static_cast<Piece>(v));
    }

    // Act / Assert: verify that load succeeded
    EXPECT_EQ(p.gridWidth, 12);
    EXPECT_EQ(p.gridHeight, 12);
    EXPECT_EQ(p.data.rowConstraints.size(), 12u);
    EXPECT_EQ(p.data.colConstraints.size(), 12u);
    EXPECT_EQ(p.data.startingGrid.size(), 12u * 12u);

    Grid g(p);
    // We should have 11 fixed pieces
    EXPECT_EQ(g.fixedCount(), 11);
    EXPECT_EQ(g.placed(), 16); // we auto placed 5 pieces
    EXPECT_EQ(g.trackInRowCount(0), 5);
    EXPECT_EQ(g.trackInRowCount(1), 1);
    PathSolver ps;
    //ConsoleReporter r(g, 1000);
    //ps.Reporter(&r);
    //g.displayConstraints(true);
    //EXPECT_TRUE(ps.Solve(g));
    std::cout << g;
    //EXPECT_EQ(ps.Steps(), 32);

    const std::string solution = R"( ┌───┘      
 │          
┌┘          
│    ┌┐     
└┐   │└────┐
 │   │    ┌┘
 │   │ ┌──┘ 
 │┌──┘┌┘    
 └┘   │ ┌──┐
┌─────┘┌┘  │
└───┐  │   └
    └──┘    )";

    g.displayConstraints(false);
    std::stringstream ss;
    ss << g;
    std::string grid_string = ss.str();
    EXPECT_EQ(grid_string, solution);
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
