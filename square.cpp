#include "square.h"

Square::Square(QWidget *parent, int sq)
	: QLabel(parent), sq(sq)
{

}

Square::~Square()
{

}

// 重写鼠标释放时间 mouseReleaseEvent()
void Square::mouseReleaseEvent(QMouseEvent *ev)
{
	Q_UNUSED(ev) // 避免未使用参数的警告

	// 如果是鼠标左键点击，则发射 clicked 信号
	if(ev->button() == Qt::LeftButton)
	{
		emit clicked(sq);
	}
}
