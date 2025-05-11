#pragma once

#include "Grid.h"

#include <memory>

namespace TrainTracks
{
    class ProgressReporter {
    public:
        ProgressReporter()
            : ProgressReporter(100000)
        { }

        ProgressReporter(uint64_t interval)
            : interval(interval)
        { }

        ~ProgressReporter() { }

        virtual void Report(uint64_t steps, const Point& pos) = 0;

        uint64_t interval;
    };

    class Solver {
    public:
        virtual bool Solve(Grid& grid) = 0;

        void Reporter(ProgressReporter *reporter) {
            _reporter = reporter;
        }

        uint64_t Steps() const {
            return _steps;
        }

    protected:
        Solver()
            : _steps(0)
            , _reporter(nullptr)
        { }

        void Step(const Point& pos) {
            _steps++;
            if (_reporter) {
                if (pos == Point(1, 2) || _steps % _reporter->interval == 0) {
                    _reporter->Report(_steps, pos);
                }
            }
        }
        ProgressReporter* _reporter;
        uint64_t _steps;
    };
} // namespace TrainTracks
