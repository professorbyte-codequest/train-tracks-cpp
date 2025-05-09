#pragma once

#include <list>
#include <algorithm>
#include <unordered_set>
#include <iostream>

#include "Hash.h"
#include "Utils.h"

namespace TrainTracks {
    struct Point {
        int32_t x{0};
        int32_t y{0};

        constexpr inline Point() noexcept = default;

        constexpr inline Point(int32_t x, int32_t y) noexcept
            : x{x}
            , y{y}
        { }

        constexpr inline Point(Point const&) noexcept = default;
        constexpr inline Point(Point &&) noexcept = default;
        constexpr inline Point &operator=(Point const&) noexcept = default;
        constexpr inline Point &operator=(Point &&) noexcept = default;

        constexpr inline Point &operator+=(Point const& o) noexcept {
            x += o.x;
            y += o.y;
            return *this;
        }

        constexpr inline Point &operator-=(Point const& o) noexcept {
            x -= o.x;
            y -= o.y;
            return *this;
        }

        constexpr inline Point &operator*=(int32_t v) noexcept {
            x *= v;
            y *= v;
            return *this;
        }

        constexpr inline Point &operator/=(int32_t v) noexcept {
            x /= v;
            y /= v;
            return *this;
        }

        constexpr inline Point operator+(Point const& o) const noexcept {
            return Point{x + o.x, y + o.y};
        }

        constexpr inline Point operator-(Point const& o) const noexcept {
            return Point{x - o.x, y - o.y};
        }

        constexpr inline Point operator*(int32_t v) const noexcept {
            return Point{x * v, y * v};
        }

        constexpr inline Point operator/(int32_t v) const noexcept {
            return Point{x / v, y / v};
        }

        friend constexpr inline bool operator==(TrainTracks::Point const& l, TrainTracks::Point const& r);
        friend constexpr inline bool operator!=(TrainTracks::Point const& l, TrainTracks::Point const& r);
        friend constexpr inline bool operator<(TrainTracks::Point const& l, TrainTracks::Point const& r);
        friend constexpr inline bool operator>(TrainTracks::Point const& l, TrainTracks::Point const& r);
        friend constexpr inline bool operator<=(TrainTracks::Point const& l, TrainTracks::Point const& r);
        friend constexpr inline bool operator>=(TrainTracks::Point const& l, TrainTracks::Point const& r);
        friend constexpr inline bool operator>=(TrainTracks::Point const& l, TrainTracks::Point const& r);
        friend constexpr inline int cmp(TrainTracks::Point const& l, TrainTracks::Point const& r);
        friend inline std::ostream& operator<<(std::ostream& os, TrainTracks::Point const &p);

        constexpr inline Point sgn() const noexcept {
            return Point{TrainTracks::sgn(x), TrainTracks::sgn(y)};
        }

        inline int32_t abs() const noexcept {
            return std::abs(x) + std::abs(y);
        }

        inline int32_t manhattan(Point const& o) const noexcept {
            return (*this - o).abs();
        }

        inline int64_t project(int32_t w) const noexcept {
            return y * w + x;
        }

        constexpr inline Point inverse() const noexcept {
            return {-x, -y};
        }

        static constexpr inline Point right() noexcept {
            return {1, 0};
        }

        static constexpr inline Point up() noexcept {
            return {0, -1};
        }

        static constexpr inline Point left() noexcept {
            return {-1, 0};
        }

        static constexpr inline Point down() noexcept {
            return {0, 1};
        }

        static constexpr inline Point origin() noexcept {
            return {0, 0};
        }
    };

    struct PointHasher {
        std::size_t operator()(const Point& p) const {
            return hash_val(p.x, p.y);
        }
    };

    using PointPair = std::pair<Point, Point>;
    using PointList = std::list<Point>;
    using PointBoolMap = std::unordered_map<Point, bool, PointHasher>;
    using PointSet = std::unordered_set<Point, PointHasher>;

    // Same comparison semantics as a std::pair
    constexpr inline bool operator<(TrainTracks::Point const& l, TrainTracks::Point const& r) {
        return l.x < r.x ? true :
            (r.x < l.x ? false :
            (l.y < r.y ? true : false));
    }
    constexpr inline bool operator<=(TrainTracks::Point const& l, TrainTracks::Point const& r) {
        return !(r < l);
    }
    constexpr inline bool operator>(TrainTracks::Point const& l, TrainTracks::Point const& r) {
        return r < l;
    }
    constexpr inline bool operator>=(TrainTracks::Point const& l, TrainTracks::Point const& r) {
        return !(l < r);
    }
    constexpr inline bool operator==(TrainTracks::Point const& l, TrainTracks::Point const& r) {
        return l.x == r.x && l.y == r.y;
    }
    constexpr inline bool operator!=(TrainTracks::Point const& l, TrainTracks::Point const& r) {
        return l.x != r.x || l.y != r.y;
    }
    constexpr inline int cmp(TrainTracks::Point const& l, TrainTracks::Point const& r) {
        return l < r ? -1 : // less
            (r < l ? 1 :    // greater
            0);             // equivalent
    }
    inline std::ostream& operator<<(std::ostream& os, TrainTracks::Point const &p) {
        os << "{" << p.x << "," << p.y << "}";
        return os;
    }
}
