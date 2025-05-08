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

        virtual void Report(uint64_t steps) = 0;

        uint64_t interval;
    };

    class Solver {
    public:
        virtual bool Solve(Grid& grid) = 0;

        void Reporter(ProgressReporter *reporter) {
            _reporter = reporter;
        }
    protected:
        Solver()
            : _steps(0)
        { }

        void Step() {
            _steps++;
            if (_reporter) {
                if (_steps % _reporter->interval == 0) {
                    _reporter->Report(_steps);
                }
            }
        }
        ProgressReporter* _reporter;
        uint64_t _steps;
    };
} // namespace TrainTracks
