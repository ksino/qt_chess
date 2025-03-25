#ifndef BOARD_H
#define BOARD_H

#include <QFrame>
#include <QWidget>
#include <QObject>
#include <QLabel>

#include "const_data.h"
#include "positions.h"
#include "square.h"
#include "search.h"


namespace Chess
{

// 窗口和绘图属性
const int SQUARE_SIZE = 56;
// 棋盘的边距
const int BOARD_EDGE = 8;
// 棋盘10行9列
const int BOARD_WIDTH = BOARD_EDGE + SQUARE_SIZE * 9 + BOARD_EDGE;
const int BOARD_HEIGHT = BOARD_EDGE + SQUARE_SIZE * 10 + BOARD_EDGE;

class Board : public QWidget
{
	Q_OBJECT
public:
	explicit Board(QWidget *parent = nullptr);
	~Board();

public slots:
	void Restart();

private slots:
	// 点击格子事件处理
	void ClickSquare(int sq);

public:
	// 播放资源声音
	inline void PlayResWav(Resource::Sound name);

private:
	// "DrawSquare"参数
	const bool DRAW_SELECTED = true;
	// 选中格子的索引
	int sqSelected;
	//上一步棋 通过将终点索引左移8位，和起点拼成一个数(dest << 8 | src)
	int mvLast;
	// 是否翻转棋盘
	bool bFlipped {false};
	bool bGameOver;  // 是否游戏结束(不让继续玩下去)
	PositionStruct pos; // 局面实例
	//棋盘显示控件
	QFrame *m_frameBoard;
	//棋盘格子数组
	Square* square[256];
	Search *search {nullptr};

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
	void ResponseMove(void);
	void MessageBoxMute(QString msg);
};
}
#endif // BOARD_H
