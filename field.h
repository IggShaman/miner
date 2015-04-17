#ifndef __MINE_FIELD_H_
#define __MINE_FIELD_H_

namespace miner {

struct coord {
    coord() : row{}, col{} {}
    coord ( size_t _row, size_t _col ) : row{_row}, col{_col} {}
    
    size_t row{};
    size_t col{};
    
    bool operator== ( const coord& other ) const { return row == other.row and col == other.col; }
};

} // namespace miner

namespace std {
template<> struct hash<miner::coord> { std::size_t operator() ( const miner::coord& c ) const { return qHash(c.col, qHash(c.row, 0)); } };
inline ostream& operator<< ( ostream& os, const miner::coord& c ) {
    os << '(' << c.row << ' ' << c.col << ')';
    return os;
}
} // namespace std

namespace miner {

class field {
public:
    void gen_random ( size_t rows, size_t cols, size_t mines_nr );
    void reset ( size_t rows, size_t cols );
    void set_mine ( coord, bool v ); // for manual minefield control; maintains mines_nr
    bool is_mine ( coord c ) const { return data_[c.row * cols_ + c.col]; }
    size_t nearby_mines_nr ( coord ) const;
    size_t rows() const { return rows_; }
    size_t cols() const { return cols_; }
    
private:
    size_t mines_nr_{};
    size_t rows_{};
    size_t cols_{};
    std::vector<bool> data_;
};

using field_ptr = std::shared_ptr<field>;


inline size_t field::nearby_mines_nr ( coord c ) const {
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

} // namespace mine

#endif // __MINE_FIELD_H_
