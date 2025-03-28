#include "board.h"
#include <QSound>
#include <QDebug>
#include <QMetaEnum>
#include <QMessageBox>
#include <QDateTime>

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

void Board::SetComputerFirst(bool ok)
{
	bFlipped = ok;
}

void Board::Restart()
{
	// TODO bug 此函数点击没有显示背景
	this->Startup();
	this->DrawBoard();
	if(bFlipped)
		this->ResponseMove();
}

void Board::init()
{
	// 设定棋盘大小和坐标
	this->setGeometry(QRect(FILE_LEFT, RANK_TOP, BOARD_WIDTH, BOARD_HEIGHT + RANK_BOTTOM));
	this->Startup();
	this->DrawBoard();
}

// 初始化棋局
void Board::Startup(void)
{
	srand((quint32) time(NULL));
	pos.Startup();
	pos.InitZobrist();
	sqSelected = 0;
	mvLast = 0;
	bGameOver = false;
	search = new Search(pos);
	search->LoadBook();

}

// 绘制棋盘
void Board::DrawBoard()
{
	// 画棋盘
	m_frameBoard = new QFrame(this);
	m_frameBoard->setGeometry(QRect(0, 0, BOARD_WIDTH, BOARD_HEIGHT));
#if !HIDE_PICTURE
	m_frameBoard->setStyleSheet("border-image: url(:/images/board.jpg)");
#endif

	//格子数组的长度为256的一维数组，可以看作是16 * 16的二维数组
	//实际棋盘占用的格子是9 * 10
	//其它多出的格子是辅助判断，一些棋子如马象的走法，需判断是否走出边界
	// TODO 如果开局是一些残局的时候，应该是刷新棋盘，不需再重新实例化
	for(int sq = 0; sq < 256; sq++)
	{
		//初始化棋盘中的格子
		if(ccInBoard[sq])
		{
			//计算格子坐标
			int x = SQ_X(sq);
			int y = SQ_Y(sq);
			// 指定格子的序号（索引）
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

/**
 * @brief 绘制格子图片
 * @param sq 索引
 * @param bSelected true显示选中背景
 */
void Board::DrawSquare(int sq, bool bSelected)
{
	// 玩家是黑方（后手）的话，翻转棋盘
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
#if !HIDE_PICTURE
	square[sq]->setPixmap(QPixmap(QString(":/images/%1.gif").arg(PIECE_NAME[pc])));
#else
	square[sq]->setPixmap(QPixmap(QString(":/images/thinking.gif")));
#endif
}

// 播放资源声音
inline void Board::PlayResWav(Resource::Sound name)
{
	QMetaEnum m = QMetaEnum::fromType<Resource::Sound>();
	//将Enum转化成字符串
	QString playName = QString(m.valueToKey(name));
	QSound::play(QString(":/sounds/%1.wav").arg(playName));
}

// 点击格子事件处理
void Board::ClickSquare(int sq)
{
	//sq 点击棋盘格子在数组中的索引
	//pc 点击棋盘格子在数组中的值（大于0的是棋子，为0则空白）
	int mv;
	int vlRep;
	sq = bFlipped ? SQUARE_FLIP(sq) : sq;
	int pc = pos.GetSquare(sq);
	L << "Click " << PIECE_NAME_CN[pos.GetSquare(sq)];

	if((pc & SIDE_TAG(pos.sdPlayer)) != 0)
	{
		// 如果点击自己的子，那么直接选中该子
		if(sqSelected != 0)
		{
			//如果之前点击的格子是自己的子，清除选中状态
			DrawSquare(sqSelected, false);
		}
		sqSelected = sq;
		DrawSquare(sq, DRAW_SELECTED);
		if(mvLast != 0)
		{
			//如果对方走了一步，清除起始和终点格子的选中状态
			DrawSquare(SRC(mvLast), false);
			DrawSquare(DST(mvLast), false);
		}
		// 播放点击的声音
		this->PlayResWav(Resource::click);
	}
	else if(sqSelected != 0 && !bGameOver)
	{
		// 如果点击的不是自己的子，但有子选中了(一定是自己的子)，那么走这个子
		//生成走法
		mv = MOVE(sqSelected, sq);
		//走法是否合法
		if(pos.LegalMove(mv))
		{
			//MakeMove有一个很巧妙的地方，即切换了走子方（sdPlayer）
			//即以下的IsMate() Checked()都是判断对方
			if(pos.MakeMove(mv))
			{
				move2Iccs(pos.GetSquare(sq), mv);
				mvLast = mv;
				// pos.MakeMove(mv)已经更新ucpcSquares
				// 现在刷新走棋界面
				// TODO 由于AI执行search->SearchMain()的耗时操作 线程阻塞
				DrawSquare(sqSelected, DRAW_SELECTED);
				DrawSquare(sq, DRAW_SELECTED);
				sqSelected = 0;
				// 检查重复局面
				vlRep = pos.RepStatus(3);
				if(pos.IsMate())
				{
					// 如果分出胜负，那么播放胜负的声音，并且弹出不带声音的提示框
					PlayResWav(Resource::win);
					MessageBoxMute("祝贺你取得胜利！");
					bGameOver = true;
				}
				else if(vlRep > 0)
				{
					vlRep = pos.RepValue(vlRep);
					// 注意："vlRep"是对电脑来说的分值
					PlayResWav(vlRep > WIN_VALUE ? Resource::loss : vlRep < -WIN_VALUE ? Resource::win : Resource::draw);
					MessageBoxMute(vlRep > WIN_VALUE ? "长打作负，请不要气馁！" :
					               vlRep < -WIN_VALUE ? "电脑长打作负，祝贺你取得胜利！" : "双方不变作和，辛苦了！");
					bGameOver = true;
				}
				else if(pos.nMoveNum > 100)
				{
					PlayResWav(Resource::draw);
					MessageBoxMute("超过自然限着作和，辛苦了！");
					bGameOver = true;
				}
				else
				{
					// 如果没有分出胜负，那么播放将军、吃子或一般走子的声音
					this->PlayResWav(pos.Checked() ? Resource::check : pos.Captured() ? Resource::capture : Resource::move);
					if(pos.Captured())
					{
						// TODO 20250321 16:17:35 为什么要清空历史走法
						// （chatGPT）当棋子被吃掉时，局面的可逆性被破坏，历史表不再适用
						// mvsList不是用于记录棋子每一步走法、悔棋
						pos.SetIrrev();
					}
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
	int vlRep;
	// 电脑走一步棋
	search->SearchMain();
	pos.MakeMove(search->context->mvResult);
	// 清除上一步棋的选择标记
	if(mvLast != 0)
	{
		DrawSquare(SRC(mvLast));
		DrawSquare(DST(mvLast));
	}
	// 把电脑走的棋标记出来
	mvLast = search->context->mvResult;
	move2Iccs(pos.GetSquare(DST(mvLast)), mvLast);
	DrawSquare(SRC(mvLast), DRAW_SELECTED);
	DrawSquare(DST(mvLast), DRAW_SELECTED);
	L << "02";
	// 检查重复局面
	vlRep = pos.RepStatus(3);
	if(pos.IsMate())
	{
		// 如果分出胜负，那么播放胜负的声音，并且弹出不带声音的提示框
		PlayResWav(Resource::loss);
		MessageBoxMute("请再接再厉！");
		bGameOver = true;
	}
	else if(vlRep > 0)
	{
		vlRep = pos.RepValue(vlRep);
		// 注意："vlRep"是对玩家来说的分值
		PlayResWav(vlRep < -WIN_VALUE ? Resource::loss : vlRep > WIN_VALUE ? Resource::win : Resource::draw);
		MessageBoxMute(vlRep < -WIN_VALUE ? "长打作负，请不要气馁！" :
		               vlRep > WIN_VALUE ? "电脑长打作负，祝贺你取得胜利！" : "双方不变作和，辛苦了！");
		bGameOver = true;
	}
	else if(pos.nMoveNum > 100)
	{
		PlayResWav(Resource::draw);
		MessageBoxMute("超过自然限着作和，辛苦了！");
		bGameOver = true;
	}
	else
	{
		// 如果没有分出胜负，那么播放将军、吃子或一般走子的声音
		PlayResWav(pos.Checked() ? Resource::check2 : pos.Captured() ? Resource::capture2 : Resource::move2);
		if(pos.Captured())
		{
			pos.SetIrrev();
		}
	}
}

void Board::MessageBoxMute(QString msg)
{
	QMessageBox::information(this, "象棋小巫师", msg, QMessageBox::Ok);
}

}
