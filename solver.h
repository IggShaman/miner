#ifndef __MINER_SOLVER_H_
#define __MINER_SOLVER_H_

#include "board.h"

namespace lp { class problem; }

namespace miner {

class solver {
public:
    enum class cellinfo : int8_t {
	has_bomb,
	clear,
	undecided,
    };
    
    explicit solver ( board_ptr );
    ~solver();
    
    void add_uncovered_cell ( coord );
    void build_problem();
    cellinfo check_cell ( coord );
    
private:
    struct unknown_neighbors {
	uint8_t nr{}; // number of neighbors, in "coords" array
	uint8_t mines_nr{}; // number of mines left
	std::array<coord, 8> coords; // cells which are not uncovered and not marked
    };
    unknown_neighbors get_unknowns ( coord );
    
    board_ptr board_;
    std::vector<coord> frontier_; // list of frontier cells - the ones which are open and supposedly have unopened neighbors
    //std::unordered_map<coord, size_t> outer_; // maps (row,col) => LP's column variable number
    lp::problem* lp_{};
};

using solver_ptr = std::shared_ptr<solver>;

} // namespace miner

#endif // __MINER_SOLVER_H_
