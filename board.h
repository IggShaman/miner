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
    
    void set_field ( field_ptr );
    cellinfo at ( coord c ) const { return data_[field_->rows() * c.row + c.col]; }
    void mark_mine ( coord, bool );
    void mark_boom ( coord c ) { edit_at(c) = cellinfo::boom_mine; }
    void uncovered_safe ( coord c, uint8_t v ) { edit_at(c) = static_cast<cellinfo>(v); ++uncovered_nr_; }
    size_t rows() const { return field_->rows(); }
    size_t cols() const { return field_->cols(); }
    size_t mines_marked() const { return mines_marked_; }
    field_ptr field() const { return field_; }
    cell_neighborhood_iterator neighborhood ( coord );
    bool is_ok ( coord c ) const { return static_cast<int>(at(c)) >= 0; }
    bool game_lost() const { return game_lost_; }
    void set_game_lost() { game_lost_ = true; }
    size_t uncovered_nr() const { return uncovered_nr_; }
    size_t left_nr() const { return data_.size() - uncovered_nr_ - mines_marked_; }
    
private:
    cellinfo& edit_at ( coord c ) { return data_[field_->rows() * c.row + c.col]; }
    
    field_ptr field_;
    std::vector<cellinfo> data_;
    size_t mines_marked_{};
    size_t uncovered_nr_{};
    bool game_lost_{};
};

using board_ptr = std::shared_ptr<board>;

class cell_neighborhood_iterator {
public:
    cell_neighborhood_iterator ( board*, coord );
    
    cell_neighborhood_iterator& operator++() { ++i_; return *this; }
    operator bool() const { return i_ < end_; }
    board::cellinfo at() { return board_->at(neigh_[i_]); }
    const coord& operator*() const { return neigh_[i_]; }
    
private:
    uint8_t i_{}, end_{};
    std::array<coord, 8> neigh_;
    board* board_{};
};

inline cell_neighborhood_iterator board::neighborhood ( coord c ) { return cell_neighborhood_iterator(this, c); }

} // namespace miner

#endif // __MINER_BOARD_H_
