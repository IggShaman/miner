#include "board.h"

namespace miner {

void board::reset ( field_ptr f ) {
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

} // namespace miner
