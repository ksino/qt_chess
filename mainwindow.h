#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <const_data.h>
#include <positions_truct.h>

QT_BEGIN_NAMESPACE
namespace Ui
{
class MainWindow;
}
QT_END_NAMESPACE



class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	MainWindow(QWidget *parent = nullptr);
	~MainWindow();

private:
	Ui::MainWindow *ui;
	int sqSelected;     // 选中的格子
	int mvLast;         //上一步棋
	bool bFlipped;      // 是否翻转棋盘
	PositionStruct pos; // 局面实例
	//HBITMAP bmpBoard, bmpSelected, bmpPieces[24]; // 资源图片句柄

private:
	void init();
	// 绘制透明图片
	inline void DrawTransBmp(int xx, int yy, QString bmp)
	{
//		SelectObject(hdcTmp, bmp);
//		TransparentBlt2(hdc, xx, yy, SQUARE_SIZE, SQUARE_SIZE, hdcTmp, 0, 0, SQUARE_SIZE, SQUARE_SIZE, MASK_COLOR);
	}

	// 绘制棋盘
	void DrawBoard()
	{
		int x, y, xx, yy, sq, pc;

		// 画棋盘
//		hdcTmp = CreateCompatibleDC(hdc);
//		SelectObject(hdcTmp, Xqwl.bmpBoard);
//		BitBlt(hdc, 0, 0, BOARD_WIDTH, BOARD_HEIGHT, hdcTmp, 0, 0, SRCCOPY);
		// 画棋子
		for (x = FILE_LEFT; x <= FILE_RIGHT; x ++)
		{
			for (y = RANK_TOP; y <= RANK_BOTTOM; y ++)
			{
				if (bFlipped)
				{
					xx = BOARD_EDGE + (pos.FILE_FLIP(x) - FILE_LEFT) * SQUARE_SIZE;
					yy = BOARD_EDGE + (pos.RANK_FLIP(y) - RANK_TOP) * SQUARE_SIZE;
				}
				else
				{
					xx = BOARD_EDGE + (x - FILE_LEFT) * SQUARE_SIZE;
					yy = BOARD_EDGE + (y - RANK_TOP) * SQUARE_SIZE;
				}
				sq = pos.COORD_XY(x, y);
				pc = pos.ucpcSquares[sq];
				if (pc != 0)
				{
					DrawTransBmp(xx, yy, "bmpPieces[pc]");
				}
				if (sq == sqSelected || sq == pos.SRC(mvLast) || sq == pos.DST(mvLast))
				{
					DrawTransBmp(xx, yy, "bmpSelected");
				}
			}
		}
	}


	// 播放资源声音
	inline void PlayResWav(int nResId)
	{
		//PlaySound(MAKEINTRESOURCE(nResId), Xqwl.hInst, SND_ASYNC | SND_NOWAIT | SND_RESOURCE);
	}

	// "DrawSquare"参数
	const bool DRAW_SELECTED = true;

	// 绘制格子
	void DrawSquare(int sq, bool bSelected = false)
	{
		int sqFlipped, xx, yy, pc;

		sqFlipped = bFlipped ? pos.SQUARE_FLIP(sq) : sq;
		xx = BOARD_EDGE + (pos.FILE_X(sqFlipped) - FILE_LEFT) * SQUARE_SIZE;
		yy = BOARD_EDGE + (pos.RANK_Y(sqFlipped) - RANK_TOP) * SQUARE_SIZE;
		//BitBlt(Xqwl.hdc, xx, yy, SQUARE_SIZE, SQUARE_SIZE, Xqwl.hdcTmp, xx, yy, SRCCOPY);
		pc = pos.ucpcSquares[sq];
		if (pc != 0)
		{
			DrawTransBmp(xx, yy, "bmpPieces[pc]");
		}
		if (bSelected)
		{
			DrawTransBmp(xx, yy, "bmpSelected");
		}
	}

	// 点击格子事件处理
	void ClickSquare(int sq)
	{
		int pc;
//		Xqwl.hdc = GetDC(Xqwl.hWnd);
//		Xqwl.hdcTmp = CreateCompatibleDC(Xqwl.hdc);
		sq = bFlipped ? pos.SQUARE_FLIP(sq) : sq;
		pc = pos.ucpcSquares[sq];

		if ((pc & pos.SIDE_TAG(pos.sdPlayer)) != 0)
		{
			// 如果点击自己的子，那么直接选中该子
			if (sqSelected != 0)
			{
				DrawSquare(sqSelected);
			}
			sqSelected = sq;
			DrawSquare(sq, DRAW_SELECTED);
			if (mvLast != 0)
			{
				DrawSquare(pos.SRC(mvLast));
				DrawSquare(pos.DST(mvLast));
			}
			//PlayResWav(IDR_CLICK); // 播放点击的声音

		}
		else if (sqSelected != 0)
		{
			// 如果点击的不是自己的子，但有子选中了(一定是自己的子)，那么走这个子
			mvLast = pos.MOVE(sqSelected, sq);
			pos.MakeMove(mvLast);
			DrawSquare(sqSelected, DRAW_SELECTED);
			DrawSquare(sq, DRAW_SELECTED);
			sqSelected = 0;
			//PlayResWav(pc == 0 ? IDR_MOVE : IDR_CAPTURE); // 播放走子或吃子的声音
		}
	}

	// 初始化棋局
	void Startup(void)
	{
		pos.Startup();
		sqSelected = mvLast = 0;
	}


};
#endif // MAINWINDOW_H
