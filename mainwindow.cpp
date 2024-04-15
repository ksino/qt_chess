#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "const_data.h"

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
	delete m_board;
}

void MainWindow::init()
{
	this->setWindowTitle(cszAbout);
	//新建棋盘实例
	m_board = new Board(ui->centralwidget);
	//设定棋盘大小和坐标
	m_board->setGeometry(QRect(57, 57, BOARD_WIDTH, BOARD_HEIGHT));
}
