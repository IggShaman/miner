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
    
    using ResultHandler = std::function<void(FeedbackState,Location,size_t)>;
    
    explicit Solver(GameBoardPtr board) : board_{board} {}
    virtual ~Solver();
    
    bool isRunning() const;
    void startAsync();
    void suspend();
    void resume();
    void stop();
    void addPoi(Location);
    void setResultHandler(ResultHandler h) { resultHandler_ = h; }
    
protected:
    virtual bool doPoi(miner::Location) = 0;
    
    struct NeighborhoodInfo {
        uint8_t mines_nr{}; // number of mines left around current cell
        uint8_t nr{};       // size of "coveredUnmarkedLocations" array
        std::array<Location, 8> coveredUnmarkedLocations;
    };
    NeighborhoodInfo getNeighborhoodInfo(Location) const;
    
    GameBoardPtr board_;
    ResultHandler resultHandler_;
    
private:
    bool okToRun();
    void asyncSolver();
    
    std::atomic<RunState> state_{RunState::kNew};

    std::mutex queue_mtx_;     // used to protect queue access
    std::deque<Location> poi_; // a list of cells of interest
    
    std::thread thread_;
    std::mutex mtx_;
    std::condition_variable cond_;
};

} // namespace miner
