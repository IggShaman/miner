#pragma once

#include "field.h"
#include "glpk_solver.h"
#include "ui_main_window.h"

namespace Ui { class MainWindow; }

namespace miner {

class Scene;

class MainWindow : public QMainWindow {
    Q_OBJECT;
public:
    MainWindow();

private slots:
    void action_about();
    void gen_new();
    void configure_field();
    void show_mines_toggled(bool);
    void run_solver(bool);
    void cell_changed(miner::Location);
    void game_lost();
    void solver_result_slot(
      miner::GlpkSolver::feedback, miner::Location center, int range);
private:
    void update_cell_info();
    
    std::unique_ptr<Ui::MainWindow> ui_;
    Scene* scene_{};
    GlpkSolver* solver_{};
    
    size_t new_rows_{3};
    size_t new_cols_{3};
    size_t new_mines_{2};
    
    QAction* run_solver_action_{};
    QAction* show_mines_action_{};
    QLabel* mines_info_label_;
};

} // namespace miner
