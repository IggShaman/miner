#include "board.h"

namespace miner {

void board::set_field ( field_ptr f ) {
    field_ = f;
    data_.resize(field_->rows() * field_->cols());
    std::fill(data_.begin(), data_.end(), cellinfo::unknown);
}


cell_neighborhood_iterator::cell_neighborhood_iterator ( board* b, coord c )
    : board_{b} {
    
    end_ = 0;
    if ( c.row > 0 ) {
	if ( c.col > 0 ) neigh_[end_++] = {c.row-1, c.col-1};
	neigh_[end_++] = {c.row-1, c.col};
	if ( c.col < b->cols() - 1 ) neigh_[end_++] = {c.row-1, c.col+1};
    }
    
    if ( c.col > 0 ) neigh_[end_++] = {c.row, c.col-1};
    if ( c.col < b->cols() - 1 ) neigh_[end_++] = {c.row, c.col+1};
    
    if ( c.row < b->rows() - 1 ) {
	if ( c.col > 0 ) neigh_[end_++] = {c.row+1, c.col-1};
	neigh_[end_++] = {c.row+1, c.col};
	if ( c.col < b->cols() - 1 ) neigh_[end_++] = {c.row+1, c.col+1};
    }
}


void board::mark_mine ( coord c, bool v ) {
    auto& ci = edit_at(c);
    
    if ( v ) {
	if ( ci != cellinfo::unknown )
	    return;
	
	ci = cellinfo::marked_mine;
	++mines_marked_;
	
    } else {
	if ( ci != cellinfo::marked_mine )
	    return;
	
	ci = cellinfo::unknown;
	--mines_marked_;
    }
}


void board::dump_region ( miner::coord poi, int range ) const {
    std::cout << "center=" << poi << "\n";
    int col0 = std::max(0, static_cast<int>(poi.col) - range - 1);
    int col1 = std::min(static_cast<int>(cols()) - 1, static_cast<int>(poi.col) + range + 1);
    std::cout << "columns: [" << col0 << " .. " << col1 << "]\n";
    for(int row = std::max(0, poi.row - range - 1); row <= std::min(rows() - 1, static_cast<int>(poi.row) + range + 1); ++row) {
	std::cout << row << ": ";
	for(int col = col0; col <= col1; ++col) {
	    coord c{row, col};
	    char ch{};
	    auto v = at(c);
	    switch(v) {
	    case cellinfo::boom_mine:
		ch = '!';
		break;
		
	    case board::cellinfo::marked_mine:
		if ( field_->is_mine(c) )
		    ch = '*';
		else
		    ch = '%';
		break;
		
	    case board::cellinfo::unknown:
		ch = '?';
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
		ch = '0' + static_cast<int>(v);
		break;
	    };
	    std::cout << ch;
	}
	std::cout << "\n";
    }
}

} // namespace miner
