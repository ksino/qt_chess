#include "board.h"
#include <QSound>
#include <QDebug>


Board::Board(QWidget *parent) : QWidget(parent)
{
	this->init();
}

void Board::init()
{
	// 画棋盘
	m_frameBoard = new QFrame(this);
	m_frameBoard->setGeometry(QRect(0, 0, BOARD_WIDTH, BOARD_HEIGHT));
	m_frameBoard->setStyleSheet("border-image: url(:/images/board.jpg)");

	for(int sq = 0; sq < 256; sq++)
	{
		if(ccInBoard[sq])
		{
			int y = BOARD_EDGE + (pos.RANK_Y(sq) - FILE_LEFT) * SQUARE_SIZE;
			int x = BOARD_EDGE + (pos.FILE_X(sq) - RANK_TOP) * SQUARE_SIZE;

//			square[sq] = new QFrame(m_frameBoard);
//			square[sq]->setGeometry(x, y, SQUARE_SIZE, SQUARE_SIZE);
//            square[sq]->setStyleSheet("border-image: url(:/images/oos.gif)");

			squareLabel[sq] = new QLabel(m_frameBoard);
			squareLabel[sq]->setGeometry(x, y, SQUARE_SIZE, SQUARE_SIZE);
			squareLabel[sq]->setStyleSheet("border-image: url(:/images/oos.gif)");
			squareLabel[sq]->setPixmap(QPixmap(QString::fromUtf8(":/images/ra.gif")));
//			connect(square[sq], &QFrame::clicked, this, &Board::onClicked);
			qDebug() << sq << x << y;
		}
		else
		{
			square[sq] = nullptr;
		}
	}
	//this->DrawBoard();
}

void Board::onClicked()
{
	qDebug() << "clicked";
}

// 绘制透明图片
inline void Board::DrawTransBmp(int xx, int yy, QString bmp)
{
//		SelectObject(hdcTmp, bmp);
//		TransparentBlt2(hdc, xx, yy, SQUARE_SIZE, SQUARE_SIZE, hdcTmp, 0, 0, SQUARE_SIZE, SQUARE_SIZE, MASK_COLOR);
}

void Board::createSquare(int x, int y)
{

}

// 绘制棋盘
void Board::DrawBoard()
{
	int x, y, xx, yy, sq, pc;
	// 画棋子
	for (x = FILE_LEFT; x <= FILE_RIGHT; x++)
	{
		for (y = RANK_TOP; y <= RANK_BOTTOM; y++)
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
inline void Board::PlayResWav(QString name)
{
	QSound::play(QString(":/sounds/%1.wav").arg(name));
}

// "DrawSquare"参数
const bool DRAW_SELECTED = true;

// 绘制格子
void Board::DrawSquare(int sq, bool bSelected)
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
void Board::ClickSquare(int sq)
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
void Board::Startup(void)
{
	pos.Startup();
	sqSelected = mvLast = 0;
}


