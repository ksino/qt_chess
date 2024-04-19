#include "board.h"
#include <QSound>
#include <QDebug>
#include <QMetaEnum>
#include <QMessageBox>

namespace Chess
{

Board::Board(QWidget *parent) : QWidget(parent)
{
	this->init();
}

Board::~Board()
{
	delete m_frameBoard;
}

void Board::init()
{
	this->Startup();
	this->DrawBoard();
//	if (Xqwl.bFlipped)
//	{
//		Xqwl.hdc = hdc;
//		Xqwl.hdcTmp = CreateCompatibleDC(Xqwl.hdc);
//		ResponseMove();
//		DeleteDC(Xqwl.hdcTmp);
//	}
}

// 初始化棋局
void Board::Startup(void)
{
	pos.Startup();
	sqSelected = 0;
	mvLast = 0;
	search = new Search(pos);
//	search->pos = pos;

}

// 绘制棋盘
void Board::DrawBoard()
{
	// 画棋盘
	m_frameBoard = new QFrame(this);
	m_frameBoard->setGeometry(QRect(0, 0, BOARD_WIDTH, BOARD_HEIGHT));
#ifndef __DEBUG
	m_frameBoard->setStyleSheet("border-image: url(:/images/board.jpg)");
#endif

	//格子数组的长度为256的一维数组，可以看作是16 * 16的二维数组
	//实际棋盘占用的格子是9 * 10
	//其它多出的格子是辅助判断，一些棋子如马象的走法，需判断是否走出边界
	//TODO 如果开局是一些残局的时候，应该是刷新棋盘，不需再重新实例化
	for(int sq = 0; sq < 256; sq++)
	{
		//初始化棋盘中的格子
		if(ccInBoard[sq])
		{
			//计算格子坐标
			int x = SQ_X(sq);
			int y = SQ_Y(sq);
			square[sq] = new Square(m_frameBoard, sq);
			square[sq]->setGeometry(x, y, SQUARE_SIZE, SQUARE_SIZE);
			DrawSquare(sq);
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
}

//根据sq计算控件的X坐标
int Board::SQ_X(int sq)
{
	return BOARD_EDGE + (FILE_X(sq) - FILE_LEFT) * SQUARE_SIZE;
}

//根据sq计算控件的Y坐标
int Board::SQ_Y(int sq)
{
	return BOARD_EDGE + (RANK_Y(sq) - RANK_TOP) * SQUARE_SIZE;
}

// 绘制格子
void Board::DrawSquare(int sq, bool bSelected)
{
	int sqFlipped = bFlipped ? SQUARE_FLIP(sq) : sq;
	DrawTransBmp(sqFlipped, bSelected);
}

// 绘制图片
inline void Board::DrawTransBmp(int sq, bool selected)
{
	//绘制背景图片，选中或者透明
	QString s = selected ? "oos" : "oo";
	square[sq]->setStyleSheet(QString("border-image: url(:/images/%1.gif)").arg(s));
	//根据格子上的棋子值绘制对应前景图片
	int pc = pos.GetSquare(sq);
#ifndef __DEBUG
	square[sq]->setPixmap(QPixmap(QString(":/images/%1.gif").arg(PIECE_NAME[pc])));
#else
	square[sq]->setPixmap(QPixmap(QString(":/images/thinking.gif")));
#endif
	L << "DrawTransBmp" << PIECE_NAME_CN[pc];
}

// 播放资源声音
inline void Board::PlayResWav(Resource::Sound name)
{
	QMetaEnum m = QMetaEnum::fromType<Resource::Sound>();
	//将Enum转化成字符串
	QString playName = QString(m.valueToKey(name));
	QSound::play(QString(":/sounds/%1.wav").arg(playName));
	L << "PlayResWav" << playName;
}

// 点击格子事件处理
void Board::ClickSquare(int sq)
{
	//sq 点击棋子在数组中的索引
	//pc 点击棋子在数组中的值
	int mv;
	sq = bFlipped ? SQUARE_FLIP(sq) : sq;
	int pc = pos.GetSquare(sq);
	L << "sq=" << sq << "pc=" << pc << "sqSelected=" << sqSelected;

	if ((pc & SIDE_TAG(pos.sdPlayer)) != 0)
	{
		// 如果点击自己的子，那么直接选中该子
		if (sqSelected != 0)
		{
			//如果之前点击的格子是自己的子，清除选中状态
			DrawSquare(sqSelected, false);
		}
		sqSelected = sq;
		DrawSquare(sq, DRAW_SELECTED);
		if (mvLast != 0)
		{
			//如果对方走了一步，清除起始和终点格子的选中状态
			DrawSquare(SRC(mvLast), false);
			DrawSquare(DST(mvLast), false);
		}
		// 播放点击的声音
		this->PlayResWav(Resource::click);
	}
	else if (sqSelected != 0)
	{
		// 如果点击的不是自己的子，但有子选中了(一定是自己的子)，那么走这个子
		//生成走法
		mv = MOVE(sqSelected, sq);
		//走法是否合法
		if(pos.LegalMove(mv))
		{
			//MakeMove有一个很巧妙的地方，即切换了走子方（sdPlayer）
			//即以下的IsMate() Checked()都是判断对方
			if(pos.MakeMove(mv, pc))
			{
				mvLast = mv;
				DrawSquare(sqSelected, DRAW_SELECTED);
				DrawSquare(sq, DRAW_SELECTED);
				sqSelected = 0;
				if (pos.IsMate())
				{
					// 如果分出胜负，那么播放胜负的声音，并且弹出不带声音的提示框
					PlayResWav(Resource::win);
					MessageBoxMute("祝贺你取得胜利！");
				}
				else
				{
					// 如果没有分出胜负，那么播放将军、吃子或一般走子的声音
					this->PlayResWav(pos.Checked() ? Resource::check : pc != 0 ? Resource::capture : Resource::move);
					this->ResponseMove(); // 轮到电脑走棋
				}
			}
			else
			{
				PlayResWav(Resource::illegal); // 播放被将军的声音
			}
		}
		// 如果根本就不符合走法(例如马不走日字)，那么程序不予理会
	}
}

// 电脑回应一步棋
void Board::ResponseMove(void)
{
	L << "ResponseMove";
	int pcCaptured;
	// 电脑走一步棋
	//SetCursor((HCURSOR) LoadImage(NULL, IDC_WAIT, IMAGE_CURSOR, 0, 0, LR_DEFAULTSIZE | LR_SHARED));
	search->SearchMain();
	//SetCursor((HCURSOR) LoadImage(NULL, IDC_ARROW, IMAGE_CURSOR, 0, 0, LR_DEFAULTSIZE | LR_SHARED));
	pos.MakeMove(search->mvResult, pcCaptured);
	// 清除上一步棋的选择标记
	DrawSquare(SRC(mvLast));
	DrawSquare(DST(mvLast));
	// 把电脑走的棋标记出来
	mvLast = search->mvResult;
	DrawSquare(SRC(mvLast), DRAW_SELECTED);
	DrawSquare(DST(mvLast), DRAW_SELECTED);
	if (pos.IsMate())
	{
		// 如果分出胜负，那么播放胜负的声音，并且弹出不带声音的提示框
		PlayResWav(Resource::loss);
		MessageBoxMute("请再接再厉！");
	}
	else
	{
		// 如果没有分出胜负，那么播放将军、吃子或一般走子的声音
		PlayResWav(pos.Checked() ? Resource::check2 : pcCaptured != 0 ? Resource::capture2 : Resource::move2);
	}
}

void Board::MessageBoxMute(QString msg)
{
	QMessageBox::information(this, "象棋小巫师", msg, QMessageBox::Ok);
}

}
