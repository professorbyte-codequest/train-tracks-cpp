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

    void Report(uint64_t steps) {
        std::cout << reset;
        std::cout << grid << std::endl;
        std::cout << "Steps: " << steps << std::endl;
    }
private:
    Grid& grid;
};
}
