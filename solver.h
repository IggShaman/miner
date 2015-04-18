#ifndef __MINER_SOLVER_H_
#define __MINER_SOLVER_H_

#include "board.h"

namespace lp { class problem; }

namespace miner {

class solver {
public:
    explicit solver ( board_ptr b ) : board_{b} {}
    ~solver();
    
    void add_new_uncovered ( coord c ) { frontier_.push_back(c); got_new_uncovered_ = true; }
    std::pair<bool, coord> current_cell();       // returns true if there is a current cell; false means evetything is solved or need input
    std::pair<bool, coord> solve_current_cell(); // returns cell coords and true if cell was solved; false otherwise
    
private:
    void prepare();
    struct unknown_neighbors {
	uint8_t nr{}; // number of neighbors, in "coords" array
	uint8_t mines_nr{}; // number of mines left
	std::array<coord, 8> coords; // cells which are not uncovered and not marked
    };
    unknown_neighbors get_unknowns ( coord );
    
    board_ptr board_;
    std::vector<coord> frontier_; // list of new frontier cells - the ones which are open and supposedly have unopened neighbors
    
    // current instance of LP solver
    lp::problem* lp_{};
    using vars_map_type = std::unordered_map<coord, size_t>;
    vars_map_type vars_; // a set of coords current LP is looking at; maps coord to LP's column variable number
    vars_map_type::iterator var_it_; // used by interface functions
    size_t solved_nr_{}; // how many variables got solved during current run; if run completes and none were solved, we will need more input
    bool got_new_uncovered_{}; // true if new uncovered cell(s) were added since the last time LP was constructed
};

} // namespace miner

#endif // __MINER_SOLVER_H_
