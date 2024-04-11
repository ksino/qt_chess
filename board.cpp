#include "board.h"

Board::Board(QWidget *parent) : QWidget(parent)
{
//    QFrame *frame_2;
//	frame_2 = new QFrame(ui->frameBoard);
//	frame_2->setGeometry(QRect(0, 0, 57, 57));
//	frame_2->setFrameShape(QFrame::StyledPanel);
//	frame_2->setFrameShadow(QFrame::Raised);
//	frame_2->setStyleSheet("border-image: url(:/images/ba.gif)");
//	this->setStyleSheet("border-image: url(:/images/board.jpg)");
	QFrame *f = new QFrame(this);
	f->setGeometry(QRect(0, 0, BOARD_WIDTH, BOARD_HEIGHT));
	f->setStyleSheet("background-color: black");
}
