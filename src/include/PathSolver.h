#pragma once

#include "Solver.h"
#include <functional>

namespace TrainTracks
{
    
    class PathSolver
        : public Solver {

    public:
        PathSolver()
            : Solver()
        { }

        bool Solve(Grid& grid) {
            _fixedPoints = grid.fixedPoints();

            const auto& entry = grid.entry();
            std::vector<bool> visited(grid.width() * grid.height());
            
            return TryBuild(grid, entry, getEntryIncoming(grid), visited, 0, 0);
        }

    protected:
        Point getEntryIncoming(const Grid& grid) const {
            const auto entry = grid.entry();
            for (const auto& d : Connections::GetConnections(grid.at(entry))) {
                const auto n = entry + d;
                if (!grid.isInBounds(n)) {
                    return d.inverse();
                }
            }
            throw std::runtime_error("Invalid entry, no incoming direction!");
        }
        bool TryBuild(Grid& grid, const Point& pos, const Point& incoming, std::vector<bool>& visited, int hit, int visited_count) {
            Step();

            // Bounds and revisit check
            if (!grid.isInBounds(pos) || visited[pos.transpose(grid.width())]) {
                return false;
            }

            // Can't exceed total count
            if (visited_count >= grid.target()) {
                return false;
            }

            // Check existing piece
            const auto existing = grid.at(pos);
            // if its a fixed piece, does it match the incoming?
            std::list<Piece> candidates;
            if (existing != Piece::Empty) {
                if (!Connections::ConnectsTo(existing, incoming.inverse())) {
                    return false;
                }
                candidates.push_back(existing);
                hit++;
            }
            visited[pos.transpose(grid.width())] = true;
            visited_count++;

            // if we reached all our fixed pieces, check for completion
            if (hit == _fixedPoints.size()) {
                return grid.isComplete();
            }

            // Precompute all remaining fixed positions
            PointList remaining;
            std::for_each(_fixedPoints.cbegin(), _fixedPoints.cend(), [&grid, &remaining, &visited](const Point& p) {
                if (!visited[p.transpose(grid.width())]) {
                    remaining.push_back(p);
                }
            });

            if (candidates.empty()) {
                std::for_each(ValidPieces.cbegin(), ValidPieces.cend(), [&grid, &candidates, &pos](const Piece& p){
                    if (grid.canPlace(pos, p)) {
                        candidates.push_back(p);
                    }
                });
            }

            for (const auto piece : candidates) {
                bool placed = false;
                if (existing == Piece::Empty) {
                    grid.place(pos, piece);
                    placed = true;
                }
                // Find each outgoing direction, there should only be one, but we could add other pieces
                // later!
                for (const auto& d : Connections::GetConnections(piece)) {
                    if (d == incoming.inverse()) {
                        continue;
                    }
                    const auto next = pos + d;
                    if (TryBuild(grid, next, d, visited, hit, visited_count)) {
                        return true;
                    }
                }

                if (placed) {
                    grid.remove(pos);
                }
            }
            visited[pos.transpose(grid.width())] = false;
            visited_count--;

            return false;
        }

        PointList _fixedPoints;
    };
} // namespace TrainTracks
