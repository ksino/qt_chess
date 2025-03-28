#include "zobrist.h"
#include <QSet>


// 用空密钥初始化RC4状态数组
void RC4Struct::InitZero()
{
	int i, j;
	quint8 uc;

	x = y = j = 0;
	// 初始化状态数组
	for(i = 0; i < 256; i++)
	{
		s[i] = i;
	}
	// 打乱状态数组，模拟RC4的密钥调度算法
	for(i = 0; i < 256; i++)
	{
		j = (j + s[i]) & 255;  // j = (j + s[i]) mod 256
		uc = s[i];
		s[i] = s[j];  // 交换s[i]和s[j]
		s[j] = uc;
	}
}

// 生成下一个伪随机字节
quint8 RC4Struct::NextByte()
{
	quint8 uc;
	// x每次增加1 由0到255循环
	x = (x + 1) & 255;
	// y = (y + s[x]) mod 256
	y = (y + s[x]) & 255;
	// 交换s[x]和s[y]
	uc = s[x];
	s[x] = s[y];
	s[y] = uc;
	return s[(s[x] + s[y]) & 255];
}

// 生成下一个32位伪随机数（4个字节）
quint32 RC4Struct::NextLong()
{
	quint8 uc0, uc1, uc2, uc3;
	uc0 = NextByte();
	uc1 = NextByte();
	uc2 = NextByte();
	uc3 = NextByte();
	// 将四个字节组合成一个32位的整数
	return uc0 + (uc1 << 8) + (uc2 << 16) + (uc3 << 24);
}

// ZobristStruct 的实现
// 用零填充Zobrist结构体
void ZobristStruct::InitZero()
{
	dwKey = dwLock0 = dwLock1 = 0;
}

// 用RC4生成的伪随机数初始化Zobrist结构体
void ZobristStruct::InitRC4(RC4Struct &rc4)
{
	dwKey = rc4.NextLong();
	dwLock0 = rc4.NextLong();
	dwLock1 = rc4.NextLong();
}

// 对当前Zobrist结构体与另一个Zobrist结构体执行异或操作
void ZobristStruct::Xor(const ZobristStruct &zobr)
{
	dwKey ^= zobr.dwKey;
	dwLock0 ^= zobr.dwLock0;
	dwLock1 ^= zobr.dwLock1;
}

// 对当前Zobrist结构体与两个Zobrist结构体执行异或操作
void ZobristStruct::Xor(const ZobristStruct &zobr1, const ZobristStruct &zobr2)
{
	dwKey ^= zobr1.dwKey ^ zobr2.dwKey;
	dwLock0 ^= zobr1.dwLock0 ^ zobr2.dwLock0;
	dwLock1 ^= zobr1.dwLock1 ^ zobr2.dwLock1;
}

// Zobrist 的实现

// 初始化Zobrist表
void Zobrist::InitZobrist()
{
	int i, j;
	RC4Struct rc4;
	QSet<quint32> keySet;  // 存储dwKey以检查重复

	rc4.InitZero();  // 初始化RC4状态
	Player.InitRC4(rc4);  // 使用RC4生成的随机数初始化Player
	for(i = 0; i < 14; i++)
	{
		for(j = 0; j < 256; j++)
		{
			Table[i][j].InitRC4(rc4);  // 使用RC4生成的随机数初始化Table中的每个元素
		}
	}
}
