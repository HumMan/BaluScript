/* Copyright (c) 2008, ������� �. �.
*
* ����������� ��������� ��������������� � ������������� ��� � ���� ��������� ����, ��� � �
* �������� �����, � ����������� ��� ���, ��� ���������� ��������� �������:
*
*     * ��� ��������� ��������������� ��������� ���� ������ ���������� ��������� ����
*       ����������� �� ��������� �����, ���� ������ ������� � ����������� ����� �� ��������.
*     * ��� ��������� ��������������� ��������� ���� ������ ����������� ��������� ����
*       ���������� �� ��������� �����, ���� ������ ������� � ����������� ����� �� �������� �
*       ������������ �/��� � ������ ����������, ������������ ��� ���������������.
*     * ��� ��������� ��������� ����, ���������� ������ ������ ���� ������������ � ��������
*       � ��������� �������.
*
* ��� ��������� ������������� ����������� ��������� ���� �/��� ������� ���������
* "��� ��� ����" ��� ������-���� ���� ��������, ���������� ���� ��� ���������������,
* �������, �� �� ������������� ���, ��������������� �������� ������������ �������� �
* ����������� ��� ���������� ����. �� � ���� ������, ���� �� ��������� ���������������
* �������, ��� �� ����������� � ������ �����, �� ���� �������� ��������� ���� � �� ����
* ������ ����, ������� ����� �������� �/��� �������� �������������� ���������, ��� ����
* ������� ����, �� ��Ѩ� ���������������, ������� ����� �����, ���������,
* ����������� ��� ������������� ������, ���������� ������������� ��� �������������
* ������������� ��������� (�������, �� �� ������������� ������� ������, ��� �������,
* �������� �������������, ��� �������� ������������ ��-�� ��� ��� ������� ���, ��� �������
* ��������� �������� ��������� � ������� �����������), ���� ���� ����� �������� ���
* ������ ���� ���� �������� � ����������� ����� �������. */

#if 0//#ifdef _MSC_VER

#define WIN32_LEAN_AND_MEAN
#include <windows.h>//��� ������� ������ �� �����������������, �� � ��� ����� �� ���� ������� ���������� _BitScanReverse ��� nix 
#include <crtdbg.h>
#include <stdlib.h>
#include <malloc.h>


#include "mballoc.h"

//������ ������������� �����, ��� ��� ����� ��������� � header-� ������ �� ����� ��� �������������� ����� �����, � ���� �� ��������� � mballoc.h �������� � �������������� ���� ������, ������������ ���������
#define DETECT_MEMORY_LEAKS
#ifdef _DEBUG //�������� ���� ������������� �������� ����� ������ � ������
#define DUMP_ALLOC_CALL_STACK //���� ���� ����� �� �����, ������ ��������������� ��� �������
#ifdef DUMP_ALLOC_CALL_STACK
#define GATHER_BLOCK_GROUPS_STATS //����������, ����� �� ������������ MBGetBlockGroups; �� �������� ��� DUMP_ALLOC_CALL_STACK
#endif
#endif
#define OPERATOR_NEW_OVERLOAD

#define MBALLOC_ALIGNMENT 4//������������ ���������� ������ (��� ������������� SSE ����� ��������� 16)

#define SUBPOWER_OF_TWO_BLOCK_SIZE_ACCURACY 2//�������������� �������� �� 0 �� 2; ����� 0, ����� ���������� �������� ������ � ������� ������, ����� =1, ����������: 2^n � (2^n)*1.5, ����� =2, ����������: 2^n, (2^n)*1.25, (2^n)*1.5 � (2^n)*1.75 � �.�.
static const int MEGABLOCK_SIZE = 128*1024;//��� ����� � ���� - �� ������� ���������� �������, � ������ ��������������� �����, ����� ���� ���� ��������� ������
static const int MAX_BLOCK_SIZE =  16*1024;//��� ���������� ����� ������� ����� ���������� � �������



#define ASSERT(x) _ASSERT(x)//do ; while (false)
#define LENGTH(a) (sizeof(a)/sizeof(a[0]))
#ifdef MB_THREADSAFE
#ifdef _MSC_VER
#define DECL_THREAD __declspec(thread)
#elif defined __GNUC__
#define DECL_THREAD __thread
#else
#error Thread safity not supported on the current platform
#endif
#else
#define DECL_THREAD
#endif

#if MBALLOC_ALIGNMENT <= _CRT_PACKING//=8=MALLOC_ALIGNMENT
#define SysAlloc(sz) malloc(sz)//��� ������� �� ������� ���� �����������������
#define SysFree(p) free(p)
#define SysMemSize(p) _msize(p)
#else
#define SysAlloc(sz) _aligned_malloc(sz, MBALLOC_ALIGNMENT)
#define SysFree(p) _aligned_free(p)
#define SysMemSize(p) _aligned_msize(p, MBALLOC_ALIGNMENT, 0)
#endif


static const int ptrSize = sizeof(void*);//������ ��������� �� ������� ���������


struct MegaBlock;
struct Context;

struct BlockHeader//��� �� ����� 4 �����, ������� ����� ����� ������ ���������� ������
{
	MegaBlock *megaBlock;//���� ��������
#ifdef _DEBUG
	//���. ���� ��� ������� (����. callstack ���������, ���� isArray ��� ��������� new[] ��� ������������� � delete[]), ������� ����� � ������ �����
#ifdef OPERATOR_NEW_OVERLOAD
	int isArray;//�������, � ������� ������ ���� ���� �� �����, �� ��������� ����� ���������� ������ ����� new, � ����� malloc - ������ ������� �����, ��� ������ ����� ���� ���� �� �������
	enum {ARRAY_SIGN = 0xAAC0DEDD, NONARRAY_SIGN = 0xDeadAAAA};//������������ �� ������ ����, � ����������� ���������, �.�. � ������ ��������, ������� ����������, ���������� ��������� � ������ ����� int - ���-�� ��������� ������� � ��������� �� ����� ��������� �� BlockHeader, � ����� ������ �� 4 �����
#endif
#endif
#ifdef DUMP_ALLOC_CALL_STACK
	DWORD callstack[7];//������ ���� ��� 4-� ������� �����
#endif
};


struct FreeBlockHeader : BlockHeader//����� � ���, ��� ����� ���� ��������, ��� ����� ������� ��� �����-���� ���. ������, ���� ������, �� ������������� �������� ���� ����� ���� �������������
{
	FreeBlockHeader *nextFreeBlock;//��������� �� ��������� ��������� ����
};


struct MegaBlock
{
	MegaBlock *prev, *next;//�������� ����� ���������� � ����� �� ���� �������, ��������� prev ������������ �� ���� ����� freeMegaBlock, �.�. �� ����� ������ �������� ������ ������� �� ��������
	FreeBlockHeader *firstBlock;//������ ��������� ���� (� ������ freeMegaBlock - �� ������������, � filledMegaBlock - ������ ���� NULL, � � partlyFreeMegaBlocks - �� ����� ���� NULL)
	int index;//������ ��������� � ������� partlyFreeMegaBlocks
	int numOfAllocatedBlocks;//����� ��������� ����, �������� ���������� � ������ ������������� ������
#ifdef MB_THREADSAFE
	Context *context;//�������� ������, � ������� ��� ������� ������ ��������, ������ ��� ����� ������������ ��� ������������ ���������
#endif
};
static const int MEGABLOCK_HEADER_SIZE = ((sizeof(MegaBlock) + sizeof(BlockHeader) + MBALLOC_ALIGNMENT-1) & ~(MBALLOC_ALIGNMENT-1)) - sizeof(BlockHeader);


struct PartlyFreeMegaBlock
{
	MegaBlock *first;//������ �** (����������, ��������� �� ������ ���� � ������ ��������-���������)
	MegaBlock *justAllocated;//������ ��� �������������� �������� (����� ���� ���� ��������� ������ � ���)
	char *nextFreeBlock;
	int blocksLeft;//���-�� ���������� ������
};


DECL_THREAD struct Context
{
	PartlyFreeMegaBlock partlyFreeMegaBlocks[ptrSize*8*(1<<SUBPOWER_OF_TWO_BLOCK_SIZE_ACCURACY)];//������ ������� ������� ��������������� (�� � �������), �.�. ����� ��������� � compile-time ������
	MegaBlock *freeMegaBlock;//��������� �� ������ ���� � ������ ������������� ������
	MegaBlock *filledMegaBlock;//��������� �� ������ ���� � ������ ����������� ������ (���� ������ ��������� ���� ��� ����������� �������� ����� ������, �.�. ��� �� ���������� � ������ �������)
	bool forceTLSmode;
	int sysAllocatedBlocks, sysAllocatedBlocksSize;
#ifdef GATHER_BLOCK_GROUPS_STATS
	MBBlockGroup blockGroups[8*1024];//��� ����� ������� ��� ��� ����������� �������� � ����������� � ������ ������ �������
#endif
} context = {{0}, NULL, NULL, false, 0, 0};


#ifdef GATHER_BLOCK_GROUPS_STATS
void UpdateBlockGroup(BlockHeader *b, Context *ct, int size)
{
	int index = b->callstack[0];
	for (int i=1; i<LENGTH(b->callstack); i++) index ^= b->callstack[i];//������� ���

	for (;;index++)//��, ����� ��� ������� ������ �� ���������, �� ����� ��������� ��� blockGroups ������� �� ������������
	{
		index &= (LENGTH(ct->blockGroups)-1);//��� � ��� ���������� �������������� ���� � ������ � ��� ������������

		DWORD *c = ct->blockGroups[index].callstack;
		if (memcmp(c, b->callstack, sizeof(b->callstack)) == 0) break;//�����!
		if (c[0] == 0)//��� ������ �������, ����� ���������
		{
			memcpy(c, b->callstack, sizeof(b->callstack));
			break;
		}
	}

	ct->blockGroups[index].blocksSize += size;
	ct->blockGroups[index].blocks -= ((size >> 30) & 2) - 1;//������������ size > 0 ? -1 : 1
}
#else
#define UpdateBlockGroup(b, ct, size)
#endif


#ifdef MB_THREADSAFE
namespace {
	Context *contextOverride = NULL;
	MBMTMode mtMode = mbmtmTLS;
	CRITICAL_SECTION critSection;
	bool critSectionInitialized = false;

	class CritSectionAutoLeave
	{
		bool enterCS;

	public:
		CritSectionAutoLeave(Context *&ct) : enterCS(false)
		{
			if (!ct->forceTLSmode)
				if (contextOverride)
				{
					ct = contextOverride;
					if (mtMode == mbmtmCS) EnterCriticalSection(&critSection), enterCS=true;
				}
		}

		~CritSectionAutoLeave()
		{
			if (enterCS) LeaveCriticalSection(&critSection);
		}
	};
}
#endif


int GetIndexFromSize(int size)//���������� ������ �� ���������� ������� �����
{
	//ASSERT(size > 0);
	int alignMask = MBALLOC_ALIGNMENT-1;
	//if (size & alignMask) alignMask = ptrSize-1;//����� ��� ������� 20 ���� � MBALLOC_ALIGNMENT=16 ���������� 24, � �� 32 �����, �.�. ��������, ��� ��� ������������� ������ �� ��������, �� � ��� ���� ����� �� �����������
	size = (size + sizeof(BlockHeader) + alignMask) & ~alignMask;//������ ������ ���������� � ������ ������� ���������, �.�. ����� � ��������� �� 128� ���������� ���� 3 ����� �� 32� ��-�� ��������� �����, ������� 25%
	int index;
	_BitScanReverse((DWORD*)&index, (size-1)|1);//|1 ���������� �.�. ���� �������� bsr 0, �� � index ����������� �����-�� ����, � |1 ���� ��������, ��� �� ������� ���� 0-� ��� ����� ���������
#if SUBPOWER_OF_TWO_BLOCK_SIZE_ACCURACY == 0
	return index + 1;
#else
	return (index<<SUBPOWER_OF_TWO_BLOCK_SIZE_ACCURACY)
		+ (((size-1) >> (index-SUBPOWER_OF_TWO_BLOCK_SIZE_ACCURACY)) & ((1<<SUBPOWER_OF_TWO_BLOCK_SIZE_ACCURACY)-1)) + 1;
#endif
}


int GetSizeFromIndex(int index)//���������� ������ ������ (������ � ����������) ��� ��������� �������
{
#if SUBPOWER_OF_TWO_BLOCK_SIZE_ACCURACY == 0
	return 1 << index;
#else
	int baseShift = (index>>SUBPOWER_OF_TWO_BLOCK_SIZE_ACCURACY);
	return (1 << baseShift) | ((index & ((1<<SUBPOWER_OF_TWO_BLOCK_SIZE_ACCURACY)-1)) << (baseShift-SUBPOWER_OF_TWO_BLOCK_SIZE_ACCURACY));
#endif
}


#ifdef DUMP_ALLOC_CALL_STACK
#pragma comment (lib, "dbghelp.lib")
#include <DbgHelp.h>
#include <stdio.h>

static void *MBAlloc_(int size, Context *ct);
void *MB_CALL MBAlloc(int size)
{
	Context *ct = &context;
#ifdef MB_THREADSAFE
	CritSectionAutoLeave al(ct);
#endif

	BlockHeader *b = (BlockHeader*)MBAlloc_(size, ct) - 1;

	//�.�. StackWalk64 �������� � ������, �������� �������� ���� ������������ ������� call-�����
	try {
	DWORD *p;
	_asm mov [p], ebp
	//p = (DWORD*)p[0];//skip 1 callstack frame
	for (int i=0; i<LENGTH(b->callstack); i++, p = (DWORD*)p[0])
		b->callstack[i] = p[1];
	} catch (...) {}//��� ������������ ����� ��� ������ �� ������� �����, �� ������ ���� � ���������� ���������� �������� /EHa (Enable C++ Exceptions: Yes With SEH Exceptions)

	UpdateBlockGroup(b, ct, MBMemSize(b+1));

	return b + 1;
}

static void *MBAlloc_(int size, Context *ct)
{
#else
void *MB_CALL MBAlloc(int size)
{
	Context *ct = &context;
#ifdef MB_THREADSAFE
	CritSectionAutoLeave al(ct);//��� ������ ��������� ����, �.�. ����. sysAllocatedBlocks ����� �������� �� ������ �������
#endif
#endif

	if (size > MAX_BLOCK_SIZE-sizeof(BlockHeader))//�������� ������ � �������
	{
		BlockHeader *p = (BlockHeader*)SysAlloc(size+sizeof(BlockHeader));
		p->megaBlock = NULL;
		ct->sysAllocatedBlocks++;
		ct->sysAllocatedBlocksSize += size;
		return p + 1;
	}

	int index = GetIndexFromSize(size);

	PartlyFreeMegaBlock *pfmb = &ct->partlyFreeMegaBlocks[index];

	if (MegaBlock *mb = pfmb->first)//1. ������� ��������� ����� � ����������
	{
		mb->numOfAllocatedBlocks++;
		void *r = (BlockHeader*)mb->firstBlock + 1;
		mb->firstBlock = mb->firstBlock->nextFreeBlock;
		if (mb->firstBlock == NULL)//� ��������� �� �������� ����� - �������� ��� � ������ ����������� ������
		{
			ASSERT(mb->numOfAllocatedBlocks == MEGABLOCK_SIZE/GetSizeFromIndex(index));

			if (mb->next) mb->next->prev = mb->prev;//������� �� ������ ��������-��������� ����������
			pfmb->first = mb->next;//�������� �� mb->prev �� �����, �.�. mb - ������ �������� � ������ � mb->prev == NULL

			mb->prev = NULL;//�������� � ������ �����������
			mb->next = ct->filledMegaBlock;
			if (ct->filledMegaBlock) ct->filledMegaBlock->prev = mb;
			ct->filledMegaBlock = mb;
		}
		return r;
	}

	if (MegaBlock *mb = pfmb->justAllocated)//2. ������ ��� ���������� (��� �� �� ����� ���������) ���������
	{
		//mb->numOfAllocatedBlocks++;//��� �� �����, �.�. ������ ��� ���������� ���� ��������� �����������
		BlockHeader *r = (BlockHeader*)pfmb->nextFreeBlock;
		pfmb->nextFreeBlock += GetSizeFromIndex(index);
		if (--pfmb->blocksLeft == 0)//����� �����������
		{
			pfmb->justAllocated = NULL;
		}
		r->megaBlock = mb;
		return r + 1;
	}

	MegaBlock *mb;
	if (ct->freeMegaBlock)//3. ������� � ������������� ������
	{
		mb = ct->freeMegaBlock;
		ct->freeMegaBlock = mb->next;
	}
	else//4. ������ �� ��������, ����� ��� ����������� ������ � �������
	{
		mb = (MegaBlock*)SysAlloc(MEGABLOCK_SIZE+MEGABLOCK_HEADER_SIZE);
	}

	int blockSize = GetSizeFromIndex(index);
	mb->firstBlock = NULL;
	mb->index = index;
	mb->numOfAllocatedBlocks = MEGABLOCK_SIZE/blockSize;
#ifdef MB_THREADSAFE
	mb->context = ct;
#endif

	//�������� �������� � ������ ����������� ����������
	mb->prev = NULL;
	mb->next = ct->filledMegaBlock;
	if (ct->filledMegaBlock) ct->filledMegaBlock->prev = mb;
	ct->filledMegaBlock = mb;

	pfmb->justAllocated = mb;
	pfmb->blocksLeft = mb->numOfAllocatedBlocks - 1;//����������� �������� �� ����� ������

	BlockHeader *r = (BlockHeader*)((char*)mb + MEGABLOCK_HEADER_SIZE);//������ (�.�. �����) ���������� ����� ��������� ��������� � ������ ������������
	pfmb->nextFreeBlock = (char*)r + blockSize;
	r->megaBlock = mb;

	return r + 1;
}


void MB_CALL MBFree(void *p)
{
	ASSERT(p);//��� �����!

	BlockHeader *b = (BlockHeader*)p - 1;

	Context *ct = &context;
#ifdef MB_THREADSAFE
	if (MegaBlock *mb = b->megaBlock) ct = mb->context;
	CritSectionAutoLeave al(ct);
#endif

	UpdateBlockGroup(b, ct, -MBMemSize(p));

	if (MegaBlock *mb = b->megaBlock)
	{
#ifdef _DEBUG
		memset(p, 0xAA, GetSizeFromIndex(mb->index) - sizeof(BlockHeader));//��������� ������ �������, ������ ���� ��� �� ��������� ���������, �.�. free � ������ ��� ����� �������� ������ 0xDD
#ifdef MB_THREADSAFE
		if (mtMode == mbmtmTLS || context.forceTLSmode)
			ASSERT(mb->context == &context);//� ������ TLS ������ ����������� ������, ���������� � ������ ������
#endif
#endif

		if (--mb->numOfAllocatedBlocks > 0)//���� ���.. ����� � ������������!... ��, �.�. ���������� �����
		{
			if (mb->firstBlock == NULL)//�������� ��������� � ������ ��������� ����������� ������
			{
				//������� �������� �� ������ ����������� ������
				if (mb->next) mb->next->prev = mb->prev;
				if (mb->prev) mb->prev->next = mb->next; else ct->filledMegaBlock = mb->next;

				//�������� � ������ ��������-��������� ����������
				PartlyFreeMegaBlock *pfmb = &ct->partlyFreeMegaBlocks[mb->index];
				mb->prev = NULL;
				mb->next = pfmb->first;
				if (pfmb->first) pfmb->first->prev = mb;
				pfmb->first = mb;
			}

			((FreeBlockHeader*)b)->nextFreeBlock = mb->firstBlock;
			mb->firstBlock = (FreeBlockHeader*)b;
		}
		else//�������� �������� - �������� ��� � ������ ������������� ������
		{
			//������� ������� �������� �� ������ ��������-��������� ���������� (partlyFreeMegaBlocks)
			/*if (mb->firstBlock == NULL)//�������� ��������� � ������ ��������� ����������� ������, � �� ��������-��������� (���� ��� �����������������, � ����� ������� mb->numOfAllocatedBlocks++ � �������� mb->numOfAllocatedBlocks = MEGABLOCK_SIZE/blockSize �� mb->numOfAllocatedBlocks = 1, �� ��� ������������ ���������� ����� � ������ ��� ���������� ���������, �������� ����� ����������)
			{
				if (mb->next) mb->next->prev = mb->prev;
				if (mb->prev) mb->prev->next = mb->next; else filledMegaBlock = mb->next;
			}
			else
			{*/
			if (mb->next) mb->next->prev = mb->prev;
			if (mb->prev) mb->prev->next = mb->next; else ct->partlyFreeMegaBlocks[mb->index].first = mb->next;
			//}

			mb->next = ct->freeMegaBlock;
			//mb->prev = NULL;//��� �� �����, �.�. � ������ freeMegaBlock ���� prev � ���������� �� ������������
			//ct->freeMegaBlock->prev = mb;
			ct->freeMegaBlock = mb;
		}
	}
	else
	{
		ct->sysAllocatedBlocks--;
		ct->sysAllocatedBlocksSize -= MBMemSize(p);
		SysFree(b);
	}
}


int MB_CALL MBMemSize(void *p)
{
	ASSERT(p);
	BlockHeader *b = (BlockHeader*)p - 1;
	return (b->megaBlock ? GetSizeFromIndex(b->megaBlock->index) : SysMemSize(b)) - sizeof(BlockHeader);
}


void *MB_CALL MBRealloc(void *p, int size)
{
	if (p)
	{
		int blockSize = MBMemSize(p);
		if (size <= blockSize) return p;//������ ������ �� ����
		void *pp = MBAlloc(size);
		memcpy(pp, p, blockSize);
		MBFree(p);
		return pp;
	}
	else return MBAlloc(size);
}


void MB_CALL MBFreeContext()//������� ������� TL-��������, ��������������� � ������ ������� ���������� �� �������� MT-������
{
#ifdef DETECT_MEMORY_LEAKS
	if (context.filledMegaBlock) goto leak_detected;//� ���� ������ ������� �� �������� ������ ������ - ��� ���� �����������, ���� ������ ��� �������������� �� ������ �����

	for (int i=0; i<LENGTH(context.partlyFreeMegaBlocks); i++)
	{
		PartlyFreeMegaBlock &pfmb = context.partlyFreeMegaBlocks[i];
		if (pfmb.first)
			if (pfmb.first == pfmb.justAllocated && pfmb.first->next == NULL)//��������� ������ ���� �������� - ���� � ������ ����, �� ������ ��� ������� � ��� ���� ��� ��� �������������� ����� ���� �����������
			{
				if (pfmb.first->numOfAllocatedBlocks != pfmb.blocksLeft) goto leak_detected;
			}
			else goto leak_detected;
	}

	goto no_leaks;
leak_detected:
	MessageBoxA(NULL, "Memory leak detected!", "MB Allocator", MB_OK|MB_SYSTEMMODAL);

#ifdef DUMP_ALLOC_CALL_STACK
	SymSetOptions(SymGetOptions() | SYMOPT_LOAD_LINES);
	static HANDLE hProcess = GetCurrentProcess();
	SymInitialize(hProcess, NULL, TRUE);
	OutputDebugStringA("<-------------- !!! MEMORY LEAKS (MBALLOC) !!! -------------->\n");

	struct Dump
	{
		static void dumpLeakedBlock(char *block)
		{
			BlockHeader *b = (BlockHeader*)block;

			for (int i=0; i<LENGTH(b->callstack); i++)
			{
				//callstack ����, ����� ������ ����������� ���� � ������, ��������������� ������ ���� ��� ������� ������ �����
				IMAGEHLP_LINE Line = {0};
				Line.SizeOfStruct = sizeof(IMAGEHLP_LINE);

				DWORD LineDisplacement = 0;
				if (!SymGetLineFromAddr(hProcess, b->callstack[i]-1, &LineDisplacement, &Line)) break;//-1 �����, �.�. ����������� ����� - ����� ��������, �.�. �����, ��������� ����� �� ����������� call
				if (Line.FileName)
				{
					char str[500];
					//Line.FileName = strrchr(Line.FileName,'\\')+1;//��������� ������ ��� �����, ������ ����
					sprintf_s(str, sizeof(str), "%s(%i) : stack frame %i\n", Line.FileName, Line.LineNumber, i);
					OutputDebugStringA(str);
				}
			}

			OutputDebugStringA("--------------------------------------------------------------\n");
		}
	};

	for (MegaBlock *mb=context.filledMegaBlock; mb; mb=mb->next)
	{
		int blockSize = GetSizeFromIndex(mb->index);
		PartlyFreeMegaBlock &pfmb = context.partlyFreeMegaBlocks[mb->index];
		char *block = (char*)(mb+1), *end;
		if (mb == pfmb.justAllocated) end = pfmb.nextFreeBlock; else end = block + mb->numOfAllocatedBlocks*blockSize;

		for (; block<end; block+=blockSize) Dump::dumpLeakedBlock(block);
	}

	for (int b=0; b<LENGTH(context.partlyFreeMegaBlocks); b++)
	{
		PartlyFreeMegaBlock &pfmb = context.partlyFreeMegaBlocks[b];
		for (MegaBlock *mb=pfmb.first; mb; mb=mb->next)
		{
			int blockSize = GetSizeFromIndex(mb->index);
			int blocks = MEGABLOCK_SIZE/blockSize;//���-�� ������ � ������ ���������
			if (mb == pfmb.justAllocated)
				if (mb->numOfAllocatedBlocks == pfmb.blocksLeft) continue;//���������� ����� ������� ��������, ��� � ������ ��������� ��� ���������� ������
				else blocks -= pfmb.blocksLeft;

			bool blockFree[MEGABLOCK_SIZE/(sizeof(BlockHeader)+ptrSize)];
			memset(blockFree, 0, blocks);

			char *blockBasis = (char*)(mb+1);
			for (FreeBlockHeader *fbh = mb->firstBlock; fbh; fbh = fbh->nextFreeBlock)
			{
				int i = ((char*)fbh-blockBasis) / blockSize;
				ASSERT(unsigned(i) < unsigned(blocks));
				blockFree[i] = true;//�������� ������������� �����
			}

			for (int i=0; i<blocks; i++)//���������� ����� - � ���� ������
				if (!blockFree[i]) Dump::dumpLeakedBlock(blockBasis+i*blockSize);
		}
	}

	SymCleanup(hProcess);
	OutputDebugStringA("<-------------- !!! END    LEAKS (MBALLOC) !!! -------------->\n");
#endif
no_leaks:
#endif

	for (int i=0; i<LENGTH(context.partlyFreeMegaBlocks); i++)
		for (MegaBlock *mb=context.partlyFreeMegaBlocks[i].first; mb;)
		{
			MegaBlock *next = mb->next;
			SysFree(mb);
			mb = next;
		}
	memset(context.partlyFreeMegaBlocks, 0, sizeof(context.partlyFreeMegaBlocks));

	for (MegaBlock *mb=context.freeMegaBlock; mb;)
	{
		MegaBlock *next = mb->next;
		SysFree(mb);
		mb = next;
	}
	context.freeMegaBlock = NULL;

	for (MegaBlock *mb=context.filledMegaBlock; mb;)
	{
		MegaBlock *next = mb->next;
		SysFree(mb);
		mb = next;
	}
	context.filledMegaBlock = NULL;
}


void MB_CALL MBFreeMainContext()
{
	MBFreeContext();

#ifdef MB_THREADSAFE
	if (critSectionInitialized)
		DeleteCriticalSection(&critSection), critSectionInitialized = false;
#endif
}


void MB_CALL MBGetMemoryStats(MBMemoryStats &stats)
{
	memset(&stats, 0, sizeof(stats));

	int startIndex = GetIndexFromSize(1);
	stats.allocatedBlocksLength = GetIndexFromSize(MAX_BLOCK_SIZE-sizeof(BlockHeader)) - startIndex + 1;
	ASSERT(stats.allocatedBlocksLength <= LENGTH(stats.allocatedBlocks));

	for (int i=0; i<stats.allocatedBlocksLength; i++)
	{
		stats.allocatedBlocks[i].size = GetSizeFromIndex(i+startIndex) - sizeof(BlockHeader);

		PartlyFreeMegaBlock &pfmb = context.partlyFreeMegaBlocks[i+startIndex];
		for (MegaBlock *mb=pfmb.first; mb; mb=mb->next)
		{
			stats.partlyFreeMegaBlocks++;

			stats.allocatedBlocks[i].count += mb->numOfAllocatedBlocks;
			if (mb == pfmb.justAllocated)
			{
				if (mb->numOfAllocatedBlocks == pfmb.blocksLeft) stats.potentiallyFreeMegaBlocks++;
				stats.allocatedBlocks[i].count -= pfmb.blocksLeft;
			}
		}
	}

	for (MegaBlock *mb=context.freeMegaBlock; mb; mb=mb->next) stats.freeMegaBlocks++;

	for (MegaBlock *mb=context.filledMegaBlock; mb; mb=mb->next)
	{
		stats.filledMegaBlocks++;

		ASSERT(mb->numOfAllocatedBlocks == MEGABLOCK_SIZE/GetSizeFromIndex(mb->index));//� ����������� ��������� ��� ������� ������ ������ �����������
		int i = mb->index - startIndex;
		ASSERT(unsigned(i) < unsigned(stats.allocatedBlocksLength));
		stats.allocatedBlocks[i].count += mb->numOfAllocatedBlocks;
		PartlyFreeMegaBlock &pfmb = context.partlyFreeMegaBlocks[mb->index];
		if (mb == pfmb.justAllocated) stats.allocatedBlocks[i].count -= pfmb.blocksLeft;
	}

	stats.totalMegaBlocks = stats.partlyFreeMegaBlocks + stats.freeMegaBlocks + stats.filledMegaBlocks;
	stats.totalMegaBlocksSize = stats.totalMegaBlocks * (stats.megaBlockSize = (MEGABLOCK_SIZE+MEGABLOCK_HEADER_SIZE));

	//������� ������� ����� � ������
	for (int i=0; i<stats.allocatedBlocksLength; i++)
	{
		stats.netAllocatedBlocksSize += stats.allocatedBlocks[i].size * stats.allocatedBlocks[i].count;
		stats.grossAllocatedBlocksSize += (stats.allocatedBlocks[i].size+sizeof(BlockHeader)) * stats.allocatedBlocks[i].count;
		stats.totalAllocatedBlocks += stats.allocatedBlocks[i].count;//����� ����� ������� ����� ���-�� ������
	}

	stats.fragmentationDegree = 100.f - float(stats.grossAllocatedBlocksSize*100./((stats.partlyFreeMegaBlocks+stats.filledMegaBlocks)*MEGABLOCK_SIZE));

	stats.sysAllocatedBlocks     = context.sysAllocatedBlocks;
	stats.sysAllocatedBlocksSize = context.sysAllocatedBlocksSize;
}


int MB_CALL MBGetBlockGroups(MBBlockGroup *arr, int arrLength)
{
	arr; arrLength;//unwarning
	int r = 0;
#ifdef GATHER_BLOCK_GROUPS_STATS
	Context *ct = &context;
#ifdef MB_THREADSAFE
	CritSectionAutoLeave al(ct);
#endif
	for (int i=0; i<LENGTH(ct->blockGroups); i++)
		if (ct->blockGroups[i].blocks)//���������� ������ �� ������, � ������� ���� �����
		{
			memcpy(arr+r++, &ct->blockGroups[i], sizeof(MBBlockGroup));
			if (r == arrLength) break;
		}
#endif
	return r;
}


#ifdef MB_THREADSAFE
/*�� ��������� MultiThreading ������� � ������ TLS, �� ���� ����� ����������� ������� ����� �� ������ �������, �� ���,
  ������������ MBAlloc (����. ������ � ���������, ����������� MBAlloc ������ ����� ������������� new) ����� �������
  �������� � ����. ������, � �� ���������� ����� ����, � �������� ������ ���������� �������
  MBSwitchMTMode(mbmtmAppControlled), �� ��������� ������ ������� � ���� �����, ����� ������� MBSwitchMTMode(mbmtmTLS).
  ���� ����� ������������ �����-�� ������, � ������� ������ MBAlloc/MBFree �� �������� � ����. ������, ��
  ����� ��������������� ����� ������� � �������� ������� mbmtmCS, ������� ����� �������� �� ����� ������ ���� �������.
  ����������: � ������� mbmtmCS � mbmtmAppControlled ������ ����� ���� ��������� ������� �� ������ ������, ���� ����
  �� ������ MBSwitchMTMode(mbmtmCS) ��� ������������ ������, � ������� ��������� ��������� ������, � ������� ����������
  ������������ ����� ����� ������. ������ ��� ����������� � ����� mbmtmTLS, ������ ���������� � ������ ������ �� �����
  ���� ����������� �� ������ �������, ����� ���������!
*/
MBMTMode MB_CALL MBSwitchMTMode(MBMTMode mode)
{
	switch (mode)
	{
	case mbmtmTLS:
		contextOverride = NULL;
		break;
	case mbmtmAppControlled:
		contextOverride = &context;//�������������� TL-�������� �� �������� �� ��������� ������ (��������, ������� ������ ���������� �� ��������� ������)
		break;
	case mbmtmCS:
		contextOverride = &context;
		if (!critSectionInitialized)
		{
			InitializeCriticalSectionAndSpinCount(&critSection, 5000);
			critSectionInitialized = true;
		}
		break;
	}

	MBMTMode prevMTMode = mtMode;
	mtMode = mode;
	return prevMTMode;
}

/*�������������� ���������� ����� ��� �������� ������, ������������ ��� � TLS ��������.
  ����� �������� ��� ������� � ������ ������� ������, ��� �������� �� �������������� �������� �����, �.�. ��� ����� �������
  ������� �� ����� �������� � ������ �������. ��� ����� ��� ��������� ���������� ��������� ����� ������������ ������� (��������,
  ��� ����������� � ����� mbmtmTLS), ����� ������������ ������� �� ������ �� ����� ������.
*/
void MB_CALL MBForceTLSMode()
{
	context.forceTLSmode = true;
}
#endif


#ifdef OPERATOR_NEW_OVERLOAD
#include <new>
#ifdef _DEBUG
#define OPERATOR_NEW_BODY(is_array) BlockHeader *b = (BlockHeader*)MBAlloc(size) - 1; b->isArray = is_array ? BlockHeader::ARRAY_SIGN : BlockHeader::NONARRAY_SIGN; return b + 1;
#else
#define OPERATOR_NEW_BODY(is_array) return MBAlloc(size);
#endif
void* _cdecl operator new(size_t size)							{OPERATOR_NEW_BODY(false)}
void* _cdecl operator new(size_t size, const std::nothrow_t&)	{OPERATOR_NEW_BODY(false)}
void* _cdecl operator new[](size_t size)						{OPERATOR_NEW_BODY(true)}
void* _cdecl operator new[](size_t size, const std::nothrow_t&)	{OPERATOR_NEW_BODY(true)}

void _cdecl operator delete(void* p)							{if (p) {ASSERT(((BlockHeader*)p-1)->isArray == BlockHeader::NONARRAY_SIGN); MBFree(p);}}
void _cdecl operator delete(void* p, const std::nothrow_t&)		{if (p) {ASSERT(((BlockHeader*)p-1)->isArray == BlockHeader::NONARRAY_SIGN); MBFree(p);}}
void _cdecl operator delete[](void* p)							{if (p) {ASSERT(((BlockHeader*)p-1)->isArray == BlockHeader::   ARRAY_SIGN); MBFree(p);}}
void _cdecl operator delete[](void* p, const std::nothrow_t&)	{if (p) {ASSERT(((BlockHeader*)p-1)->isArray == BlockHeader::   ARRAY_SIGN); MBFree(p);}}
#endif

#endif
