#ifndef BOARD_H
#define BOARD_H

#include <QFrame>
#include <QWidget>
#include <QObject>
#include <QLabel>
#include <const_data.h>
#include <positions_truct.h>
#include "square.h"

namespace Chess
{

class Board : public QWidget
{
	Q_OBJECT
public:
	explicit Board(QWidget *parent = nullptr);
	~Board();

private slots:
	// 点击格子事件处理
	void ClickSquare(int sq);

public:
	// 播放资源声音
	inline void PlayResWav(Resource::Sound name);

private:
	// "DrawSquare"参数
	const bool DRAW_SELECTED = true;
	// 选中的格子
	int sqSelected;
	//上一步棋
	int mvLast;
	// 是否翻转棋盘
	bool bFlipped {false};
	PositionStruct pos; // 局面实例
	//棋盘显示控件
	QFrame *m_frameBoard;
	//棋盘格子数组
	Square* square[256];

private:
	void init();

	// 绘制图片
	inline void DrawTransBmp(int sq, bool selected = false);

	// 绘制棋盘
	void DrawBoard();

	// 绘制格子
	void DrawSquare(int sq, bool bSelected = false);

	// 初始化棋局
	void Startup(void);

	int SQ_X(int sq);
	int SQ_Y(int sq);
};
}
#endif // BOARD_H
