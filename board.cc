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

} // namespace miner
