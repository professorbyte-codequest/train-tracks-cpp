#pragma once

#include <iostream>
#include "Grid.h"
#include "Solver.h"

namespace TrainTracks {

class ConsoleReporter
    : public ProgressReporter
{
public:
    ConsoleReporter(Grid& grid, uint64_t interval = 100000)
        : ProgressReporter(interval)
        , grid(grid)
    {
    }

    void Report(uint64_t steps, const Point& pos) override {
        std::cout << reset << std::endl;
        std::cout << grid << std::endl;
        std::cout << "Steps: " << steps << std::endl;
        std::cout << "Current: " << pos << std::endl;
    }
private:
    Grid& grid;
};
}
