#include "board.h"
#include "scene.h"

namespace miner {

scene::scene()
    : board_{new miner::board},
      cell_border_{200,200,200},
      cell_opened_bg_{220,220,220},
      cell_unknown_bg_{100,100,100}
{
    per_nr_colors_[0] = Qt::black;
    per_nr_colors_[1] = Qt::darkBlue;
    per_nr_colors_[2] = Qt::darkGreen;
    per_nr_colors_[3] = Qt::darkCyan;
    per_nr_colors_[4] = Qt::darkMagenta;
    per_nr_colors_[5] = Qt::black;
    per_nr_colors_[6] = Qt::black;
    per_nr_colors_[7] = Qt::black;
    
    cell_font_.setPointSize(kCellSize - 4);
    cell_font_.setBold(true);
    
    board_->reset(std::make_shared<field>());
}


void scene::set_board ( board_ptr b ) {
    board_ = b;
    setFixedSize(board_->rows() * kCellSize, board_->cols() * kCellSize);
    update();
}


void scene::paintEvent ( QPaintEvent* ev ) {
    QPainter painter(this);
    for ( size_t row = y2row(ev->rect().top()); row <= y2row(ev->rect().bottom()); ++row )
	for ( size_t col = x2col(ev->rect().left()); col <= x2col(ev->rect().right()); ++col )
	    paint_cell(painter, {row, col});
}


void scene::paint_cell ( QPainter& painter, coord c ) {
    painter.save();
    
    painter.translate(col2x(c.col), row2y(c.row));
    
    painter.setPen(cell_border_);
    painter.drawLine(0, 0, kCellSize - 1, 0);
    painter.drawLine(0, 0, 0, kCellSize - 1);
    painter.setFont(cell_font_);
    QRect r{1,1,kCellSize-1,kCellSize-1};
    
    auto ci = board_->at(c);
    switch(ci) {
    case board::cellinfo::boom_mine:
	painter.fillRect(r, QBrush(Qt::black));
	break;
	
    case board::cellinfo::marked_mine:
	painter.fillRect(r, QBrush(Qt::red));
	break;
	
    case board::cellinfo::unknown:
	painter.fillRect(r, cell_unknown_bg_);
	break;
	
    case board::cellinfo::n0:
	break;
	
    case board::cellinfo::n1:
    case board::cellinfo::n2:
    case board::cellinfo::n3:
    case board::cellinfo::n4:
    case board::cellinfo::n5:
    case board::cellinfo::n6:
    case board::cellinfo::n7:
    case board::cellinfo::n8:
	painter.fillRect(r, cell_opened_bg_);
	painter.setPen(per_nr_colors_[static_cast<int>(ci)]);
	painter.drawText(1, kCellSize - 1, QString::number(static_cast<int>(ci)));
	break;
    };
    
    if ( show_mines_ and board_->field()->is_mine(c) ) {
	painter.setPen(Qt::red);
	for ( size_t r = 1; r < 8; ++r )
	    for ( size_t c = kCellSize - 8 + r; c < kCellSize; ++c )
		painter.drawPoint(c,r);
    }
    
    painter.restore();
}


void scene::mouseReleaseEvent ( QMouseEvent* ev ) {
    ev->accept();
    
    coord c{y2row(ev->y()), x2col(ev->x())};
    
    switch(ev->button()) {
    case Qt::LeftButton: {
	//NOTE: no modifiers pressed
	if ( board_->at(c) != board::cellinfo::unknown )
	    return;
	
	if ( board_->field()->is_mine(c) ) {
	    board_->mark_boom(c);
	    
	} else {
	    board_->uncovered_safe(c, board_->field()->nearby_mines_nr(c));
	    emit cell_uncovered(c);
	}
	
	update_cell(c);
	break;
    }
	
    case Qt::RightButton: {
	switch(board_->at(c)) {
	case board::cellinfo::marked_mine:
	    board_->mark_mine(c, false);
	    update_cell(c);
	    break;
	    
	case board::cellinfo::unknown:
	    board_->mark_mine(c, true);
	    update_cell(c);
	    break;
	    
	default:
	    break;
	};
	break;
    }
	
    default:
	break;
    };
}


void scene::update_cell ( coord c ) {
    update(col2x(c.col), row2y(c.row), kCellSize, kCellSize);
}

} // namespace miner
