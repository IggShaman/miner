#include "field.h"

namespace miner {

void field::reset ( size_t rows, size_t cols ) {
    mines_nr_ = 0;
    rows_ = rows;
    cols_ = cols;
    data_.resize(rows_ * cols_);
    std::fill(data_.begin(), data_.end(), false);
}


void field::set_mine ( coord c, bool v ) {
    size_t idx = c.row * cols_ + c.col;
    if ( data_[idx] ) {
	if ( !v ) {
	    data_[idx] = false;
	    mines_nr_--;
	}
	
    } else {
	if ( v ) {
	    data_[idx] = true;
	    ++mines_nr_;
	}
    }
}


void field::gen_random ( size_t rows, size_t cols, size_t mines_nr ) {
    srand48(time(nullptr));
    reset(rows, cols);
    
    mines_nr_ = mines_nr;
    while(mines_nr) {
	coord c{size_t(drand48() * rows_), size_t(drand48() * cols_)};
	if ( !is_mine(c) ) {
	    data_[c.row * rows_ + c.col] = true;
	    --mines_nr;
	}
    }
}


size_t field::nearby_mines_nr ( coord c ) const {
    size_t rv{};
    
    if ( c.row > 0 ) {
	if ( c.col > 0 ) rv += is_mine({c.row-1, c.col-1});
	rv += is_mine({c.row-1, c.col});
	if ( c.col < cols_ - 1 ) rv += is_mine({c.row-1, c.col+1});
    }
    
    if ( c.col > 0 ) rv += is_mine({c.row, c.col-1});
    if ( c.col < cols_ - 1 ) rv += is_mine({c.row, c.col+1});
    
    if ( c.row < rows_ - 1 ) {
	if ( c.col > 0 ) rv += is_mine({c.row+1, c.col-1});
	rv += is_mine({c.row+1, c.col});
	if ( c.col < cols_ - 1 ) rv += is_mine({c.row+1, c.col+1});
    }
    
    return rv;
}

} // namespace miner
