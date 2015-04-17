#ifndef __MINER_BOARD_H_
#define __MINER_BOARD_H_

#include "field.h"

namespace miner {

class cell_neighborhood_iterator;

class board {
public:
    enum class cellinfo : int8_t {	
	boom_mine = -3,
	marked_mine = -2,
	unknown = -1,
	n0 = 0,
	n1 = 1,
	n2 = 2,
	n3 = 3,
	n4 = 4,
	n5 = 5,
	n6 = 6,
	n7 = 7,
	n8 = 8,
    };
    
    void reset ( field_ptr );
    cellinfo& at ( coord c ) { return data_[field_->rows() * c.row + c.col]; }
    size_t rows() const { return field_->rows(); }
    size_t cols() const { return field_->cols(); }
    field_ptr field() const { return field_; }
    cell_neighborhood_iterator neighborhood ( coord );
    
private:
    field_ptr field_;
    std::vector<cellinfo> data_;
};

using board_ptr = std::shared_ptr<board>;

class cell_neighborhood_iterator {
public:
    cell_neighborhood_iterator ( board*, coord );
    
    cell_neighborhood_iterator& operator++() { ++i_; return *this; }
    operator bool() const { return i_ < end_; }
    board::cellinfo& at() { return board_->at(neigh_[i_]); }
    const coord& operator*() const { return neigh_[i_]; }
    
private:
    uint8_t i_{}, end_{};
    std::array<coord, 8> neigh_;
    board* board_{};
};

inline cell_neighborhood_iterator board::neighborhood ( coord c ) { return cell_neighborhood_iterator(this, c); }

} // namespace miner

#endif // __MINER_BOARD_H_
