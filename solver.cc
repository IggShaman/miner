#include "lp_problem.h"
#include "solver.h"

namespace miner {

solver::~solver() {
    stop();
    if ( thread_.joinable() )
	thread_.join();
}


void solver::start_async() {
    I_ASSERT(state_ == state::kNew, EX_LOG("state==" << static_cast<int>(state_.load()) << " != kNew"));
    I_ASSERT(!thread_.joinable(), EX_LOG("thread is joinable"));
    
    state_ = state::kSuspended;
    thread_ = std::thread(&solver::async_solver, this);
}


bool solver::ok_to_run() {
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


void solver::suspend() {
    I_ASSERT(state_ != state::kExit, EX_LOG("state == kExit"));
    std::lock_guard<std::mutex> lck(mtx_);
    state_ = state::kSuspending;
    cond_.notify_one();
}


void solver::resume() {
    I_ASSERT(state_ != state::kExit, EX_LOG("state == kExit"));
    std::lock_guard<std::mutex> lck(mtx_);
    state_ = state::kRunning;
    cond_.notify_one();
}


void solver::stop() {
    std::lock_guard<std::mutex> lck(mtx_);
    state_ = state::kExit;
    cond_.notify_one();
}


void solver::add_poi ( coord c ) {
    I_ASSERT(!is_running(), EX_LOG("tried to add POI while solver is running"));
    poi_.push_back(c);
}


solver::unknown_neighbors solver::get_unknowns ( coord c ) const {
    unknown_neighbors rv;
    
    auto ci = board_->at(c);
    switch(ci) {
    case board::cellinfo::boom_mine:
    case board::cellinfo::marked_mine:
    case board::cellinfo::unknown:
	I_FAIL("internal error: cell (" << c.row << ' ' << c.col << ") is of type " << static_cast<int>(ci) << ": not a free open one");
	break;
	
    case board::cellinfo::n0:
    case board::cellinfo::n1:
    case board::cellinfo::n2:
    case board::cellinfo::n3:
    case board::cellinfo::n4:
    case board::cellinfo::n5:
    case board::cellinfo::n6:
    case board::cellinfo::n7:
    case board::cellinfo::n8:
	rv.mines_nr = static_cast<size_t>(ci);
	break;
    };
    
    {
	auto it = board_->neighborhood(c);
	while(it) {
	    switch(it.at()) {
	    case board::cellinfo::marked_mine:
		--rv.mines_nr;
		break;
		
	    case board::cellinfo::unknown:
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
    lp_row_info ( uint8_t v, std::string n ) : fixed_value{v}, name{n} {}
    
    uint8_t fixed_value;
    std::string name;
};


void solver::async_solver() {
    while(ok_to_run()) {
	if ( poi_.empty() ) {
	    state_ = state::kSuspended;
	    result_handler_(feedback::kSuspended, coord{}, 0);
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


bool solver::do_poi ( miner::coord poi ) {
    if ( board_->is_ok(poi) ) {
	auto poi_u = get_unknowns(poi);
	if ( !poi_u.nr )
	    return true;
    }
    
    std::unique_ptr<lp::problem> lp(new lp::problem);
    vars_map_type vars; // a set of coords current LP is looking at; maps coord to LP's column variable number
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
	    if ( board_->field()->is_mine(v.first) ) {
		xlog << "ERROR: game is lost at " << v.first << ": shold've been empty, has a mine"
		     << "\nobj=" << obj
		     << "\npoi=" << poi
		     << "\nLP: " << lp->dump() << "\n";
		board_->dump_region(poi, kRange);
		result_handler_(feedback::kGameLost, coord{}, 0);
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
		if ( !board_->field()->is_mine(v.first) ) {
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


void solver::prepare ( lp::problem* lp, coord poi, vars_map_type& vars ) {
    std::ostringstream oss;
    
    int vars_nr{};
    
    //
    // setup initial LP
    //
    std::vector<lp_row_info> rows; // used to set row names and constraints
    lp::matrix m;
    
    for(int row = std::max(0, poi.row - kRange); row <= std::min(board_->rows() - 1, static_cast<int>(poi.row) + kRange); ++row) {
	for(int col = std::max(0, static_cast<int>(poi.col) - kRange); col <= std::min(static_cast<int>(board_->cols()) - 1, static_cast<int>(poi.col) + kRange); ++col) {
	    coord c{row, col};
	    auto ci = board_->at(c);
	    if ( static_cast<int>(ci) < 0 )
		continue;
	    
	    auto u = get_unknowns(c);
	    if ( !u.nr )
		continue;
	    
	    oss.str("");
	    oss << 'n' << c;
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
