#include "board.h"
#include <QSound>
#include <QDebug>
#include <QMetaEnum>

Board::Board(QWidget *parent) : QWidget(parent)
{
	this->Startup();
	this->init();
}

Board::~Board()
{
	delete m_frameBoard;
}

void Board::init()
{
	// 画棋盘
	m_frameBoard = new QFrame(this);
	m_frameBoard->setGeometry(QRect(0, 0, BOARD_WIDTH, BOARD_HEIGHT));
	m_frameBoard->setStyleSheet("border-image: url(:/images/board.jpg)");
	for(int sq = 0; sq < 256; sq++)
	{
		//初始化棋盘中的格子
		if(ccInBoard[sq])
		{
			//计算格子坐标
			int x = BOARD_EDGE + (pos.FILE_X(sq) - RANK_TOP) * SQUARE_SIZE;
			int y = BOARD_EDGE + (pos.RANK_Y(sq) - FILE_LEFT) * SQUARE_SIZE;
			square[sq] = new Square(m_frameBoard, sq);
			square[sq]->setGeometry(x, y, SQUARE_SIZE, SQUARE_SIZE);
			DrawTransBmp(sq);
			//绑定点击事件
			connect(square[sq], &Square::clicked, this, &Board::ClickSquare);
			//qDebug() << sq << x << y;
		}
		else
		{
			//非棋盘中的格子初始化为空指针
			square[sq] = nullptr;
		}
	}
	//this->DrawBoard();
}

// 绘制透明图片
inline void Board::DrawTransBmp(int sq, bool selected)
{
	L << "DrawTransBmp";
	QString s = selected ? "oos" : "oo";
	square[sq]->setStyleSheet(QString("border-image: url(:/images/%1.gif)").arg(s));
	int pc = pos.ucpcSquares[sq];
	square[sq]->setPixmap(QPixmap(QString(":/images/%1.gif").arg(QString::fromStdString(PIECE_NAME[pc]))));
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
				DrawTransBmp(sq);
			}
			if (sq == sqSelected || sq == pos.SRC(mvLast) || sq == pos.DST(mvLast))
			{
				DrawTransBmp(sq, true);
			}
		}
	}
}


// 播放资源声音
inline void Board::PlayResWav(Resource::Sound name)
{
	QMetaEnum m = QMetaEnum::fromType<Resource::Sound>();
	QString playName = QString(m.valueToKey(name));
	QSound::play(QString(":/sounds/%1.wav").arg(playName));
	qDebug() << "PlayResWav" << playName;
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
	DrawTransBmp(sq, bSelected);
}

// 点击格子事件处理
void Board::ClickSquare(int sq)
{
	L << "sq" << sq << bFlipped;
	square[sq]->setStyleSheet("border-image: url(:/images/oos.gif)");
	int pc;
	sq = bFlipped ? pos.SQUARE_FLIP(sq) : sq;
	pc = pos.ucpcSquares[sq];
	L << "sq" << sq << "pc" << pc;

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
		// 播放点击的声音
		this->PlayResWav(Resource::click);
	}
	else if (sqSelected != 0)
	{
		// 如果点击的不是自己的子，但有子选中了(一定是自己的子)，那么走这个子
		mvLast = pos.MOVE(sqSelected, sq);
		pos.MakeMove(mvLast);
		DrawSquare(sqSelected, DRAW_SELECTED);
		DrawSquare(sq, DRAW_SELECTED);
		sqSelected = 0;
		this->PlayResWav(pc == 0 ? Resource::draw : Resource::capture);
		//PlayResWav(pc == 0 ? IDR_MOVE : IDR_CAPTURE); // 播放走子或吃子的声音
	}
}

// 初始化棋局
void Board::Startup(void)
{
	pos.Startup();
	sqSelected = 0;
	mvLast = 0;
}


