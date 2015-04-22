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
    void update_box ( coord center, int range );
    void set_scale ( float );
    void set_rw ( bool v ) { rw_ = v; }

public slots:
    void zoom_in();
    void zoom_out();
    void set_point_mode(); // sets minimal zoom, which uses individual pixes to draw field
    
signals:
    void cell_changed ( miner::coord );
    void game_lost();
    
protected:
    void paintEvent ( QPaintEvent* ) override;
    void mouseReleaseEvent ( QMouseEvent* ) override;
    void wheelEvent ( QWheelEvent* ) override;
    bool is_point_mode() const { return scale_ == 0.05; }
    
private:
    void paint_cell ( QPainter&, coord );
    void paint_point_cell ( QPainter&, coord );
    int x2col ( int x ) { return x / scale_ / kCellSize; }
    int y2row ( int y ) { return y / scale_ / kCellSize; }
    int row2y ( int row ) { return scale_ * row * kCellSize; }
    int col2x ( int col ) { return scale_ * col * kCellSize; }
    
    board_ptr board_;
    bool show_mines_{};
    bool rw_{};
    
    QColor cell_border_;
    QColor cell_opened_bg_;
    QColor cell_unknown_bg_;
    QFont cell_font_;
    QColor per_nr_colors_text_[8];
    QColor per_nr_colors_box_[8];
    float scale_{0.5};
};

} // namespace miner

#endif // __MINER_SCENE_H_
