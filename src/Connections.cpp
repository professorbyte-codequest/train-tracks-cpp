#include "include/Connections.h"
#include "include/Piece.h"

namespace TrainTracks
{

Connections Connections::instance;

const std::array<Piece, 6>ValidPieces{ Piece::Horizontal, 
    Piece::Vertical, Piece::CornerNE,
    Piece::CornerSE, Piece::CornerSW,
    Piece::CornerNW };

} // namespace TrainTracks
