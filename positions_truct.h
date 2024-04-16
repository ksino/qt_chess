#ifndef POSITIONSTRUCT_H
#define POSITIONSTRUCT_H

#include <stdint.h>
#include <const_data.h>

namespace Chess
{

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

	int MovePiece(int mv);                      // 搬一步棋的棋子
	void UndoMovePiece(int mv, int pcCaptured); // 撤消搬一步棋的棋子
	bool MakeMove(int mv);                      // 走一步棋
	int GenerateMoves(int *mvs) const;          // 生成所有走法
	bool LegalMove(int mv) const;               // 判断走法是否合理
	bool Checked(void) const;                   // 判断是否被将军
	bool IsMate(void);                          // 判断是否被杀








};
}
#endif // POSITIONSTRUCT_H
