#include "board.h"

namespace miner {

void GameBoard::uncovered_safe(Location l, uint8_t v) {
    edit_at(l) = static_cast<CellInfo>(v);
    ++uncovered_nr_;
}


void GameBoard::set_field(FieldPtr field) {
    field_ = field;
    data_.resize(field_->rows() * field_->cols());
    std::fill(data_.begin(), data_.end(), CellInfo::Unknown);
}


CellNeighborhoodIterator::CellNeighborhoodIterator(GameBoard* board, Location l)
    : board_{board} {
    
    end_ = 0;
    if (l.row > 0) {
	if (l.col > 0)
            neighbors_[end_++] = {l.row-1, l.col-1};
	neighbors_[end_++] = {l.row-1, l.col};
	if (l.col < board_->cols() - 1)
            neighbors_[end_++] = {l.row-1, l.col+1};
    }
    
    if (l.col > 0)
        neighbors_[end_++] = {l.row, l.col-1};
    if (l.col < board_->cols() - 1)
        neighbors_[end_++] = {l.row, l.col+1};
    
    if (l.row < board_->rows() - 1) {
	if (l.col > 0)
            neighbors_[end_++] = {l.row+1, l.col-1};
	neighbors_[end_++] = {l.row+1, l.col};
	if (l.col < board_->cols() - 1)
            neighbors_[end_++] = {l.row+1, l.col+1};
    }
}


void GameBoard::mark_mine(Location l, bool v) {
    auto& ci = edit_at(l);
    
    if (v) {
	if (ci != CellInfo::Unknown)
	    return;
	
	ci = CellInfo::MarkedMine;
	++mines_marked_;
	
    } else {
	if (ci != CellInfo::MarkedMine)
	    return;
        
	ci = CellInfo::Unknown;
	--mines_marked_;
    }
}


void GameBoard::dump_region(Location poi, size_t range) const {
    std::cout << "center=" << poi << "\n";
    size_t col0 = poi.col > range + 1 ? poi.col - range - 1 : 0;
    size_t col1 = std::min(cols() - 1, poi.col + range + 1);
    std::cout << "columns: [" << col0 << " .. " << col1 << "]\n";
    for(size_t row = poi.row > range + 1 ? poi.row - range - 1 : 0;
        row <= std::min(rows() - 1, poi.row + range + 1);
        ++row) {
        
	std::cout << row << ": ";
	for(size_t col = col0; col <= col1; ++col) {
	    Location l{row, col};
	    char ch{};
	    auto v = at(l);
	    switch(v) {
	    case CellInfo::Exploded:
		ch = '!';
		break;
		
	    case GameBoard::CellInfo::MarkedMine:
		if ( field_->is_mined(l) )
		    ch = '*';
		else
		    ch = '%';
		break;
		
	    case GameBoard::CellInfo::Unknown:
		ch = '?';
		break;
		
	    case GameBoard::CellInfo::N0:
	    case GameBoard::CellInfo::N1:
	    case GameBoard::CellInfo::N2:
	    case GameBoard::CellInfo::N3:
	    case GameBoard::CellInfo::N4:
	    case GameBoard::CellInfo::N5:
	    case GameBoard::CellInfo::N6:
	    case GameBoard::CellInfo::N7:
	    case GameBoard::CellInfo::N8:
		ch = '0' + static_cast<int>(v);
		break;
	    };
	    std::cout << ch;
	}
	std::cout << "\n";
    }
}

} // namespace miner
