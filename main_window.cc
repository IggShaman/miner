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
    
    a = new QAction("Show &mines", this);
    a->setStatusTip("Show mines");
    a->setCheckable(true);
    connect(a, SIGNAL(toggled(bool)), SLOT(show_mines_toggled(bool)));
    ui_->toolBar->addAction(a);
    
    a = new QAction("&Run solver", this);
    a->setStatusTip("&Run solver");
    connect(a, SIGNAL(triggered()), SLOT(build_solver_lp()));
    ui_->toolBar->addAction(a);
    
    gen_new();
}


main_window::~main_window() {
    delete ui_;
}


void main_window::gen_new() {
    auto f = std::make_shared<field>();
    f->gen_random(new_rows_, new_cols_, new_mines_);
    auto b = scene_->board();
    b->reset(f);
    
    scene_->set_board(b);
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


void main_window::build_solver_lp() {
    scene_->solver()->build_problem();
    scene_->update();
}


void main_window::action_about() {
    QMessageBox::about(this, "Miner",
		       "Miner: a simple mines game with solver.\n"
		       "Copyright (C) 2015 Igor Shevchenko <igor.shevchenko@gmail.com>\n"
		       "This program comes with ABSOLUTELY NO WARRANTY.\n"
		       "This is free software, and you are welcome to redistribute it\n"
		       "under certain conditions. Look here for GPL3 license: http://www.gnu.org/licenses/");
}

} // namespace miner
