#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFrame>

MainWindow::MainWindow(QWidget *parent)
	: QMainWindow(parent)
	, ui(new Ui::MainWindow)
{
	ui->setupUi(this);
//	ui->frame->setFrameStyle(QFrame.Panel | QFrame.Sunken);
//	ui->frameBoard->setStyleSheet("background-color: gray");
	ui->frameBoard->setStyleSheet("border-image: url(:/images/board.jpg)");
	ui->frame_2->setStyleSheet("background-color: pink");
	this->init();



}

MainWindow::~MainWindow()
{
	delete ui;
}

void MainWindow::init()
{
	QFrame *frame_2;
	frame_2 = new QFrame(ui->frameBoard);
	frame_2->setGeometry(QRect(0, 0, 57, 57));
	frame_2->setFrameShape(QFrame::StyledPanel);
	frame_2->setFrameShadow(QFrame::Raised);
	frame_2->setStyleSheet("border-image: url(:/images/ba.gif)");
//    frame_2->setStyleSheet("background-color: red");

	QFrame *frame_3;
	frame_3 = new QFrame(ui->frameBoard);
	frame_3->setGeometry(QRect(57, 0, 57, 57));
	frame_3->setFrameShape(QFrame::StyledPanel);
	frame_3->setFrameShadow(QFrame::Raised);
	frame_3->setStyleSheet("border-image: url(:/images/oo.gif)");
}
