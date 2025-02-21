#include "search.h"
#include <time.h>
#include <algorithm>

namespace Chess
{

Search::Search(PositionStruct &pos_) : pos(&pos_)
{
	sd = new SearchData();
//	ss = new SortStruct(pos, sd);
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
	int nHashFlag, vl, vlBest;
	int mv, mvBest;
	int mvHash = 0;
	SortStruct ss = SortStruct(pos, sd);
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

		// 1-3. 尝试置换表裁剪，并得到置换表走法
		vl = ProbeHash(vlAlpha, vlBeta, nDepth, mvHash);
		if(vl > -MATE_VALUE)
		{
			return vl;
		}

		// 1-4. 尝试空步裁剪(根节点的Beta值是"MATE_VALUE"，所以不可能发生空步裁剪)
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
		else
		{
			mvHash = 0;
		}
	}

	// 2. 初始化最佳值和最佳走法
	nHashFlag = HASH_ALPHA;
	vlBest = -MATE_VALUE; // 这样可以知道，是否一个走法都没走过(杀棋)
	mvBest = 0;           // 这样可以知道，是否搜索到了Beta走法或PV走法，以便保存到历史表

	// 3. 初始化走法排序结构
	ss.Init(mvHash);

	// 4. 逐一走这些走法，并进行递归
	while((mv = ss.Next()) != 0)
	{
		//L << "mv" << mv;
		if(pos->MakeMove(mv))
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
					nHashFlag = HASH_BETA;
					mvBest = mv;  // Beta走法要保存到历史表
					break;            // Beta截断
				}
				if(vl > vlAlpha)    // 找到一个PV走法
				{
					nHashFlag = HASH_PV;
					mvBest = mv;  // PV走法要保存到历史表
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
	// 记录到置换表
	RecordHash(nHashFlag, vlBest, nDepth, mvBest);
	if(mvBest != 0)
	{
		// 如果不是Alpha走法，就将最佳走法保存到历史表
		SetBestMove(mvBest, nDepth);
		if(pos->nDistance == 0)
		{
			// 搜索根节点时，总是有一个最佳走法(因为全窗口搜索不会超出边界)，将这个走法保存下来
			sd->mvResult = mvBest;
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
	memset(sd->nHistoryTable, 0, 65536 * sizeof(int)); // 清空历史表
	memset(sd->mvKillers, 0, LIMIT_DEPTH * 2 * sizeof(int)); // 清空杀手走法表
	memset(sd->HashTable, 0, HASH_SIZE * sizeof(HashItem));  // 清空置换表
	t = clock();       // 初始化定时器
	pos->nDistance = 0; // 初始步数

	// 迭代加深过程
	for(i = 1; i <= LIMIT_DEPTH; i ++)
	{
		vl = SearchFull(-MATE_VALUE, MATE_VALUE, i);
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

// 提取置换表项
int Search::ProbeHash(int vlAlpha, int vlBeta, int nDepth, int &mv)
{
	bool bMate; // 杀棋标志：如果是杀棋，那么不需要满足深度条件
	HashItem hsh;

	hsh = HashTable[pos->zobr.dwKey & (HASH_SIZE - 1)];
	if(hsh.dwLock0 != pos->zobr.dwLock0 || hsh.dwLock1 != pos->zobr.dwLock1)
	{
		mv = 0;
		return -MATE_VALUE;
	}
	mv = hsh.wmv;
	bMate = false;
	if(hsh.svl > WIN_VALUE)
	{
		hsh.svl -= pos->nDistance;
		bMate = true;
	}
	else if(hsh.svl < -WIN_VALUE)
	{
		hsh.svl += pos->nDistance;
		bMate = true;
	}
	if(hsh.ucDepth >= nDepth || bMate)
	{
		if(hsh.ucFlag == HASH_BETA)
		{
			return (hsh.svl >= vlBeta ? hsh.svl : -MATE_VALUE);
		}
		else if(hsh.ucFlag == HASH_ALPHA)
		{
			return (hsh.svl <= vlAlpha ? hsh.svl : -MATE_VALUE);
		}
		return hsh.svl;
	}
	return -MATE_VALUE;
}

// 保存置换表项
void Search::RecordHash(int nFlag, int vl, int nDepth, int mv)
{
	HashItem hsh;
	hsh = HashTable[pos->zobr.dwKey & (HASH_SIZE - 1)];
	if(hsh.ucDepth > nDepth)
	{
		return;
	}
	hsh.ucFlag = nFlag;
	hsh.ucDepth = nDepth;
	if(vl > WIN_VALUE)
	{
		hsh.svl = vl + pos->nDistance;
	}
	else if(vl < -WIN_VALUE)
	{
		hsh.svl = vl - pos->nDistance;
	}
	else
	{
		hsh.svl = vl;
	}
	hsh.wmv = mv;
	hsh.dwLock0 = pos->zobr.dwLock0;
	hsh.dwLock1 = pos->zobr.dwLock1;
	HashTable[pos->zobr.dwKey & (HASH_SIZE - 1)] = hsh;
}

int SortStruct::Next()
{
	int mv;
	switch(nPhase)
	{
	// "nPhase"表示着法启发的若干阶段，依次为：
	// 0. 置换表着法启发，完成后立即进入下一阶段；
	case PHASE_HASH:
		nPhase = PHASE_KILLER_1;
		if(mvHash != 0)
		{
			//L << "0 PHASE_HASH" << mvHash;
			return mvHash;
		}
	// 技巧：这里没有"break"，表示"switch"的上一个"case"执行完后紧接着做下一个"case"，下同

	// 1. 杀手着法启发(第一个杀手着法)，完成后立即进入下一阶段；
	case PHASE_KILLER_1:
		nPhase = PHASE_KILLER_2;
		if(mvKiller1 != mvHash && mvKiller1 != 0 && pos->LegalMove(mvKiller1))
		{
			return mvKiller1;
		}

	// 2. 杀手着法启发(第二个杀手着法)，完成后立即进入下一阶段；
	case PHASE_KILLER_2:
		nPhase = PHASE_GEN_MOVES;
		if(mvKiller2 != mvHash && mvKiller2 != 0 && pos->LegalMove(mvKiller2))
		{
			return mvKiller2;
		}

	// 3. 生成所有着法，完成后立即进入下一阶段；
	case PHASE_GEN_MOVES:
		nPhase = PHASE_REST;
		nGenMoves = pos->GenerateMoves(mvs);
		sort(mvs, mvs + nGenMoves, [this](int mv1, int mv2)
		{
			return this->sd->nHistoryTable[mv1] > this->sd->nHistoryTable[mv2];
		});
		nIndex = 0;

	// 4. 对剩余着法进行历史表启发；
	case PHASE_REST:
		while(nIndex < nGenMoves)
		{
			mv = mvs[nIndex];
			nIndex++;
			if(mv != mvHash && mv != mvKiller1 && mv != mvKiller2)
			{
				return mv;
			}
		}
		break;
	}
	return 0;
}

}
