#ifndef SEARCH_H
#define SEARCH_H

#include "positions_struct.h"

namespace Chess
{

// "qsort"按历史表排序的比较的类
class Compare
{
private:
	int *history;
public:
	Compare(int *h): history(h) {}
	bool operator()(int mv1, int mv2)
	{
		//按照历史表的顺序，由大以小排列
		return history[mv1] > history[mv2];
	}
};

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

	// "qsort"按MVV/LVA值排序的比较函数
//	static int CompareMvvLva(const void *lpmv1, const void *lpmv2)
//	{
//		return MvvLva(*(int *) lpmv2) - MvvLva(*(int *) lpmv1);
//	}
	// 比较函数，用于按 MVV/LVA 值降序排序
	bool CompareMvvLva(int mv1, int mv2)
	{
		return MvvLva(mv2) < MvvLva(mv1);
	}
};

}

#endif // SEARCH_H
