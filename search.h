#ifndef SEARCH_H
#define SEARCH_H

#include "positions_truct.h"

namespace Chess
{

// "qsort"按历史表排序的比较的类
class Compare
{
private:
	int *history;
public:
	Compare(int *h): history(h) {}
	bool operator () (int mv1, int mv2)
	{
		return history[mv2] - history[mv1];
	}
};

// 与搜索有关的全局变量
class Search
{
public:
	Search(PositionStruct &pos);
	int mvResult;             // 电脑走的棋
	int nHistoryTable[65536]; // 历史表
	PositionStruct pos;

	int SearchFull(int vlAlpha, int vlBeta, int nDepth);
	void SearchMain(void);
	void print(int *mv, int len);
};

}

#endif // SEARCH_H
