#include "lp_problem.h"
#include "solver.h"

namespace miner {

solver::solver ( board_ptr b ) : board_{b} {
}


solver::~solver() {
    if ( lp_ )
	delete lp_;
}


void solver::add_uncovered_cell ( coord c ) {
    frontier_.emplace_back(c);
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


void solver::build_problem() {
    if ( lp_ )
	delete lp_;
    lp_ = new lp::problem();
    
    std::ostringstream oss;
    
    size_t vars_nr{};
    std::unordered_map<coord, size_t> vars; // maps coord => LP's column variable number
    std::vector<lp_row_info> rows;          // used to set row names and constraints
    lp::matrix m;
    
    decltype(frontier_) old_frontier;
    std::swap(frontier_, old_frontier);
    
    for(auto c: old_frontier) {
	//xlog << "looing at frontier " << c << "\n";
	
	auto u = get_unknowns(c);
	//xlog << "has " << (int)u.nr << " unknown neighbors, " << (int)u.mines_nr << " mine(s)\n";
	
	if ( !u.nr ) // no longer a frontier
	    continue;
	
	//for(uint8_t i = 0; i < u.nr; ++i) {
	//xlog << "  at " << u.coords[i] << "\n";
	//}
	
	frontier_.push_back(c);

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
    
    if ( vars.empty() )
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
    for(auto it = vars.begin(); it != vars.end(); ++it) {
	oss.str("");
	oss << 'u' << it->first;
	lp_->set_column_name(it->second, oss.str().data());
	lp_->set_column_bounded(it->second, 0, 1);
    }
    
    lp_->set_matrix(m);
    
    std::cout << lp_->dump() << "\n";

    if ( !lp_->presolve() ) {
	errlog << "could not solve LP\n";
	return;
    }
    
    //TODO: go over every column variable and try it...
    for(auto it = vars.begin(); it != vars.end(); ++it) {
	xlog << "solving for " << it->first << "\n";

	lp_->set_objective_coefficient(it->second, 1);
	lp_->set_maximize();
	lp_->solve();
	
	if ( lp_->get_objective_value() == 0 ) {
	    xlog << "!! can't have a mine at " << it->first << "\n";
	    I_ASSERT(!board_->field()->is_mine(it->first), EX_LOG("ERROR: calculated " << it->first << " to be non-mine, turned out to be a mine!"));
	    board_->at(it->first) = static_cast<board::cellinfo>(board_->field()->nearby_mines_nr(it->first));
	    add_uncovered_cell(it->first);
	    
	} else {
	    lp_->set_minimize();
	    lp_->solve();
	    auto rv = lp_->get_objective_value();
	    if ( rv > 0 ) {
		xlog << "!! must have mine at " << it->first << " as minimium is " << rv << "\n";
		board_->at(it->first) = board::cellinfo::marked_mine;
	    }
	}
	
    	lp_->set_objective_coefficient(it->second, 0);
    }
}

} // namespace miner
