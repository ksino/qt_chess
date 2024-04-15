#ifndef POSITIONSTRUCT_H
#define POSITIONSTRUCT_H

#include <stdint.h>
#include <const_data.h>

// 局面结构
class PositionStruct
{
public:
	PositionStruct();
	int sdPlayer;                   // 轮到谁走，0=红方，1=黑方
	uint8_t ucpcSquares[256];          // 棋盘上的棋子

	void Startup(void)              // 初始化棋盘
	{
		sdPlayer = 0;
		memcpy(ucpcSquares, cucpcStartup, 256);
	}

	uint8_t GetSquare(int sq)              // 取得格子上的棋子值
	{
		return ucpcSquares[sq];
	}

	void ChangeSide(void)           // 交换走子方
	{
		sdPlayer = 1 - sdPlayer;
	}

	void AddPiece(int sq, int pc)   // 在棋盘上放一枚棋子
	{
		ucpcSquares[sq] = pc;
	}

	void DelPiece(int sq)           // 从棋盘上拿走一枚棋子
	{
		ucpcSquares[sq] = 0;
	}

	void MovePiece(int mv);         // 搬一步棋的棋子
	void MakeMove(int mv)           // 走一步棋
	{
		MovePiece(mv);
		ChangeSide();
	}

	// 获得格子的横坐标（行数）
	inline int RANK_Y(int sq)
	{
		//sq / 16
		return sq >> 4;
	}

	// 获得格子的纵坐标（列数）
	inline int FILE_X(int sq)
	{
		// sq % 16
		return sq & 15;
	}

	// 根据纵坐标和横坐标获得格子
	inline int COORD_XY(int x, int y)
	{
		//x + (y * 16)
		return x + (y << 4);
	}

	// 翻转格子
	inline int SQUARE_FLIP(int sq)
	{
		return 254 - sq;
	}

	// 纵坐标水平镜像
	inline int FILE_FLIP(int x)
	{
		return 14 - x;
	}

	// 横坐标垂直镜像
	inline int RANK_FLIP(int y)
	{
		return 15 - y;
	}

	// 获得红黑标记(红子是8，黑子是16)
	inline int SIDE_TAG(int sd)
	{
		return 8 + (sd << 3);
	}

	// 获得对方红黑标记
	inline int OPP_SIDE_TAG(int sd)
	{
		return 16 - (sd << 3);
	}

	// 获得走法的起点
	inline int SRC(int mv)
	{
		return mv & 255;
	}

	// 获得走法的终点
	inline int DST(int mv)
	{
		return mv >> 8;
	}

	// 根据起点和终点获得走法
	inline int MOVE(int sqSrc, int sqDst)
	{
		//相当于两个字节拼接即 (sqDst << 8) | sqSrc
		return sqSrc + sqDst * 256;
	}
};

#endif // POSITIONSTRUCT_H
