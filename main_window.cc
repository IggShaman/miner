#include "board.h"
#include "solver.h"
#include "scene.h"
#include "main_window.h"
#include "ui_main_window.h"
#include "ui_configure_field_dialog.h"

namespace miner {

main_window::main_window() : ui_{new Ui::main_window} {
    ui_->setupUi(this);
    
    ui_->scrollArea->setAlignment(Qt::AlignVCenter | Qt::AlignHCenter);
    scene_ = new scene;
    ui_->scrollArea->setWidget(scene_);
    scene_->show();
    connect(scene_, SIGNAL(cell_changed(miner::coord)), SLOT(cell_changed(miner::coord)));
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
    
    a = new QAction("Minimal zoom", this);
    a->setStatusTip("Set minimal zoom");
    a->setShortcuts({Qt::CTRL + Qt::Key_0});
    connect(a, SIGNAL(triggered()), scene_, SLOT(zoom_min()));
    ui_->toolBar->addAction(a);
    
    mines_info_label_ = new QLabel();
    statusBar()->addPermanentWidget(mines_info_label_);
    
    gen_new();
}


main_window::~main_window() {
    delete ui_;
}


void main_window::gen_new() {
    show_mines_action_->setChecked(false);
    
    auto f = std::make_shared<field>();
    f->gen_random(new_rows_, new_cols_, new_mines_);
    
    auto b = std::make_shared<board>();
    b->set_field(f);
    scene_->set_board(b);
    
    if ( solver_ )
	delete solver_;
    solver_ = new solver(b);
    solver_->set_result_handler([this](auto ft, miner::coord c){
	    QMetaObject::invokeMethod(this, "solver_result_slot", Qt::QueuedConnection,
				      Q_ARG(miner::solver::feedback, ft),
				      Q_ARG(miner::coord, c));
	});
    solver_->start_async();
    
    update_cell_info();
    
    scene_->set_rw(true);
}


void main_window::configure_field() {
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


void main_window::show_mines_toggled ( bool v ) {
    scene_->set_show_mines(v);
}


void main_window::run_solver ( bool v ) {
    if ( scene_->board()->game_lost() )
	return;
    
    if ( v ) {
	scene_->set_rw(false);
	solver_->resume();

    } else {
	solver_->suspend();
    }
}


void main_window::action_about() {
    QMessageBox::about(this, "Miner",
		       "Miner: a simple mines game with solver.\n"
		       "Copyright (C) 2015 Igor Shevchenko <igor.shevchenko@gmail.com>\n"
		       "This program comes with ABSOLUTELY NO WARRANTY.\n"
		       "This is free software, and you are welcome to redistribute it\n"
		       "under certain conditions. Look here for GPL3 license: http://www.gnu.org/licenses/");
}


void main_window::cell_changed ( miner::coord c ) {
    solver_->add_poi(c);
    update_cell_info();
}


void main_window::update_cell_info() {
    mines_info_label_->setText(QString("Mines: %1 / %2 Uncovered: %3 Left: %4")
			       .arg(scene_->board()->mines_marked())
			       .arg(scene_->board()->field()->mines_nr())
			       .arg(scene_->board()->uncovered_nr())
			       .arg(scene_->board()->left_nr()));
}


void main_window::game_lost() {
    scene_->board()->set_game_lost();
    show_mines_action_->setChecked(true);
}


void main_window::solver_result_slot ( solver::feedback ft, miner::coord c ) {
    switch(ft) {
    case solver::feedback::kSolved:
	scene_->update_cell(c);
	update_cell_info();
	break;
	
    case solver::feedback::kSuspended:
	scene_->set_rw(true);
	run_solver_action_->setChecked(false);
	break;
	
    case solver::feedback::kGameLost:
	run_solver_action_->setChecked(false);
	game_lost();
	break;
    };
}

} // namespace miner
