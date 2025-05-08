#pragma once

#include "Piece.h"
#include "Point.h"
#include "Puzzle.h"
#include "Connections.h"

#include <vector>
#include <queue>

#include "Log.h"

namespace TrainTracks {

    class Grid {
    private:
        Grid(int rows, int cols)
            : _rows(rows)
            , _cols(cols)
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
                    const auto piece = p.data.startingGrid[pt.transpose(_cols)];
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
            return _grid[p.transpose(_cols)];
        }

        Piece at(const Point& p) const {
            return _grid[p.transpose(_cols)];
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
                pt.x == _cols - 1 ||
                pt.y == _rows - 1;
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
                    if (pt.x == 0 || pt.x == _cols - 1) { return false; }
                    break;
                case Piece::Vertical:
                    if (pt.y == 0 || pt.y == _rows - 1) { return false; }
                    break;
                case Piece::CornerNW:
                    if (pt.y == 0 || pt.x == 0) { return false; }
                    break;
                case Piece::CornerNE:
                    if (pt.y == 0 || pt.x == _cols - 1) { return false; }
                    break;
                case Piece::CornerSE:
                    if (pt.y == _rows - 1 || pt.x == 0) { return false; }
                    break;
                case Piece::CornerSW:
                    if (pt.y == _rows - 1 || pt.x == 0) { return false; }
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
            visited[first.transpose(_cols)] = true;
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
                        !visited[next.transpose(_cols)])
                    {
                        const auto p = at(pt);
                        const auto n = at(next);
                        if (Connections::ConnectsTo(p, d) && Connections::ConnectsTo(n, d.inverse())) {
                            visited[next.transpose(_cols)] = true;
                            dfs.emplace(std::move(next));
                        }
                    }
                }
            }
        
            // Ensure all non-empty cells are visited
            for (int y = 0; y < _rows; y++) {
                for (int x = 0; x < _cols; x++) {
                    const Point p{x, y};
                    if (!isEmpty(p) && !visited[p.transpose(_cols)]) {
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
    private:

        void extractEntryAndExit() {
            std::array<Point, 4>corners = { Point{0, 0}, Point{0, _rows -1 },
                Point{_cols -1, 0}, Point{_cols - 1, _rows - 1}};
            
            std::vector<Point>exits;
            for (const auto& pt : corners) {
                int offDirs = getOffCountForPieceAt(pt);
                if (offDirs == 1) {
                    exits.emplace_back(pt);
                }
            }
            
            for (int x = 1; x < _cols - 1; x++) {
                int offDirs = getOffCountForPieceAt({x, 0});
                if (offDirs == 1) {
                    exits.emplace_back(x, 0);
                }
                offDirs = getOffCountForPieceAt({x, _rows - 1});
                if (offDirs == 1) {
                    exits.emplace_back(x, _rows - 1);
                }
            }

            for (int y = 1; y < _rows - 1; y++) {
                int offDirs = getOffCountForPieceAt({0, y});
                if (offDirs == 1) {
                    exits.emplace_back(0, y);
                }
                offDirs = getOffCountForPieceAt({_cols -1 , y});
                if (offDirs == 1) {
                    exits.emplace_back(_cols -1 , y);
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
                os << PieceSymbol(grid.at(pt));
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