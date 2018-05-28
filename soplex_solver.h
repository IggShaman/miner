#pragma once

#include "solver.h"

namespace miner {

class SoplexSolver : public Solver {
public:
    static constexpr float kEpsilon = 1e-3;
    static constexpr size_t kRange = 7;
    using Solver::Solver;
    
private:
    bool doPoi(Location) override;
};

} // namespace miner
