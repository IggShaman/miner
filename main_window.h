#pragma once

#include "field.h"
#include "glpk_solver.h"

namespace Ui { class MainWindow; }

namespace miner {

class GameBoardWidget;

class MainWindow : public QMainWindow {
    Q_OBJECT;
public:
    MainWindow();
    ~MainWindow();
                 
private slots:
    void action_about();
    void gen_new();
    void configure_field();
    void show_mines_toggled(bool);
    void run_solver(bool);
    void cell_changed(miner::Location);
    void game_lost();
    void solver_result_slot(
      miner::Solver::FeedbackState, miner::Location center, size_t range);
    
private:
    void update_cell_info();
    
    std::unique_ptr<Ui::MainWindow> ui_;
    GameBoardWidget* game_board_widget_{};
    std::unique_ptr<GlpkSolver> solver_{};
    
    size_t new_rows_{3};
    size_t new_cols_{3};
    size_t new_mines_{2};
    
    QAction* run_solver_action_{};
    QAction* show_mines_action_{};
    QLabel* mines_info_label_{};
};

} // namespace miner
