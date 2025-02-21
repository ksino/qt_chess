#include "search.h"
#include <time.h>
#include <algorithm>

namespace Chess
{

Search::Search(PositionStruct &pos) : pos(&pos)
{

}

int Search::SearchQuiesc(int vlAlpha, int vlBeta)
{
	int i, nGenMoves;
	int vl, vlBest;
	int mvs[MAX_GEN_MOVES];
	// 一个静态搜索分为以下几个阶段

	// 1. 检查重复局面
	vl = pos->RepStatus();
	if(vl != 0)
	{
		return pos->RepValue(vl);
	}

	// 2. 到达极限深度就返回局面评价
	if(pos->nDistance == LIMIT_DEPTH)
	{
		return pos->Evaluate();
	}

	// 3. 初始化最佳值
	vlBest = -MATE_VALUE; // 这样可以知道，是否一个走法都没走过(杀棋)

	if(pos->InCheck())
	{
		// 4. 如果被将军，则生成全部走法
		nGenMoves = pos->GenerateMoves(mvs);
		sort(mvs, mvs + nGenMoves, [this](int mv1, int mv2)
		{
			return this->MvvLva(mv1) > this->MvvLva(mv2); // 降序排序
		});
	}
	else
	{

		// 5. 如果不被将军，先做局面评价
		vl = pos->Evaluate();
		if(vl > vlBest)
		{
			vlBest = vl;
			if(vl >= vlBeta)
			{
				return vl;
			}
			if(vl > vlAlpha)
			{
				vlAlpha = vl;
			}
		}

		// 6. 如果局面评价没有截断，再生成吃子走法
		nGenMoves = pos->GenerateMoves(mvs, GEN_CAPTURE);
		sort(mvs, mvs + nGenMoves, [this](int mv1, int mv2)
		{
			return this->MvvLva(mv1) > this->MvvLva(mv2); // 降序排序
		});
	}

	// 7. 逐一走这些走法，并进行递归
	for(i = 0; i < nGenMoves; i ++)
	{
		if(pos->MakeMove(mvs[i]))
		{
			vl = -SearchQuiesc(-vlBeta, -vlAlpha);
			pos->UndoMakeMove();

			// 8. 进行Alpha-Beta大小判断和截断
			if(vl > vlBest)       // 找到最佳值(但不能确定是Alpha、PV还是Beta走法)
			{
				vlBest = vl;        // "vlBest"就是目前要返回的最佳值，可能超出Alpha-Beta边界
				if(vl >= vlBeta)    // 找到一个Beta走法
				{
					return vl;        // Beta截断
				}
				if(vl > vlAlpha)    // 找到一个PV走法
				{
					vlAlpha = vl;     // 缩小Alpha-Beta边界
				}
			}
		}
	}

	// 9. 所有走法都搜索完了，返回最佳值
	return vlBest == -MATE_VALUE ? pos->nDistance - MATE_VALUE : vlBest;
}

// 超出边界(Fail-Soft)的Alpha-Beta搜索过程
int Search::SearchFull(int vlAlpha, int vlBeta, int nDepth, bool bNoNull)
{
	//L << "    SearchFull" << vlAlpha << vlBeta << nDepth;

	int i, nGenMoves;
	int vl, vlBest, mvBest;
	int mvs[MAX_GEN_MOVES];
	// 一个Alpha-Beta完全搜索分为以下几个阶段

	if(pos->nDistance > 0)
	{
		// 1. 到达水平线，则调用静态搜索(注意：由于空步裁剪，深度可能小于零)
		if(nDepth <= 0)
		{
			return SearchQuiesc(vlAlpha, vlBeta);
		}

		// 1-1. 检查重复局面(注意：不要在根节点检查，否则就没有走法了)
		vl = pos->RepStatus();
		if(vl != 0)
		{
			return pos->RepValue(vl);
		}

		// 1-2. 到达极限深度就返回局面评价
		if(pos->nDistance == LIMIT_DEPTH)
		{
			return pos->Evaluate();
		}

		// 1-3. 尝试空步裁剪(根节点的Beta值是"MATE_VALUE"，所以不可能发生空步裁剪)
		if(!bNoNull && !pos->InCheck() && pos->NullOkay())
		{
			pos->NullMove();
			vl = -SearchFull(-vlBeta, 1 - vlBeta, nDepth - NULL_DEPTH - 1, NO_NULL);
			pos->UndoNullMove();
			if(vl >= vlBeta)
			{
				return vl;
			}
		}
	}

	// 2. 初始化最佳值和最佳走法
	vlBest = -MATE_VALUE; // 这样可以知道，是否一个走法都没走过(杀棋)
	mvBest = 0;           // 这样可以知道，是否搜索到了Beta走法或PV走法，以便保存到历史表

	// 3. 生成全部走法，并根据历史表排序
	nGenMoves = pos->GenerateMoves(mvs);
	sort(mvs, mvs + nGenMoves, [this](int mv1, int mv2)
	{
		return this->nHistoryTable[mv1] > this->nHistoryTable[mv2];
	});
	// 4. 逐一走这些走法，并进行递归
	for(i = 0; i < nGenMoves; i++)
	{
		if(pos->MakeMove(mvs[i]))
		{
			// 将军延伸
			vl = -SearchFull(-vlBeta, -vlAlpha, pos->InCheck() ? nDepth : nDepth - 1);
			pos->UndoMakeMove();

			// 5. 进行Alpha-Beta大小判断和截断
			if(vl > vlBest)       // 找到最佳值(但不能确定是Alpha、PV还是Beta走法)
			{
				vlBest = vl;        // "vlBest"就是目前要返回的最佳值，可能超出Alpha-Beta边界
				if(vl >= vlBeta)    // 找到一个Beta走法
				{
					mvBest = mvs[i];  // Beta走法要保存到历史表
					break;            // Beta截断
				}
				if(vl > vlAlpha)    // 找到一个PV走法
				{
					mvBest = mvs[i];  // PV走法要保存到历史表
					vlAlpha = vl;     // 缩小Alpha-Beta边界
				}
			}
		}
	}

	// 5. 所有走法都搜索完了，把最佳走法(不能是Alpha走法)保存到历史表，返回最佳值
	if(vlBest == -MATE_VALUE)
	{
		// 如果是杀棋，就根据杀棋步数给出评价
		return pos->nDistance - MATE_VALUE;
	}
	if(mvBest != 0)
	{
		// 如果不是Alpha走法，就将最佳走法保存到历史表
		nHistoryTable[mvBest] += nDepth * nDepth;
		if(pos->nDistance == 0)
		{
			// 搜索根节点时，总是有一个最佳走法(因为全窗口搜索不会超出边界)，将这个走法保存下来
			mvResult = mvBest;
			//L << "history" << mvBest;
		}
	}
	return vlBest;
}

// 迭代加深搜索过程
void Search::SearchMain(void)
{
	int i, t, vl;

	// 初始化
	memset(nHistoryTable, 0, 65536 * sizeof(int)); // 清空历史表
	t = clock();       // 初始化定时器
	pos->nDistance = 0; // 初始步数

	// 迭代加深过程
	for(i = 1; i <= LIMIT_DEPTH; i ++)
	{
		vl = SearchFull(-MATE_VALUE, MATE_VALUE, i);
		L << "vl" << vl;
		// 搜索到杀棋，就终止搜索
		if(vl > WIN_VALUE || vl < -WIN_VALUE)
		{
			break;
		}
		// 超过一秒，就终止搜索
		if(clock() - t > CLOCKS_PER_SEC)
		{
			L << "Time out";
			break;
		}
	}
}

void Search::print(int *mv, int len)
{
	auto qdebug = QDebug(QtDebugMsg);   // 这种直接一点，构造一个QDebug()对象
	qdebug << len << " [";
	for(int var = 0; var < len; ++var)
	{
		qdebug.nospace() << mv[var] << ",";
	}
	qdebug << "]";
}


}
