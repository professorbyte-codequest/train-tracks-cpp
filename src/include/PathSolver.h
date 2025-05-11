#pragma once

#include "Solver.h"
#include <algorithm>
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
            const auto fps = grid.fixedPoints();
            std::for_each(fps.cbegin(), fps.cend(), [&](const Point& fp) {
                _fixedPoints.emplace(fp);
            });

            const auto& entry = grid.entry();
            std::vector<bool> visited(grid.width() * grid.height(), false);
            int visited_count = 0;
            int hit = 0;

            DEBUG_LOG(entry, grid.at(entry), grid.exit(), grid.target(), grid.placed());
            
            return TryBuild(grid, entry, getEntryIncoming(grid), visited, visited_count, hit);
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
        bool TryBuild(Grid& grid, const Point& pos, const Point& incoming, std::vector<bool>& visited, int& visited_count, int hit) {
            Step(pos);
            const auto idx = grid.flatten(pos);

            // Bounds
            if (!grid.isInBounds(pos)) {
                DEBUG_LOG(pos, !grid.isInBounds(pos));
                return false;
            }

            // revist check
            if (visited[idx]) {
                DEBUG_LOG(pos, visited[idx]);
                return false;
            }

            // Can't exceed total count
            if (visited_count > grid.target()) {
                DEBUG_LOG(visited_count, grid.target());
                return false;
            }

            // Check existing piece
            const auto existing = grid.at(pos);
            // if its a fixed piece, does it match the incoming?
            std::list<Piece> candidates;
            bool isFixed = false;
            if (existing != Piece::Empty) {
                if (!Connections::ConnectsTo(existing, incoming.inverse())) {
                    DEBUG_LOG(existing, !Connections::ConnectsTo(existing, incoming.inverse()));
                    return false;
                }

                // if we reached the exit, check for completion
                if (pos == grid.exit()) {
                    DEBUG_LOG(hit, _fixedPoints.size());
                    return grid.isComplete();
                }

                candidates.push_back(existing);

                isFixed = _fixedPoints.find(pos) != _fixedPoints.end();
                hit += isFixed;
            }
            visited[idx] = true;
            visited_count++;

            if (candidates.empty()) {
                std::for_each(ValidPieces.crbegin(), ValidPieces.crend(), [&grid, &candidates, &pos](const Piece& p){
                    if (grid.canPlace(pos, p)) {
                        DEBUG_LOG(pos, p, grid.canPlace(pos, p));
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
                DEBUG_LOG(pos, placed, grid.at(pos));
                // Find each outgoing direction, there should only be one, but we could add other pieces
                // later!
                for (const auto& d : Connections::GetConnections(piece)) {
                    if (d == incoming.inverse()) {
                        continue;
                    }
                    const auto next = pos + d;
                    if (TryBuild(grid, next, d, visited, visited_count, hit)) {
                        return true;
                    }
                }

                if (placed) {
                    grid.remove(pos);
                }
            }
            DEBUG_LOG(pos, candidates.size(), visited_count, hit);
            visited[idx] = false;
            visited_count--;
            hit -= isFixed;

            return false;
        }

        PointSet _fixedPoints;
    };
} // namespace TrainTracks
