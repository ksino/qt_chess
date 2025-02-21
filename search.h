#ifndef SEARCH_H
#define SEARCH_H

#include "positions.h"

namespace Chess
{

struct HashItem
{
	quint8 ucDepth;      // 深度
	quint8 ucFlag;       // 标志位
	qint16 svl;          // 分数
	quint16 wmv;         // 最佳走法
	quint16 wReserved;   // 保留字段
	quint32 dwLock0;     // Zobrist 锁 0
	quint32 dwLock1;     // Zobrist 锁 1
};

// 与搜索有关的变量
struct SearchData
{
	int mvResult;                  // 电脑走的棋
	int nHistoryTable[65536];      // 历史表
	int mvKillers[LIMIT_DEPTH][2]; // 杀手走法表
	HashItem HashTable[HASH_SIZE]; // 置换表
};


class SortStruct
{
public:
	SortStruct(PositionStruct *pos, SearchData *sd)
		: pos(pos), sd(sd)
	{
	}

	// 得到下一个走法
	void Init(int mvHash_)   // 初始化，设定置换表走法和两个杀手走法
	{
		mvHash = mvHash_;
		mvKiller1 = sd->mvKillers[pos->nDistance][0];
		mvKiller2 = sd->mvKillers[pos->nDistance][1];
		nPhase = PHASE_HASH;
	}
	int Next();

private:
	PositionStruct *pos; // 指向 Position 实例的指针
	SearchData *sd;
	int mvHash;    // 置换表走法
	int mvKiller1, mvKiller2; // 两个杀手走法
	int nPhase;    // 当前阶段
	int nIndex;    // 当前采用第几个走法
	int nGenMoves; // 总共有几个走法
	int mvs[MAX_GEN_MOVES]; // 所有的走法

	// 阶段枚举
	enum
	{
		PHASE_HASH,      // 置换表着法启发
		PHASE_KILLER_1,  // 第一个杀手着法启发
		PHASE_KILLER_2,  // 第二个杀手着法启发
		PHASE_GEN_MOVES, // 生成所有着法
		PHASE_REST       // 剩余着法启发
	};
};


class Search
{
public:
	Search(PositionStruct &pos);
	PositionStruct *pos;
	SearchData *sd {nullptr};
	int SearchQuiesc(int vlAlpha, int vlBeta);
	int SearchFull(int vlAlpha, int vlBeta, int nDepth, bool bNoNull = false);
	void SearchMain(void);
	// 求MVV/LVA值
	inline int MvvLva(int mv)
	{
		return (cucMvvLva[pos->ucpcSquares[DST(mv)]] << 3) - cucMvvLva[pos->ucpcSquares[SRC(mv)]];
	}
	int mvKillers[LIMIT_DEPTH][2]; // 杀手走法表
	HashItem HashTable[HASH_SIZE]; // 置换表
	int ProbeHash(int vlAlpha, int vlBeta, int nDepth, int &mv); // 提取置换表项
	void RecordHash(int nFlag, int vl, int nDepth, int mv); // 保存置换表项

	// 对最佳走法的处理
	inline void SetBestMove(int mv, int nDepth)
	{
		int *lpmvKillers;
		sd->nHistoryTable[mv] += nDepth * nDepth;
		lpmvKillers = sd->mvKillers[pos->nDistance];
		if(lpmvKillers[0] != mv)
		{
			lpmvKillers[1] = lpmvKillers[0];
			lpmvKillers[0] = mv;
		}
	}
};

}

#endif // SEARCH_H
