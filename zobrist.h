#ifndef ZOBRISTSTRUCT_H
#define ZOBRISTSTRUCT_H

#include <QtGlobal>

// RC4Struct类：实现RC4流密码算法，用于生成伪随机字节流
class RC4Struct
{
public:
	quint8 s[256];  // RC4的状态数组，包含256个字节
	int x, y;       // 两个状态变量，用于生成伪随机字节

	void InitZero();       // 用空密钥初始化RC4状态数组
	quint8 NextByte();     // 生成下一个伪随机字节
	quint32 NextLong();    // 生成下一个32位伪随机数（4个字节）
};

// ZobristStruct类：用于存储Zobrist哈希值，支持初始化、异或操作等
class ZobristStruct
{
public:
	quint32 dwKey;   // 32位的哈希值，用于表示键
	quint32 dwLock0; // 32位的哈希值，用于表示锁0
	quint32 dwLock1; // 32位的哈希值，用于表示锁1

	void InitZero();       // 用零填充Zobrist结构体
	void InitRC4(RC4Struct &rc4);  // 用RC4生成的伪随机数初始化Zobrist结构体
	void Xor(const ZobristStruct &zobr);  // 对当前Zobrist结构体与另一个Zobrist结构体执行异或操作
	void Xor(const ZobristStruct &zobr1, const ZobristStruct &zobr2);  // 对当前Zobrist结构体与两个Zobrist结构体执行异或操作
};

// Zobrist类：用于管理Zobrist表，通常用于棋类游戏中的哈希计算
class Zobrist
{
public:
	ZobristStruct Player;         // 用于表示玩家的Zobrist哈希值
	ZobristStruct Table[14][256]; // Zobrist表，14行，每行256个ZobristStruct

	void InitZobrist();  // 初始化Zobrist表
};

#endif // ZOBRISTSTRUCT_H
