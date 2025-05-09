#pragma once

#include "Piece.h"
#include "Point.h"
#include "Puzzle.h"
#include "Connections.h"
#include "Debug.h"
#include <vector>
#include <queue>
#include <assert.h>

namespace TrainTracks {

    class Grid {
    private:
        Grid(int rows, int cols)
            : _rows(rows)
            , _cols(cols)
            , _bottom(_rows - 1)
            , _right(cols - 1)
            , _totalCount(0)
            , _placedCount(0)
            , _displayConstraints(false)
            , _grid(rows * cols, Piece::Empty)
            , _rowConstraints(rows, 0)
            , _colConstraints(cols, 0)
            , _placedInRow(rows, 0)
            , _placedInCol(cols, 0)
        {
        }
    public:

        Grid(const Puzzle& p)
            : Grid(p.gridHeight, p.gridWidth)
        {
            _rowConstraints = p.data.rowConstraints;
            _colConstraints = p.data.colConstraints;

            for (int r = 0; r < _rows; r++)
            {
                for (int c = 0; c < _cols; c++)
                {
                    const Point pt{c, r};
                    const auto piece = p.data.startingGrid[pt.project(_cols)];
                    if (piece != Piece::Empty) {
                        place(pt, piece);
                    }
                }
            }

            extractEntryAndExit();

            for (const auto i : _rowConstraints) {
                _totalCount += i;
            }
            int cols = 0;
            for (const auto i : _colConstraints) {
                cols += i;
            }

            if (cols != _totalCount) {
                throw std::runtime_error("Row and Column constraint missmatch");
            }

            placeObviousPieces();
        }

        int width() const {
            return _cols;
        }

        int height() const {
            return _rows;
        }

        int placed() const {
            return _placedCount;
        }

        int target() const {
            return _totalCount;
        }

        PointList fixedPoints() const {
            PointList pts;
            for (int y = 0; y < _rows; y++) {
                for (int x = 0; x < _cols; x++) {
                    Point pt(x, y);
                    const auto p = at(pt);
                    if (p != Piece::Empty) {
                        pts.emplace_back(pt);
                    }
                }
            }
            return pts;
        }

        const Point& entry() const {
            return _entry;
        }

        const Point& exit() const {
            return _exit;
        }
        
        Piece& at(const Point& p) {
            return _grid[flatten(p)];
        }

        Piece at(const Point& p) const {
            return _grid[flatten(p)];
        }

        Piece at(int x, int y) const {
            return at(Point(x, y));
        }

        bool isInBounds(const Point& pt) const {
            return pt.x >= 0 &&
                pt.y >= 0 &
                pt.x < _cols &&
                pt.y < _rows;
        }

        bool isOnEdge(const Point& pt) const {
            return pt.x == 0 ||
                pt.y == 0 ||
                pt.x == _right ||
                pt.y == _bottom;
        }

        bool isEmpty(const Point& pt) const {
            return at(pt) == Piece::Empty;
        }

        bool isFilled(const Point& pt) const {
            return !isEmpty(pt);
        }

        bool canPlace(const Point& pt, Piece p) const {
            // Must be inbounds
            if (!isInBounds(pt)) { return false; }
            // Must be empty
            if (isFilled(pt)) { return false; }
            // Must satisfy row counts
            if (trackInRowCount(pt.y) >= _rowConstraints[pt.y]) { return false; }
            if (trackInColCount(pt.x) >= _colConstraints[pt.x]) { return false; }

            // Entry/Exit requirements - we can't leave the grid
            switch (p) {
                case Piece::Horizontal:
                    if (pt.x == 0 || pt.x == _right) { return false; }
                    break;
                case Piece::Vertical:
                    if (pt.y == 0 || pt.y == _bottom) { return false; }
                    break;
                case Piece::CornerNW:
                    if (pt.y == 0 || pt.x == 0) { return false; }
                    break;
                case Piece::CornerNE:
                    if (pt.y == 0 || pt.x == _right) { return false; }
                    break;
                case Piece::CornerSE:
                    if (pt.y == _bottom || pt.x == 0) { return false; }
                    break;
                case Piece::CornerSW:
                    if (pt.y == _bottom || pt.x == 0) { return false; }
                    break;
                case Piece::Empty:
                    break;
            }

            // Existing neighbor alignment
            bool hasNeighbor = false;
            bool connectsToAny = false;

            const std::array<Point, 4> dirs{ Point{0, 1}, Point{1, 0}, Point{0, -1}, Point{-1, 0}};

            for (const auto &d : dirs) {
                const auto n = pt + d;
                if (!isInBounds(n)) { continue; }
                const auto np = at(n);
                if (np == Piece::Empty) { continue; }
                hasNeighbor = true;

                bool thisConn = Connections::ConnectsTo(p, d);
                bool thatConn = Connections::ConnectsTo(np, d.inverse());
                if (thisConn != thatConn) { return false; }
                if (thisConn) { connectsToAny = true; }
            }
            if (hasNeighbor && !connectsToAny) return false;
            
            // Look-ahead to ensure we have capacity in neighboring row/col
            for (const auto d : Connections::GetConnections(p)) {
                const auto n = pt + d;
                if (!isInBounds(n)) { return false; }

                if (isEmpty(n)) {
                    if (trackInRowCount(n.y) >= _rowConstraints[n.y] ||
                        trackInColCount(n.x) >= _colConstraints[n.x]) {
                        return false;
                    }
                }
            }
    
            return true;
        }

        int trackInRowCount(int r) const {
            return _placedInRow[r];
        }

        int trackInColCount(int c) const {
            return _placedInCol[c];
        }

        void place(const Point& pt, Piece p) {
            if (p == Piece::Empty) {
                throw std::runtime_error("Cannot place empty piece");
            }
            at(pt) = p;
            DEBUG_LOG(pt, p);
            _placedInCol[pt.x]++;
            _placedInRow[pt.y]++;
            _placedCount++;
        }

        void remove(const Point& pt) {
            if (!isEmpty(pt)) {
                _placedInCol[pt.x]--;
                _placedInRow[pt.y]--;
                _placedCount--;
            }
            at(pt) = Piece::Empty;
        }

        bool isSingleConnectedPath() const {
            Point first;
            if (!findFirst(first)) {
                return false;
            }

            std::vector<bool> visited(_rows * _cols, false);
            std::queue<Point> dfs;
            visited[flatten(first)] = true;
            dfs.emplace(std::move(first));

            const std::array<Point, 4> dirs{ Point{0, 1}, Point{1, 0}, Point{0, -1}, Point{-1, 0}};
            while (!dfs.empty()) {
                const auto pt = dfs.front();
                dfs.pop();
                for (const auto& d : dirs) {
                    auto next = pt + d;
                    // Check if the neighbor is in bounds and not visited,
                    // the neighbor connects to the current piece, and the
                    // current piece connects to the neighbor
                    if (isInBounds(next) && !isEmpty(next) &&
                        !visited[flatten(next)])
                    {
                        const auto p = at(pt);
                        const auto n = at(next);
                        if (Connections::ConnectsTo(p, d) && Connections::ConnectsTo(n, d.inverse())) {
                            visited[flatten(next)] = true;
                            dfs.emplace(std::move(next));
                        }
                    }
                }
            }
        
            // Ensure all non-empty cells are visited
            for (int y = 0; y < _rows; y++) {
                for (int x = 0; x < _cols; x++) {
                    const Point p{x, y};
                    if (!isEmpty(p) && !visited[flatten(p)]) {
                        return false;
                    }
                }
            }

            return true;
        }

        bool constraintsSatisfied() const {
            for (int r = 0; r < _rows; r++) {
                if (_placedInRow[r] != _rowConstraints[r]) {
                    return false;
                }
            }

            for(int c = 0; c < _cols; c++) {
                if (_placedInCol[c] != _colConstraints[c]) {
                    return false;
                }
            }
            return true;
        }

        bool isComplete() const {
            return isSingleConnectedPath() && constraintsSatisfied();
        }

        bool canStillSatisfy() {
            // Rows
            for (int r = 0; r < _rows; r++)
            {
                const auto placed = _placedInRow[r];
                if (placed > _rowConstraints[r])
                    return false;
            }
            // Columns
            for (int c = 0; c < _cols; c++)
            {
                const auto placed = _placedInCol[c];
                if (placed > _colConstraints[c])
                    return false;
            }
            return true;
        }

        std::string toString() const {
            std::string s;

            for (int y = 0; y < _rows; y++) {
                for (int x = 0; x < _cols; x++) {
                    s.append(PieceSymbol(at(x, y)));
                }
                s.append(1, '\n');
            }

            return s;
        }

        friend std::ostream& operator<<(std::ostream& os, const Grid& grid);

        void displayConstraints(bool v) {
            _displayConstraints = v;
        }

        int64_t flatten(const Point& p) const {
            return p.y * _rows + p.x;
        }
    private:
        void placeObviousPieces() {
            // on the edges, if there are a max of two, we can place if there
            // is already one piece in the row/col
            if (_rowConstraints[0] == 2) {
                for (int c = 0; _placedInRow[0] == 1 && c < _cols; c++) {
                    const Point pt{c, 0};
                    const auto p = at(pt);
                    if (p == Piece::Empty) {
                        continue;
                    }
                    switch (p) {
                        case Piece::Horizontal:
                            // This must be an entry or exit
                            if (c == 0) {
                                assert(canPlace(Point(c + 1, 0), Piece::CornerSW));
                                place(Point(c + 1, 0), Piece::CornerSW);
                            } else if (c == _right) {
                                assert(canPlace(Point(c - 1, 0), Piece::CornerSE));
                                place(Point(c - 1, 0), Piece::CornerSE);
                            }
                            break;
                        
                        case Piece::CornerSE:
                            // This must loop back in to the grid
                            assert(canPlace(Point(c + 1, 0), Piece::CornerSW));
                            place(Point(c + 1, 0), Piece::CornerSW);
                            break;
                        case Piece::CornerSW:
                            // This must loop back in to the grid
                            assert(canPlace(Point(c - 1, 0), Piece::CornerSE));
                            place(Point(c - 1, 0), Piece::CornerSE);
                            break;
                        case Piece::CornerNE:
                            // This must be an entry or exit, so loop back in
                            assert(canPlace(Point(c + 1, 0), Piece::CornerSW));
                            place(Point(c + 1, 0), Piece::CornerSW);
                            break;
                        case Piece::CornerNW:
                            // This must be an entry or exit, so loop back in
                            assert(canPlace(Point(c - 1, 0), Piece::CornerSE));
                            place(Point(c - 1, 0), Piece::CornerSE);
                            break;
                        default:
                            break;
                    }
                }
            }
        
            if (_rowConstraints[_right] == 2) {
                for (int c = 0; _placedInRow[_right] == 1 && c < _cols; c++) {
                    const Point pt{c, _right};
                    const auto p = at(pt);
                    if (p == Piece::Empty) {
                        continue;
                    }
                    switch (p) {
                        case Piece::Horizontal:
                            // This must be an entry or exit
                            if (c == 0) {
                                assert(canPlace(Point(c + 1, _right), Piece::CornerNW));
                                place(Point(c + 1, _right), Piece::CornerNW);
                            } else if (c == _right) {
                                assert(canPlace(Point(c - 1, _right), Piece::CornerNE));
                                place(Point(c - 1, _right), Piece::CornerNE);
                            }
                            break;
                        
                        case Piece::CornerNE:
                            // This must loop back in to the grid
                            assert(canPlace(Point(c + 1, _right), Piece::CornerNW));
                            place(Point(c + 1, _right), Piece::CornerNW);
                            break;
                        case Piece::CornerNW:
                            // This must loop back in to the grid
                            assert(canPlace(Point(c - 1, _right), Piece::CornerNE));
                            place(Point(c - 1, _right), Piece::CornerNE);
                            break;
                        case Piece::CornerSE:
                            // This must be an entry or exit, so loop back in
                            assert(canPlace(Point(c + 1, _right), Piece::CornerNW));
                            place(Point(c + 1, _right), Piece::CornerNW);
                            break;
                        case Piece::CornerSW:
                            // This must be an entry or exit, so loop back in
                            assert(canPlace(Point(c - 1, _right), Piece::CornerNE));
                            place(Point(c - 1, _right), Piece::CornerNE);
                            break;
                        default:
                            break;
                    }
                }
            }
        
            if (_colConstraints[0] == 2) {
                for (int r = 0; _placedInCol[0] == 1 && r < _rows; r++) {
                    const Point pt{0, r};
                    const auto p = at(pt);
                    if (p == Piece::Empty) {
                        continue;
                    }
                    switch (p) {
                        case Piece::Vertical:
                            // This must be an entry or exit
                            if (r == 0) {
                                assert(canPlace(Point(0, r + 1), Piece::CornerSW));
                                place(Point(0, r + 1), Piece::CornerSW);
                            } else if (r == _bottom) {
                                assert(canPlace(Point(0, r - 1), Piece::CornerNW));
                                place(Point(0, r - 1), Piece::CornerNW);
                            }
                            break;
                        
                        case Piece::CornerNE:
                            if (r > 0) {
                                // This must loop back in to the grid
                                assert(canPlace(Point(0, r - 1), Piece::CornerSW));
                                place(Point(0, r - 1), Piece::CornerSW);
                            }
                            break;
                        case Piece::CornerSE:
                            if (r < _bottom) {
                                // This must loop back in to the grid
                                assert(canPlace(Point(0, r + 1), Piece::CornerNW));
                                place(Point(0, r + 1), Piece::CornerNW);
                            }
                            break;
                        case Piece::CornerNW:
                            // This must be an entry or exit, so loop back in
                            assert(canPlace(Point(0, r - 1), Piece::CornerSW));
                            place(Point(0, r - 1), Piece::CornerSW);
                            break;
                        case Piece::CornerSW:
                            // This must be an entry or exit, so loop back in
                            assert(canPlace(Point(0, r - 1), Piece::CornerNW));
                            place(Point(0, r - 1), Piece::CornerNW);
                            break;
                        default:
                            break;
                    }
                }
            }
       
            if (_colConstraints[_bottom] == 2) {
                for (int r = 0; _placedInCol[_bottom] == 1 && r < _rows; r++) {
                    const Point pt{_bottom, r};
                    const auto p = at(pt);
                    if (p == Piece::Empty) {
                        continue;
                    }
                    switch (p) {
                        case Piece::Vertical:
                            // This must be an entry or exit
                            if (r == 0) {
                                assert(canPlace(Point(_bottom, r + 1), Piece::CornerNW));
                                place(Point(_bottom, r + 1), Piece::CornerNW);
                            } else if (r == _bottom) {
                                assert(canPlace(Point(_bottom, r - 1), Piece::CornerNE));
                                place(Point(_bottom, r - 1), Piece::CornerNE);
                            }
                            break;
                        
                        case Piece::CornerNW:
                            if (r > 0) {
                                // This must loop back in to the grid
                                assert(canPlace(Point(_bottom, r - 1), Piece::CornerNW));
                                place(Point(_bottom, r - 1), Piece::CornerNW);
                            }
                            break;
                        case Piece::CornerSW:
                            if (r < _bottom) {
                                // This must loop back in to the grid
                                assert(canPlace(Point(_bottom, r + 1), Piece::CornerNE));
                                place(Point(_bottom, r + 1), Piece::CornerNE);
                            }
                            break;
                        case Piece::CornerNE:
                            // This must be an entry or exit, so loop back in
                            assert(canPlace(Point(_bottom, r - 1), Piece::CornerSW));
                            place(Point(_bottom, r - 1), Piece::CornerSW);
                            break;
                        case Piece::CornerSE:
                            // This must be an entry or exit, so loop back in
                            assert(canPlace(Point(_bottom, r + 1), Piece::CornerSE));
                            place(Point(_bottom, r + 1), Piece::CornerSE);
                            break;
                        default:
                            break;
                    }
                }
            }

            // For rows with a single piece, we know the adjacent rows must connect
            // through it
            for (int r = 1; r < _rows - 1; r++) {
                // We need to place a veritcal piece in the row, but to do that, we
                // need to connect to it.  find the nearest piece in the row above
                if (_rowConstraints[r] == 1 && _placedInRow[r] == 0) {
                    // If there is only a single piece in the row above, we
                    // can place a vertical piece in this row, with a corner
                    // in the row above connecting to the piece in that row
                    if (_placedInRow[r - 1] == 1) {
                        int toPlaceAbove = _rowConstraints[r - 1] - _placedInRow[r - 1];
                        // Find the piece in the row above
                        Point ptAbove;
                        Piece pAbove = Piece::Empty;
                        for (int c = 0; c < _cols; c++) {
                            const Point pt{c, r - 1};
                            if (!isEmpty(pt)) {
                                ptAbove = pt;
                                pAbove = at(pt);
                                break;
                            }
                        }
                        assert(pAbove != Piece::Empty);
                        if (pAbove == Piece::Horizontal &&
                            ptAbove != _exit &&
                            ptAbove != _entry) {
                            // We need more information about the piece, so skip it
                            continue;
                        }

                        // Get the horizontal connection direction for pAbove
                        const auto conns = Connections::GetConnections(pAbove);
                        Point connDir;
                        for (const auto& d : conns) {
                            if (d.y == 0 && isInBounds(ptAbove + d)) {
                                connDir = d;
                                break;
                            }
                        }
                        assert(connDir != Point::origin());
                        // Now, from that piece above, we can walk toPlaceAbove - 1
                        // and place horizontals in each, then a corner to this row,
                        // and then a vertical piece in this row
                        for (int i = 0; i < toPlaceAbove - 1; i++) {
                            const Point pt{ptAbove.x + connDir.x * (i + 1),
                                ptAbove.y};
                            assert(canPlace(pt, Piece::Horizontal));
                            place(pt, Piece::Horizontal);
                        }
                        // Now, place the corner
                        if (sgn(connDir.x) == 1) {
                            // This is a CornerSW
                            assert(canPlace(Point(ptAbove.x + connDir.x * toPlaceAbove, ptAbove.y), Piece::CornerSW));
                            place(Point(ptAbove.x + connDir.x * toPlaceAbove, ptAbove.y), Piece::CornerSW);
                        } else {
                            // This is a CornerSE
                            assert(canPlace(Point(ptAbove.x + connDir.x * toPlaceAbove, ptAbove.y), Piece::CornerSE));
                            place(Point(ptAbove.x + connDir.x * toPlaceAbove, ptAbove.y), Piece::CornerSE);
                        }
                        // Now, place the vertical piece
                        assert(canPlace(Point(ptAbove.x + connDir.x * toPlaceAbove, r), Piece::Vertical));
                        place(Point(ptAbove.x + connDir.x * toPlaceAbove, r), Piece::Vertical);
                    }
                
                    // Repeat for the row below
                    if (_placedInRow[r + 1] == 1) {
                        int toPlaceBelow = _rowConstraints[r + 1] - _placedInRow[r + 1];
                        // Find the piece in the row below
                        Point ptBelow;
                        Piece pBelow = Piece::Empty;
                        for (int c = 0; c < _cols; c++) {
                            const Point pt{c, r + 1};
                            if (!isEmpty(pt)) {
                                ptBelow = pt;
                                pBelow = at(pt);
                                break;
                            }
                        }
                        assert(pBelow != Piece::Empty);
                        if (pBelow == Piece::Horizontal &&
                            ptBelow != _exit &&
                            ptBelow != _entry) {
                            // We need more information about the piece, so skip it
                            continue;
                        }

                        // Get the horizontal connection direction for pBelow
                        const auto conns = Connections::GetConnections(pBelow);
                        Point connDir;
                        for (const auto& d : conns) {
                            if (d.y == 0 && isInBounds(ptBelow + d)) {
                                connDir = d;
                                break;
                            }
                        }
                        assert(connDir != Point::origin());
                        // Now, from that piece be,low, we can walk toPlaceBelow - 1
                        // and place horizontals in each, then a corner to this row,
                        // and then a vertical piece in this row
                        for (int i = 0; i < toPlaceBelow - 1; i++) {
                            const Point pt{ptBelow.x + connDir.x * (i + 1),
                                ptBelow.y};
                            assert(canPlace(pt, Piece::Horizontal));
                            place(pt, Piece::Horizontal);
                        }

                        // Now, place the corner
                        if (sgn(connDir.x) == 1) {
                            // This is a CornerNW
                            assert(canPlace(Point(ptBelow.x + connDir.x * toPlaceBelow, ptBelow.y), Piece::CornerNW));
                            place(Point(ptBelow.x + connDir.x * toPlaceBelow, ptBelow.y), Piece::CornerNW);
                        } else {
                            // This is a CornerNE
                            assert(canPlace(Point(ptBelow.x + connDir.x * toPlaceBelow, ptBelow.y), Piece::CornerNE));
                            place(Point(ptBelow.x + connDir.x * toPlaceBelow, ptBelow.y), Piece::CornerNE);
                        }
                        // Now, place the vertical piece (if we didn't already)
                        Point pt{ptBelow.x + connDir.x * toPlaceBelow, r};
                        if (isEmpty(pt)) {
                            assert(canPlace(pt, Piece::Vertical));
                            place(pt, Piece::Vertical);
                        }
                    }
                }
            }
        
            // For columns with a single piece, we know the adjacent columns must connect
            // through it
            for (int c = 1; c < _cols - 1; c++) {
                // We need to place a horizontal piece in the column, but to do that, we
                // need to connect to it.  find the nearest piece in the column above
                if (_colConstraints[c] == 1 && _placedInCol[c] == 0) {
                    // If there is only a single piece in the column above, we
                    // can place a horizontal piece in this column, with a corner
                    // in the column to the left connecting to the piece in that column
                    if (_placedInCol[c - 1] == 1) {
                        int toPlaceLeft = _colConstraints[c - 1] - _placedInCol[c - 1];
                        // Find the piece in the column above
                        Point ptLeft;
                        Piece pLeft = Piece::Empty;
                        for (int r = 0; r < _rows; r++) {
                            const Point pt{c, r};
                            if (!isEmpty(pt)) {
                                ptLeft = pt;
                                pLeft = at(pt);
                                break;
                            }
                        }
                        assert(pLeft != Piece::Empty);
                        if (pLeft == Piece::Vertical &&
                            ptLeft != _exit &&
                            ptLeft != _entry) {
                            // We need more information about the piece, so skip it
                            continue;
                        }

                        // Get the vertical connection direction for pLeft
                        const auto conns = Connections::GetConnections(pLeft);
                        Point connDir;
                        for (const auto& d : conns) {
                            if (d.x == 0) {
                                connDir = d;
                                break;
                            }
                        }
                        assert(connDir != Point::origin());
                        // Now, from that piece above, we can walk toPlaceLeft - 1
                        // and place horizontals in each, then a corner to this row,
                        // and then a vertical piece in this row
                        for (int i = 0; i < toPlaceLeft - 1; i++) {
                            const Point pt{ptLeft.x,
                                ptLeft.y + connDir.y * (i + 1)};
                            assert(canPlace(pt, Piece::Vertical));
                            place(pt, Piece::Vertical);
                        }
                        // Now, place the corner
                        if (sgn(connDir.y) == 1) {
                            // This is a CornerSE
                            assert(canPlace(Point(ptLeft.x, ptLeft.y + connDir.y * toPlaceLeft), Piece::CornerSE));
                            place(Point(ptLeft.x, ptLeft.y + connDir.y * toPlaceLeft), Piece::CornerSE);
                        } else {
                            // This is a CornerNE
                            assert(canPlace(Point(ptLeft.x, ptLeft.y + connDir.y * toPlaceLeft), Piece::CornerNE));
                            place(Point(ptLeft.x, ptLeft.y + connDir.y * toPlaceLeft), Piece::CornerNE);
                        }

                        // Now, place the horizontal piece
                        assert(canPlace(Point(c, ptLeft.y + connDir.y * toPlaceLeft), Piece::Horizontal));
                        place(Point(c, ptLeft.y + connDir.y * toPlaceLeft), Piece::Horizontal);
                    }
                
                    // Repeat to the right
                    if (_placedInCol[c + 1] == 1) {
                        int toPlaceRight = _colConstraints[c + 1] - _placedInCol[c + 1];
                        // Find the piece in the column above
                        Point ptRight;
                        Piece pRight = Piece::Empty;
                        for (int r = 0; r < _rows; r++) {
                            const Point pt{c, r};
                            if (!isEmpty(pt)) {
                                ptRight = pt;
                                pRight = at(pt);
                                break;
                            }
                        }
                        assert(pRight != Piece::Empty);
                        if (pRight == Piece::Vertical &&
                            ptRight != _exit &&
                            ptRight != _entry) {
                            // We need more information about the piece, so skip it
                            continue;
                        }

                        // Get the vertical connection direction for pRight
                        const auto conns = Connections::GetConnections(pRight);
                        Point connDir;
                        for (const auto& d : conns) {
                            if (d.x == 0) {
                                connDir = d;
                                break;
                            }
                        }
                        assert(connDir != Point::origin());
                        // Now, from that piece right, we can walk toPlaceRight - 1
                        // and place verticals in each, then a corner to this col,
                        // and then a horizontal piece in this col
                        for (int i = 0; i < toPlaceRight - 1; i++) {
                            const Point pt{ptRight.x,
                                ptRight.y + connDir.y * (i + 1)};
                            assert(canPlace(pt, Piece::Vertical));
                            place(pt, Piece::Vertical);
                        }
                        // Now, place the corner
                        if (sgn(connDir.y) == 1) {
                            // This is a CornerSW
                            assert(canPlace(Point(ptRight.x, ptRight.y + connDir.y * toPlaceRight), Piece::CornerSW));
                            place(Point(ptRight.x, ptRight.y + connDir.y * toPlaceRight), Piece::CornerSW);
                        } else {
                            // This is a CornerNW
                            assert(canPlace(Point(ptRight.x, ptRight.y + connDir.y * toPlaceRight), Piece::CornerNW));
                            place(Point(ptRight.x, ptRight.y + connDir.y * toPlaceRight), Piece::CornerNW);
                        }

                        // Now, place the horizontal piece
                        assert(canPlace(Point(c, ptRight.y + connDir.y * toPlaceRight), Piece::Horizontal));
                        place(Point(c, ptRight.y + connDir.y * toPlaceRight), Piece::Horizontal);
                    }
                }
            }
        }

        void extractEntryAndExit() {
            std::array<Point, 4>corners = { Point{0, 0}, Point{0, _bottom },
                Point{_right, 0}, Point{_right, _bottom}};
            
            std::vector<Point>exits;
            for (const auto& pt : corners) {
                int offDirs = getOffCountForPieceAt(pt);
                if (offDirs == 1) {
                    exits.emplace_back(pt);
                }
            }
            
            for (int x = 1; x < _right; x++) {
                int offDirs = getOffCountForPieceAt({x, 0});
                if (offDirs == 1) {
                    exits.emplace_back(x, 0);
                }
                offDirs = getOffCountForPieceAt({x, _bottom});
                if (offDirs == 1) {
                    exits.emplace_back(x, _bottom);
                }
            }

            for (int y = 1; y < _bottom; y++) {
                int offDirs = getOffCountForPieceAt({0, y});
                if (offDirs == 1) {
                    exits.emplace_back(0, y);
                }
                offDirs = getOffCountForPieceAt({_right , y});
                if (offDirs == 1) {
                    exits.emplace_back(_right , y);
                }
            }

            if (exits.size() != 2) {
                throw std::runtime_error("Invalid number of exits");
            }

            _entry = exits[0];
            _exit = exits[1];
        }

        int getOffCountForPieceAt(const Point& pt) {
            if (!isOnEdge(pt)) {
                return 0;
            }

            const auto p = at(pt);
            if (p == Piece::Empty) {
                return 0;
            }

            // check the connections, and if one is off grid, then mark it
            const auto conns = Connections::GetConnections(p);
            int offDirs = 0;
            std::for_each(conns.cbegin(), conns.cend(), [&](const Point& d) {
                const Point e = pt + d;
                if (!isInBounds(e)) {
                    offDirs++;
                }
            });
            if (offDirs > 1) {
                throw std::runtime_error("Piece goes off grid more than once!");
            }
            return offDirs;
        }

        bool findFirst(Point& pt) const {
            for (int r = 0; r < _rows; r++) {
                for (int c = 0; c < _cols; c++) {
                    pt.x = c;
                    pt.y = r;
                    if (!isEmpty(pt)) {
                        return true;
                    }
                }
            }
            return false;
        }

        const int _rows;
        const int _cols;
        const int _bottom;
        const int _right;
        int _totalCount;
        int _placedCount;
        bool _displayConstraints;

        Point _entry;
        Point _exit;
        
        std::vector<Piece> _grid;

        std::vector<int> _rowConstraints;
        std::vector<int> _colConstraints;

        std::vector<int> _placedInRow;
        std::vector<int> _placedInCol;
    };

    inline std::ostream& operator<<(std::ostream& os, const Grid& grid) {
        //os << reset << std::endl;
        if (grid._displayConstraints) {
            os << "  ";
            for (int x = 0; x < grid._cols; x++) {
                os << grid._colConstraints[x] << " ";
            }
            os << std::endl;
        }
        for (int y = 0; y < grid._rows; y++) {
            if (grid._displayConstraints) { os << grid._rowConstraints[y] << " "; }

            for (int x = 0; x < grid._cols; x++) {
                Point pt{x, y};
                if (pt == grid._entry || pt == grid._exit) {
                    os << bold_on;
                }
                os << grid.at(pt);
                if (grid._displayConstraints) { os << " "; }
                if (pt == grid._entry || pt == grid._exit) {
                    os << bold_off;
                }
            }
            os << std::endl;
        }
        return os;
    }
}