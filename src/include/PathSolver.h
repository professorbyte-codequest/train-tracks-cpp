#pragma once

#include "Solver.h"
#include <functional>

#include "Debug.h"

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
            std::vector<bool> visited(grid.width() * grid.height(), false);
            
            return TryBuild(grid, entry, getEntryIncoming(grid), visited, 0, 0);
        }

    protected:
        Point getEntryIncoming(const Grid& grid) const {
            const auto entry = grid.entry();
            for (const auto& d : Connections::GetConnections(grid.at(entry))) {
                const auto n = entry + d;
                if (!grid.isInBounds(n)) {
                    DEBUG_LOG(entry, d, d.inverse());
                    return d.inverse();
                }
            }
            throw std::runtime_error("Invalid entry, no incoming direction!");
        }
        bool TryBuild(Grid& grid, const Point& pos, const Point& incoming, std::vector<bool>& visited, int hit, int visited_count) {
            Step();

            // Bounds and revisit check
            DEBUG_LOG(pos, !grid.isInBounds(pos), visited[pos.transpose(grid.width())]);
            if (!grid.isInBounds(pos) || visited[pos.transpose(grid.width())]) {
                return false;
            }

            // Can't exceed total count
            DEBUG_LOG(visited_count, grid.target());
            if (visited_count >= grid.target()) {
                return false;
            }

            // Check existing piece
            const auto existing = grid.at(pos);
            DEBUG_LOG(existing);
            // if its a fixed piece, does it match the incoming?
            std::list<Piece> candidates;
            if (existing != Piece::Empty) {
                DEBUG_LOG(!Connections::ConnectsTo(existing, incoming.inverse()));
                if (!Connections::ConnectsTo(existing, incoming.inverse())) {
                    return false;
                }
                candidates.push_back(existing);
                hit++;
            }
            visited[pos.transpose(grid.width())] = true;
            visited_count++;

            // if we reached all our fixed pieces, check for completion
            DEBUG_LOG(hit, _fixedPoints.size(), (hit == _fixedPoints.size()));
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
                    DEBUG_LOG(pos, p, grid.canPlace(pos, p));
                    if (grid.canPlace(pos, p)) {
                        candidates.push_back(p);
                    }
                });
            }
            DEBUG_LOG(candidates.empty(), candidates.size());
            if (candidates.empty()) {
                return false;
            }

            for (const auto piece : candidates) {
                DEBUG_LOG(piece);
                bool placed = false;
                if (existing == Piece::Empty) {
                    grid.place(pos, piece);
                    placed = true;
                }
                DEBUG_LOG(pos, grid.at(pos));
                // Find each outgoing direction, there should only be one, but we could add other pieces
                // later!
                for (const auto& d : Connections::GetConnections(piece)) {
                    if (d == incoming.inverse()) {
                        continue;
                    }
                    const auto next = pos + d;
                    DEBUG_LOG(d, next);
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
