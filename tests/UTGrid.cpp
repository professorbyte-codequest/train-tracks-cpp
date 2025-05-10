// Unit tests for the Grid class
#include <gtest/gtest.h>
#include <sstream>
#include "Grid.h"
#include "Puzzle.h"
#include "Piece.h"
#include "Point.h"
#include "PathSolver.h"

using namespace TrainTracks;

// Helper to build a 3x3 puzzle with two fixed exits
static Puzzle makeSimplePuzzle() {
    Puzzle p;
    p.data.rowConstraints = {1, 1, 1};
    p.data.colConstraints = {1, 1, 1};
    p.gridWidth = 3;
    p.gridHeight = 3;
    // Two vertical pieces at (1,0) and (1,2) as exits
    p.data.startingGrid.assign(9, Piece::Empty);
    p.data.startingGrid[Point{1, 0}.project(3)] = Piece::Vertical;
    p.data.startingGrid[Point{1, 2}.project(3)] = Piece::Vertical;
    return p;
}

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

/*
  -+...
  .|...
  .+-+.
  ...++
  ....+
*/
static Puzzle makeSimplePuzzle5x5() {
    Puzzle p;
    p.data.rowConstraints = {2, 1, 3, 2, 1};
    p.data.colConstraints = {1, 3, 1, 2, 2};
    p.gridWidth = 5;
    p.gridHeight = 5;
    p.data.startingGrid.assign(25, Piece::Empty);
    p.data.startingGrid[Point{0, 0}.project(5)] = Piece::Horizontal;
    p.data.startingGrid[Point{4, 4}.project(5)] = Piece::CornerNE;
    return p;
}

TEST(GridTest, ConstructorThrowsOnNoExits) {
    // Puzzle with no fixed pieces should throw due to missing exits
    Puzzle p;
    p.data.rowConstraints = {0, 0};
    p.data.colConstraints = {0, 0};
    p.gridWidth = 2;
    p.gridHeight = 2;
    p.data.startingGrid.assign(4, Piece::Empty);
    EXPECT_THROW(Grid g(p), std::runtime_error);
}

TEST(GridTest, ValidConstructionProperties) {
    Puzzle p = makeSimplePuzzle();
    Grid g(p);
    EXPECT_EQ(g.width(), 3);
    EXPECT_EQ(g.height(), 3);
    // Two fixed placed, target sum = 3
    EXPECT_EQ(g.placed(), 2);
    EXPECT_EQ(g.target(), 3);
    auto fixed = g.fixedPoints();
    EXPECT_EQ(fixed.size(), 2);
    EXPECT_NE(std::find(fixed.begin(), fixed.end(), Point{1, 0}), fixed.end());
    EXPECT_NE(std::find(fixed.begin(), fixed.end(), Point{1, 2}), fixed.end());
    // Entry/exit
    EXPECT_EQ(g.entry(), Point(1, 0));
    EXPECT_EQ(g.exit(),  Point(1, 2));
}

TEST(GridTest, BasicQueries) {
    Puzzle p = makeSimplePuzzle();
    Grid g(p);
    EXPECT_TRUE(g.isInBounds(Point{0, 0}));
    EXPECT_FALSE(g.isInBounds(Point{-1, 1}));
    EXPECT_TRUE(g.isOnEdge(Point{2, 1}));
    EXPECT_FALSE(g.isOnEdge(Point{1, 1}));
    EXPECT_TRUE(g.isEmpty(Point{0, 1}));
    EXPECT_FALSE(g.isFilled(Point{0, 1}));
    EXPECT_TRUE(g.isFilled(Point{1, 0}));
    EXPECT_FALSE(g.isEmpty(Point{1, 0}));
}

TEST(GridTest, PlaceAndRemoveUpdatesCounts) {
    Puzzle p = makeSimplePuzzle();
    Grid g(p);
    Point pt{0, 1};
    g.place(pt, Piece::CornerNE);
    EXPECT_EQ(g.placed(), 3);
    EXPECT_EQ(g.trackInRowCount(1), 1);
    EXPECT_EQ(g.trackInColCount(0), 1);
    EXPECT_EQ(g.at(pt), Piece::CornerNE);
    g.remove(pt);
    EXPECT_EQ(g.placed(), 2);
    EXPECT_EQ(g.trackInRowCount(1), 0);
    EXPECT_TRUE(g.isEmpty(pt));
}

TEST(GridTest, CanPlaceConditions) {
    Puzzle p = makeSimplePuzzle();
    Grid g(p);
    // Out of bounds
    EXPECT_FALSE(g.canPlace(Point{-1, 0}, Piece::Horizontal));
    // Already filled
    EXPECT_FALSE(g.canPlace(Point{1, 0}, Piece::Vertical));
    // Valid empty but wrong piece orientation at edge
    EXPECT_FALSE(g.canPlace(Point{0, 1}, Piece::Horizontal));
    // Empty and valid for an empty piece (no connectivity checks)
    EXPECT_TRUE(g.canPlace(Point{0, 1}, Piece::Empty));
}

TEST(GridTest, ConstraintsSatisfiedAndCanStillSatisfy) {
    Puzzle p = makeSimpleSolvablePuzzle();
    Grid g(p);
    EXPECT_FALSE(g.constraintsSatisfied());
    EXPECT_TRUE(g.canStillSatisfy());
    // Fill remaining slot at row 1, col 1
    g.place(Point{1, 1}, Piece::Vertical);
    EXPECT_TRUE(g.constraintsSatisfied());
    EXPECT_TRUE(g.canStillSatisfy());
    EXPECT_TRUE(g.isComplete());
}

TEST(GridTest, ToStringAndOstream) {
    Puzzle p = makeSimplePuzzle();
    Grid g(p);
    std::string s = g.toString();
    // should contain three lines
    size_t lines = std::count(s.begin(), s.end(), '\n');
    EXPECT_EQ(lines, 3);
    std::ostringstream oss;
    // without constraints
    oss << g;
    std::string out = oss.str();
    EXPECT_NE(out.find("│"), std::string::npos);
    // with constraints
    g.displayConstraints(true);
    oss.str(""); oss.clear();
    oss << g;
    out = oss.str();
    EXPECT_NE(out.find('0'), std::string::npos);
}

TEST(GridTest, PlaceObviousPieces) {
    Puzzle p = makeSimplePuzzle5x5();
    Grid g(p);

    // The obvious pieces should be placed
    // Two fixed, plus 2 obvious pieces
    EXPECT_EQ(g.fixedCount(), 2);
    EXPECT_EQ(g.placed(), 6);
    EXPECT_EQ(g.trackInRowCount(0), 2);
    EXPECT_EQ(g.trackInRowCount(3), 1);
    EXPECT_EQ(g.trackInColCount(1), 3);
    EXPECT_EQ(g.trackInColCount(4), 2);
    EXPECT_EQ(g.at(Point{1, 0}), Piece::CornerSW);
    EXPECT_EQ(g.at(Point{1, 1}), Piece::Vertical);
    EXPECT_EQ(g.at(Point{1, 2}), Piece::CornerNE);
    EXPECT_EQ(g.at(Point{4, 3}), Piece::CornerSW);

    // How we should be able to solve it
    PathSolver s;
    EXPECT_TRUE(s.Solve(g));
    EXPECT_TRUE(g.isComplete());

    std::cout << g;
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
