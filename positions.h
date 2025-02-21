#ifndef POSITIONS_H
#define POSITIONS_H

#include <stdint.h>
#include <const_data.h>
#include "zobrist.h"

namespace Chess
{
// 历史走法信息(占4字节)
class MoveStruct
{
public:
	quint16 wmv;          // 走法信息
	quint8 ucpcCaptured;  // 被吃掉的棋子
	quint8 ucbCheck;      // 是否将军
	quint32 dwKey;        // 哈希键值

	void Set(int mv, int pcCaptured, bool bCheck, quint32 dwKey_)
	{
		wmv = static_cast<quint16>(mv);
		ucpcCaptured = static_cast<quint8>(pcCaptured);
		ucbCheck = static_cast<quint8>(bCheck);
		dwKey = dwKey_;
	}
};

// 局面结构
class PositionStruct
{
public:
	PositionStruct();
	int sdPlayer;                   // 轮到谁走，0=红方，1=黑方
	uint8_t ucpcSquares[256];          // 棋盘上的棋子
	int vlWhite, vlBlack;           // 红、黑双方的子力价值
	int nDistance;                  // 距离根节点的步数
	int nMoveNum;        // 历史走法数
	Zobrist zobrist;
	MoveStruct mvsList[MAX_MOVES];  // 历史走法信息列表
	ZobristStruct zobr;             // Zobrist

	void SetIrrev(void)             // 清空(初始化)历史走法信息
	{
		mvsList[0].Set(0, 0, Checked(), zobr.dwKey);
		nMoveNum = 1;
	}

	void Startup(void);              // 初始化棋盘

	void InitZobrist(void)
	{
		zobrist.InitZobrist();
	}

	uint8_t GetSquare(int sq)              // 取得格子上的棋子值
	{
		return ucpcSquares[sq];
	}

	void ChangeSide(void)           // 交换走子方
	{
		sdPlayer = 1 - sdPlayer;
		zobr.Xor(zobrist.Player);
	}

	void AddPiece(int sq, int pc)   // 在棋盘上放一枚棋子
	{
		ucpcSquares[sq] = pc;
		// 红方加分，黑方(注意"cucvlPiecePos"取值要颠倒)减分
		if(pc < 16)
		{
			vlWhite += cucvlPiecePos[pc - 8][sq];
			zobr.Xor(zobrist.Table[pc - 8][sq]);
		}
		else
		{
			vlBlack += cucvlPiecePos[pc - 16][SQUARE_FLIP(sq)];
			zobr.Xor(zobrist.Table[pc - 9][sq]);
		}
	}

	void DelPiece(int sq, int pc)   // 从棋盘上拿走一枚棋子
	{
		ucpcSquares[sq] = 0;
		// 红方减分，黑方(注意"cucvlPiecePos"取值要颠倒)加分
		if(pc < 16)
		{
			vlWhite -= cucvlPiecePos[pc - 8][sq];
			zobr.Xor(zobrist.Table[pc - 8][sq]);
		}
		else
		{
			vlBlack -= cucvlPiecePos[pc - 16][SQUARE_FLIP(sq)];
			zobr.Xor(zobrist.Table[pc - 9][sq]);
		}
	}

	int Evaluate(void) const        // 局面评价函数
	{
		return (sdPlayer == 0 ? vlWhite - vlBlack : vlBlack - vlWhite) + ADVANCED_VALUE;
	}

	bool InCheck(void) const        // 是否被将军
	{
		return mvsList[nMoveNum - 1].ucbCheck;
	}

	bool Captured(void) const       // 上一步是否吃子
	{
		return mvsList[nMoveNum - 1].ucpcCaptured != 0;
	}
	int MovePiece(int mv);                      // 搬一步棋的棋子
	void UndoMovePiece(int mv, int pcCaptured); // 撤消搬一步棋的棋子
	bool MakeMove(int mv);                      // 走一步棋

	void UndoMakeMove(void)   // 撤消走一步棋
	{
		nDistance --;
		nMoveNum --;
		ChangeSide();
		UndoMovePiece(mvsList[nMoveNum].wmv, mvsList[nMoveNum].ucpcCaptured);
	}
	void NullMove(void)                         // 走一步空步
	{
		quint32  dwKey;
		dwKey = zobr.dwKey;
		ChangeSide();
		mvsList[nMoveNum].Set(0, 0, false, dwKey);
		nMoveNum ++;
		nDistance ++;
	}
	void UndoNullMove(void)                     // 撤消走一步空步
	{
		nDistance --;
		nMoveNum --;
		ChangeSide();
	}

	// 生成所有走法，如果"bCapture"为"TRUE"则只生成吃子走法
	int GenerateMoves(int *mvs, bool bCapture = false) const;
	bool LegalMove(int mv) const;               // 判断走法是否合理
	bool Checked(void) const;                   // 判断是否被将军
	bool IsMate(void);                          // 判断是否被杀
	int RepStatus(int nRecur = 1) const;        // 检测重复局面
	int RepValue(int nRepStatus) const          // 重复局面分值
	{
		int vlReturn;
		vlReturn = ((nRepStatus & 2) == 0 ? 0 : nDistance - MATE_VALUE) +
		           ((nRepStatus & 4) == 0 ? 0 : MATE_VALUE - nDistance);
		return vlReturn == 0 ? -DRAW_VALUE : vlReturn;
	}
	bool NullOkay(void) const                   // 判断是否允许空步裁剪
	{
		return (sdPlayer == 0 ? vlWhite : vlBlack) > NULL_MARGIN;
	}








};
}
#endif // POSITIONS_H
