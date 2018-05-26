#pragma once

#include "board.h"

namespace miner {

class Solver {
    static constexpr const size_t kUpdateRange = 1;
    enum class RunState : uint8_t {
	kNew,
	kRunning,
	kSuspending,
	kSuspended,
	kExit,
    };
    
public:
    enum FeedbackState : uint8_t {
	kSolved,
	kSuspended,
	kGameLost
    };
    
    using result_handler = std::function<void(FeedbackState,Location,size_t)>;
    
    explicit Solver(GameBoardPtr board) : board_{board} {}
    virtual ~Solver();
    
    bool is_running() const;
    void start_async();
    void suspend();
    void resume();
    void stop();
    void add_poi(Location);
    void set_result_handler(result_handler h) { result_handler_ = h; }
    
protected:
    virtual bool do_poi(miner::Location) = 0;
    
    struct unknown_neighbors {
        uint8_t nr{};                   // number of neighbors, in "coords" array
        uint8_t mines_nr{};             // number of mines left
        std::array<Location, 8> coords; // covered unmarked cells
    };
    unknown_neighbors get_unknowns(Location) const;
    
    GameBoardPtr board_;
    result_handler result_handler_;
    
private:
    bool ok_to_run();
    void async_solver();
    
    std::atomic<RunState> state_{RunState::kNew};

    std::mutex queue_mtx_;     // used to protect queue access
    std::deque<Location> poi_; // a list of cells of interest
    
    std::thread thread_;
    std::mutex mtx_;
    std::condition_variable cond_;
};

} // namespace miner
