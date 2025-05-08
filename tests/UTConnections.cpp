#include <gtest/gtest.h>

#include <vector>
#include <list>

#include "include/Connections.h"

using namespace TrainTracks;

TEST(ConnectionsTest, GetConnections) {
    std::vector<std::pair<Piece, PointList>> Theories{
        { Piece::Empty, PointList{} },
        { Piece::Horizontal, PointList{ {1, 0}, {-1, 0}}},
        { Piece::Vertical, PointList{ {0, 1}, {0, -1}}},
        { Piece::CornerNE, PointList{ {1, 0}, {0, -1}}},
        { Piece::CornerSE, PointList{ {1, 0}, {0, 1}}},
        { Piece::CornerSW, PointList{ {-1, 0}, {0, 1}}},
        { Piece::CornerNW, PointList{ {-1, 0}, {0, -1}}},
    };

    for (const auto& kv : Theories) {
        const auto it = Connections::GetConnections(kv.first);
        EXPECT_EQ(it.size(), kv.second.size());

        int seen = 0;
        std::for_each(it.cbegin(), it.cend(), [&seen, &kv](const Point& pt) {
            for (const auto p : kv.second) {
                if (pt == p) {
                    seen++;
                    break;
                }
            }
        });
        EXPECT_EQ(seen, it.size()) << (int)kv.first;

        // Validate the reverse lookup is true
        std::for_each(kv.second.cbegin(), kv.second.cend(), [&kv](const Point& pt) {
            EXPECT_TRUE(Connections::ConnectsTo(kv.first, pt)) << (int)kv.first;
        });
    }
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}