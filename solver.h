#ifndef __MINER_SOLVER_H_
#define __MINER_SOLVER_H_

#include "board.h"

namespace lp { class problem; }

namespace miner {

class solver {
    enum class state : uint8_t {
	kNew,
	kRunning,
	kSuspending,
	kSuspended,
	kExit,
    };
    
public:
    explicit solver ( board_ptr b ) : state_{state::kNew}, board_{b} {}
    ~solver();
    
    enum feedback : uint8_t {
	kSolved,
	kSuspended,
	kGameLost
    };
    using result_handler = std::function<void(feedback,coord,int)>;
    
    bool is_running() const { return state::kRunning == state_ or state::kSuspending == state_; }
    void start_async();
    void suspend();
    void resume();
    void stop();
    void add_poi ( coord c );
    void set_result_handler ( result_handler h ) { result_handler_ = h; }
    
private:
    static constexpr int kRange = 7;
    
    bool ok_to_run();
    void async_solver();
    bool do_poi ( miner::coord );
    using vars_map_type = std::unordered_map<coord, int>;
    void prepare ( lp::problem*, miner::coord, vars_map_type& );
    
    struct unknown_neighbors {
	uint8_t nr{};                // number of neighbors, in "coords" array
	uint8_t mines_nr{};          // number of mines left
	std::array<coord, 8> coords; // cells which are not uncovered and not marked
    };
    unknown_neighbors get_unknowns ( coord ) const;
    
    std::atomic<state> state_;
    std::thread thread_;
    std::mutex mtx_;
    std::condition_variable cond_;
    
    result_handler result_handler_;
    board_ptr board_;
    std::deque<coord> poi_; // a list of cells of interest
};

} // namespace miner

#endif // __MINER_SOLVER_H_
