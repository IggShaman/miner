#ifndef __MINER_MW_H_
#define __MINER_MW_H_

#include "field.h"

namespace Ui { class main_window; }

namespace miner {

class scene;
class solver;

class main_window : public QMainWindow {
    Q_OBJECT;
public:
    main_window();
    ~main_window();
    
private slots:
    void action_about();
    void gen_new();
    void configure_field();
    void show_mines_toggled ( bool );
    void run_solver ( bool );
    void cell_changed ( miner::coord );
    void game_lost();
    
private:
    void update_cell_info();
    
    Ui::main_window* ui_{};
    scene* scene_{};
    solver* solver_{};
    
    size_t new_rows_{3};
    size_t new_cols_{3};
    size_t new_mines_{2};

    QAction* run_solver_action_{};
    QAction* show_mines_action_{};
    QLabel* mines_info_label_;
};

} // namespace miner

#endif // __MINER_MW_H_
