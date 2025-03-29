#include "search.h"
#include <time.h>
#include <algorithm>
#include <QFile>
#include <QDataStream>

namespace Chess
{

Search::Search(PositionStruct &pos_) : pos(&pos_)
{
	context = new SearchContext();
}

// 迭代加深搜索过程
void Search::SearchMain(void)
{
	int i, t, vl, nGenMoves;
	int mvs[MAX_GEN_MOVES];

	// 初始化
	memset(context->nHistoryTable, 0, 65536 * sizeof(int));       // 清空历史表
	memset(context->mvKillers, 0, LIMIT_DEPTH * 2 * sizeof(int)); // 清空杀手走法表
	memset(context->HashTable, 0, HASH_SIZE * sizeof(HashItem));  // 清空置换表
	t = clock();       // 初始化定时器
	pos->nDistance = 0; // 初始步数

	// 搜索开局库 寻找电脑的走法
	context->mvResult = SearchBook();
	if(context->mvResult != 0)
	{
		// 执行走法
		pos->MakeMove(context->mvResult);
		if(pos->RepStatus(3) == 0)
		{
			// 不是重复局面 撤消这步走法并返回
			// 因为真正的走法都是在Board类中执行
			// 这里的MakeMove都只是假设走了这一步，可以继续分析局面
			// 而最后还是UndoMakeMove，撤消这步走法的
			pos->UndoMakeMove();
			return;
		}
		pos->UndoMakeMove();
	}

	// 检查是否只有唯一走法
	vl = 0;
	// 生成所有可能的走法（只是符合棋子的走法，但走这步棋可能会导致将军） 储存到mvs
	nGenMoves = pos->GenerateMoves(mvs);
	for(i = 0; i < nGenMoves; i ++)
	{
		if(pos->MakeMove(mvs[i]))
		{
			// 记录可以执行走法的数量
			pos->UndoMakeMove();
			context->mvResult = mvs[i];
			vl ++;
		}
	}
	if(vl == 1)
	{
		// 只有一个走法则返回
		return;
	}

	// 迭代加深过程 从深度1到最大深度LIMIT_DEPTH
	for(i = 1; i <= LIMIT_DEPTH; i ++)
	{
		vl = SearchRoot(i);
		// 搜索到杀棋，就终止搜索
		if(vl > WIN_VALUE || vl < -WIN_VALUE)
		{
			break;
		}
		// 超过一秒，就终止搜索
		if(clock() - t > CLOCKS_PER_SEC)
		{
			L << "Time out" << clock() - t;
			break;
		}
	}
}

void Search::LoadBook()
{
	QFile bookFile(":/BOOK.DAT");  // 从资源文件中加载开局库数据
	if(!bookFile.open(QIODevice::ReadOnly))
	{
		qWarning() << "Failed to open book resource file!";
		return;
	}

	// 读取资源文件的大小
	qint64 fileSize = bookFile.size();
	context->nBookSize = fileSize / sizeof(BookItem);  // 计算开局库项的数量

	// 如果开局库大小超过最大限制，则截断
	if(context->nBookSize > BOOK_SIZE)
	{
		context->nBookSize = BOOK_SIZE;
	}

	// 读取数据到开局库表
	QDataStream in(&bookFile);
	in.readRawData(reinterpret_cast<char*>(context->BookTable), context->nBookSize * sizeof(BookItem));
	L << "LoadBook" << context->BookTable[10].dwLock;

	bookFile.close();  // 关闭文件
}

int Search::SearchBook()
{
	int i, vl, nBookMoves, mv;
	int mvs[MAX_GEN_MOVES], vls[MAX_GEN_MOVES];
	bool bMirror;
	BookItem bkToSearch, *lpbk;
	PositionStruct posMirror;
	// 搜索开局库的过程有以下几个步骤

	// 1. 如果没有开局库，则立即返回
	if(context->nBookSize == 0)
	{
		return 0;
	}
	// 2. 搜索当前局面
	bMirror = false;
	bkToSearch.dwLock = pos->zobr.dwLock1;
	// 二分查找 已排序开局库
	lpbk = (BookItem *) bsearch(&bkToSearch, context->BookTable, context->nBookSize, sizeof(BookItem), CompareBook);
	// 3. 如果没有找到，那么搜索当前局面的镜像局面
	if(lpbk == nullptr)
	{
		bMirror = true;
		pos->Mirror(posMirror);
		bkToSearch.dwLock = posMirror.zobr.dwLock1;
		lpbk = (BookItem *) bsearch(&bkToSearch, context->BookTable, context->nBookSize, sizeof(BookItem), CompareBook);
	}
	// 4. 如果镜像局面也没找到，则立即返回
	if(lpbk == nullptr)
	{
		return 0;
	}
	// 5. 如果找到，则向前查第一个开局库项
	while(lpbk >= context->BookTable && lpbk->dwLock == bkToSearch.dwLock)
	{
		lpbk --;
	}
	lpbk ++;
	// 6. 把走法和分值写入到"mvs"和"vls"数组中
	vl = nBookMoves = 0;
	while(lpbk < context->BookTable + context->nBookSize && lpbk->dwLock == bkToSearch.dwLock)
	{
		mv = (bMirror ? MIRROR_MOVE(lpbk->wmv) : lpbk->wmv);
		if(pos->LegalMove(mv))
		{
			mvs[nBookMoves] = mv;
			vls[nBookMoves] = lpbk->wvl;
			vl += vls[nBookMoves];
			nBookMoves ++;
			if(nBookMoves == MAX_GEN_MOVES)
			{
				break; // 防止"BOOK.DAT"中含有异常数据
			}
		}
		lpbk ++;
	}
	if(vl == 0)
	{
		return 0; // 防止"BOOK.DAT"中含有异常数据
	}
	// 7. 根据权重随机选择一个走法
	vl = rand() % vl;
	for(i = 0; i < nBookMoves; i ++)
	{
		vl -= vls[i];
		if(vl < 0)
		{
			break;
		}
	}
	return mvs[i];
}

int Search::SearchRoot(int nDepth)
{
	int vl, vlBest, mv, nNewDepth;
	SortStruct sort_ = SortStruct(pos, context);

	// 最佳分数为最小值
	vlBest = -MATE_VALUE;
	sort_.Init(context->mvResult);
	while((mv = sort_.Next()) != 0)
	{
		if(pos->MakeMove(mv))
		{
			nNewDepth = pos->InCheck() ? nDepth : nDepth - 1;
			/* PVS算法的核心思想
			在 Alpha-Beta 剪枝的基础上，PVS 假设最佳走法通常不会改变，因此：
			对第一个走法（主要变化走法，Principal Variation） 进行完整搜索。
			对后续走法（假设它们不会比最优走法更好） 只进行 零窗口搜索（Null Window Search）。
			只有当零窗口搜索的结果 超过当前最优分数 时，才进行完整搜索。
			窗口指的是alpha和beta的区间，全窗即无穷小到无穷大；零窗vlBest-1到vlBest；区间缩小能够快速剪枝
			*/
			if(vlBest == -MATE_VALUE)
			{
				// 第一个走法使用全窗口，即最小值到最大值
				vl = -SearchFull(-MATE_VALUE, MATE_VALUE, nNewDepth, NO_NULL);
			}
			else
			{
				// 否则使用零窗口，即-vlBest - 1到-vlBest
				vl = -SearchFull(-vlBest - 1, -vlBest, nNewDepth);
				if(vl > vlBest)
				{
					// 得到的分值优于上一步的分值，重新使用全窗口搜索
					vl = -SearchFull(-MATE_VALUE, -vlBest, nNewDepth, NO_NULL);
				}
			}
			pos->UndoMakeMove();
			if(vl > vlBest)
			{
				vlBest = vl;
				context->mvResult = mv;
				// 不是必胜或必败的走法
				if(vlBest > -WIN_VALUE && vlBest < WIN_VALUE)
				{
					// 让 vlBest 在 -RANDOM_MASK 到 RANDOM_MASK 之间进行小幅度的随机扰动
					vlBest += (rand() & RANDOM_MASK) - (rand() & RANDOM_MASK);
				}
			}
		}
	}
	L << vlBest << nDepth << context->mvResult;
	RecordHash(HASH_PV, vlBest, nDepth, context->mvResult);
	SetBestMove(context->mvResult, nDepth);
	return vlBest;
}

// 超出边界(Fail-Soft)的Alpha-Beta搜索过程
int Search::SearchFull(int vlAlpha, int vlBeta, int nDepth, bool bNoNull)
{
	int nHashFlag, vl, vlBest;
	int mv, mvBest;
	int mvHash = 0;
	int nNewDepth;
	SortStruct sort_ = SortStruct(pos, context);
	// 一个Alpha-Beta完全搜索分为以下几个阶段

	// 1. 到达水平线，则调用静态搜索(注意：由于空步裁剪，深度可能小于零)
	if(nDepth <= 0)
	{
		L << "go to SearchQuiesc" << pos->nDistance;
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

	// 2. 初始化最佳值和最佳走法
	nHashFlag = HASH_ALPHA;
	vlBest = -MATE_VALUE; // 这样可以知道，是否一个走法都没走过(杀棋)
	mvBest = 0;           // 这样可以知道，是否搜索到了Beta走法或PV走法，以便保存到历史表

	// 3. 初始化走法排序结构
	sort_.Init(mvHash);

	// 4. 逐一走这些走法，并进行递归
	while((mv = sort_.Next()) != 0)
	{
		//L << "mv" << mv;
		if(pos->MakeMove(mv))
		{
			// 将军延伸
			//vl = -SearchFull(-vlBeta, -vlAlpha, pos->InCheck() ? nDepth : nDepth - 1);

			nNewDepth = pos->InCheck() ? nDepth : nDepth - 1;
			// PVS
			if(vlBest == -MATE_VALUE)
			{
				vl = -SearchFull(-vlBeta, -vlAlpha, nNewDepth);
			}
			else
			{
				vl = -SearchFull(-vlAlpha - 1, -vlAlpha, nNewDepth);
				if(vl > vlAlpha && vl < vlBeta)
				{
					vl = -SearchFull(-vlBeta, -vlAlpha, nNewDepth);
				}
			}

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
	}
	return vlBest;
}

/**
 * @brief 静态搜索的核心目标是 高效解决“地平线效应”，即在普通搜索深度耗尽（nDepth <= 0）时，
 * 避免因停止搜索而漏算关键战术动作（如吃子、将军等）,
 * 非战术走法（如移动兵、调整棋子位置）通常不会立即改变局面价值。
 * 静态评估函数（Evaluate）对这些局面的评分已经足够准确，无需进一步搜索。
 * 战术局面的“非静止性”
 * 吃子、将军、升变等走法会直接改变子力平衡或胜负状态：
 * 如果仅依赖静态评估，引擎会严重误判这些局面的真实价值。
 * @param vlAlpha
 * @param vlBeta
 * @return
 */
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

// 提取置换表项
/**
 * @brief Search::ProbeHash
 * @param vlAlpha
 * @param vlBeta
 * @param nDepth
 * @param mv
 * @return
 */
int Search::ProbeHash(int vlAlpha, int vlBeta, int nDepth, int &mv)
{
	bool bMate; // 杀棋标志：如果是杀棋，那么不需要满足深度条件

	// 计算hash条目位置
	HashItem hsh = HashTable[pos->zobr.dwKey & (HASH_SIZE - 1)];
	// 计算hash条目有效性
	if(hsh.dwLock0 != pos->zobr.dwLock0 || hsh.dwLock1 != pos->zobr.dwLock1)
	{
		mv = 0;
		return -MATE_VALUE;
	}
	mv = hsh.wmv;
	bMate = false;
	// 杀棋分数处理
	if(hsh.svl > WIN_VALUE)
	{
		if(hsh.svl < BAN_VALUE)
		{
			return -MATE_VALUE; // 可能导致搜索的不稳定性，立刻退出，但最佳着法可能拿到
		}
		hsh.svl -= pos->nDistance;
		bMate = true;
	}
	else if(hsh.svl < -WIN_VALUE)
	{
		if(hsh.svl > -BAN_VALUE)
		{
			return -MATE_VALUE; // 同上
		}
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

/**
 * @brief 将搜索结果存入置换表(Transposition Table)，实现高效的信息缓存和重用机制。
 * @param nFlag 节点类型标记
 * @param vl 搜索得到分值
 * @param nDepth 搜索深度
 * @param mv 最佳走法
 */
void Search::RecordHash(int nFlag, int vl, int nDepth, int mv)
{
	HashItem hsh;
	// 使用Zobrist哈希值的前20位位作为索引
	// dwKey(32位) & HASH_SIZE - 1(20位）
	hsh = HashTable[pos->zobr.dwKey & (HASH_SIZE - 1)];
	if(hsh.ucDepth > nDepth)
	{
		// 已有更深搜索结果，不覆盖
		return;
	}
	hsh.ucFlag = nFlag;
	hsh.ucDepth = nDepth;
	if(vl > WIN_VALUE)
	{
		// 没有最佳走法(mv==0)且分数接近边界时不存储（避免污染）
		if(mv == 0 && vl <= BAN_VALUE)
		{
			return; // 可能导致搜索的不稳定性，并且没有最佳着法，立刻退出
		}
		// 将杀棋分数转换为与距离相关的具体值
		hsh.svl = vl + pos->nDistance;
	}
	else if(vl < -WIN_VALUE)
	{
		if(mv == 0 && vl >= -BAN_VALUE)
		{
			return; // 同上
		}
		hsh.svl = vl - pos->nDistance;
	}
	else
	{
		hsh.svl = vl;
	}
	// 写入当前数据
	hsh.wmv = mv;
	hsh.dwLock0 = pos->zobr.dwLock0;
	hsh.dwLock1 = pos->zobr.dwLock1;
	// 更新数据
	HashTable[pos->zobr.dwKey & (HASH_SIZE - 1)] = hsh;
}

SortStruct::SortStruct(PositionStruct *pos, SearchContext *context)
	: pos(pos), context(context)
{
}

void SortStruct::Init(int mvHash_)
{
	mvHash = mvHash_;
	mvKiller1 = context->mvKillers[pos->nDistance][0];
	mvKiller2 = context->mvKillers[pos->nDistance][1];
	nPhase = PHASE_HASH;
}

/**
 * @brief 按优先级返回下一个待尝试的走法
 * @return 走法编码，返回0表示没有更多走法
 */
int SortStruct::Next()
{
	int mv;
	switch(nPhase)
	{
	// "nPhase"表示着法启发的若干阶段，依次为：
	// 0. 置换表着法启发，完成后立即进入下一阶段；
	// 尝试置换表中存储的最佳走法
	case PHASE_HASH:
		nPhase = PHASE_KILLER_1;
		if(mvHash != 0)
		{
			return mvHash;
		}
	// 技巧：这里没有"break"，表示"switch"的上一个"case"执行完后紧接着做下一个"case"，下同

	// 1. 杀手着法启发(第一个杀手着法)，完成后立即进入下一阶段；
	// 尝试同一深度下第一个引发剪枝的走法
	// 杀手走法不包含吃子和将军走法
	case PHASE_KILLER_1:
		nPhase = PHASE_KILLER_2;
		if(mvKiller1 != mvHash && mvKiller1 != 0 && pos->LegalMove(mvKiller1))
		{
			return mvKiller1;
		}

	// 2. 杀手着法启发(第二个杀手着法)，完成后立即进入下一阶段；
	// 尝试同一深度下第二个引发剪枝的走法
	case PHASE_KILLER_2:
		nPhase = PHASE_GEN_MOVES;
		if(mvKiller2 != mvHash && mvKiller2 != 0 && pos->LegalMove(mvKiller2))
		{
			return mvKiller2;
		}

	// 3. 生成所有着法，完成后立即进入下一阶段；
	// 生成全部合法走法
	// 按历史表分数降序排序
	// 历史表记录走法的历史表现，分数越高表示该走法越容易引发剪枝
	case PHASE_GEN_MOVES:
		nPhase = PHASE_REST;
		nGenMoves = pos->GenerateMoves(mvs);
		sort(mvs, mvs + nGenMoves, [this](int mv1, int mv2)
		{
			return this->context->nHistoryTable[mv1] > this->context->nHistoryTable[mv2];
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
