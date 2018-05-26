#include "field.h"

namespace miner {

void Field::reset(size_t rows, size_t cols) {
    mines_nr_ = 0;
    rows_ = rows;
    cols_ = cols;
    data_.resize(rows_ * cols_);
    std::fill(data_.begin(), data_.end(), false);
}


size_t Field::to_index(Location l) const {
    return l.row * cols_ + l.col;
}


void Field::mark_mined(Location l, bool v) {
    auto idx = to_index(l);
    if (data_[idx]) {
	if (!v) {
	    data_[idx] = false;
	    mines_nr_--;
	}
	
    } else {
	if (v) {
	    data_[idx] = true;
	    ++mines_nr_;
	}
    }
}


void Field::gen_random(size_t rows, size_t cols, size_t mines_nr) {
    srand48(time(nullptr));
    reset(rows, cols);

    // TODO: throw exception?
    if (mines_nr >= rows * cols)
	return;
    
    // For low fill rates (<= 30%), this should work well.
    mines_nr_ = mines_nr;
    while(mines_nr) {
	Location l{size_t(drand48() * rows_), size_t(drand48() * cols_)};
        if (is_mined(l))
            continue;
        
        data_[to_index(l)] = true;
        --mines_nr;
    }
}


uint8_t Field::nearby_mines_nr(Location l) const {
    uint8_t nr{};
    
    if (l.row > 0) {
	if (l.col > 0)
            nr += is_mined({l.row - 1, l.col - 1});
	nr += is_mined({l.row - 1, l.col});
	if (l.col < cols_ - 1)
            nr += is_mined({l.row - 1, l.col + 1});
    }
    
    if (l.col > 0)
        nr += is_mined({l.row, l.col-1});
    
    if (l.col < cols_ - 1)
        nr += is_mined({l.row, l.col + 1});
    
    if (l.row < rows_ - 1) {
	if (l.col > 0)
            nr += is_mined({l.row + 1, l.col - 1});
	nr += is_mined({l.row + 1, l.col});
	if (l.col < cols_ - 1)
            nr += is_mined({l.row + 1, l.col + 1});
    }
    
    return nr;
}

} // namespace miner
