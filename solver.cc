#include "solver.h"

namespace miner {

Solver::~Solver() {
    stop();
    if (thread_.joinable())
	thread_.join();
}

void Solver::startAsync() {
    I_ASSERT(
      state_ == RunState::kNew,
      EX_LOG("state==" << static_cast<int>(state_.load()) << " != kNew"));
    I_ASSERT(
      !thread_.joinable(),
      EX_LOG("thread is joinable"));
    
    state_ = RunState::kSuspended;
    thread_ = std::thread(&Solver::asyncSolver, this);
}


bool Solver::isRunning() const {
    return RunState::kRunning == state_
        or RunState::kSuspending == state_;
}


bool Solver::okToRun() {
    while(true) {
	switch(state_) {
	case RunState::kNew:
	case RunState::kSuspended: {
	    std::unique_lock<std::mutex> lck{mtx_};
	    cond_.wait(lck);
	    break;
	}
	    
	case RunState::kSuspending:
	    state_ = RunState::kSuspended;
	    break;
	    
	case RunState::kRunning:
	    return true;
	    
	case RunState::kExit:
	    return false;
	};
    }
}


void Solver::suspend() {
    I_ASSERT(state_ != RunState::kExit, EX_LOG("state == kExit"));
    std::lock_guard<std::mutex> lck(mtx_);
    state_ = RunState::kSuspending;
    cond_.notify_one();
}


void Solver::resume() {
    I_ASSERT(state_ != RunState::kExit, EX_LOG("state == kExit"));
    std::lock_guard<std::mutex> lck(mtx_);
    state_ = RunState::kRunning;
    cond_.notify_one();
}


void Solver::stop() {
    std::lock_guard<std::mutex> lck(mtx_);
    state_ = RunState::kExit;
    cond_.notify_one();
}


void Solver::addPoi(Location l) {
    std::lock_guard<std::mutex> lock{queue_mtx_};
    poi_.push_back(l);
}


Solver::NeighborhoodInfo Solver::getNeighborhoodInfo(Location l) const {
    NeighborhoodInfo rv;
    
    auto ci = board_->at(l);
    switch(ci) {
    case GameBoard::CellInfo::Exploded:
    case GameBoard::CellInfo::MarkedMine:
    case GameBoard::CellInfo::Unknown:
	I_FAIL("internal error: cell " << l << " is of type "
               << static_cast<int>(ci) << ": not a free open one");
	break;
	
    case GameBoard::CellInfo::N0:
    case GameBoard::CellInfo::N1:
    case GameBoard::CellInfo::N2:
    case GameBoard::CellInfo::N3:
    case GameBoard::CellInfo::N4:
    case GameBoard::CellInfo::N5:
    case GameBoard::CellInfo::N6:
    case GameBoard::CellInfo::N7:
    case GameBoard::CellInfo::N8:
	rv.mines_nr = static_cast<size_t>(ci);
	break;
    };
    
    {
	auto it = board_->neighborhood(l);
	while(it) {
	    switch(it.at()) {
	    case GameBoard::CellInfo::MarkedMine:
                --rv.mines_nr;
                break;
                
	    case GameBoard::CellInfo::Unknown:
		rv.coveredUnmarkedLocations[rv.nr++] = *it;
		break;
                
	    default:
		break;
	    }
	    
	    ++it;
	}
    }
    
    return rv;
}


void Solver::asyncSolver() {
    while(okToRun()) {
        Location poi;
        
        {
            std::unique_lock<std::mutex> lock{queue_mtx_};
            if (poi_.empty()) {
                lock.unlock();
                state_ = RunState::kSuspended;
                resultHandler_(FeedbackState::kSuspended, Location{}, 0);
                continue;
            }

            poi = poi_.front();
            poi_.pop_front();
        }
        
	if (!doPoi(poi)) {
	    state_ = RunState::kExit;
	    return;
	}
	
	resultHandler_(FeedbackState::kSolved, poi, kUpdateRange);
    }
}

} // namespace miner
