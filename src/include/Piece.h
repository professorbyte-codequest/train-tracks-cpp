#pragma once

#include <array>
#include <map>
#include <string_view>
#include <iostream>

namespace TrainTracks {

    enum class Piece {
        Empty = 0,

        Horizontal = 3,
        Vertical = 4,
        CornerNE = 5,
        CornerSE = 6,
        CornerSW = 7,
        CornerNW = 8,
    };

    inline const char* PieceSymbol(const Piece p) {
        switch (p) {
            case Piece::Empty: return " ";
            case Piece::Horizontal: return "─";  // nicer horizontal line
            case Piece::Vertical: return "│";  // nicer vertical line
            case Piece::CornerNE: return "└";  // connects north & east
            case Piece::CornerNW: return "┘";  // connects north & west
            case Piece::CornerSE: return "┌";  // connects south & east
            case Piece::CornerSW: return "┐";  // connects south & west
        }
        return "?";
    }

    inline std::ostream& operator<<(std::ostream& os, const Piece& p) {
        os << PieceSymbol(p);
        return os;
    }

    inline Piece fromString(const std::string_view s) {
        static const std::string_view Horizontal{"Horizontal"};
        static const std::string_view Vertical{"Vertical"};
        static const std::string_view CornerNE{"CornerNE"};
        static const std::string_view CornerNW{"CornerNW"};
        static const std::string_view CornerSE{"CornerSE"};
        static const std::string_view CornerSW{"CornerSW"};
        static const std::map<std::string_view, Piece> map {
            { Horizontal, Piece:: Horizontal },
            { Vertical, Piece:: Vertical },
            { CornerNE, Piece:: CornerNE },
            { CornerNW, Piece:: CornerNW },
            { CornerSE, Piece:: CornerSE },
            { CornerSW, Piece:: CornerSW },
        };

        const auto it = map.find(s);
        if (it != map.end()) {
            return it->second;
        }
        throw std::runtime_error("Invalid piece type"); 
    }

    extern const std::array<Piece, 6>ValidPieces;
}