#pragma once

#include "solver.h"
#include "board.h"

namespace lp { class problem; }

namespace miner {

class GlpkSolver : public Solver {
public:
    static constexpr float kEpsilon = 1e-3;
    static constexpr size_t kRange = 7;
    using Solver::Solver;
    
private:
    // maps location to variable id in an LP
    using VariablesMapType = std::unordered_map<Location, int>;
    
    void prepare(lp::problem*, miner::Location, VariablesMapType&);
    bool doPoi(miner::Location) override;
};

} // namespace miner
