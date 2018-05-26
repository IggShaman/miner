#include "board.h"
#include "game_board_widget.h"

namespace miner {

GameBoardWidget::GameBoardWidget()
    : board_{new miner::GameBoard},
      cell_border_{200,200,200},
      cell_opened_bg_{220,220,220},
      cell_unknown_bg_{100,100,100},
      per_nr_colors_text_{
          Qt::black, Qt::darkBlue, Qt::darkGreen, Qt::darkCyan,
          Qt::darkMagenta, Qt::black, Qt::black, Qt::black},
      per_nr_colors_box_{
          Qt::black, Qt::blue, Qt::green, Qt::cyan, Qt::magenta,
          Qt::yellow, Qt::yellow, Qt::yellow}
{
    cell_font_.setPixelSize(kCellSize - 4);
    cell_font_.setBold(true);
    board_->set_field(std::make_shared<Field>());
}


void GameBoardWidget::set_board(GameBoardPtr b) {
    board_ = b;
    update_widget_size();
}


void GameBoardWidget::update_widget_size() {
    setFixedSize(
      board_->cols() * scaled_cell_size(),
      board_->rows() * scaled_cell_size());
    update();
}


void GameBoardWidget::set_scale_step(size_t s) {
    if (s < kMinScaleStep) {
        s = kMinScaleStep;
    } else if (s > kMaxScaleStep) {
        s = kMaxScaleStep;
    }
    
    if (scale_step_ != s) {
        scale_step_ = s;
        update_widget_size();
        // update();
    }
}


void GameBoardWidget::paintEvent(QPaintEvent* ev) {
    QPainter painter{this};
    
    if (is_point_mode()) {
	for (size_t row = (size_t)std::max(0, ev->rect().top());
             row <= (size_t)std::max(0, ev->rect().bottom()); ++row)
	    for (size_t col = (size_t)std::max(0, ev->rect().left());
                 col <= (size_t)std::max(0, ev->rect().right()); ++col)
		paint_point_cell(painter, {row, col});
	
    } else {
	for (size_t row = y2row((size_t)std::max(0, ev->rect().top()));
             row <= y2row((size_t)std::max(0, ev->rect().bottom()));
             ++row) {
	    for (size_t col = x2col((size_t)std::max(0, ev->rect().left()));
                 col <= x2col((size_t)std::max(0, ev->rect().right()));
                 ++col) {
		paint_cell(painter, {row, col});
            }
        }
    }
}


void GameBoardWidget::paint_cell(QPainter& painter, Location l) {
    painter.save();
    
    painter.translate(col2x(l.col), row2y(l.row));
    auto scale_factor = get_scale_factor();
    painter.scale(scale_factor, scale_factor);
    
    QRect r;
    if (scale_step_ >= kDrawBorderScaleStep) {
        // draw border
	painter.setPen(cell_border_);
	painter.drawLine(0, 0, kCellSize - 1, 0);
	painter.drawLine(0, 0, 0, kCellSize - 1);
	r = {1, 1, kCellSize - 1, kCellSize - 1};
	
    } else {
        // no border
	r = {0, 0, kCellSize, kCellSize};
    }
    
    if (scale_step_ >= kDrawTextScaleStep)
	painter.setFont(cell_font_);
    
    auto ci = board_->at(l);
    switch(ci) {
    case GameBoard::CellInfo::Exploded:
	painter.fillRect(r, QBrush(Qt::black));
	break;
	
    case GameBoard::CellInfo::MarkedMine:
	painter.fillRect(r, QBrush(Qt::red));
	break;
	
    case GameBoard::CellInfo::Unknown:
	painter.fillRect(r, cell_unknown_bg_);
	break;
	
    case GameBoard::CellInfo::N0:
	break;
	
    case GameBoard::CellInfo::N1:
    case GameBoard::CellInfo::N2:
    case GameBoard::CellInfo::N3:
    case GameBoard::CellInfo::N4:
    case GameBoard::CellInfo::N5:
    case GameBoard::CellInfo::N6:
    case GameBoard::CellInfo::N7:
    case GameBoard::CellInfo::N8:
	if (scale_step_ >= kDrawTextScaleStep) {
	    painter.fillRect(r, cell_opened_bg_);
	    painter.setPen(per_nr_colors_text_[static_cast<int>(ci)]);
	    painter.drawText(1, kCellSize - 1, QString::number(static_cast<int>(ci)));
	    
	} else {
	    painter.fillRect(r, per_nr_colors_box_[static_cast<int>(ci)]);
	}
	break;
    };
    
    if (show_mines_ and board_->field()->is_mined(l)) {
	if (scale_step_ >= kDrawTextScaleStep) {
	    painter.setPen(Qt::red);
	    for (size_t r = 1; r < 8; ++r)
		for (size_t c = kCellSize - 8 + r; c < kCellSize; ++c)
		    painter.drawPoint(c, r);
	} else {
	    painter.fillRect(r, Qt::darkRed);
	}
    }
    
    painter.restore();
}


void GameBoardWidget::paint_point_cell(QPainter& painter, Location l) {
    auto ci = board_->at(l);
    switch(ci) {
    case GameBoard::CellInfo::Exploded:
	painter.setPen(Qt::black);
	break;
	
    case GameBoard::CellInfo::MarkedMine:
	painter.setPen(Qt::red);
	break;
	
    case GameBoard::CellInfo::Unknown:
	if (show_mines_ and board_->field()->is_mined(l))
	    painter.setPen(Qt::darkRed);
	else
	    painter.setPen(cell_unknown_bg_);
	break;
	
    case GameBoard::CellInfo::N0:
	painter.setPen(cell_opened_bg_);
	break;
	
    case GameBoard::CellInfo::N1:
    case GameBoard::CellInfo::N2:
    case GameBoard::CellInfo::N3:
    case GameBoard::CellInfo::N4:
    case GameBoard::CellInfo::N5:
    case GameBoard::CellInfo::N6:
    case GameBoard::CellInfo::N7:
    case GameBoard::CellInfo::N8:
	painter.setPen(per_nr_colors_box_[static_cast<int>(ci)]);
	break;
    };
    
    painter.drawPoint(l.col, l.row);
}


void GameBoardWidget::mouseReleaseEvent(QMouseEvent* ev) {
    if (!rw_  or board_->game_lost())
	return;
    
    Location l;
    if (is_point_mode())
	l = {(size_t)std::min(0, ev->y()),
             (size_t)std::min(0, ev->x())};
    else
	l = {y2row((size_t)std::min(0, ev->y())),
             x2col((size_t)std::min(0, ev->x()))};
    
    switch(ev->button()) {
    case Qt::LeftButton: {
	//NOTE: no modifiers pressed
	if (board_->at(l) != GameBoard::CellInfo::Unknown)
	    return;
	
	ev->accept();
	if (board_->field()->is_mined(l)) {
	    board_->mark_exploded(l);
	    emit cell_changed(l);
	    emit game_lost();
	    
	} else {
	    board_->uncovered_safe(l, board_->field()->nearby_mines_nr(l));
	    emit cell_changed(l);
	}
	
	update_cell(l);
	break;
    }
	
    case Qt::RightButton: {
	ev->accept();
	switch(board_->at(l)) {
	case GameBoard::CellInfo::MarkedMine:
	    board_->mark_mine(l, false);
	    update_cell(l);
	    emit cell_changed(l);
	    break;
	    
	case GameBoard::CellInfo::Unknown:
	    board_->mark_mine(l, true);
	    update_cell(l);
	    emit cell_changed(l);
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


void GameBoardWidget::update_cell(Location l) {
    if (is_point_mode())
	update(l.col, l.row, 1, 1);
    else
	update(col2x(l.col), row2y(l.row),
               scaled_cell_size(), scaled_cell_size());
}


void GameBoardWidget::update_box(Location center, size_t range) {
    if (is_point_mode()) {
	update(center.col - range, center.row - range, 1 + range * 2, 1 + range * 2);
	
    } else {
	update(
          col2x(center.col - range),
          row2y(center.row - range),
          (1 + range * 2) * scaled_cell_size(),
          (1 + range * 2) * scaled_cell_size());
    }
}


void GameBoardWidget::wheelEvent(QWheelEvent* ev) {
    if (ev->modifiers() & Qt::ControlModifier) {
	ev->accept();
	
	auto steps = ev->angleDelta() / 8 / 15;
	if (steps.isNull())
	    return;
        
	set_scale_step((size_t)std::max(0, steps.y() + (int)scale_step_));
        return;
    }
}


void GameBoardWidget::zoom_out() {
    set_scale_step(scale_step_ - 1);
}


void GameBoardWidget::zoom_in() {
    set_scale_step(scale_step_ + 1);
}


void GameBoardWidget::set_point_mode() {
    set_scale_step(kPointModeScaleStep);
}

} // namespace miner
