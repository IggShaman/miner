#ifndef __MINER_SCENE_H_
#define __MINER_SCENE_H_

#include "field.h"

namespace miner {

class board;
using board_ptr = std::shared_ptr<board>;

class scene : public QWidget {
    Q_OBJECT;
public:
    static constexpr size_t kCellSize = 20;
    
    scene();
    board_ptr board() { return board_; }
    void set_board ( board_ptr );
    void set_show_mines ( bool v ) { show_mines_ = v; update(); }
    void update_cell ( coord );
    void set_scale ( float );

public slots:
    void zoom_in();
    void zoom_out();
    
signals:
    void cell_changed ( miner::coord );
    void game_lost();
    
protected:
    void paintEvent ( QPaintEvent* ) override;
    void mouseReleaseEvent ( QMouseEvent* ) override;
    void wheelEvent ( QWheelEvent* ) override;
    
private:
    void paint_cell ( QPainter&, coord );
    size_t x2col ( size_t x ) { return x / scale_ / kCellSize; }
    size_t y2row ( size_t y ) { return y / scale_ / kCellSize; }
    size_t row2y ( size_t row ) { return scale_ * row * kCellSize; }
    size_t col2x ( size_t col ) { return scale_ * col * kCellSize; }
    
    board_ptr board_;
    bool show_mines_{};
    
    QColor cell_border_;
    QColor cell_opened_bg_;
    QColor cell_unknown_bg_;
    QFont cell_font_;
    QColor per_nr_colors_[8];
    float scale_{0.5};
};

} // namespace miner

#endif // __MINER_SCENE_H_
