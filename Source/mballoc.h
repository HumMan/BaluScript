#if 0//#ifdef _MSC_VER

#ifndef MBALLOC_H
#define MBALLOC_H

//#define MB_THREADSAFE//~10% �������� � ����� ������� ������� (TLS ��� AppControlled), � 2.5 ���� ��������� � ������ mbmtmCS

#define MB_CALL __fastcall


extern void *MB_CALL MBAlloc(int size);
extern void *MB_CALL MBRealloc(void *p, int size);
extern int   MB_CALL MBMemSize(void *p);
extern void  MB_CALL MBFree(void *p);
extern void  MB_CALL MBFreeMainContext();//��� ������� ����� �������� � �������� ������
extern void  MB_CALL MBFreeContext();//� ��� - �� ���� �������������� �������

struct MBMemoryStats
{
	struct {int size, count;} allocatedBlocks[128];//���������� �� ���������� ������ (������ �����, ���-�� ������)
	int allocatedBlocksLength;//������ ������� allocatedBlocks
	int totalAllocatedBlocks;//����� ���������� ���������� ����������� ������
	int partlyFreeMegaBlocks, freeMegaBlocks, filledMegaBlocks;//���������� ���������� ��������� �����
	int totalMegaBlocks;//����� ���������� �������� = ����� ����. 3-� �����
	int totalMegaBlocksSize;//��� ������, ���������� �����������
	int megaBlockSize;//������ ������ ������ ���������, ������� ���������
	int potentiallyFreeMegaBlocks;//������������ ��������� ��������� (��, ������� ��������� � ������ �������� ���������, �� ������� ��� ��������� ��������)
	int netAllocatedBlocksSize;//������ ���������� ����������� ������ ��� ����� ��������� ���������� ���������� (���������� ������ ����.)
	int grossAllocatedBlocksSize;//������ ���������� ����������� ������ � ������ ��������� ���������� ����������, ��������� ����� ��������� � totalMegaBlocksSize ����� ������������� ��� ����� ��� ���������� �� ������� ������
	float fragmentationDegree;//��������������� ������� ������������ ������ �� 0 �� 100% (��� ������, ��� ����)
	int sysAllocatedBlocks, sysAllocatedBlocksSize;//���-�� � ��������� ������ ���������������� ����������� (���������) ������
};
extern void MB_CALL MBGetMemoryStats(MBMemoryStats &stats);//�������� ���������� �� ���������� ������ ������ ��� �������� ������ (����������� ������ ����� �������� �� MAX_BLOCK_SIZE)

struct MBBlockGroup
{
	unsigned long callstack[7];
	int blocks, blocksSize;
};
extern int MB_CALL MBGetBlockGroups(MBBlockGroup *arr, int arrLength);//��������� ��������� �� ����� � ���-�� ��������� � ���; ���������� ���-�� ����� ������, ���������� � �����
//������� ����� ����������� ����� �� �������, ����� ������ ����� ����� ������, ����� �� ������ ������� ����� ���������� �����-� ����/������, ��� ����� ������� ��� ��� ����� �������������, �.�. SymGetLineFromAddr �� ����� �������

#ifdef MB_THREADSAFE
enum MBMTMode
{
	mbmtmTLS = 0,
	mbmtmAppControlled,//����� ������� ����������� � ����. ������ ������ MBAlloc/MBFree
	mbmtmCS,
};
extern MBMTMode MB_CALL MBSwitchMTMode(MBMTMode mode = mbmtmTLS);
extern void MB_CALL MBForceTLSMode();
#endif

#endif
#endif
