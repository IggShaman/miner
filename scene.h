#pragma once

#include "field.h"

namespace miner {

class GameBoard;
using GameBoardPtr = std::shared_ptr<GameBoard>;

class Scene : public QWidget {
    Q_OBJECT;
public:
    static constexpr size_t kCellSize = 20;
    
    Scene();
    
    GameBoardPtr board() { return board_; }
    void set_board(GameBoardPtr);
    void set_show_mines(bool v) { show_mines_ = v; update(); }
    void update_cell(Location);
    void update_box(Location center, size_t range);
    void set_scale(float);
    void set_rw(bool v) { rw_ = v; }

public slots:
    void zoom_in();
    void zoom_out();
    void set_point_mode(); // sets minimal zoom, which uses individual pixes to draw field
    
signals:
    void cell_changed(miner::Location);
    void game_lost();
    
protected:
    void paintEvent(QPaintEvent*) override;
    void mouseReleaseEvent(QMouseEvent*) override;
    void wheelEvent(QWheelEvent*) override;
    bool is_point_mode() const { return scale_ == 0.05; }
    
private:
    void paint_cell(QPainter&, Location);
    void paint_point_cell(QPainter&, Location);
    size_t x2col(size_t x) { return x / scale_ / kCellSize; }
    size_t y2row(size_t y) { return y / scale_ / kCellSize; }
    size_t row2y(size_t row) { return scale_ * row * kCellSize; }
    size_t col2x(size_t col) { return scale_ * col * kCellSize; }
    
    GameBoardPtr board_;
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
