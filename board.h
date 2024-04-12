#ifndef BOARD_H
#define BOARD_H

#include <QFrame>
#include <QWidget>
#include <QObject>
#include <QLabel>
#include <const_data.h>
#include <positions_truct.h>

class Board : public QWidget
{
	Q_OBJECT
public:
	explicit Board(QWidget *parent = nullptr);

signals:
private slots:
	void onClicked();
public:
	inline void PlayResWav(QString name);
private:
	int sqSelected;     // 选中的格子
	int mvLast;         //上一步棋
	bool bFlipped;      // 是否翻转棋盘
	PositionStruct pos; // 局面实例
	QFrame *m_frameBoard;
	QFrame* square[256];
	QLabel* squareLabel[256];
	//HBITMAP bmpBoard, bmpSelected, bmpPieces[24]; // 资源图片句柄
private:
	void init();
	// 绘制透明图片
	inline void DrawTransBmp(int xx, int yy, QString bmp);

	// 绘制棋盘
	void DrawBoard();

	// 播放资源声音

	// "DrawSquare"参数
	const bool DRAW_SELECTED = true;

	// 绘制格子
	void DrawSquare(int sq, bool bSelected = false);

	// 点击格子事件处理
	void ClickSquare(int sq);

	// 初始化棋局
	void Startup(void);
	void createSquare(int x, int y);

};

#endif // BOARD_H
