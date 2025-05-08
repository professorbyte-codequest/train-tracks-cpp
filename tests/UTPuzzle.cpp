// Expanded unit tests for the Puzzle class
#include <gtest/gtest.h>
#include <fstream>
#include <cstdio>
#include "Puzzle.h"

using namespace TrainTracks;

TEST(PuzzleTest, DefaultConstructor) {
    Puzzle p;
    EXPECT_EQ(p.gridWidth, 0);
    EXPECT_EQ(p.gridHeight, 0);
    EXPECT_TRUE(p.data.rowConstraints.empty());
    EXPECT_TRUE(p.data.colConstraints.empty());
    EXPECT_TRUE(p.data.startingGrid.empty());
}

TEST(PuzzleTest, CopyAndMoveConstructors) {
    Puzzle original;
    original.gridWidth = 4;
    original.gridHeight = 5;
    original.data.rowConstraints = {1, 2, 3, 4};
    original.data.colConstraints = {5, 6, 7, 8, 9};
    original.data.startingGrid = std::vector<Piece>(original.gridWidth * original.gridHeight);

    Puzzle copy(original);
    EXPECT_EQ(copy.gridWidth, original.gridWidth);
    EXPECT_EQ(copy.gridHeight, original.gridHeight);
    EXPECT_EQ(copy.data.rowConstraints, original.data.rowConstraints);
    EXPECT_EQ(copy.data.colConstraints, original.data.colConstraints);
    EXPECT_EQ(copy.data.startingGrid.size(), original.data.startingGrid.size());

    Puzzle moved(std::move(original));
    EXPECT_EQ(moved.gridWidth, 4);
    EXPECT_EQ(moved.gridHeight, 5);
    EXPECT_EQ(moved.data.rowConstraints, std::vector<int>({1, 2, 3, 4}));
    EXPECT_EQ(moved.data.colConstraints, std::vector<int>({5, 6, 7, 8, 9}));
    EXPECT_EQ(moved.data.startingGrid.size(), 20);
}

TEST(PuzzleTest, AssignmentOperators) {
    Puzzle a;
    a.gridWidth = 2;
    a.gridHeight = 2;
    a.data.rowConstraints = {1, 2};
    a.data.colConstraints = {3, 4};
    a.data.startingGrid = std::vector<Piece>(4);

    Puzzle b;
    b = a;
    EXPECT_EQ(b.gridWidth, a.gridWidth);
    EXPECT_EQ(b.gridHeight, a.gridHeight);
    EXPECT_EQ(b.data.rowConstraints, a.data.rowConstraints);
    EXPECT_EQ(b.data.colConstraints, a.data.colConstraints);
    EXPECT_EQ(b.data.startingGrid.size(), a.data.startingGrid.size());

    Puzzle c;
    c = std::move(a);
    EXPECT_EQ(c.gridWidth, 2);
    EXPECT_EQ(c.gridHeight, 2);
    EXPECT_EQ(c.data.rowConstraints, std::vector<int>({1, 2}));
    EXPECT_EQ(c.data.colConstraints, std::vector<int>({3, 4}));
    EXPECT_EQ(c.data.startingGrid.size(), 4);
}

TEST(PuzzleTest, ToString) {
    Puzzle p;
    p.gridWidth = 3;
    p.gridHeight = 6;
    std::string s = p.toString();
    EXPECT_EQ(s, "GridWidth: 3\nGridHeight: 6");
}

TEST(PuzzleTest, LoadFromFileValid) {
    const std::string filename = "testPuzzle.txt";
    std::ofstream ofs(filename);
    ofs << "# sample puzzle\n";
    ofs << "ROWS: 10 20 30\n";
    ofs << "COLS: 1 2\n";
    ofs.close();

    Puzzle p = Puzzle::loadFromFile(filename);
    EXPECT_EQ(p.data.rowConstraints, std::vector<int>({10, 20, 30}));
    EXPECT_EQ(p.data.colConstraints, std::vector<int>({1, 2}));
    EXPECT_EQ(p.gridWidth, 3);
    EXPECT_EQ(p.gridHeight, 2);
    EXPECT_EQ(p.data.startingGrid.size(), 6);

    std::remove(filename.c_str());
}

TEST(PuzzleTest, LoadFromFileMissingRows) {
    const std::string filename = "noRows.txt";
    std::ofstream ofs(filename);
    ofs << "# missing rows\n";
    ofs << "COLS: 1 2 3\n";
    ofs.close();

    EXPECT_THROW(Puzzle::loadFromFile(filename), std::runtime_error);
    std::remove(filename.c_str());
}

TEST(PuzzleTest, LoadFromFileMissingCols) {
    const std::string filename = "noCols.txt";
    std::ofstream ofs(filename);
    ofs << "# missing cols\n";
    ofs << "ROWS: 4 5\n";
    ofs.close();

    EXPECT_THROW(Puzzle::loadFromFile(filename), std::runtime_error);
    std::remove(filename.c_str());
}

TEST(PuzzleTest, LoadFromFileNonexistent) {
    EXPECT_THROW(Puzzle::loadFromFile("nonexistent_file.txt"), std::runtime_error);
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
