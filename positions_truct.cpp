#include "positions_truct.h"

namespace Chess
{

PositionStruct::PositionStruct()
{

}

// 搬一步棋的棋子
void PositionStruct::MovePiece(int mv)
{
	int sqSrc, sqDst, pc;
	sqSrc = SRC(mv);
	sqDst = DST(mv);
	DelPiece(sqDst);
	pc = ucpcSquares[sqSrc];
	DelPiece(sqSrc);
	AddPiece(sqDst, pc);
}
}
