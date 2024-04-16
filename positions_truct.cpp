#include "positions_truct.h"

namespace Chess
{

PositionStruct::PositionStruct()
{

}


// 搬一步棋的棋子
int PositionStruct::MovePiece(int mv)
{
	int sqSrc, sqDst, pc, pcCaptured;
	sqSrc = SRC(mv);
	sqDst = DST(mv);
	pcCaptured = ucpcSquares[sqDst];
	DelPiece(sqDst);
	pc = ucpcSquares[sqSrc];
	DelPiece(sqSrc);
	AddPiece(sqDst, pc);
	return pcCaptured;
}

// 撤消搬一步棋的棋子
void PositionStruct::UndoMovePiece(int mv, int pcCaptured)
{
	int sqSrc, sqDst, pc;
	sqSrc = SRC(mv);
	sqDst = DST(mv);
	pc = ucpcSquares[sqDst];
	DelPiece(sqDst);
	AddPiece(sqSrc, pc);
	AddPiece(sqDst, pcCaptured);
}

// 走一步棋
bool PositionStruct::MakeMove(int mv)
{
	int pc;
	pc = MovePiece(mv);
	if (Checked())
	{
		UndoMovePiece(mv, pc);
		return false;
	}
	ChangeSide();
	return true;
}

// 生成所有走法
int PositionStruct::GenerateMoves(int *mvs) const
{
	int i, j, nGenMoves, nDelta, sqSrc, sqDst;
	int pcSelfSide, pcOppSide, pcSrc, pcDst;
	// 生成所有走法，需要经过以下几个步骤：

	nGenMoves = 0;
	pcSelfSide = SIDE_TAG(sdPlayer);
	pcOppSide = OPP_SIDE_TAG(sdPlayer);
	for (sqSrc = 0; sqSrc < 256; sqSrc ++)
	{

		// 1. 找到一个本方棋子，再做以下判断：
		pcSrc = ucpcSquares[sqSrc];
		if ((pcSrc & pcSelfSide) == 0)
		{
			continue;
		}

		// 2. 根据棋子确定走法
		switch (pcSrc - pcSelfSide)
		{
		case PIECE_KING:
			for (i = 0; i < 4; i ++)
			{
				sqDst = sqSrc + ccKingDelta[i];
				if (!IN_FORT(sqDst))
				{
					continue;
				}
				pcDst = ucpcSquares[sqDst];
				if ((pcDst & pcSelfSide) == 0)
				{
					mvs[nGenMoves] = MOVE(sqSrc, sqDst);
					nGenMoves ++;
				}
			}
			break;
		case PIECE_ADVISOR:
			for (i = 0; i < 4; i ++)
			{
				sqDst = sqSrc + ccAdvisorDelta[i];
				if (!IN_FORT(sqDst))
				{
					continue;
				}
				pcDst = ucpcSquares[sqDst];
				if ((pcDst & pcSelfSide) == 0)
				{
					mvs[nGenMoves] = MOVE(sqSrc, sqDst);
					nGenMoves ++;
				}
			}
			break;
		case PIECE_BISHOP:
			for (i = 0; i < 4; i ++)
			{
				sqDst = sqSrc + ccAdvisorDelta[i];
				if (!(IN_BOARD(sqDst) && HOME_HALF(sqDst, sdPlayer) && ucpcSquares[sqDst] == 0))
				{
					continue;
				}
				sqDst += ccAdvisorDelta[i];
				pcDst = ucpcSquares[sqDst];
				if ((pcDst & pcSelfSide) == 0)
				{
					mvs[nGenMoves] = MOVE(sqSrc, sqDst);
					nGenMoves ++;
				}
			}
			break;
		case PIECE_KNIGHT:
			for (i = 0; i < 4; i ++)
			{
				sqDst = sqSrc + ccKingDelta[i];
				if (ucpcSquares[sqDst] != 0)
				{
					continue;
				}
				for (j = 0; j < 2; j ++)
				{
					sqDst = sqSrc + ccKnightDelta[i][j];
					if (!IN_BOARD(sqDst))
					{
						continue;
					}
					pcDst = ucpcSquares[sqDst];
					if ((pcDst & pcSelfSide) == 0)
					{
						mvs[nGenMoves] = MOVE(sqSrc, sqDst);
						nGenMoves ++;
					}
				}
			}
			break;
		case PIECE_ROOK:
			for (i = 0; i < 4; i ++)
			{
				nDelta = ccKingDelta[i];
				sqDst = sqSrc + nDelta;
				while (IN_BOARD(sqDst))
				{
					pcDst = ucpcSquares[sqDst];
					if (pcDst == 0)
					{
						mvs[nGenMoves] = MOVE(sqSrc, sqDst);
						nGenMoves ++;
					}
					else
					{
						if ((pcDst & pcOppSide) != 0)
						{
							mvs[nGenMoves] = MOVE(sqSrc, sqDst);
							nGenMoves ++;
						}
						break;
					}
					sqDst += nDelta;
				}
			}
			break;
		case PIECE_CANNON:
			for (i = 0; i < 4; i ++)
			{
				nDelta = ccKingDelta[i];
				sqDst = sqSrc + nDelta;
				while (IN_BOARD(sqDst))
				{
					pcDst = ucpcSquares[sqDst];
					if (pcDst == 0)
					{
						mvs[nGenMoves] = MOVE(sqSrc, sqDst);
						nGenMoves ++;
					}
					else
					{
						break;
					}
					sqDst += nDelta;
				}
				sqDst += nDelta;
				while (IN_BOARD(sqDst))
				{
					pcDst = ucpcSquares[sqDst];
					if (pcDst != 0)
					{
						if ((pcDst & pcOppSide) != 0)
						{
							mvs[nGenMoves] = MOVE(sqSrc, sqDst);
							nGenMoves ++;
						}
						break;
					}
					sqDst += nDelta;
				}
			}
			break;
		case PIECE_PAWN:
			sqDst = SQUARE_FORWARD(sqSrc, sdPlayer);
			if (IN_BOARD(sqDst))
			{
				pcDst = ucpcSquares[sqDst];
				if ((pcDst & pcSelfSide) == 0)
				{
					mvs[nGenMoves] = MOVE(sqSrc, sqDst);
					nGenMoves ++;
				}
			}
			if (AWAY_HALF(sqSrc, sdPlayer))
			{
				for (nDelta = -1; nDelta <= 1; nDelta += 2)
				{
					sqDst = sqSrc + nDelta;
					if (IN_BOARD(sqDst))
					{
						pcDst = ucpcSquares[sqDst];
						if ((pcDst & pcSelfSide) == 0)
						{
							mvs[nGenMoves] = MOVE(sqSrc, sqDst);
							nGenMoves ++;
						}
					}
				}
			}
			break;
		}
	}
	return nGenMoves;
}

// 判断走法是否合理
bool PositionStruct::LegalMove(int mv) const
{
	int sqSrc, sqDst, sqPin;
	int pcSelfSide, pcSrc, pcDst, nDelta;
	// 判断走法是否合法，需要经过以下的判断过程：

	// 1. 判断起始格是否有自己的棋子
	sqSrc = SRC(mv);
	pcSrc = ucpcSquares[sqSrc];
	pcSelfSide = SIDE_TAG(sdPlayer);
	if ((pcSrc & pcSelfSide) == 0)
	{
		return false;
	}

	// 2. 判断目标格是否有自己的棋子
	sqDst = DST(mv);
	pcDst = ucpcSquares[sqDst];
	if ((pcDst & pcSelfSide) != 0)
	{
		return false;
	}

	// 3. 根据棋子的类型检查走法是否合理
	switch (pcSrc - pcSelfSide)
	{
	case PIECE_KING:
		return IN_FORT(sqDst) && KING_SPAN(sqSrc, sqDst);
	case PIECE_ADVISOR:
		return IN_FORT(sqDst) && ADVISOR_SPAN(sqSrc, sqDst);
	case PIECE_BISHOP:
		return SAME_HALF(sqSrc, sqDst) && BISHOP_SPAN(sqSrc, sqDst) &&
		       ucpcSquares[BISHOP_PIN(sqSrc, sqDst)] == 0;
	case PIECE_KNIGHT:
		sqPin = KNIGHT_PIN(sqSrc, sqDst);
		return sqPin != sqSrc && ucpcSquares[sqPin] == 0;
	case PIECE_ROOK:
	case PIECE_CANNON:
		if (SAME_RANK(sqSrc, sqDst))
		{
			nDelta = (sqDst < sqSrc ? -1 : 1);
		}
		else if (SAME_FILE(sqSrc, sqDst))
		{
			nDelta = (sqDst < sqSrc ? -16 : 16);
		}
		else
		{
			return false;
		}
		sqPin = sqSrc + nDelta;
		while (sqPin != sqDst && ucpcSquares[sqPin] == 0)
		{
			sqPin += nDelta;
		}
		if (sqPin == sqDst)
		{
			return pcDst == 0 || pcSrc - pcSelfSide == PIECE_ROOK;
		}
		else if (pcDst != 0 && pcSrc - pcSelfSide == PIECE_CANNON)
		{
			sqPin += nDelta;
			while (sqPin != sqDst && ucpcSquares[sqPin] == 0)
			{
				sqPin += nDelta;
			}
			return sqPin == sqDst;
		}
		else
		{
			return false;
		}
	case PIECE_PAWN:
		if (AWAY_HALF(sqDst, sdPlayer) && (sqDst == sqSrc - 1 || sqDst == sqSrc + 1))
		{
			return true;
		}
		return sqDst == SQUARE_FORWARD(sqSrc, sdPlayer);
	default:
		return false;
	}
}

// 判断是否被将军
bool PositionStruct::Checked() const
{
	int i, j, sqSrc, sqDst;
	int pcSelfSide, pcOppSide, pcDst, nDelta;
	pcSelfSide = SIDE_TAG(sdPlayer);
	pcOppSide = OPP_SIDE_TAG(sdPlayer);
	// 找到棋盘上的帅(将)，再做以下判断：

	for (sqSrc = 0; sqSrc < 256; sqSrc ++)
	{
		if (ucpcSquares[sqSrc] != pcSelfSide + PIECE_KING)
		{
			continue;
		}

		// 1. 判断是否被对方的兵(卒)将军
		if (ucpcSquares[SQUARE_FORWARD(sqSrc, sdPlayer)] == pcOppSide + PIECE_PAWN)
		{
			return true;
		}
		for (nDelta = -1; nDelta <= 1; nDelta += 2)
		{
			if (ucpcSquares[sqSrc + nDelta] == pcOppSide + PIECE_PAWN)
			{
				return true;
			}
		}

		// 2. 判断是否被对方的马将军(以仕(士)的步长当作马腿)
		for (i = 0; i < 4; i ++)
		{
			if (ucpcSquares[sqSrc + ccAdvisorDelta[i]] != 0)
			{
				continue;
			}
			for (j = 0; j < 2; j ++)
			{
				pcDst = ucpcSquares[sqSrc + ccKnightCheckDelta[i][j]];
				if (pcDst == pcOppSide + PIECE_KNIGHT)
				{
					return true;
				}
			}
		}

		// 3. 判断是否被对方的车或炮将军(包括将帅对脸)
		for (i = 0; i < 4; i ++)
		{
			nDelta = ccKingDelta[i];
			sqDst = sqSrc + nDelta;
			while (IN_BOARD(sqDst))
			{
				pcDst = ucpcSquares[sqDst];
				if (pcDst != 0)
				{
					if (pcDst == pcOppSide + PIECE_ROOK || pcDst == pcOppSide + PIECE_KING)
					{
						return true;
					}
					break;
				}
				sqDst += nDelta;
			}
			sqDst += nDelta;
			while (IN_BOARD(sqDst))
			{
				int pcDst = ucpcSquares[sqDst];
				if (pcDst != 0)
				{
					if (pcDst == pcOppSide + PIECE_CANNON)
					{
						return true;
					}
					break;
				}
				sqDst += nDelta;
			}
		}
		return false;
	}
	return false;
}

// 判断是否被杀
bool PositionStruct::IsMate(void)
{
	int i, nGenMoveNum, pcCaptured;
	int mvs[MAX_GEN_MOVES];

	nGenMoveNum = GenerateMoves(mvs);
	for (i = 0; i < nGenMoveNum; i ++)
	{
		pcCaptured = MovePiece(mvs[i]);
		if (!Checked())
		{
			UndoMovePiece(mvs[i], pcCaptured);
			return false;
		}
		else
		{
			UndoMovePiece(mvs[i], pcCaptured);
		}
	}
	return true;
}

}
