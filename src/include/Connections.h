#pragma once

#include "Piece.h"

#include "Point.h"

#include <map>
#include <list>

namespace TrainTracks {

    class Connections {

        public:

            static const PointList& GetConnections(const Piece& p) {
                static const PointList empty{};
                if (p == Piece::Empty) {
                    return empty;
                }

                const auto& it = instance.Directions.find(p);
                if (it == instance.Directions.end()) {
                    return empty;
                }

                return it->second;
            }

            static bool ConnectsTo(Piece p, Point d) {
                for (const auto& c : GetConnections(p)) {
                    if (d == c) {
                        return true;
                    }
                }
                return false;
            }

            static Piece GetPiece(const Point& p1, const Point& pt) {
                for (const auto& p : ValidPieces) {
                    if (ConnectsTo(p, p1) && ConnectsTo(p, pt)) {
                        return p;
                    }
                }
                return Piece::Empty;
            }

        private:
            Connections() {
                Directions.emplace(Piece::Horizontal, PointList{ { -1, 0 }, { 1, 0 } });
                Directions.emplace(Piece::Vertical, PointList{ { 0, 1 }, { 0, -1 } });
                Directions.emplace(Piece::CornerNE, PointList{ { 0, -1 }, { 1, 0 } });
                Directions.emplace(Piece::CornerNW, PointList{ { 0, -1 }, { -1, 0 } });
                Directions.emplace(Piece::CornerSE, PointList{ { 0, 1 }, { 1, 0 } });
                Directions.emplace(Piece::CornerSW, PointList{ { 0, 1 }, { -1, 0 } });
            }
            std::map<Piece, PointList> Directions;

            static Connections instance;
    };
}