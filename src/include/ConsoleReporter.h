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
        //std::cout << reset;
        std::cout << grid << std::endl;
        std::cout << "Steps: " << steps << std::endl;
        std::cout << "Current: " << pos << std::endl;

        if (pos == Point(1, 2)) {
            std::cout << "Press Enter to continue..." << std::endl;
            std::cin.get();
        } else {
            std::cout << "                          " << std::endl;
        }
    }
private:
    Grid& grid;
};
}
