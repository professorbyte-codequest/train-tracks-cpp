// Expanded unit tests for the Point class
#include <gtest/gtest.h>
#include "Point.h"
#include <sstream>
#include <unordered_set>

using namespace TrainTracks;

TEST(PointTest, DefaultConstructor) {
    Point p;
    EXPECT_EQ(p.x, 0);
    EXPECT_EQ(p.y, 0);
}

TEST(PointTest, ParameterizedConstructor) {
    Point p(3, -5);
    EXPECT_EQ(p.x, 3);
    EXPECT_EQ(p.y, -5);
}

TEST(PointTest, CopyAndMoveConstructors) {
    Point original(7, 8);
    Point copy(original);
    EXPECT_EQ(copy, original);

    Point moved(std::move(original));
    EXPECT_EQ(moved.x, 7);
    EXPECT_EQ(moved.y, 8);
    // original remains unchanged for trivial move
    EXPECT_EQ(original.x, 7);
    EXPECT_EQ(original.y, 8);
}

TEST(PointTest, AssignmentOperators) {
    Point a(1, 2);
    Point b;
    b = a;
    EXPECT_EQ(b, a);

    Point c;
    c = std::move(a);
    EXPECT_EQ(c.x, 1);
    EXPECT_EQ(c.y, 2);
}

TEST(PointTest, CompoundArithmeticOperators) {
    Point p(2, 3);
    p += Point(1, -1);
    EXPECT_EQ(p, Point(3, 2));

    p -= Point(1, 1);
    EXPECT_EQ(p, Point(2, 1));

    p *= 3;
    EXPECT_EQ(p, Point(6, 3));

    p /= 3;
    EXPECT_EQ(p, Point(2, 1));
}

TEST(PointTest, BinaryArithmeticOperators) {
    Point p1(4, 5);
    Point p2(1, 2);
    EXPECT_EQ(p1 + p2, Point(5, 7));
    EXPECT_EQ(p1 - p2, Point(3, 3));
    EXPECT_EQ(p2 * 3, Point(3, 6));
    EXPECT_EQ(p1 / 2, Point(2, 2));
}

TEST(PointTest, RelationalOperatorsAndCmp) {
    Point a(1, 2);
    Point b(2, 1);
    EXPECT_TRUE(a < b);
    EXPECT_TRUE(a <= b);
    EXPECT_FALSE(a > b);
    EXPECT_FALSE(a >= b);
    EXPECT_EQ(cmp(a, b), -1);
    EXPECT_EQ(cmp(b, a), 1);
    EXPECT_EQ(cmp(a, a), 0);
}

TEST(PointTest, EqualityOperators) {
    Point p1(0, 0);
    Point p2(0, 0);
    Point p3(1, 1);
    EXPECT_TRUE(p1 == p2);
    EXPECT_FALSE(p1 != p2);
    EXPECT_TRUE(p1 != p3);
    EXPECT_FALSE(p1 == p3);
}

TEST(PointTest, StreamOperator) {
    Point p(3, 4);
    std::ostringstream oss;
    oss << p;
    EXPECT_EQ(oss.str(), std::string("{3,4}"));
}

TEST(PointTest, SignFunction) {
    EXPECT_EQ(Point(5, -7).sgn(), Point(1, -1));
    EXPECT_EQ(Point(0, 0).sgn(), Point(0, 0));
    EXPECT_EQ(Point(-3, 0).sgn(), Point(-1, 0));
}

TEST(PointTest, AbsAndManhattan) {
    Point p(-3, 4);
    EXPECT_EQ(p.abs(), 7);
    EXPECT_EQ(p.manhattan(Point(0, 0)), 7);
    EXPECT_EQ(Point(1, 1).manhattan(Point(4, 6)), 8);
}

TEST(PointTest, Transpose) {
    Point p(2, 3);
    EXPECT_EQ(p.transpose(10), 3 * 10 + 2);
}

TEST(PointTest, Inverse) {
    Point p(3, -4);
    EXPECT_EQ(p.inverse(), Point(-3, 4));
}

TEST(PointTest, StaticConstructors) {
    EXPECT_EQ(Point::right(), Point(1, 0));
    EXPECT_EQ(Point::up(), Point(0, -1));
    EXPECT_EQ(Point::left(), Point(-1, 0));
    EXPECT_EQ(Point::down(), Point(0, 1));
    EXPECT_EQ(Point::origin(), Point(0, 0));
}

TEST(PointTest, HashFunction) {
    Point p1(5, 6);
    Point p2(5, 6);
    Point p3(6, 5);
    PointHasher hasher;
    EXPECT_EQ(hasher(p1), hasher(p2));
    EXPECT_NE(hasher(p1), hasher(p3));
    // Ensure usage in unordered_set
    std::unordered_set<Point, PointHasher> s;
    s.insert(p1);
    EXPECT_TRUE(s.find(p2) != s.end());
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
