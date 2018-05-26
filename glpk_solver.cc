#include "lp_problem.h"
#include "glpk_solver.h"

namespace miner {

GlpkSolver::~GlpkSolver() {
    stop();
    if ( thread_.joinable() )
	thread_.join();
}


void GlpkSolver::start_async() {
    I_ASSERT(state_ == state::kNew, EX_LOG("state==" << static_cast<int>(state_.load()) << " != kNew"));
    I_ASSERT(!thread_.joinable(), EX_LOG("thread is joinable"));
    
    state_ = state::kSuspended;
    thread_ = std::thread(&GlpkSolver::async_solver, this);
}


bool GlpkSolver::ok_to_run() {
    while(true) {
	switch(state_) {
	case state::kNew:
	case state::kSuspended: {
	    std::unique_lock<std::mutex> lck(mtx_);
	    cond_.wait(lck);
	    break;
	}
	    
	case state::kSuspending:
	    state_ = state::kSuspended;
	    break;
	    
	case state::kRunning:
	    return true;
	    
	case state::kExit:
	    return false;
	};
    }
}


void GlpkSolver::suspend() {
    I_ASSERT(state_ != state::kExit, EX_LOG("state == kExit"));
    std::lock_guard<std::mutex> lck(mtx_);
    state_ = state::kSuspending;
    cond_.notify_one();
}


void GlpkSolver::resume() {
    I_ASSERT(state_ != state::kExit, EX_LOG("state == kExit"));
    std::lock_guard<std::mutex> lck(mtx_);
    state_ = state::kRunning;
    cond_.notify_one();
}


void GlpkSolver::stop() {
    std::lock_guard<std::mutex> lck(mtx_);
    state_ = state::kExit;
    cond_.notify_one();
}


void GlpkSolver::add_poi(Location l) {
    I_ASSERT(!is_running(), EX_LOG("tried to add POI while solver is running"));
    poi_.push_back(l);
}


GlpkSolver::unknown_neighbors GlpkSolver::get_unknowns(Location l) const {
    unknown_neighbors rv;
    
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
		rv.coords[rv.nr++] = *it;
		break;
                
	    default:
		break;
	    }
	    
	    ++it;
	}
    }
    
    return rv;
}


struct lp_row_info {
    lp_row_info(uint8_t v, std::string n)
        : fixed_value{v}, name{n} {}
    
    uint8_t fixed_value;
    std::string name;
};


void GlpkSolver::async_solver() {
    while(ok_to_run()) {
	if ( poi_.empty() ) {
	    state_ = state::kSuspended;
	    result_handler_(feedback::kSuspended, Location{}, 0);
	    continue;
	}
	
	if ( !do_poi(poi_.front()) ) {
	    poi_.pop_front();
	    state_ = state::kExit;
	    return;
	}
	
	result_handler_(feedback::kSolved, poi_.front(), kRange);
	poi_.pop_front();
    }
}


bool GlpkSolver::do_poi(miner::Location poi) {
    if ( board_->is_ok(poi) ) {
	auto poi_u = get_unknowns(poi);
	if ( !poi_u.nr )
	    return true;
    }
    
    std::unique_ptr<lp::problem> lp(new lp::problem);
    // a set of coords current LP is looking at; maps coord to LP's column variable number
    vars_map_type vars;
    prepare(lp.get(), poi, vars);
    if ( vars.empty() )
	return true;
    
    for(auto& v: vars) {
	lp->set_objective_coefficient(v.second, 1);
	lp->set_maximize();
	lp->solve();
	
	auto obj = lp->get_objective_value();
	if ( obj < 1e-10 ) {
	    // can't have a mine here
	    if ( board_->field()->is_mined(v.first) ) {
		xlog << "ERROR: game is lost at " << v.first << ": shold've been empty, has a mine"
		     << "\nobj=" << obj
		     << "\npoi=" << poi
		     << "\nLP: " << lp->dump() << "\n";
		board_->dump_region(poi, kRange);
		result_handler_(feedback::kGameLost, Location{}, 0);
		return false;
	    }
	    
	    board_->uncovered_safe(v.first, board_->field()->nearby_mines_nr(v.first));
	    lp->set_column_fixed_bound(v.second, 0);
	    poi_.push_back(v.first);
	    //result_handler_(feedback::kSolved, v.first);
	    
	} else {
	    lp->set_minimize();
	    lp->solve();
	    auto obj = lp->get_objective_value();
	    if ( obj >= 0.1 ) { // must have a mine here
		if ( !board_->field()->is_mined(v.first) ) {
		    xlog << "ERROR: calculated " << v.first << " to contain a mine, but it doesn't"
			 << "\nobj=" << obj
			 << "\npoi=" << poi
			 << "\nLP: " << lp->dump() << "\n";
		    board_->dump_region(poi, kRange);
		    return false;
		}
		
		board_->mark_mine(v.first, true);
		lp->set_column_fixed_bound(v.second, 1);
		poi_.push_back(v.first);
		//result_handler_(feedback::kSolved, v.first);
	    }
	}
	
	lp->set_objective_coefficient(v.second, 0);
    }
    
    return true;
}


void GlpkSolver::prepare(lp::problem* lp, Location poi, vars_map_type& vars) {
    std::ostringstream oss;
    
    int vars_nr{};
    
    //
    // setup initial LP
    //
    std::vector<lp_row_info> rows; // used to set row names and constraints
    lp::matrix m;
    
    for(size_t row = poi.row > kRange ? poi.row - kRange : 0;
        row <= std::min(board_->rows() - 1, poi.row + kRange);
        ++row) {
        
        for(size_t col = poi.col > kRange ? poi.col - kRange : 0;
            col <= std::min(board_->cols() - 1, poi.col + kRange);
            ++col) {
            
	    Location l{row, col};
	    auto ci = board_->at(l);
	    if ( static_cast<int>(ci) < 0 )
		continue;
	    
	    auto u = get_unknowns(l);
	    if ( !u.nr )
		continue;
	    
	    oss.str("");
	    oss << 'n' << l;
	    rows.push_back({u.mines_nr, oss.str()});
	    
	    for(uint8_t i = 0; i < u.nr; ++i) {
		// find/add column variable for an uncovered cell
		auto iv = vars.insert({u.coords[i], vars_nr + 1});
		if ( iv.second )
		    ++vars_nr;
		
		// set coefficient to 1
		m.add(rows.size(), iv.first->second, 1);
	    }
	}
    }
    
    if ( !vars_nr )
	return;
    
    //
    // add variables, populate constraints
    //
    
    // add rows
    lp->add_row_variables(rows.size());
    for(size_t row = 0; row < rows.size(); ++row) {
	lp->set_row_name(row + 1, rows[row].name.data());
	lp->set_row_fixed_bound(row + 1, rows[row].fixed_value);
    }
    
    // add cols
    lp->add_column_variables(vars.size());
    for(auto v: vars) {
	oss.str("");
	oss << 'u' << v.first;
	lp->set_column_name(v.second, oss.str().data());
	lp->set_column_bounded(v.second, 0, 1);
    }
    
    lp->set_matrix(m);
    
    //std::cout << lp->dump() << "\n";
    //I_ASSERT(lp->presolve(), EX_LOG("could not presolve LP"));
    if ( !lp->presolve() ) {
	errlog << "ERROR: could not presolve: " << lp->last_errmsg()
	       << "\npoi=" << poi
	       << "\nLP: " << lp->dump() << "\n";
	board_->dump_region(poi, kRange);
	exit(-1);
    }
}

} // namespace miner
