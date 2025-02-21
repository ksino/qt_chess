#ifndef SEARCH_H
#define SEARCH_H

#include "positions.h"

namespace Chess
{
// 与搜索有关的全局变量
class Search
{
public:
	Search(PositionStruct &pos);
	int mvResult;             // 电脑走的棋
	int nHistoryTable[65536]; // 历史表
	PositionStruct *pos;
	int SearchQuiesc(int vlAlpha, int vlBeta);
	int SearchFull(int vlAlpha, int vlBeta, int nDepth, bool bNoNull = false);
	void SearchMain(void);
	void print(int *mv, int len);
	// 求MVV/LVA值
	inline int MvvLva(int mv)
	{
		return (cucMvvLva[pos->ucpcSquares[DST(mv)]] << 3) - cucMvvLva[pos->ucpcSquares[SRC(mv)]];
	}
};

}

#endif // SEARCH_H
