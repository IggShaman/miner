#include "board.h"
#include "scene.h"

namespace miner {

scene::scene()
    : board_{new miner::board},
      cell_border_{200,200,200},
      cell_opened_bg_{220,220,220},
      cell_unknown_bg_{100,100,100},
      per_nr_colors_text_{Qt::black, Qt::darkBlue, Qt::darkGreen, Qt::darkCyan, Qt::darkMagenta, Qt::black, Qt::black, Qt::black},
      per_nr_colors_box_{Qt::black, Qt::blue, Qt::green, Qt::cyan, Qt::magenta, Qt::yellow, Qt::yellow, Qt::yellow}
{
    cell_font_.setPointSize(kCellSize - 4);
    cell_font_.setBold(true);
    
    board_->set_field(std::make_shared<field>());
}


void scene::set_board ( board_ptr b ) {
    board_ = b;
    setFixedSize(board_->rows() * kCellSize, board_->cols() * kCellSize);
    set_scale(scale_);
}


void scene::set_scale ( float s ) {
    scale_ = s;
    setFixedSize(board_->rows() * kCellSize * scale_, board_->cols() * kCellSize * scale_);
    update();
}


void scene::paintEvent ( QPaintEvent* ev ) {
    QPainter painter(this);

    if ( is_point_mode() ) {
	for ( int row = ev->rect().top(); row <= ev->rect().bottom(); ++row )
	    for ( int col = ev->rect().left(); col <= ev->rect().right(); ++col )
		paint_point_cell(painter, {row, col});
	
    } else {
	for ( int row = y2row(ev->rect().top()); row <= y2row(ev->rect().bottom()); ++row )
	    for ( int col = x2col(ev->rect().left()); col <= x2col(ev->rect().right()); ++col )
		paint_cell(painter, {row, col});
    }
}


void scene::paint_cell ( QPainter& painter, coord c ) {
    painter.save();
    
    painter.translate(col2x(c.col), row2y(c.row));
    painter.scale(scale_, scale_);
    
    QRect r;
    if ( scale_ >= 0.5 ) {
	painter.setPen(cell_border_);
	painter.drawLine(0, 0, kCellSize - 1, 0);
	painter.drawLine(0, 0, 0, kCellSize - 1);
	r = {1,1,kCellSize-1,kCellSize-1};
	
    } else {
	r = {0,0,kCellSize,kCellSize};
    }

    if ( scale_ >= 0.2 )
	painter.setFont(cell_font_);
    
    
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
	if ( scale_ >= 0.2 ) {
	    painter.fillRect(r, cell_opened_bg_);
	    painter.setPen(per_nr_colors_text_[static_cast<int>(ci)]);
	    painter.drawText(1, kCellSize - 1, QString::number(static_cast<int>(ci)));
	    
	} else {
	    painter.fillRect(r, per_nr_colors_box_[static_cast<int>(ci)]);
	}
	break;
    };
    
    if ( show_mines_ and board_->field()->is_mine(c) ) {
	if ( scale_ >= 0.2 ) {
	    painter.setPen(Qt::red);
	    for ( size_t r = 1; r < 8; ++r )
		for ( size_t c = kCellSize - 8 + r; c < kCellSize; ++c )
		    painter.drawPoint(c,r);
	} else {
	    painter.fillRect(r, Qt::darkRed);
	}
    }
    
    painter.restore();
}


void scene::paint_point_cell ( QPainter& painter, coord c ) {
    auto ci = board_->at(c);
    switch(ci) {
    case board::cellinfo::boom_mine:
	painter.setPen(Qt::black);
	break;
	
    case board::cellinfo::marked_mine:
	painter.setPen(Qt::red);
	break;
	
    case board::cellinfo::unknown:
	if ( show_mines_ and board_->field()->is_mine(c) )
	    painter.setPen(Qt::darkRed);
	else
	    painter.setPen(cell_unknown_bg_);
	break;
	
    case board::cellinfo::n0:
	painter.setPen(cell_opened_bg_);
	break;
	
    case board::cellinfo::n1:
    case board::cellinfo::n2:
    case board::cellinfo::n3:
    case board::cellinfo::n4:
    case board::cellinfo::n5:
    case board::cellinfo::n6:
    case board::cellinfo::n7:
    case board::cellinfo::n8:
	painter.setPen(per_nr_colors_box_[static_cast<int>(ci)]);
	break;
    };
    painter.drawPoint(c.col, c.row);
}


void scene::mouseReleaseEvent ( QMouseEvent* ev ) {
    if ( !rw_  or board_->game_lost() )
	return;

    coord c;
    if ( is_point_mode() )
	c = {ev->y(), ev->x()};
    else
	c = {y2row(ev->y()), x2col(ev->x())};
    
    switch(ev->button()) {
    case Qt::LeftButton: {
	//NOTE: no modifiers pressed
	if ( board_->at(c) != board::cellinfo::unknown )
	    return;
	
	ev->accept();
	if ( board_->field()->is_mine(c) ) {
	    board_->mark_boom(c);
	    emit cell_changed(c);
	    emit game_lost();
	    
	} else {
	    board_->uncovered_safe(c, board_->field()->nearby_mines_nr(c));
	    emit cell_changed(c);
	}
	
	update_cell(c);
	break;
    }
	
    case Qt::RightButton: {
	ev->accept();
	switch(board_->at(c)) {
	case board::cellinfo::marked_mine:
	    board_->mark_mine(c, false);
	    update_cell(c);
	    emit cell_changed(c);
	    break;
	    
	case board::cellinfo::unknown:
	    board_->mark_mine(c, true);
	    update_cell(c);
	    emit cell_changed(c);
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
    if ( is_point_mode() )
	update(c.col, c.row, 1, 1);
    else
	update(col2x(c.col), row2y(c.row), kCellSize * scale_, kCellSize * scale_);
}


void scene::update_box ( coord center, int range ) {
    if ( is_point_mode() ) {
	update(center.col - range, center.row - range, 1 + range * 2, 1 + range * 2);
	
    } else {
	update(col2x(center.col - range), row2y(center.row - range), (1 + range * 2) * kCellSize * scale_, (1 + range * 2) * kCellSize * scale_);
    }
}


void scene::wheelEvent ( QWheelEvent* ev ) {
    if ( ev->modifiers() & Qt::ControlModifier ) {
	ev->accept();
	
	auto steps = ev->angleDelta() / 8 / 15;
	if ( steps.isNull() )
	    return;
	
	scale_ += 0.05 * steps.y();
	if ( scale_ < 0.05 )
	    scale_ = 0.05;
	else if ( scale_ > 1 )
	    scale_ = 1;
	
	set_scale(scale_);
	return;
    }
}


void scene::zoom_out() {
    if ( scale_ > 0.05 )
	set_scale(std::max({0.05, scale_ - 0.05}));
}


void scene::zoom_in() {
    if ( scale_ < 1 )
	set_scale(std::min({scale_ + 0.05, 1.0}));
}


void scene::set_point_mode() {
    if ( scale_ != 0.05 )
	set_scale(0.05);
}

} // namespace miner
