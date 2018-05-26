#pragma once

#include "field.h"

namespace miner {

class CellNeighborhoodIterator;

class GameBoard {
public:
    enum class CellInfo : int8_t {
	Exploded = -3,
	MarkedMine = -2,
	Unknown = -1,
	N0 = 0,
	N1 = 1,
	N2 = 2,
	N3 = 3,
	N4 = 4,
	N5 = 5,
	N6 = 6,
	N7 = 7,
	N8 = 8,
    };
    
    void set_field(FieldPtr);
    CellInfo at(Location l) const { return data_[to_index(l)]; }
    void mark_mine(Location, bool);
    void mark_exploded(Location l) { edit_at(l) = CellInfo::Exploded; }
    void uncovered_safe(Location, uint8_t);
    size_t rows() const { return field_->rows(); }
    size_t cols() const { return field_->cols(); }
    size_t mines_marked() const { return mines_marked_; }
    FieldCPtr field() const { return field_; }
    CellNeighborhoodIterator neighborhood(Location);
    bool is_uncovered(Location l) const { return static_cast<int>(at(l)) >= 0; }
    bool game_lost() const { return game_lost_; }
    void set_game_lost() { game_lost_ = true; }
    size_t uncovered_nr() const { return uncovered_nr_; }
    size_t left_nr() const { return data_.size() - uncovered_nr_ - mines_marked_; }
    void dump_region(Location, size_t range) const;
    
private:
    size_t to_index(const Location& l) const { return field_->cols() * l.row + l.col; }
    CellInfo& edit_at(Location l) { return data_[to_index(l)]; }
    
    FieldPtr field_;
    std::vector<CellInfo> data_;
    size_t mines_marked_{};
    size_t uncovered_nr_{};
    bool game_lost_{};
};

using GameBoardPtr = std::shared_ptr<GameBoard>;

class CellNeighborhoodIterator {
public:
    CellNeighborhoodIterator(GameBoard*, Location);
    
    CellNeighborhoodIterator& operator++() { ++i_; return *this; }
    operator bool() const { return i_ < end_; }
    GameBoard::CellInfo at() { return board_->at(neighbors_[i_]); }
    const Location& operator*() const { return neighbors_[i_]; }
    
private:
    uint8_t i_{}, end_{};
    std::array<Location, 8> neighbors_;
    GameBoard* board_{};
};

inline CellNeighborhoodIterator
GameBoard::neighborhood(Location l) {
    return CellNeighborhoodIterator(this, l);
}

} // namespace miner
