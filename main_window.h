#ifndef __MINER_MW_H_
#define __MINER_MW_H_

namespace Ui { class main_window; }

namespace miner {

class scene;

class main_window : public QMainWindow {
    Q_OBJECT;
public:
    main_window();
    ~main_window();
    
private:
    scene* scene_{};
    Ui::main_window* ui_{};
    
    size_t new_rows_{3};
    size_t new_cols_{3};
    size_t new_mines_{2};
    
private slots:
    void action_about();
    void gen_new();
    void configure_field();
    void show_mines_toggled(bool);
    void build_solver_lp();
};

} // namespace miner

#endif // __MINER_MW_H_
