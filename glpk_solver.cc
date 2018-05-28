#include "glpk_lp_problem.h"
#include "glpk_solver.h"

namespace miner {

struct lp_row_info {
    lp_row_info(uint8_t v, std::string n)
        : fixed_value{v}, name{n} {}
    
    uint8_t fixed_value;
    std::string name;
};


bool GlpkSolver::doPoi(miner::Location poi) {
    if (board_->is_uncovered(poi)) {
	auto pois = getNeighborhoodInfo(poi);
	if (!pois.nr)
	    return true;
    }
    
    auto lp = std::make_unique<lp::problem>();
    
    // a set of locations LP is looking at; maps coord to LP's column variable number
    VariablesMapType vars;
    prepare(lp.get(), poi, vars);
    if (vars.empty())
	return true;
    
    for(auto& v: vars) {
	lp->set_objective_coefficient(v.second, 1);
	lp->set_maximize();
	lp->solve();
	
	auto obj = lp->get_objective_value();
	if (obj <= 1 - kEpsilon) {
	    // can't have a mine here
	    if (board_->field()->is_mined(v.first)) {
		xlog << "ERROR: game is lost at " << v.first << ": shold've been empty, has a mine"
		     << "\nobj=" << obj
		     << "\npoi=" << poi
		     << "\nLP: " << lp->dump() << "\n";
		board_->dump_region(poi, 3);
		resultHandler_(FeedbackState::kGameLost, Location{}, 0);
		return false;
	    }
	    
	    board_->uncovered_safe(v.first, board_->field()->nearby_mines_nr(v.first));
	    lp->set_column_fixed_bound(v.second, 0);
            addPoi(v.first);
            
	} else {
	    lp->set_minimize();
	    lp->solve();
	    auto obj = lp->get_objective_value();
	    if (obj >= kEpsilon) { // must have a mine here
		if (!board_->field()->is_mined(v.first)) {
		    xlog << "ERROR: calculated " << v.first << " to contain a mine, but it doesn't"
			 << "\nobj=" << obj
			 << "\npoi=" << poi
			 << "\nLP: " << lp->dump() << "\n";
		    board_->dump_region(poi, 3);
		    return false;
		}
		
		board_->mark_mine(v.first, true);
		lp->set_column_fixed_bound(v.second, 1);
                addPoi(v.first);
	    }
	}
	
	lp->set_objective_coefficient(v.second, 0);
    }
    
    return true;
}


void GlpkSolver::prepare(lp::problem* lp, Location poi, VariablesMapType& vars) {
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
	    if (static_cast<int>(ci) < 0)
		continue;
	    
	    auto pois = getNeighborhoodInfo(l);
	    if (!pois.nr)
		continue;
	    
	    oss.str("");
	    oss << 'n' << l;
	    rows.push_back({pois.mines_nr, oss.str()});
	    
	    for(uint8_t i = 0; i < pois.nr; ++i) {
		// find/add column variable for an uncovered cell
		auto iv = vars.insert({pois.coveredUnmarkedLocations[i], vars_nr + 1});
		if (iv.second)
		    ++vars_nr;
		
		// set coefficient to 1
		m.add(rows.size(), iv.first->second, 1);
	    }
	}
    }
    
    if (!vars_nr)
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
    
    if (!lp->presolve()) {
	errlog << "ERROR: could not presolve: " << lp->last_errmsg()
	       << "\npoi=" << poi
	       << "\nLP: " << lp->dump() << "\n";
	board_->dump_region(poi, kRange);
	exit(-1);
    }
}

} // namespace miner
