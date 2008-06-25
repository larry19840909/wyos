/***************************************************************************
			WYOS process.h
			进程相关头文件
						编码:WY.lslrt				editor	 :WY.lslrt
						日期:2006/6/29			date	 	 :2006/6/29
						版权:WY 和 WY.lslrt所有 	copyright  :WY and WY.lslrt
						URL:http://www.wyos.net http://wylslrt.go.3322.org
***************************************************************************/
#ifndef	_WYOS_PROCESS_H
#define	_WYOS_PROCESS_H

#include "message.h"
#include "thread.h"

#define	MAX_PROC_NUM	32

#define	WYOS_PROC_ENTRY_POINT	0x80000000		//页目录索引512
#define	WYOS_PROC_STACK_ADDR	0x7FC00000		//页目录索引511

typedef	struct _PROCESS_TABLE
{
/*--进程控制部分--*/
	ulong					WY_ulPID;					//进程PID
	ulong					WY_ulFID;					//父进程PID
	ulong					WY_ulPriority;				//进程优先级
	ulong					WY_ulCurPriQueue;			//当前所在优先级队列
	ulong					WY_ulTimeslice;				//进程时间片
	ulong					WY_ulUseableTime;			//可用时间片
	ushort					WY_usProcState:8;			//进程状态
	ushort					WY_usProcProperty:8;		//进程属性
	ushort					WY_usTableUse;				//表是否使用,0未用，非0使用
	struct _PROCESS_TABLE *	WY_pNextTable;				//下一个进程表,在优先级队列中使用
/*--进程状态部分--*/
	ulong					WY_ulForceState;			//焦点状态，键盘和鼠标消息只会发给拥有焦点的进程
	ulong					WY_ulLDTSel;				//LDT选择子
//	ulong					WY_ulTSSSel;				//TSS选择子
	WY_SystemDesc			WY_LDT[16];					//LDT段
//	WY_TSS					WY_MainTSS;				//进程主线程
//	ulong					WY_ulIOMapEnd:8;			//TSS I/O位图结束标志
//	ulong					WY_ulReserved:24;			//为了保证四字节对齐,为一个字节的I/O结束补3字节
/*--线程控制部分--*/	
	ulong					WY_ulThreadNum;
	ulong					WY_ulCurTID;
	WY_pTHREAD				WY_pThreadCtr;				//线程控制表
/*--内存控制部分--*/	
	WY_pMemRecord			WY_pMemFirstRec;			//内存使用链表起始线性地址
	ulong					WY_ulFirstUsePhy;			//........................................物理.......
	WY_pMemRecord			WY_pMemLastRec;			//..............................结束线性地址
	ulong					WY_ulLastUsePhy;			//........................................物理.......
	WY_pMemRecord			WY_FirstRecTalbe;			//第一个可用记录表线性地址
	ulong					WY_ulPDTPhy;				//页目录物理地址
	ulong*					WY_ulPDTLine;				//页目录线性地址
	ulong					WY_ulPTELine[1024];			//1024个页表的线性地址
/*--消息管理部分--*/	
	WY_MSG					WY_Msgbuf[30];				//消息缓冲区
	ulong					WY_ulEarlyPos;				//最早的消息在缓冲区中的位置
	ulong					WY_ulLastPos;				//最新的消息在缓冲区中的位置
/*--文件系统部分--*/
	ulong					WY_ulVolumeType;			//卷类型
	ulong					WY_ulFileOpenNum;			//文件打开数目
	char						WY_CurFilePathName[256];	//当前程序完整路径
	PVOID					WY_pFileQueue;
	PVOID					WY_pDirQueue;
	ulong					reserved[662];
}WY_ProcTable,*WY_pProcTable;


//进程优先级
#define	WYOS_PROC_PRI_KIND_NUM	6				//优先级种类

#define	WYOS_PROC_PRI_REALTIME	0
#define	WYOS_PROC_PRI_HIGH		1
#define	WYOS_PROC_PRI_MIDDLE		2
#define	WYOS_PROC_PRI_LOW		3
#define	WYOS_PROC_PRI_USEUP		4				//时间片用完
#define	WYOS_PROC_PRI_IDLE		5				//此优先级是在CPU空闲时才调度
													//以上4种优先级的进程没有了，
													//才会去调度
//进程时间片长度				
#define	WYOS_PROC_TS_LONG		6
#define	WYOS_PROC_TS_MIDDLE		4
#define	WYOS_PROC_TS_SHORT		2
//进程状态
#define	WYOS_PROC_STATE_RUN		0x1			//运行状态
#define	WYOS_PROC_STATE_BLOCK	0x2			//阻塞状态,所有线程都被阻塞才会变为此状态
#define	WYOS_PROC_STATE_READY	0x4			//就绪状态
//#define	WYOS_PROC_STATE_SLEEP	0x8			//休眠状态，可以人工唤醒
//进程属性
#define	WYOS_PROC_STATE_USER	0x4				//用户进程
#define	WYOS_PROC_STATE_SYSTEM	0x2				//系统线程
#define	WYOS_PROC_STATE_KERNEL	0x1				//系统内核

//阻塞队列
#define	WYOS_PROC_BLOCK_TYPE_NUM	0x10
//阻塞类型
#define	WYOS_PROC_BLOCK_TYPE_KEYBOARD	0x0
#define	WYOS_PROC_BLOCk_TYPE_FLOPPY		0x1
#define	WYOS_PROC_BLOCK_TYPE_HARDDISK	0x2		
#define	WYOS_PROC_BLOCK_TYPE_CDROM		0x3
#define	WYOS_PROC_BLOCK_TYPE_PE			0x4		//外部设备
#define	WYOS_PROC_BLOCK_TYPE_SLEEP		0x5
#define	WYOS_PROC_BLOCK_TYPE_SOFTWARE	0x6
#define	WYOS_PROC_BLOCK_TYPE_EVENT		0x7
#define	WYOS_PROC_BLOCK_TYPE_MUTEX		0x8
#define	WYOS_PROC_BLOCK_TYPE_MSG		0x9

//阻塞队列节点结构
typedef	struct	_BLOCK_QUEUE_NODE
{
	ulong	WY_ulPID;
	ulong	WY_ulWaitTID;
	int		WY_ulWaitTime;
	ulong	WY_ulWaitParam1;
	ulong	WY_ulWaitParam2;
	ulong	WY_ulCompleteState;
	struct 	_BLOCK_QUEUE_NODE * WY_pNextNode;
}WY_BlockNode,*WY_pBlockHead;

#define	WYOS_BLOCK_STATE_WAIT			0x0
#define	WYOS_BLOCK_STATE_TIMEOUT		0x1
#define	WYOS_BLOCK_STATE_COMPLETE		0x2
//#define	WYOS_BLOCK_STATE_OUTSTANDING	0x3		//在此状态下，进程仍然需要等待
													//因为其事物可能被分阶段执行了
													//比如访问磁盘，连续的扇区数可能
													//跨越了磁道，此时将其操作分割成了
													//两部分
#define	WYOS_BLOCK_STATE_ERROR			0x4		//错误，等待的事件未成功

#define	WYOS_BLOCK_WAITTIME_INFINITY	0xFFFFFFFF
#define	WYOS_BLOCK_WAITTIME_IMMEDIATE	0x0

//
//初始化进程模块，并且初始化内核的进程表
//
void InitProc();

//
//获得当前进程PID
//
ushort GetCurrentPID();

//
//申请进程表
//
WY_pProcTable AllocProcTable(ulong *WY_ulPID);

//
//初始化进程控制表
//
void	InitProcCtrTable(WY_pProcTable WY_pTablePtr,ulong WY_ulFID,ulong WY_ulPID,
					     ulong WY_ulPriority,ulong WY_ulTimeslice,ushort WY_usProcProperty);

//
//构造进程表
//
BOOL ConstructProcTable(WY_pProcTable WY_pTablePtr,ushort WY_usProcProperty,
							ulong WY_ulTextPhyAddr,ulong WY_ulTextRange);

//
//主调度函数，返回下一个允许执行的进程PID
//
ushort MasterScheudler(); 

//
//任务调度函数，返回允许执行进程的一个线程
//任务选择子该任务在MasterScheudler函数后调用，
//在支持线程的版本中实现。
//
ulong TaskScheudler(ushort WY_usPID,ulong *WY_ulUseableTID);

//
//阻塞自己的函数
//
//ulong Block();
//
//解除阻塞指定进程的函数
//
//ulong Unblock(ushort WY_usPID,ulong WY_BlockType,ulong WY_ulUnblockReason);


//
//系统调用Sleep内核部分
//
ulong SleepKnl(pSyscallParam WY_pInputParam);

//
//插入优先级队列
//
//void InserttoPri(ushort WY_usPID);

//
//从优先级队列中删除
//
//void DeleteinPri(ushort WY_usPID);
#endif

