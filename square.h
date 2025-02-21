#ifndef SQUARE_H
#define SQUARE_H

#include <QWidget>
#include <QLabel>
#include <QMouseEvent>

//棋盘中每个格子显示的控件
class Square : public QLabel
{
	Q_OBJECT
public:
	explicit Square(QWidget *parent, int sq);
	~Square();
signals:
	void clicked(int sq);
protected:
	// QLabel并没有clicked信号，声明鼠标左键点击事件
	virtual void mouseReleaseEvent(QMouseEvent * ev);
private:
	int sq; //格子索引
};

#endif // SQUARE_H
