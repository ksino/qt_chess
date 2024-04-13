#include "square.h"

Square::Square(QWidget *parent, int sq): QLabel(parent), sq(sq)
{

}

Square::~Square()
{

}

// 重写鼠标释放时间 mouseReleaseEvent()
void Square::mouseReleaseEvent(QMouseEvent *ev)
{
	Q_UNUSED(ev)
	if(ev->button() == Qt::LeftButton)
	{
		emit clicked(sq);
	}
}
