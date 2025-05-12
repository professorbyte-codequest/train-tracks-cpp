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
            , _fixedCount(0)
            , _totalCount(0)
            , _placedCount(0)
            , _displayConstraints(false)
            , _bold(true)
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
                        _fixedCount++;
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
            if (!isInBounds(pt)) { DEBUG_LOG(!isInBounds(pt)); return false; }
            // Must be empty
            if (isFilled(pt)) { DEBUG_LOG(isFilled(pt)); return false; }
            // Must satisfy row counts
            if (trackInRowCount(pt.y) >= _rowConstraints[pt.y]) { DEBUG_LOG(_rowConstraints[pt.y]); return false; }
            if (trackInColCount(pt.x) >= _colConstraints[pt.x]) { DEBUG_LOG(_colConstraints[pt.x]); return false; }

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
                    if (pt.y == _bottom || pt.x == _right) { return false; }
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
            
            int newTrackingInRowCount = trackInRowCount(pt.y) + 1;
            int newTrackingInColCount = trackInColCount(pt.x) + 1;
            // Look-ahead to ensure we have capacity in neighboring row/col
            for (const auto d : Connections::GetConnections(p)) {
                const auto n = pt + d;
                if (!isInBounds(n)) { return false; }

                if (isEmpty(n)) {
                    const auto rowCount = n.y == pt.y ? newTrackingInRowCount : trackInRowCount(n.y);
                    const auto colCount = n.x == pt.x ? newTrackingInColCount : trackInColCount(n.x);
                    if (rowCount >= _rowConstraints[n.y] ||
                        colCount >= _colConstraints[n.x]) {
                        return false;
                    }
                }
            }

            DEBUG_LOG(pt, p, trackInRowCount(pt.y), _rowConstraints[pt.y], trackInColCount(pt.x), _colConstraints[pt.x]);
    
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

        void bold(bool v) {
            _bold = v;
        }

        int64_t flatten(const Point& p) const {
            return p.y * _rows + p.x;
        }

        int fixedCount() const {
            return _fixedCount;
        }
    private:

        struct EdgeConstrains {
            int idx; // the row or column index
            bool isRow; // true if row, false if column
            Point iterator; // iterator to the next row/col
            std::vector<int>& constraints;
            std::vector<int>& placed;
        };

        struct SingleRowConstraints {
            bool isRow;
            int max;
            Point iterator;
            std::vector<int>& constraints;
            std::vector<int>& placed;
        };
        void placeObviousPieces() {

            std::array<EdgeConstrains, 4> edgeConstraints { {
                  { 0, true, Point{1, 0}, _rowConstraints, _placedInRow },
                  { _bottom, true, Point{1, 0}, _rowConstraints, _placedInRow },
                  { 0, false, Point{0, 1},  _colConstraints, _placedInCol },
                  {_right, false, Point{0, 1}, _colConstraints, _placedInCol }
                }
            };

            for (const auto& ec : edgeConstraints) {
                if (ec.constraints[ec.idx] != 2) {
                    continue;
                }

                DEBUG_LOG(ec.idx, ec.isRow);
        
                Point pos { ec.isRow ? 0 : ec.idx, ec.isRow ? ec.idx : 0 };
                while (isInBounds(pos) && ec.placed[ec.idx] == 1) {
                    const auto p = at(pos);
                    if (p != Piece::Empty) {
                        DEBUG_LOG(pos, p);
                        // Get the connections for the piece which go back into the grid
                        int dirs = 0;
                        Point placeAt = pos;
                        for (const auto &d : Connections::GetConnections(p)) {
                            const auto n = pos + d;
                            if (isInBounds(n) && isEmpty(n)) {
                                dirs++;
                                placeAt = n;
                            }
                        }
                        DEBUG_LOG(pos, p, placeAt, dirs);
                        // If we don't have a single connection, just move on
                        if (dirs != 1) {
                            break;
                        }

                        // Find a piece we can place, canPlace will help us
                        for (const auto p : ValidPieces) {
                            if (canPlace(placeAt, p)) {
                                place(placeAt, p);
                                break;
                            }
                        }
                        break;
                    }

                    pos += ec.iterator;
                }
            }

            std::array<SingleRowConstraints, 2> singleRowConstraints { {
                  { true, _rows, Point{0, 1}, _rowConstraints, _placedInRow },
                  { false, _cols, Point{1, 0}, _colConstraints, _placedInCol }
                }
            };

            for (const auto& src : singleRowConstraints) {
                for (int i = 1; i < src.max - 1; i++) {
                    if (src.constraints[i] - src.placed[i] == 1) {
                        // We can maybe place a piece in this row/col
                        // First, find the nearest piece in adjacent row/col
                        // which faces us where the row/col has only a
                        // single piece
                        Point pos{src.isRow ? 0 : i, src.isRow ? i : 0};
                        std::array<Point, 2> adjacent{ pos + src.iterator,
                            pos - src.iterator };
                        for (const auto& adj : adjacent) {
                            if (isInBounds(adj) && src.placed[ src.isRow ? adj.y : adj.x] == 1)
                            {
                                const auto toPlace = src.constraints[ src.isRow ? adj.y : adj.x] - src.placed[ src.isRow ? adj.y : adj.x];
                                // Now walk along the row/col until we find the piece
                                const Point step = src.isRow ? Point{1, 0} : Point{0, 1};
                                DEBUG_LOG(pos, adj, step);
                                Point pt = adj;
                                while (isInBounds(pt) && src.placed[ src.isRow ? pt.y : pt.x] == 1) {
                                    const auto p = at(pt);
                                    if (p == Piece::Empty) {
                                        pt += step;
                                        continue;
                                    }
                                    DEBUG_LOG(pt, p);
                                    // Get the connections for the piece which go perpendicular
                                    int dirs = 0;
                                    Point dir = Point::origin();
                                    for (const auto &d : Connections::GetConnections(p)) {
                                        if (step.x == 0 && d.x != 0 ||
                                            step.y == 0 && d.y != 0) {
                                            continue;
                                        }
                                        dir = d;
                                        dirs++;
                                    }
                                    DEBUG_LOG(pt, p, dirs);
                                    // If we don't have a single connection, just move on
                                    if (dirs != 1) {
                                        break;
                                    }

                                    // Now, we step in the direction of the piece, toPlace - 1 places
                                    // and place the horizontal/vertical piece
                                    for (int j = 1; j < toPlace; j++) {
                                        const Point t{pt + dir * j};
                                        const Piece toPlace = src.isRow ? Piece::Horizontal : Piece::Vertical;
                                        DEBUG_LOG(j, t, toPlace, at(t));
                                        if (!canPlace(t, toPlace)) {
                                            break;
                                        }
                                        place(t, toPlace);
                                    }
                                    // Now, place the corner
                                    // we need to know if it is a south or north corner
                                    auto placeAt = pt + dir * toPlace;
                                    const auto exit = dir.inverse();
                                    const auto entry = pos - adj;
                                    const auto corner = Connections::GetPiece(exit, entry);
                                    DEBUG_LOG(entry, exit, placeAt, corner);

                                    if(canPlace(placeAt, corner)) {
                                        place(placeAt, corner);
                                        // Now, place the piece
                                        placeAt = placeAt + entry;
                                        if (canPlace(placeAt, src.isRow ? Piece::Vertical : Piece::Horizontal)) {
                                            place(placeAt, src.isRow ? Piece::Vertical : Piece::Horizontal);
                                        }
                                    }

                                    break;
                                }
                            }
                        }
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
        int _fixedCount;
        int _totalCount;
        int _placedCount;
        bool _displayConstraints;
        bool _bold;

        Point _entry;
        Point _exit;
        
        std::vector<Piece> _grid;

        std::vector<int> _rowConstraints;
        std::vector<int> _colConstraints;

        std::vector<int> _placedInRow;
        std::vector<int> _placedInCol;
    };

    inline std::ostream& operator<<(std::ostream& os, const Grid& grid) {
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
                if (grid._bold && (pt == grid._entry || pt == grid._exit)) {
                    os << bold_on;
                }
                os << grid.at(pt);
                if (grid._displayConstraints) { os << " "; }
                if (grid._bold && (pt == grid._entry || pt == grid._exit)) {
                    os << bold_off;
                }
            }
            os << std::endl;
        }
        return os;
    }
}