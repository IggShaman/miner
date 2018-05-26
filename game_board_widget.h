#pragma once

#include "field.h"

namespace miner {

class GameBoard;
using GameBoardPtr = std::shared_ptr<GameBoard>;

class GameBoardWidget : public QWidget {
    Q_OBJECT;
public:
    static constexpr size_t kCellSize = 20; // in pixels
    static constexpr float kScaleStep = 0.05;
    static constexpr float kMaxScale = 10.0;

    static constexpr size_t kPointModeScaleStep = 1;
    static constexpr size_t kDrawBorderScaleStep = 0.5 / kScaleStep;
    static constexpr size_t kDrawTextScaleStep = 0.2 / kScaleStep;
    static constexpr size_t kMinScaleStep = 1;
    static constexpr size_t kMaxScaleStep = kMaxScale / kScaleStep;
    
    GameBoardWidget();
    
    GameBoardPtr board() { return board_; }
    void set_board(GameBoardPtr);
    void set_show_mines(bool v) { show_mines_ = v; update(); }
    void update_cell(Location);
    void update_box(Location center, size_t range);
    void set_scale_step(size_t step);
    void set_rw(bool v) { rw_ = v; }

public slots:
    void zoom_in();
    void zoom_out();
    // Sets minimal zoom, which uses individual pixes to draw field
    void switch_point_mode(bool);
    
signals:
    void cell_changed(miner::Location);
    void game_lost();
    
protected:
    void paintEvent(QPaintEvent*) override;
    void mouseReleaseEvent(QMouseEvent*) override;
    void wheelEvent(QWheelEvent*) override;
    bool is_point_mode() const { return scale_step_ == kPointModeScaleStep; }
    
private:
    void paint_cell(QPainter&, Location);
    void paint_point_cell(QPainter&, Location);
    size_t x2col(size_t x) { return x / get_scale_factor() / kCellSize; }
    size_t y2row(size_t y) { return y / get_scale_factor() / kCellSize; }
    size_t row2y(size_t row) { return get_scale_factor() * row * kCellSize; }
    size_t col2x(size_t col) { return get_scale_factor() * col * kCellSize; }
    float get_scale_factor() const { return scale_step_ * kScaleStep; }
    size_t scaled_cell_size() const { return get_scale_factor() * kCellSize; }
    void update_widget_size();
    
    GameBoardPtr board_;
    bool show_mines_{};
    bool rw_{};
    
    QColor cell_border_;
    QColor cell_opened_bg_;
    QColor cell_unknown_bg_;
    QFont cell_font_;
    QColor per_nr_colors_text_[8];
    QColor per_nr_colors_box_[8];
    size_t scale_step_ = 20;
    size_t prev_scale_step_ = 20; // go back to this when toggling scale mode
};

} // namespace miner
