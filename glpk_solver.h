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
    using vars_map_type = std::unordered_map<Location, int>;
    void prepare(lp::problem*, miner::Location, vars_map_type&);
    bool do_poi(miner::Location) override;
};

} // namespace miner
