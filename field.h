#ifndef __MINE_FIELD_H_
#define __MINE_FIELD_H_

namespace miner {

struct coord {
    coord() : row{}, col{} {}
    coord ( int _row, int _col ) : row{_row}, col{_col} {}
    
    int row{};
    int col{};
    
    bool operator== ( const coord& other ) const { return row == other.row and col == other.col; }
};

} // namespace miner

namespace std {

template<>
struct hash<miner::coord> {
    std::size_t operator() ( const miner::coord& c ) const {
	return qHash(c.col, qHash(c.row, 0));
    }
};

inline ostream& operator<< ( ostream& os, const miner::coord& c ) {
    os << '(' << c.row << ' ' << c.col << ')';
    return os;
}

} // namespace std

namespace miner {

class field {
public:
    void gen_random ( int rows, int cols, int mines_nr );
    void reset ( int rows, int cols );
    void set_mine ( coord, bool v ); // for manual minefield control; maintains mines_nr
    bool is_mine ( coord c ) const { return data_[c.row * cols_ + c.col]; }
    int nearby_mines_nr ( coord ) const;
    int rows() const { return rows_; }
    int cols() const { return cols_; }
    int mines_nr() const { return mines_nr_; }
    
private:
    int mines_nr_{};
    int rows_{};
    int cols_{};
    std::vector<bool> data_;
};

using field_ptr = std::shared_ptr<field>;
using field_cptr = std::shared_ptr<const field>;

} // namespace mine

#endif // __MINE_FIELD_H_
