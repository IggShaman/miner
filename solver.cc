#include "lp_problem.h"
#include "solver.h"

namespace miner {

solver::~solver() {
    if ( lp_ )
	delete lp_;
}


solver::unknown_neighbors solver::get_unknowns ( coord c ) {
    unknown_neighbors rv;
    
    auto ci = board_->at(c);
    switch(ci) {
    case board::cellinfo::boom_mine:
    case board::cellinfo::marked_mine:
    case board::cellinfo::unknown:
	EX_LOG("internal error: cell (" << c.row << ' ' << c.col << ") is of type " << static_cast<int>(ci) << ": not a free open one");
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


void solver::prepare() {
    if ( lp_ )
	delete lp_;
    lp_ = new lp::problem();
    solved_nr_ = 0;
    have_new_info_ = false;
    
    std::ostringstream oss;
    
    size_t vars_nr{};
    vars_.clear();
    std::vector<lp_row_info> rows; // used to set row names and constraints
    lp::matrix m;
    
    decltype(frontier_) old_frontier;
    std::swap(frontier_, old_frontier);
    
    //xlog << "frontier size: " << old_frontier.size() << "\n";
    
    for(auto c: old_frontier) {
	//xlog << "looing at frontier " << c << "\n";
	auto u = get_unknowns(c);
	//xlog << "has " << (int)u.nr << " unknown neighbors, " << (int)u.mines_nr << " mine(s)\n";
	
	if ( !u.nr ) // no longer a frontier
	    continue;
	
	//for(uint8_t i = 0; i < u.nr; ++i) {
	//xlog << "  at " << u.coords[i] << "\n";
	//}
	
	frontier_.push_back(c); // this is an old frontier, don't set "have_new_info_" to true
	
	oss.str("");
	oss << 'n' << c;
	rows.push_back({u.mines_nr, oss.str()});
	
	for(uint8_t i = 0; i < u.nr; ++i) {
	    // find/add column variable for an uncovered cell
	    auto iv = vars_.insert({u.coords[i], vars_nr + 1});
	    if ( iv.second )
		++vars_nr;
	    
	    // set coefficient to 1
	    m.add(rows.size(), iv.first->second, 1);
	}
    }
    
    var_it_ = vars_.begin();
    if ( !vars_nr )
	return;
    
    //
    // add variables, populate constraints
    //
    
    // add rows
    lp_->add_row_variables(rows.size());
    for(size_t row = 0; row < rows.size(); ++row) {
	lp_->set_row_name(row + 1, rows[row].name.data());
	lp_->set_row_fixed_bound(row + 1, rows[row].fixed_value);
    }
    
    // add cols
    lp_->add_column_variables(vars_nr);
    for(auto v: vars_) {
	oss.str("");
	oss << 'u' << v.first;
	lp_->set_column_name(v.second, oss.str().data());
	lp_->set_column_bounded(v.second, 0, 1);
    }
    
    lp_->set_matrix(m);
    
    //std::cout << lp_->dump() << "\n";
    I_ASSERT(lp_->presolve(), EX_LOG("could not presolve LP"));
}


std::pair<bool, coord> solver::current_cell() {
    if ( var_it_ == vars_.end() ) {
	if ( !have_new_info_ )
	    return {false, {}};
	
	prepare();
    }
    
    if ( var_it_ == vars_.end() ) // still no variables
	return {false, {}};
    
    return {true, var_it_->first};
}


solver::solve_info solver::solve_current_cell() {
    I_ASSERT(var_it_ != vars_.end(), EX_LOG("don't have a var to look at"));
    
    lp_->set_objective_coefficient(var_it_->second, 1);
    lp_->set_maximize();
    lp_->solve();
    
    solve_info rv;
    rv.cell_at = var_it_->first;
    if ( lp_->get_objective_value() == 0 ) {
	// can't have a mine here
	if ( board_->field()->is_mine(var_it_->first) ) {
	    rv.game_was_lost = true;
	    return rv;
	}
	
	board_->uncovered_safe(var_it_->first, board_->field()->nearby_mines_nr(var_it_->first));
	add_new_uncovered(var_it_->first);
	++solved_nr_;
	rv.was_solved = true;
	
    } else {
	lp_->set_minimize();
	lp_->solve();
	auto v = lp_->get_objective_value();
	if ( v > 0 ) {
	    // must have a mine here
	    board_->mark_mine(var_it_->first, true);
	    ++solved_nr_;
	    rv.was_solved = true;
	}
    }
    
    lp_->set_objective_coefficient(var_it_->second, 0);
    ++var_it_;
    return rv;
}

} // namespace miner
