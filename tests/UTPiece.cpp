// Expanded unit tests for the Piece class
#include <gtest/gtest.h>
#include "Piece.h"
#include <stdexcept>
#include <array>

using namespace TrainTracks;

TEST(PieceTest, EnumUnderlyingValues) {
    EXPECT_EQ(static_cast<int>(Piece::Empty), 0);
    EXPECT_EQ(static_cast<int>(Piece::Horizontal), 3);
    EXPECT_EQ(static_cast<int>(Piece::Vertical), 4);
    EXPECT_EQ(static_cast<int>(Piece::CornerNE), 5);
    EXPECT_EQ(static_cast<int>(Piece::CornerSE), 6);
    EXPECT_EQ(static_cast<int>(Piece::CornerSW), 7);
    EXPECT_EQ(static_cast<int>(Piece::CornerNW), 8);
}

TEST(PieceTest, PieceSymbol) {
    EXPECT_STREQ(PieceSymbol(Piece::Empty), " ");
    EXPECT_STREQ(PieceSymbol(Piece::Horizontal), "─");
    EXPECT_STREQ(PieceSymbol(Piece::Vertical), "│");
    EXPECT_STREQ(PieceSymbol(Piece::CornerNE), "└");
    EXPECT_STREQ(PieceSymbol(Piece::CornerSE), "┌");
    EXPECT_STREQ(PieceSymbol(Piece::CornerSW), "┐");
    EXPECT_STREQ(PieceSymbol(Piece::CornerNW), "┘");
}

TEST(PieceTest, FromStringValid) {
    EXPECT_EQ(fromString("Horizontal"), Piece::Horizontal);
    EXPECT_EQ(fromString("Vertical"), Piece::Vertical);
    EXPECT_EQ(fromString("CornerNE"), Piece::CornerNE);
    EXPECT_EQ(fromString("CornerNW"), Piece::CornerNW);
    EXPECT_EQ(fromString("CornerSE"), Piece::CornerSE);
    EXPECT_EQ(fromString("CornerSW"), Piece::CornerSW);
}

TEST(PieceTest, FromStringInvalidThrows) {
    EXPECT_THROW(fromString(""), std::runtime_error);
    EXPECT_THROW(fromString("InvalidPiece"), std::runtime_error);
    EXPECT_THROW(fromString("horizontal"), std::runtime_error);  // case-sensitive
}

TEST(PieceTest, ValidPiecesArrayContents) {
    std::array<Piece,6> expected = {
        Piece::Horizontal,
        Piece::Vertical,
        Piece::CornerNE,
        Piece::CornerNW,
        Piece::CornerSE,
        Piece::CornerSW
    };
    std::sort(expected.begin(), expected.end());
    EXPECT_EQ(ValidPieces.size(), expected.size());
    EXPECT_EQ(ValidPieces, expected);
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
