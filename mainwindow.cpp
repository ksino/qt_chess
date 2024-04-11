#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFrame>

MainWindow::MainWindow(QWidget *parent)
	: QMainWindow(parent)
	, ui(new Ui::MainWindow)
{
	ui->setupUi(this);
	this->init();
}

MainWindow::~MainWindow()
{
	delete ui;
}

void MainWindow::init()
{
	m_board = new Board(ui->centralwidget);
	m_board->setGeometry(QRect(57, 57, BOARD_WIDTH, BOARD_HEIGHT));
}
