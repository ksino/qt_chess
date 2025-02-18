#include "positions_struct.h"

namespace Chess
{

PositionStruct::PositionStruct()
{

}

// 初始化棋盘
void PositionStruct::Startup(void)
{
	int sq, pc;
	sdPlayer = 0;
	vlWhite = 0;
	vlBlack = 0;
	nDistance = 0;
	memset(ucpcSquares, 0, 256);
	for(sq = 0; sq < 256; sq ++)
	{
		pc = cucpcStartup[sq];
		if(pc != 0)
		{
			AddPiece(sq, pc);
		}
	}
}

// 搬一步棋的棋子
int PositionStruct::MovePiece(int mv)
{
	int sqSrc, sqDst, pc, pcCaptured;
	sqSrc = SRC(mv);
	sqDst = DST(mv);
	pcCaptured = ucpcSquares[sqDst];
	if(pcCaptured != 0)
	{
		DelPiece(sqDst, pcCaptured);
	}
	pc = ucpcSquares[sqSrc];
	DelPiece(sqSrc, pc);
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
	DelPiece(sqDst, pc);
	AddPiece(sqSrc, pc);
	if(pcCaptured != 0)
	{
		AddPiece(sqDst, pcCaptured);
	}
}

// 走一步棋
bool PositionStruct::MakeMove(int mv, int &pcCaptured)
{
	pcCaptured = MovePiece(mv);
	if(Checked())
	{
		UndoMovePiece(mv, pcCaptured);
		return false;
	}
	ChangeSide();
	nDistance ++;
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
	for(sqSrc = 0; sqSrc < 256; sqSrc ++)
	{

		// 1. 找到一个本方棋子，再做以下判断：
		pcSrc = ucpcSquares[sqSrc];
		if((pcSrc & pcSelfSide) == 0)
		{
			continue;
		}

		// 2. 根据棋子确定走法
		switch(pcSrc - pcSelfSide)
		{
		case PIECE_KING: //将
			for(i = 0; i < 4; i ++)
			{
				sqDst = sqSrc + ccKingDelta[i];
				//目标位置不在九宫格内 跳过
				if(!IN_FORT(sqDst))
				{
					continue;
				}
				pcDst = ucpcSquares[sqDst];
				//目标位置为空或对方棋子，即是合法走法
				if((pcDst & pcSelfSide) == 0)
				{
					mvs[nGenMoves] = MOVE(sqSrc, sqDst);
					nGenMoves ++;
				}
			}
			break;
		case PIECE_ADVISOR: //士
			for(i = 0; i < 4; i ++)
			{
				sqDst = sqSrc + ccAdvisorDelta[i];
				if(!IN_FORT(sqDst))
				{
					continue;
				}
				pcDst = ucpcSquares[sqDst];
				if((pcDst & pcSelfSide) == 0)
				{
					mvs[nGenMoves] = MOVE(sqSrc, sqDst);
					nGenMoves ++;
				}
			}
			break;
		case PIECE_BISHOP: //象
			for(i = 0; i < 4; i ++)
			{
				sqDst = sqSrc + ccAdvisorDelta[i];
				if(!(IN_BOARD(sqDst) && HOME_HALF(sqDst, sdPlayer) && ucpcSquares[sqDst] == 0))
				{
					continue;
				}
				sqDst += ccAdvisorDelta[i];
				pcDst = ucpcSquares[sqDst];
				if((pcDst & pcSelfSide) == 0)
				{
					mvs[nGenMoves] = MOVE(sqSrc, sqDst);
					nGenMoves ++;
				}
			}
			break;
		case PIECE_KNIGHT: //马
			for(i = 0; i < 4; i ++)
			{
				sqDst = sqSrc + ccKingDelta[i];
				if(ucpcSquares[sqDst] != 0)
				{
					continue;
				}
				for(j = 0; j < 2; j ++)
				{
					sqDst = sqSrc + ccKnightDelta[i][j];
					if(!IN_BOARD(sqDst))
					{
						continue;
					}
					pcDst = ucpcSquares[sqDst];
					if((pcDst & pcSelfSide) == 0)
					{
						mvs[nGenMoves] = MOVE(sqSrc, sqDst);
						nGenMoves ++;
					}
				}
			}
			break;
		case PIECE_ROOK: //车
			for(i = 0; i < 4; i ++)
			{
				nDelta = ccKingDelta[i];
				sqDst = sqSrc + nDelta;
				while(IN_BOARD(sqDst))
				{
					pcDst = ucpcSquares[sqDst];
					if(pcDst == 0)
					{
						mvs[nGenMoves] = MOVE(sqSrc, sqDst);
						nGenMoves ++;
					}
					else
					{
						if((pcDst & pcOppSide) != 0)
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
		case PIECE_CANNON: //炮
			for(i = 0; i < 4; i ++)
			{
				nDelta = ccKingDelta[i];
				sqDst = sqSrc + nDelta;
				while(IN_BOARD(sqDst))
				{
					pcDst = ucpcSquares[sqDst];
					if(pcDst == 0)
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
				while(IN_BOARD(sqDst))
				{
					pcDst = ucpcSquares[sqDst];
					if(pcDst != 0)
					{
						if((pcDst & pcOppSide) != 0)
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
		case PIECE_PAWN: //卒
			sqDst = SQUARE_FORWARD(sqSrc, sdPlayer);
			if(IN_BOARD(sqDst))
			{
				pcDst = ucpcSquares[sqDst];
				if((pcDst & pcSelfSide) == 0)
				{
					mvs[nGenMoves] = MOVE(sqSrc, sqDst);
					nGenMoves ++;
				}
			}
			if(AWAY_HALF(sqSrc, sdPlayer))
			{
				for(nDelta = -1; nDelta <= 1; nDelta += 2)
				{
					sqDst = sqSrc + nDelta;
					if(IN_BOARD(sqDst))
					{
						pcDst = ucpcSquares[sqDst];
						if((pcDst & pcSelfSide) == 0)
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
	if((pcSrc & pcSelfSide) == 0)
	{
		//起始格不是自己的棋子
		return false;
	}

	// 2. 判断目标格是否有自己的棋子
	sqDst = DST(mv);
	pcDst = ucpcSquares[sqDst];
	if((pcDst & pcSelfSide) != 0)
	{
		//目标格不是空白或对方棋子
		return false;
	}

	// 3. 根据棋子的类型检查走法是否合理
	//pcSrc - pcSelfSide得到棋子值
	switch(pcSrc - pcSelfSide)
	{
	case PIECE_KING:    //帅(将)
		return IN_FORT(sqDst) && KING_SPAN(sqSrc, sqDst);
	case PIECE_ADVISOR: //仕(士)
		return IN_FORT(sqDst) && ADVISOR_SPAN(sqSrc, sqDst);
	case PIECE_BISHOP:  //相(象)
		return SAME_HALF(sqSrc, sqDst) && BISHOP_SPAN(sqSrc, sqDst) &&
		       ucpcSquares[BISHOP_PIN(sqSrc, sqDst)] == 0;
	case PIECE_KNIGHT:  //马
		sqPin = KNIGHT_PIN(sqSrc, sqDst);
		return sqPin != sqSrc && ucpcSquares[sqPin] == 0;
	case PIECE_ROOK:   //车
	case PIECE_CANNON: //炮
		if(SAME_RANK(sqSrc, sqDst))
		{
			nDelta = (sqDst < sqSrc ? -1 : 1);
		}
		else if(SAME_FILE(sqSrc, sqDst))
		{
			nDelta = (sqDst < sqSrc ? -16 : 16);
		}
		else
		{
			return false;
		}
		sqPin = sqSrc + nDelta;
		//找到起点到终点间的阻隔棋子的位置
		while(sqPin != sqDst && ucpcSquares[sqPin] == 0)
		{
			sqPin += nDelta;
		}
		//起点上的第一个阻隔的子是终点
		if(sqPin == sqDst)
		{
			//终点没有棋子
			//或者起点棋子是车
			return pcDst == 0 || pcSrc - pcSelfSide == PIECE_ROOK;
		}
		//阻隔棋子在中间，终点有棋子且起点棋子是炮
		else if(pcDst != 0 && pcSrc - pcSelfSide == PIECE_CANNON)
		{
			sqPin += nDelta;
			//查找第一个阻隔棋子和终点还有没有其它棋子
			while(sqPin != sqDst && ucpcSquares[sqPin] == 0)
			{
				sqPin += nDelta;
			}
			return sqPin == sqDst;
		}
		else
		{
			return false;
		}
	case PIECE_PAWN: //兵
		if(AWAY_HALF(sqDst, sdPlayer) && (sqDst == sqSrc - 1 || sqDst == sqSrc + 1))
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

	for(sqSrc = 0; sqSrc < 256; sqSrc ++)
	{
		if(ucpcSquares[sqSrc] != pcSelfSide + PIECE_KING)
		{
			continue;
		}

		// 1. 判断是否被对方的兵(卒)将军
		//将（帅）的前一列是兵（卒）
		if(ucpcSquares[SQUARE_FORWARD(sqSrc, sdPlayer)] == pcOppSide + PIECE_PAWN)
		{
			return true;
		}
		//将（帅）的左右是兵（卒）
		for(nDelta = -1; nDelta <= 1; nDelta += 2)
		{
			if(ucpcSquares[sqSrc + nDelta] == pcOppSide + PIECE_PAWN)
			{
				return true;
			}
		}

		// 2. 判断是否被对方的马将军(以仕(士)的步长当作马腿)
		for(i = 0; i < 4; i ++)
		{
			//马腿位置有棋子则跳过
			if(ucpcSquares[sqSrc + ccAdvisorDelta[i]] != 0)
			{
				continue;
			}
			//一个马腿的位置对应有两只马的位置
			for(j = 0; j < 2; j ++)
			{
				//如果将（帅）被马将军，对应马的位置
				pcDst = ucpcSquares[sqSrc + ccKnightCheckDelta[i][j]];
				if(pcDst == pcOppSide + PIECE_KNIGHT)
				{
					return true;
				}
			}
		}

		// 3. 判断是否被对方的车或炮将军(包括将帅对脸)
		for(i = 0; i < 4; i ++)
		{
			//在棋盘内，往将（帅）的上下左右四条直线位置查找棋子
			nDelta = ccKingDelta[i];
			sqDst = sqSrc + nDelta;
			while(IN_BOARD(sqDst))
			{
				pcDst = ucpcSquares[sqDst];
				if(pcDst != 0)
				{
					//查到的第一个子是对方的将或车时
					if(pcDst == pcOppSide + PIECE_ROOK || pcDst == pcOppSide + PIECE_KING)
					{
						return true;
					}
					//跳出循环，得到第一个子
					break;
				}
				sqDst += nDelta;
			}
			sqDst += nDelta;
			while(IN_BOARD(sqDst))
			{
				int pcDst = ucpcSquares[sqDst];
				if(pcDst != 0)
				{
					//查到的第二个子是对方的炮
					if(pcDst == pcOppSide + PIECE_CANNON)
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

// 判断sdPlayer方是否被杀
bool PositionStruct::IsMate(void)
{
	int i, nGenMoveNum, pcCaptured;
	int mvs[MAX_GEN_MOVES];
	//生成sdPlayer方所有棋子的走法
	nGenMoveNum = GenerateMoves(mvs);
	for(i = 0; i < nGenMoveNum; i ++)
	{
		pcCaptured = MovePiece(mvs[i]);
		//假设sdPlayer方走了一步棋后，没有出现将军，即没被杀死
		if(!Checked())
		{
			UndoMovePiece(mvs[i], pcCaptured);
			return false;
		}
		else
		{
			UndoMovePiece(mvs[i], pcCaptured);
		}
	}
	//如果所有走法走完，没有返回false，即已被杀死
	return true;
}

}
