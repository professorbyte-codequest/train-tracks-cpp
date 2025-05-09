#pragma once

#include <vector>
#include <fstream>

#include "Utils.h"
#include "Piece.h"
#include "Point.h"

namespace TrainTracks
{

    class Data {
    public:
        Data() {}
        Data(const Data&) = default;
        Data(Data&&) = default;
        Data& operator=(const Data&) = default;
        Data& operator=(Data&&) = default;

        std::vector<Piece> startingGrid;
        std::vector<int> rowConstraints;
        std::vector<int> colConstraints;
    };

    class Puzzle {
    public:
        Puzzle() {}
        Puzzle(const Puzzle&) = default;
        Puzzle(Puzzle&&) = default;
        Puzzle& operator=(const Puzzle&) = default;
        Puzzle& operator=(Puzzle&&) = default;

        int gridWidth = 0;
        int gridHeight = 0;

        Data data;

        static Puzzle loadFromFile(std::string path) {
            Puzzle puzzle;

            std::vector<std::pair<Point, Piece>> fixedPieces;

            std::ifstream ifs;
            ifs.open(path);

            bool fixed = false;
            std::string l;
            while (getline(ifs, l)) {
                std::string_view line{l.data(), l.size()};
                if (startsWith(line, "#")) { continue; }

                if (fixed) {
                    // format is:
                    // x,y: piece
                    
                    // split on the colon, parse ints on the left, and piece on the right
                    const auto colon = line.find(':');
                    if (colon == std::string_view::npos) {
                        continue;
                    }
                    const auto points = line.substr(0, colon);
                    const auto piece = trim(line.substr(colon + 1));

                    std::vector<int> ints;
                    parse_as_integers(std::string(points), ',', [&ints](int x) {
                        ints.push_back(x);
                    });
                    fixedPieces.push_back(std::make_pair(Point{ints[0], ints[1]}, fromString(piece)));
                }

                if (!fixed) {
                    if (startsWith(line, "ROWS:")) {
                        const auto rest = line.substr(5);
                        parse_as_integers(std::string(rest), ' ', [&puzzle](int i) {
                            puzzle.data.rowConstraints.push_back(i);
                        });
                    } else if (startsWith(line, "COLS:")) {
                        const auto rest = line.substr(5);
                        parse_as_integers(std::string(rest), ' ', [&puzzle](int i) {
                            puzzle.data.colConstraints.push_back(i);
                        });
                    } else if (startsWith(line, "FIXED:")) {
                        fixed = true;
                    }
                }
            }

            if (puzzle.data.rowConstraints.empty() ||
                puzzle.data.colConstraints.empty()) {
                throw std::runtime_error("Invalid puzzle format. Missing ROWS or COLS.");
            }

            puzzle.gridWidth = puzzle.data.rowConstraints.size();
            puzzle.gridHeight = puzzle.data.colConstraints.size();
            puzzle.data.startingGrid.resize(puzzle.gridWidth * puzzle.gridHeight);
            
            for (const auto &fp : fixedPieces) {
                puzzle.data.startingGrid[fp.first.project(puzzle.gridWidth)] = fp.second;
            }

            return puzzle;
        }

        std::string toString() {
            std::string out;

            out.append("GridWidth: ");
            out.append(std::to_string(this->gridWidth));
            out.append("\n");
            out.append("GridHeight: ");
            out.append(std::to_string(this->gridHeight));

            return out;
        }
    };

} // namespace TrainTracks
