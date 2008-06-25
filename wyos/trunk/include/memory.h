/***************************************************************************
			WYOS Memory.h
			内存管理头文件
						编码:WY.lslrt				editor	 :WY.lslrt
						日期:2005/12/2			date	 :2005/12/2
						版权:WY 和 WY.lslrt所有 	copyright:WY and WY.lslrt
						URL:http://www.wyos.net http://wylslrt.go.3322.org
***************************************************************************/
#ifndef	_WYOS_MEMORY_H
#define	_WYOS_MEMORY_H

extern	void	PageException();
//int15h E802号功能返回的地址描述符结构块
typedef	struct	_E802_STRUCT
{
	ulong	WY_ulBaseAddrLow;				 //范围基地址低32 位
											//在我们的32位机上就是
											//基地址了
	ulong	WY_ulBaseAddrHigh;				//范围基地址高32 位
											//在我们的32 为机上为0
	ulong	WY_ulLengthLow;				//范围长度的低32 位
	ulong	WY_ulLengthHigh;				//范围长度高32 位
	ulong	WY_ulAddrRangeType;			//地址范围的类型
}WY_E802MM,*WY_pE802MM;

typedef	struct _MEMORY_USE_RECORD
{
	unsigned long	WY_ulLinearAddress;
	unsigned long	WY_ulRangeSize;
	unsigned long	WY_ulPhysicalPage:20;
	unsigned long	WY_ulRecordType:1;							//记录的类型，0表示内存记录块，1表示记录表的描述
	unsigned long	WY_ulIdentity:10;							//如果是内存记录块，则是在该表中的位置
															//如果是记录表描述，则是这个表的标示
	unsigned long	WY_ulKNLFlag:1;								//该块记录为1表示是内核内存							
	union	
	{
		struct
		{
			unsigned long	WY_ulTableOfNextRecord:20;			//下一个记录块所在的记录表的虚页框号
			unsigned long	WY_ulTablePosOfNextRecord:8;		//下一个记录块在记录表中的位置
															//如果这个值为0 则表示是记录
															//链表中最后一个，此时前一个成员也要设为0
			unsigned long	WY_ulFlag:1;						//如果是内存记录块，0表示未使用，1表示已使用
			unsigned long	WY_ulFirst:1;						//表示是否为链表第一个节点，1表示是
			unsigned long	WY_ulRecordReserved:2;
		}WY_RecordCharacter;	//下一个记录块
		struct
		{
			unsigned long	WY_ulNextTable:20;					//下一个记录表的虚页框号，因为一个表正好是4K，所以通过页框
															//找到这个表的起始地址
															//如果这个值为0 则表示是当前
															//的记录表示是最后一个表
			unsigned long	WY_ulTableUsed:8;					//该表已经使用了多少
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

#define	ADDRESS_RANGE_MEMORY		0x1	//只有1 表示这块地址范围
											//可供操作系统使用
#define	MEMORY_USE_RECORD_START		(WY_pMemRecord)0x4000
#define	MEMORY_FREE_RECORD_START	(WY_pFreeMemRec)0x6000

/*************************************/
//
//内存管理器初始化函数，
//检查物理内存，构造内存空闲链表
//内核内存使用链表
//
/*************************************/
void	meminit();

/*************************************/
//
//构造一个内存使用记录
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
//构造一个物理内存空闲记录
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
//将物理地址映射为线性地址
//
/*************************************/
ulong PhyToLinear(ulong WY_ulPhysicalAddress,int WY_nPageNum,BOOL WY_bKnl);

/*************************************/
//
//取消线性地址的物理映射
//
/*************************************/
ulong UnmappedLinear(ulong WY_ulLinearAddress,int WY_nPageNum,BOOL WY_bKnl);

/*************************************/
//
//获得线性地址的物理地址映射
//
/*************************************/
ulong LineartToPhy(ulong WY_ulLinearAddress);

/*************************************/
//
//申请一个内存使用记录
//
/*************************************/
WY_pMemRecord	allocrec_use(BOOL WY_bSystem);

/*************************************/
//
//申请一个空闲内存记录
//
/*************************************/
WY_pFreeMemRec allocrec_free();

/*************************************/
//
//插入一个内存使用记录
//
/*************************************/
void InsertRec_use(WY_pMemRecord WY_pMemUseRec,BOOL WY_bSystem);

/*************************************/
//
//插入一个内存使用记录
//
/*************************************/
void InsertRec_free(WY_pFreeMemRec WY_pFreeMemRecord);

/*************************************/
//
//删除一个内存使用记录
//
/*************************************/
void DeleteRec_use(WY_pMemRecord WY_pMemUseRec,BOOL WY_bSystem);

/*************************************/
//
//删除一个空闲内存记录
//
/*************************************/
void DeleteRec_free(WY_pFreeMemRec WY_pFreeRec);

/*************************************/
//
//申请以页面为单位的内存
//
/*************************************/
WY_pMemRecord AllocPage(int WY_nPageNum,BOOL WY_bKnl,BOOL WY_bSystem);

/*************************************/
//
//申请物理页面
//
/*************************************/
ulong AllocPhyPage(int WY_nPageNum);

/*************************************/
//
//释放以页面为单位的内存
//
/*************************************/
void FreePage(ulong WY_ulPhysicalAddress,int WY_nPageNum,BOOL WY_bKnl);

/*************************************/
//
//重新整理空闲链表，将在连续的内存整合在一起
//
/*************************************/
void ClearupFreeChain();

/*************************************/
//
//申请指定字节数大小的内存
//
/*************************************/
void * mallocmem(int WY_nSize,BOOL WY_bKnl,BOOL WY_bSystem);

/*************************************/
//
//释放一块内存
//
/*************************************/
void freemem(void * WY_pAddr,BOOL WY_bKnl,BOOL WY_bSystem);

void *mallock(int WY_nSize);

void freek(void * WY_pAddr);

void *mallocs(int WY_nSize);

void frees(void * WY_pAddr);

/*************************************/
//
//页面异常处理函数
//
/*************************************/
void PageExpProc(ulong WY_ulErrCode,ulong WY_ulFaultLinearAddr);

ulong SyscallGetMmInfo();

ulong SyscallMalloc(pSyscallParam WY_pInputParam);

void  SyscallFree(pSyscallParam WY_pInputParam);
#endif

