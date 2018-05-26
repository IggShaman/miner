#include "board.h"
#include "glpk_solver.h"
#include "scene.h"
#include "main_window.h"
#include "ui_configure_field_dialog.h"

namespace miner {

MainWindow::MainWindow() : ui_{new Ui::MainWindow} {
    ui_->setupUi(this);
    
    ui_->scrollArea->setAlignment(Qt::AlignVCenter | Qt::AlignHCenter);
    scene_ = new Scene;
    ui_->scrollArea->setWidget(scene_);
    scene_->show();
    connect(scene_, SIGNAL(cell_changed(miner::Location)),
            SLOT(cell_changed(miner::Location)));
    connect(scene_, SIGNAL(game_lost()), SLOT(game_lost()));
    
    connect(ui_->action_About, SIGNAL(triggered()), SLOT(action_about()));
    
    //QIcon(":/images/xxx.png")
    auto* a = new QAction("&New", this);
    a->setShortcuts({Qt::CTRL + Qt::Key_N, Qt::Key_F2});
    a->setStatusTip("New field");
    connect(a, SIGNAL(triggered()), SLOT(gen_new()));
    ui_->toolBar->addAction(a);
    
    a = new QAction("&Configure", this);
    a->setShortcuts({Qt::CTRL + Qt::Key_C, Qt::Key_F3});
    a->setStatusTip("Configure field");
    connect(a, SIGNAL(triggered()), SLOT(configure_field()));
    ui_->toolBar->addAction(a);
    
    show_mines_action_ = a = new QAction("Show &mines", this);
    a->setStatusTip("Show mines");
    a->setCheckable(true);
    connect(a, SIGNAL(toggled(bool)), SLOT(show_mines_toggled(bool)));
    ui_->toolBar->addAction(a);
    
    run_solver_action_ = a = new QAction("&Solve", this);
    a->setStatusTip("Solve");
    a->setCheckable(true);
    connect(a, SIGNAL(toggled(bool)), SLOT(run_solver(bool)));
    ui_->toolBar->addAction(a);

    a = new QAction("-", this);
    a->setStatusTip("Zoom out");
    a->setShortcuts({Qt::CTRL + Qt::Key_Minus});
    connect(a, SIGNAL(triggered()), scene_, SLOT(zoom_out()));
    ui_->toolBar->addAction(a);
    
    a = new QAction("+", this);
    a->setStatusTip("Zoom in");
    a->setShortcuts({Qt::CTRL + Qt::Key_Plus});
    connect(a, SIGNAL(triggered()), scene_, SLOT(zoom_in()));
    ui_->toolBar->addAction(a);
    
    a = new QAction("Point mode", this);
    a->setStatusTip("Set point mode");
    a->setShortcuts({Qt::CTRL + Qt::Key_0});
    connect(a, SIGNAL(triggered()), scene_, SLOT(set_point_mode()));
    ui_->toolBar->addAction(a);
    
    mines_info_label_ = new QLabel();
    statusBar()->addPermanentWidget(mines_info_label_);
    
    gen_new();
}


void MainWindow::gen_new() {
    show_mines_action_->setChecked(false);
    
    auto f = std::make_shared<Field>();
    f->gen_random(new_rows_, new_cols_, new_mines_);
    
    auto b = std::make_shared<GameBoard>();
    b->set_field(f);
    scene_->set_board(b);
    
    if (solver_)
	delete solver_;
    solver_ = new GlpkSolver{b};
    solver_->set_result_handler([this](auto ft, miner::Location l, int range){
	    //QThread::usleep(0); // slow down a bit for nice animation effect
	    QMetaObject::invokeMethod(this, "solver_result_slot", Qt::QueuedConnection,
				      Q_ARG(miner::GlpkSolver::feedback, ft),
				      Q_ARG(miner::Location, l),
				      Q_ARG(int, range));
	});
    solver_->start_async();
    
    update_cell_info();
    
    scene_->set_rw(true);
}


void MainWindow::configure_field() {
    QDialog d;
    Ui::ConfigureFieldDialog ui;
    ui.setupUi(&d);
    ui.rows_sb->setValue(new_rows_);
    ui.cols_sb->setValue(new_cols_);
    ui.mines_sb->setValue(new_mines_);
    if ( !d.exec() )
	return;
    
    new_rows_ = ui.rows_sb->value();
    new_cols_ = ui.cols_sb->value();
    new_mines_ = ui.mines_sb->value();
    gen_new();
}


void MainWindow::show_mines_toggled ( bool v ) {
    scene_->set_show_mines(v);
}


void MainWindow::run_solver ( bool v ) {
    if ( scene_->board()->game_lost() )
	return;
    
    if ( v ) {
	scene_->set_rw(false);
	solver_->resume();

    } else {
	solver_->suspend();
    }
}


void MainWindow::action_about() {
    QMessageBox::about(
      this, "Miner",
      "Miner: a simple mines game with solver.\n"
      "Copyright (C) 2015-2018 Igor Shevchenko <igor.shevchenko@gmail.com>\n"
      "This program comes with ABSOLUTELY NO WARRANTY.\n"
      "This is free software, and you are welcome to redistribute it\n"
      "under certain conditions. Look here for GPL3 license: http://www.gnu.org/licenses/");
}


void MainWindow::cell_changed(miner::Location l) {
    solver_->add_poi(l);
    update_cell_info();
}


void MainWindow::update_cell_info() {
    mines_info_label_->setText(
      QString("Mines: %1 / %2 Uncovered: %3 Left: %4")
      .arg(scene_->board()->mines_marked())
      .arg(scene_->board()->field()->mines_nr())
      .arg(scene_->board()->uncovered_nr())
      .arg(scene_->board()->left_nr()));
}


void MainWindow::game_lost() {
    scene_->board()->set_game_lost();
    show_mines_action_->setChecked(true);
}


void MainWindow::solver_result_slot(GlpkSolver::feedback ft, miner::Location l, int range ) {
    switch(ft) {
    case GlpkSolver::feedback::kSolved:
	scene_->update_box(l, range);
	update_cell_info();
	break;
	
    case GlpkSolver::feedback::kSuspended:
	scene_->set_rw(true);
	run_solver_action_->setChecked(false);
	break;
	
    case GlpkSolver::feedback::kGameLost:
	run_solver_action_->setChecked(false);
	game_lost();
	break;
    };
}

} // namespace miner
