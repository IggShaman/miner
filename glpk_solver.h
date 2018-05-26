#pragma once

#include "board.h"

namespace lp { class problem; }

namespace miner {

class GlpkSolver {
    enum class state : uint8_t {
	kNew,
	kRunning,
	kSuspending,
	kSuspended,
	kExit,
    };
    
public:
    static constexpr float kEpsilon = 1e-3;
    
    explicit GlpkSolver(GameBoardPtr board)
        : state_{state::kNew}, board_{board} {}
    ~GlpkSolver();
    
    enum feedback : uint8_t {
	kSolved,
	kSuspended,
	kGameLost
    };
    using result_handler = std::function<void(feedback,Location,int)>;
    
    bool is_running() const { return state::kRunning == state_ or state::kSuspending == state_; }
    void start_async();
    void suspend();
    void resume();
    void stop();
    void add_poi(Location);
    void set_result_handler(result_handler h) { result_handler_ = h; }
    
private:
    static constexpr int kRange = 7;
    
    bool ok_to_run();
    void async_solver();
    bool do_poi(miner::Location);
    using vars_map_type = std::unordered_map<Location, int>;
    void prepare(lp::problem*, miner::Location, vars_map_type&);
    
    struct unknown_neighbors {
	uint8_t nr{};                   // number of neighbors, in "coords" array
	uint8_t mines_nr{};             // number of mines left
	std::array<Location, 8> coords; // covered unmarked cells
    };
    unknown_neighbors get_unknowns(Location) const;
    
    std::atomic<state> state_;
    std::thread thread_;
    std::mutex mtx_;
    std::condition_variable cond_;
    
    result_handler result_handler_;
    GameBoardPtr board_;
    std::deque<Location> poi_; // a list of cells of interest
};

} // namespace miner
