/***************************************************************************
			WYOS Memory.h
			�ڴ����ͷ�ļ�
						����:WY.lslrt				editor	 :WY.lslrt
						����:2005/12/2			date	 :2005/12/2
						��Ȩ:WY �� WY.lslrt���� 	copyright:WY and WY.lslrt
						URL:http://www.wyos.net http://wylslrt.go.3322.org
***************************************************************************/
#ifndef	_WYOS_MEMORY_H
#define	_WYOS_MEMORY_H

extern	void	PageException();
//int15h E802�Ź��ܷ��صĵ�ַ�������ṹ��
typedef	struct	_E802_STRUCT
{
	ulong	WY_ulBaseAddrLow;				 //��Χ����ַ��32 λ
											//�����ǵ�32λ���Ͼ���
											//����ַ��
	ulong	WY_ulBaseAddrHigh;				//��Χ����ַ��32 λ
											//�����ǵ�32 Ϊ����Ϊ0
	ulong	WY_ulLengthLow;				//��Χ���ȵĵ�32 λ
	ulong	WY_ulLengthHigh;				//��Χ���ȸ�32 λ
	ulong	WY_ulAddrRangeType;			//��ַ��Χ������
}WY_E802MM,*WY_pE802MM;

typedef	struct _MEMORY_USE_RECORD
{
	unsigned long	WY_ulLinearAddress;
	unsigned long	WY_ulRangeSize;
	unsigned long	WY_ulPhysicalPage:20;
	unsigned long	WY_ulRecordType:1;							//��¼�����ͣ�0��ʾ�ڴ��¼�飬1��ʾ��¼�������
	unsigned long	WY_ulIdentity:10;							//������ڴ��¼�飬�����ڸñ��е�λ��
															//����Ǽ�¼�����������������ı�ʾ
	unsigned long	WY_ulKNLFlag:1;								//�ÿ��¼Ϊ1��ʾ���ں��ڴ�							
	union	
	{
		struct
		{
			unsigned long	WY_ulTableOfNextRecord:20;			//��һ����¼�����ڵļ�¼�����ҳ���
			unsigned long	WY_ulTablePosOfNextRecord:8;		//��һ����¼���ڼ�¼���е�λ��
															//������ֵΪ0 ���ʾ�Ǽ�¼
															//���������һ������ʱǰһ����ԱҲҪ��Ϊ0
			unsigned long	WY_ulFlag:1;						//������ڴ��¼�飬0��ʾδʹ�ã�1��ʾ��ʹ��
			unsigned long	WY_ulFirst:1;						//��ʾ�Ƿ�Ϊ�����һ���ڵ㣬1��ʾ��
			unsigned long	WY_ulRecordReserved:2;
		}WY_RecordCharacter;	//��һ����¼��
		struct
		{
			unsigned long	WY_ulNextTable:20;					//��һ����¼�����ҳ��ţ���Ϊһ����������4K������ͨ��ҳ��
															//�ҵ���������ʼ��ַ
															//������ֵΪ0 ���ʾ�ǵ�ǰ
															//�ļ�¼��ʾ�����һ����
			unsigned long	WY_ulTableUsed:8;					//�ñ��Ѿ�ʹ���˶���
			unsigned long WY_ulTableReserved:4;
		}WY_TableCharacter;
	}WY_Use;
}WY_MemRecord,*WY_pMemRecord;

#define MEM_USE_RECORD_SIZE	sizeof(WY_MemRecord)
#define NUM_USEREC_TABLE		(int)(0x1000 / MEM_USE_RECORD_SIZE)

typedef	struct _MEMORY_FREE_RECORD
{
	union
	{
		struct
		{
			unsigned long	WY_ulPhysicalAddress;
			unsigned long	WY_ulRangeSize;
			unsigned long	WY_ulTableOfNextRecord;
			unsigned long	WY_ulThisRecNum:8;
			unsigned long	WY_ulTablePosOfNextRecord:8;
			unsigned long	WY_ulUseFlag:1;		//0 unused 1 used
			unsigned long	WY_ulFirstFlag:1;       //0 not first 1 first
			unsigned long	WY_ulRecReserved:14;
		}WY_Record;
		struct
		{
			unsigned long	WY_ulThisTableId;
			unsigned long	WY_ulRecordUsed;
			unsigned long WY_ulNextTable;
			unsigned long WY_ulReserved;
		}WY_Table;
	}WY_Free;
}WY_FreeMemRec,*WY_pFreeMemRec;

#define	MEM_FREE_RECORD_SIZE		sizeof(WY_FreeMemRec)
#define	NUM_FREEREC_TABLE		(int)(0x1000 / MEM_FREE_RECORD_SIZE)

#define	ADDRESS_RANGE_MEMORY		0x1	//ֻ��1 ��ʾ����ַ��Χ
											//�ɹ�����ϵͳʹ��
#define	MEMORY_USE_RECORD_START		(WY_pMemRecord)0x4000
#define	MEMORY_FREE_RECORD_START	(WY_pFreeMemRec)0x6000

/*************************************/
//
//�ڴ��������ʼ��������
//��������ڴ棬�����ڴ��������
//�ں��ڴ�ʹ������
//
/*************************************/
void	meminit();

/*************************************/
//
//����һ���ڴ�ʹ�ü�¼
//
/*************************************/
void ConstructUseRecord(WY_pMemRecord WY_pMemUseRec,
							ulong	WY_ulLinearAddr,
							ulong	WY_ulBlockSize,
							ulong	WY_ulPhyPageNum,
							ulong	WY_ulRecNum,
							ulong	WY_ulNextRecTable,
							ulong	WY_ulNextRecNum,
							ulong	WY_ulFirstFlag,
							BOOL	WY_bKnl);

/*************************************/
//
//����һ�������ڴ���м�¼
//
/*************************************/
void ConstructFreeRecord(WY_pFreeMemRec WY_pFreeRecord,
							 ulong	WY_ulPhysicalAddress,
							 ulong	WY_ulBlockSize,
							 ulong	WY_ulTableOfNextRec,
							 ulong	WY_ulThisRecNum,
							 ulong	WY_ulNextRecNum,
							 ulong	WY_ulIsFirst
							 );

/*************************************/
//
//�������ַӳ��Ϊ���Ե�ַ
//
/*************************************/
ulong PhyToLinear(ulong WY_ulPhysicalAddress,int WY_nPageNum,BOOL WY_bKnl);

/*************************************/
//
//ȡ�����Ե�ַ������ӳ��
//
/*************************************/
ulong UnmappedLinear(ulong WY_ulLinearAddress,int WY_nPageNum,BOOL WY_bKnl);

/*************************************/
//
//������Ե�ַ�������ַӳ��
//
/*************************************/
ulong LineartToPhy(ulong WY_ulLinearAddress);

/*************************************/
//
//����һ���ڴ�ʹ�ü�¼
//
/*************************************/
WY_pMemRecord	allocrec_use(BOOL WY_bSystem);

/*************************************/
//
//����һ�������ڴ��¼
//
/*************************************/
WY_pFreeMemRec allocrec_free();

/*************************************/
//
//����һ���ڴ�ʹ�ü�¼
//
/*************************************/
void InsertRec_use(WY_pMemRecord WY_pMemUseRec,BOOL WY_bSystem);

/*************************************/
//
//����һ���ڴ�ʹ�ü�¼
//
/*************************************/
void InsertRec_free(WY_pFreeMemRec WY_pFreeMemRecord);

/*************************************/
//
//ɾ��һ���ڴ�ʹ�ü�¼
//
/*************************************/
void DeleteRec_use(WY_pMemRecord WY_pMemUseRec,BOOL WY_bSystem);

/*************************************/
//
//ɾ��һ�������ڴ��¼
//
/*************************************/
void DeleteRec_free(WY_pFreeMemRec WY_pFreeRec);

/*************************************/
//
//������ҳ��Ϊ��λ���ڴ�
//
/*************************************/
WY_pMemRecord AllocPage(int WY_nPageNum,BOOL WY_bKnl,BOOL WY_bSystem);

/*************************************/
//
//��������ҳ��
//
/*************************************/
ulong AllocPhyPage(int WY_nPageNum);

/*************************************/
//
//�ͷ���ҳ��Ϊ��λ���ڴ�
//
/*************************************/
void FreePage(ulong WY_ulPhysicalAddress,int WY_nPageNum,BOOL WY_bKnl);

/*************************************/
//
//��������������������������ڴ�������һ��
//
/*************************************/
void ClearupFreeChain();

/*************************************/
//
//����ָ���ֽ�����С���ڴ�
//
/*************************************/
void * mallocmem(int WY_nSize,BOOL WY_bKnl,BOOL WY_bSystem);

/*************************************/
//
//�ͷ�һ���ڴ�
//
/*************************************/
void freemem(void * WY_pAddr,BOOL WY_bKnl,BOOL WY_bSystem);

void *mallock(int WY_nSize);

void freek(void * WY_pAddr);

void *mallocs(int WY_nSize);

void frees(void * WY_pAddr);

/*************************************/
//
//ҳ���쳣������
//
/*************************************/
void PageExpProc(ulong WY_ulErrCode,ulong WY_ulFaultLinearAddr);

ulong SyscallGetMmInfo();

ulong SyscallMalloc(pSyscallParam WY_pInputParam);

void  SyscallFree(pSyscallParam WY_pInputParam);
#endif

