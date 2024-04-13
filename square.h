#ifndef SQUARE_H
#define SQUARE_H

#include <QWidget>
#include <QLabel>
#include <QMouseEvent>

class Square : public QLabel
{
	Q_OBJECT
public:
	explicit Square(QWidget *parent, int sq);
	~Square();
signals:
	void clicked(int sq);
protected:
	// 声明鼠标左键点击事件
	virtual void mouseReleaseEvent(QMouseEvent * ev);
private:
	int sq;
};

#endif // SQUARE_H
