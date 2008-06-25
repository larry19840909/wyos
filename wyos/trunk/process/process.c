/***************************************************************************
			WYOS process.c
			进程相关源文件
						编码:WY.lslrt				editor	 :WY.lslrt
						日期:2006/6/29			date	 	 :2006/6/29
						版权:WY 和 WY.lslrt所有 	copyright  :WY and WY.lslrt
						URL:http://www.wyos.net http://wylslrt.go.3322.org
***************************************************************************/
#include "..\include\WYOSType.h"
#include "..\include\syscall.h"
#include "..\include\cpu.h"
#include "..\video\video.h"
#include "..\include\string.h"
#include "..\include\memory.h"
#include "..\include\math.h"
#include "..\include\mutex.h"
#include "..\include\process.h"

extern WY_pBlockHead	WY_BlockQueue[WYOS_PROC_BLOCK_TYPE_NUM];


WY_ProcTable			WY_PROCTABLE[MAX_PROC_NUM];
ushort				WY_usCurrentPID = 0;
//进程优先级队列头NULL为结束
WY_pProcTable		WY_PRIQueue[WYOS_PROC_PRI_KIND_NUM];
//用于进程表互斥的自旋锁
ulong				WY_ulTableLock = 1;

void InitProc()
{
	WY_pSystemDesc		WY_pKnlDesc;	//内核描述符指针

	//设置全局变量
	//清空进程表
	memset((char*)&WY_PROCTABLE,0,sizeof(WY_ProcTable) * MAX_PROC_NUM);
	memset((char*)WY_PRIQueue,0,sizeof(WY_pProcTable) * WYOS_PROC_PRI_KIND_NUM);
	memset((char*)WY_BlockQueue,0,sizeof(WY_pBlockHead) * WYOS_PROC_BLOCK_TYPE_NUM);
	WY_ulTableLock = 1;
	
	//初始化内核进程表
	//设置内核进程表内存控制部分
	WY_PROCTABLE[0].WY_ulPDTPhy = 0;
	WY_PROCTABLE[0].WY_ulPDTLine = NULL;
	WY_PROCTABLE[0].WY_ulPTELine[0] = 0x1000;
	WY_PROCTABLE[0].WY_FirstRecTalbe = MEMORY_USE_RECORD_START;
	
	//申请LDT和TSS段描述符
	WY_pKnlDesc = allocGlobalDesc(FALSE);
	if(!WY_pKnlDesc)
	{
		printk("System Error !Please Restart!\n");
		while(1)
			{}
	}
	//设置内核进程表
	WY_PROCTABLE[0].WY_ulPID = 0;
	WY_PROCTABLE[0].WY_ulFID = 0;
	WY_PROCTABLE[0].WY_ulPriority = WYOS_PROC_PRI_IDLE;
	WY_PROCTABLE[0].WY_usProcProperty =  WYOS_PROC_STATE_KERNEL;
	WY_PROCTABLE[0].WY_usProcState = WYOS_PROC_STATE_RUN;
	WY_PROCTABLE[0].WY_ulTimeslice = WYOS_PROC_TS_SHORT;
	WY_PROCTABLE[0].WY_ulUseableTime = 4;
	WY_PROCTABLE[0].WY_usTableUse = 1;
	//设置LDT段描述符
	//设置LDT段描述符属性
	WY_pKnlDesc->WY_ulPresent = 1;
	WY_pKnlDesc->WY_ulDescType = 0;
	WY_pKnlDesc->WY_ulSegTYPE = SYSTEM_LDT;
	WY_pKnlDesc->WY_ulDescDPL = SEGMENT_RING0;
	WY_pKnlDesc->WY_ulGranularity = 0;
	WY_pKnlDesc->WY_ulD = 0;
	WY_pKnlDesc->WY_ulSoftUse = 0;
	WY_pKnlDesc->WY_ulReserved = 0;
	//设置LDT段基址及段限
	WY_pKnlDesc->WY_ulLowSegLimit = 64;
	WY_pKnlDesc->WY_ulHighSegLimit = 0;
	WY_pKnlDesc->WY_ulLowSegBase = ((ulong)WY_PROCTABLE[0].WY_LDT) & 0xFFFFFF;
	WY_pKnlDesc->WY_ulHighSegBase = (((ulong)WY_PROCTABLE[0].WY_LDT) & 0xFF000000) >> 24;
	//计算LDT段选择子
	WY_PROCTABLE[0].WY_ulLDTSel = ((ulong)WY_pKnlDesc - WYOS_GDT_BASE);

	WY_usCurrentPID = 0;
	//设置线程控制表，将第一个表映射
	WY_PROCTABLE[0].WY_pThreadCtr = (WY_pTHREAD)mallocs(sizeof(WY_THREAD) * MAX_THREAD_NUM + 1);
	memset((char *)WY_PROCTABLE[0].WY_pThreadCtr,0,sizeof(WY_THREAD) * MAX_THREAD_NUM + 1);
	WY_PROCTABLE[0].WY_pThreadCtr[0].WY_ulUseFlag = 1;
	WY_PROCTABLE[0].WY_pThreadCtr[0].WY_usThreadState = WYOS_THREAD_RUN;
	WY_PROCTABLE[0].WY_pThreadCtr[0].WY_usUseTime = THREAD_RUN_TIME;

	//设置TSS段描述符
	WY_pKnlDesc++;
	WY_pKnlDesc->WY_ulPresent = 1;
	WY_pKnlDesc->WY_ulDescType = 0;
	WY_pKnlDesc->WY_ulSegTYPE = SYSTEM_USEABLE_386TSS;
	WY_pKnlDesc->WY_ulDescDPL = SEGMENT_RING0;
	WY_pKnlDesc->WY_ulGranularity = 0;
	WY_pKnlDesc->WY_ulD = 0;
	WY_pKnlDesc->WY_ulSoftUse = 0;
	WY_pKnlDesc->WY_ulReserved = 0;
	//设置TSS段基址及段限
	WY_pKnlDesc->WY_ulLowSegLimit = 0x69;			//105个字节，长度应该包括I/O MAP结束标志
	WY_pKnlDesc->WY_ulHighSegLimit = 0;
	WY_pKnlDesc->WY_ulLowSegBase = ((ulong)&WY_PROCTABLE[0].WY_pThreadCtr[0].WY_ThreadTss) & 0xFFFFFF;
	WY_pKnlDesc->WY_ulHighSegBase = (((ulong)&WY_PROCTABLE[0].WY_pThreadCtr[0].WY_ThreadTss) & 0xFF000000) >> 24;

	//设置TSS段
	WY_PROCTABLE[0].WY_pThreadCtr[0].WY_ThreadTss.WY_ulCR3 = WY_PROCTABLE[0].WY_ulPDTPhy;
	//设置I/O MAP结束标志
	WY_PROCTABLE[0].WY_pThreadCtr[0].WY_ulIOMapEnd = 0xFF;
	//装入tr寄存器
	WY_PROCTABLE[0].WY_pThreadCtr[0].WY_usTSSSel = ((ulong)WY_pKnlDesc - WYOS_GDT_BASE);
	__asm__("ltr %%ax" : : "a"(WY_PROCTABLE[0].WY_pThreadCtr[0].WY_usTSSSel));
	
	
	WY_PROCTABLE[0].WY_ulCurTID = 0;
	WY_PROCTABLE[0].WY_ulThreadNum = 1;


	//将kernel进程放入到idle队列,采用头插法
	WY_PROCTABLE[0].WY_ulCurPriQueue = WYOS_PROC_PRI_MIDDLE;
	WY_PROCTABLE[0].WY_pNextTable = WY_PRIQueue[WYOS_PROC_PRI_MIDDLE];
	WY_PRIQueue[WYOS_PROC_PRI_MIDDLE] = &WY_PROCTABLE[0];
//	printk("%x PID 0 Next %x \n",&WY_PROCTABLE[0],WY_PROCTABLE[0].WY_pNextTable);
	//安装系统调用
	SetupSyscall((SysCallProc)GetTIDSyscall,9);
	SetupSyscall((SysCallProc)GetCurrentPID,10);
	//11是留给CreateProcess的
	//16-19是留给进程休眠和消息的
	SetupSyscall((SysCallProc)SleepKnl,16);
	SetupSyscall((SysCallProc)SendMsgSyscall,17);
	SetupSyscall((SysCallProc)KReciveMessage,18);
}

ushort GetCurrentPID()
{
	return WY_usCurrentPID;
}

WY_pProcTable AllocProcTable(ulong *WY_ulPID)
{
	ulong	WY_ulSpinKey = 0;
	int		i;
	
	for(i = 1;i < MAX_PROC_NUM;i++)
	{
		if(!WY_PROCTABLE[i].WY_usTableUse)
		{
			//找到一个没有使用的
			//加锁
			SpinLock(&WY_ulSpinKey,&WY_ulTableLock);
			//因为在加锁过程中,该任务可能会被
			//另一个任务给锁住,因此还需要在查
			//看一次使用标志
			if(WY_PROCTABLE[i].WY_usTableUse)
			{
				//加锁后该表被占用,释放并查看下一个表
				ReleaseSpinLock(&WY_ulSpinKey,&WY_ulTableLock);
			}
			else
			{
				//该表可以使用
				WY_PROCTABLE[i].WY_usTableUse = 1;
				ReleaseSpinLock(&WY_ulSpinKey,&WY_ulTableLock);
				*WY_ulPID = i;
				return &WY_PROCTABLE[i];
			}
		}
	}
}

void InitProcCtrTable(WY_pProcTable WY_pTablePtr, ulong WY_ulFID, ulong WY_ulPID, ulong WY_ulPriority, ulong WY_ulTimeslice, ushort WY_usProcProperty)
{
	WY_pTablePtr->WY_ulFID = WY_ulFID;
	WY_pTablePtr->WY_ulPID = WY_ulPID;
	WY_pTablePtr->WY_ulPriority = WY_ulPriority;
	WY_pTablePtr->WY_ulCurPriQueue = WY_ulPriority;
	WY_pTablePtr->WY_ulTimeslice = WY_ulTimeslice;
	WY_pTablePtr->WY_ulUseableTime = WY_ulTimeslice;
	WY_pTablePtr->WY_usProcState = WYOS_PROC_STATE_READY;
	WY_pTablePtr->WY_usProcProperty = WY_usProcProperty;
	//插入优先级队列
	WY_pTablePtr->WY_pNextTable = WY_PRIQueue[WY_ulPriority];
	WY_PRIQueue[WY_ulPriority] = WY_pTablePtr;
	
}

BOOL ConstructProcTable(WY_pProcTable WY_pTablePtr, ushort WY_usProcProperty, ulong WY_ulTextPhyAddr, ulong WY_ulTextRange)
{
	ulong	WY_ulPhyAddr,WY_ulTextPTEPhy,WY_ulStackPhy,WY_ulStackPTEPhy,WY_ulKnlStackPhy,WY_ulTextPageNum;
	ulong	WY_ulLinearAddr,WY_ulTextPTELine,WY_ulStackLine,WY_ulStackPTELine,WY_ulKnlStackLine;
	ulong	WY_ulMemRecTablePhy,WY_ulMemRecTableLine;
	ulong	*WY_pPTE,WY_ulRecPosArr[4] = {1,2,3,4};
	int		i,j;
	WY_pMemRecord	WY_pTable,WY_pRecord;
	WY_pSystemDesc	WY_pProcDesc;

	//首先为页目录申请空间
	WY_ulPhyAddr = AllocPhyPage(1);
	if(WY_ulPhyAddr == 0)						//申请失败，释放该进程表
	{
		
		WY_pTablePtr->WY_usTableUse = 0;
		return FALSE;
	}
	
	//映射该页面
	WY_ulLinearAddr = PhyToLinear(WY_ulPhyAddr,1,TRUE);
	if(WY_ulLinearAddr == 0)					//映射失败，释放该表和页目录空间
	{
		WY_pTablePtr->WY_usTableUse = 0;
		FreePage(WY_ulPhyAddr,1,TRUE);
		return FALSE;
	}
	//为进程代码区域申请页表，和堆栈区域申请页表
	//申请代码区域页表
	if(WY_ulTextRange % 0x1000 == 0)
	{
		WY_ulTextPageNum = WY_ulTextRange / 0x1000;
	}
	else
	{
		WY_ulTextPageNum = WY_ulTextRange / 0x1000 + 1;
	}
	WY_ulTextPTEPhy = AllocPhyPage(1);		//申请页表空间
	if(WY_ulTextPTEPhy == 0)
	{
		WY_pTablePtr->WY_usTableUse = 0;
		FreePage(WY_ulPhyAddr,1,TRUE);
		UnmappedLinear(WY_ulLinearAddr,1,TRUE);
		return FALSE;
	}
	WY_ulTextPTELine = PhyToLinear(WY_ulTextPTEPhy,1,TRUE);	//映射页表到内核空间
	if(WY_ulTextPTELine == 0)
	{
		WY_pTablePtr->WY_usTableUse = 0;
		FreePage(WY_ulPhyAddr,1,TRUE);
		UnmappedLinear(WY_ulLinearAddr,1,TRUE);
		FreePage(WY_ulTextPTEPhy,1,TRUE);
		return FALSE;
	}
	//申请堆栈1M，并且为其申请页表
	WY_ulStackPhy = AllocPhyPage(256);			//申请堆栈空间
	if(WY_ulStackPhy == 0)
	{
		WY_pTablePtr->WY_usTableUse = 0;
		FreePage(WY_ulPhyAddr,1,TRUE);
		UnmappedLinear(WY_ulLinearAddr,1,TRUE);
		FreePage(WY_ulTextPTEPhy,1,TRUE);
		UnmappedLinear(WY_ulTextPTELine,1,TRUE);
	}
	WY_ulStackPTEPhy = AllocPhyPage(1);			//为堆栈空间申请页表空间
	if(WY_ulStackPTEPhy == 0)
	{
		WY_pTablePtr->WY_usTableUse = 0;
		FreePage(WY_ulPhyAddr,1,TRUE);
		UnmappedLinear(WY_ulLinearAddr,1,TRUE);
		FreePage(WY_ulTextPTEPhy,1,TRUE);
		UnmappedLinear(WY_ulTextPTELine,1,TRUE);
		FreePage(WY_ulStackPhy,256,TRUE);
	}
	WY_ulStackPTELine = PhyToLinear(WY_ulStackPTEPhy,1,TRUE);		//映射 页表到内核空间
	if(WY_ulStackPTELine == 0)
	{
		WY_pTablePtr->WY_usTableUse = 0;
		FreePage(WY_ulPhyAddr,1,TRUE);
		UnmappedLinear(WY_ulLinearAddr,1,TRUE);
		FreePage(WY_ulTextPTEPhy,1,TRUE);
		UnmappedLinear(WY_ulTextPTELine,1,TRUE);
		FreePage(WY_ulStackPhy,256,TRUE);
		FreePage(WY_ulStackPTEPhy,1,TRUE);
	}
	//申请内核堆栈64K,因为其位于内核空间
	//因此不需要再为其申请页表空间
	WY_ulKnlStackPhy = AllocPhyPage(16);			//申请进程内核堆栈
	if(WY_ulKnlStackPhy == 0)
	{
		WY_pTablePtr->WY_usTableUse = 0;
		FreePage(WY_ulPhyAddr,1,TRUE);
		UnmappedLinear(WY_ulLinearAddr,1,TRUE);
		FreePage(WY_ulTextPTEPhy,1,TRUE);
		UnmappedLinear(WY_ulTextPTELine,1,TRUE);
		FreePage(WY_ulStackPhy,256,TRUE);
		FreePage(WY_ulStackPTEPhy,1,TRUE);
		UnmappedLinear(WY_ulStackPTELine,1,TRUE);
	}
	WY_ulKnlStackLine = PhyToLinear(WY_ulKnlStackPhy,16,TRUE);	//映射堆栈到内核空间
	if(WY_ulKnlStackLine == 0)
	{
		WY_pTablePtr->WY_usTableUse = 0;
		FreePage(WY_ulPhyAddr,1,TRUE);
		UnmappedLinear(WY_ulLinearAddr,1,TRUE);
		FreePage(WY_ulTextPTEPhy,1,TRUE);
		UnmappedLinear(WY_ulTextPTELine,1,TRUE);
		FreePage(WY_ulStackPhy,256,TRUE);
		FreePage(WY_ulStackPTEPhy,1,TRUE);
		UnmappedLinear(WY_ulStackPTELine,1,TRUE);
		FreePage(WY_ulKnlStackPhy,16,0);
	}

	//为记录表申请空间，和内核堆栈一样，因为都处于
	//内核空间，因此不需要为其申请页表空间。
	WY_ulMemRecTablePhy = AllocPhyPage(1);
	if(WY_ulMemRecTablePhy == 0)
	{
		WY_pTablePtr->WY_usTableUse = 0;
		FreePage(WY_ulPhyAddr,1,TRUE);
		UnmappedLinear(WY_ulLinearAddr,1,TRUE);
		FreePage(WY_ulTextPTEPhy,1,TRUE);
		UnmappedLinear(WY_ulTextPTELine,1,TRUE);
		FreePage(WY_ulStackPhy,256,TRUE);
		FreePage(WY_ulStackPTEPhy,1,TRUE);
		UnmappedLinear(WY_ulStackPTELine,1,TRUE);
		FreePage(WY_ulKnlStackPhy,16,0);
		UnmappedLinear(WY_ulKnlStackPhy,16,0);
	}
	WY_ulMemRecTableLine = PhyToLinear(WY_ulMemRecTablePhy,1,TRUE);
	if(WY_ulMemRecTableLine == 0)
	{
		WY_pTablePtr->WY_usTableUse = 0;
		FreePage(WY_ulPhyAddr,1,TRUE);
		UnmappedLinear(WY_ulLinearAddr,1,TRUE);
		FreePage(WY_ulTextPTEPhy,1,TRUE);
		UnmappedLinear(WY_ulTextPTELine,1,TRUE);
		FreePage(WY_ulStackPhy,256,TRUE);
		FreePage(WY_ulStackPTEPhy,1,TRUE);
		UnmappedLinear(WY_ulStackPTELine,1,TRUE);
		FreePage(WY_ulKnlStackPhy,16,0);
		UnmappedLinear(WY_ulKnlStackPhy,16,0);
		FreePage(WY_ulMemRecTablePhy,1,TRUE);
	}

	
	//设置内存控制表
	WY_pTablePtr->WY_ulPDTLine = (ulong *)WY_ulLinearAddr;
	WY_pTablePtr->WY_ulPDTPhy = WY_ulPhyAddr;
	//复制内核页目录和页表线性地址到
	//进程页目录，和进程页表线性地址数组
	
	//memcpy(WY_pTablePtr->WY_ulPDTLine,WY_PROCTABLE[0].WY_ulPDTLine,4 * 256);	//还未测试的函数
	//memcpy(WY_pTablePtr->WY_ulPTELine,WY_PROCTABLE[0].WY_ulPTELine,4 * 256);
	for(i = 0;i < 256;i++)
	{
		WY_pTablePtr->WY_ulPDTLine[i] = WY_PROCTABLE[0].WY_ulPDTLine[i];
		WY_pTablePtr->WY_ulPTELine[i] = WY_PROCTABLE[0].WY_ulPTELine[i];
	}
	//映射进程代码空间，进程代码入口地址为0x80000000
	WY_pTablePtr->WY_ulPDTLine[512] = (WY_ulTextPTEPhy & 0xFFFFF000) | 0x7;
	WY_pTablePtr->WY_ulPTELine[512] = WY_ulTextPTELine;
	WY_pPTE = (ulong*)WY_ulTextPTELine;
	for(i = 0;i < WY_ulTextPageNum;i++)
	{
		WY_pPTE[i] = (0x1000000 + i * 0x1000) | 0x7;
	}
	//映射堆栈
	WY_pTablePtr->WY_ulPDTLine[511] = (WY_ulStackPTEPhy & 0xFFFFF000) | 0x7;
	WY_pTablePtr->WY_ulPTELine[511] = WY_ulStackPTELine;
	WY_pPTE = (ulong*)WY_ulStackPTELine;
	for(i = 0;i < 256;i++)
	{
		WY_pPTE[i] = (WY_ulStackPhy + i * 0x1000) | 0x7;
	}

	//设置内存使用记录表，并初始化记录链表
	WY_pTable = (WY_pMemRecord)WY_ulMemRecTableLine;
	WY_pTable[0].WY_ulLinearAddress = WY_ulMemRecTableLine;
	WY_pTable[0].WY_ulRangeSize = 0x1000;
	WY_pTable[0].WY_ulPhysicalPage = WY_ulMemRecTablePhy >> 12;
	WY_pTable[0].WY_ulRecordType = 1;
	WY_pTable[0].WY_ulIdentity = 0;
	WY_pTable[0].WY_Use.WY_TableCharacter.WY_ulNextTable = 0;
	WY_pTable[0].WY_Use.WY_TableCharacter.WY_ulTableUsed = 4;

	//设置四个使用记录，分别是记录表占用空间
	//代码区域，堆栈区域，内核堆栈区域，并组
	//成链表	
	ConstructUseRecord(&WY_pTable[1], WY_ulMemRecTableLine, 0x1000, WY_ulMemRecTablePhy >> 12, 1, WY_ulMemRecTableLine >> 12, 0, 0, TRUE);
	ConstructUseRecord(&WY_pTable[2], WYOS_PROC_ENTRY_POINT, WY_ulTextRange, WY_ulTextPhyAddr >> 12, 2, WY_ulMemRecTableLine >> 12, 0, 0, FALSE);
	ConstructUseRecord(&WY_pTable[3], WYOS_PROC_STACK_ADDR, 0x100000, WY_ulStackPhy>> 12, 3, WY_ulMemRecTableLine >> 12, 0, 0, FALSE);
	ConstructUseRecord(&WY_pTable[4], WY_ulKnlStackLine, 0x10000, WY_ulKnlStackPhy>> 12, 4, WY_ulMemRecTableLine >> 12, 0, 0, TRUE);
	//组成链表，首先对物理地址排序
	//整序完成后，数组是记录表按照
	//物理地址增序的记录序号
	for(i = 0;i < 3;i++)
	{
		for(j = i + 1;j < 4;j++)
		{
			if((WY_pTable[WY_ulRecPosArr[i]].WY_ulPhysicalPage << 12) > (WY_pTable[WY_ulRecPosArr[j]].WY_ulPhysicalPage << 12))
			{
				swap(&WY_ulRecPosArr[i],&WY_ulRecPosArr[j]);
			}
		}
	}
	
	WY_pTable[WY_ulRecPosArr[0]].WY_Use.WY_RecordCharacter.WY_ulFirst = 1;
	WY_pTable[WY_ulRecPosArr[3]].WY_Use.WY_RecordCharacter.WY_ulTableOfNextRecord = 0;
	for(i = 0;i < 3;i++)
	{
		WY_pTable[WY_ulRecPosArr[i]].WY_Use.WY_RecordCharacter.WY_ulTablePosOfNextRecord = WY_ulRecPosArr[i + 1];
	}
	//设置内存控制表中记录链表相关数据
	WY_pTablePtr->WY_pMemFirstRec = &WY_pTable[WY_ulRecPosArr[0]];
	WY_pTablePtr->WY_pMemLastRec = &WY_pTable[WY_ulRecPosArr[4]];
	WY_pTablePtr->WY_FirstRecTalbe = WY_pTable;

	//设置进程状态表
	WY_pProcDesc = allocGlobalDesc(FALSE);

	//设置LDT段描述符
	//设置LDT段描述符属性
	WY_pProcDesc->WY_ulPresent = 1;
	WY_pProcDesc->WY_ulDescType = 0;
	WY_pProcDesc->WY_ulSegTYPE = SYSTEM_LDT;
	WY_pProcDesc->WY_ulDescDPL = SEGMENT_RING0;
	WY_pProcDesc->WY_ulGranularity = 0;
	WY_pProcDesc->WY_ulD = 0;
	WY_pProcDesc->WY_ulSoftUse = 0;
	WY_pProcDesc->WY_ulReserved = 0;
	//设置LDT段基址及段限
	WY_pProcDesc->WY_ulLowSegLimit = 64;
	WY_pProcDesc->WY_ulHighSegLimit = 0;
	WY_pProcDesc->WY_ulLowSegBase = ((ulong)WY_pTablePtr->WY_LDT) & 0xFFFFFF;
	WY_pProcDesc->WY_ulHighSegBase = (((ulong)WY_pTablePtr->WY_LDT) & 0xFF000000) >> 24;
	//计算LDT段选择子
	WY_pTablePtr->WY_ulLDTSel = ((ulong)WY_pProcDesc - WYOS_GDT_BASE);
	//在LDT中设置内核堆栈段，进程代码段和数据段
	memset((char*)WY_pTablePtr->WY_LDT,0,8 * 16);
	//内核堆栈段
	WY_pTablePtr->WY_LDT[0].WY_ulPresent = 1;
	WY_pTablePtr->WY_LDT[0].WY_ulDescType = 1;
	WY_pTablePtr->WY_LDT[0].WY_ulSegTYPE = SEGMENT_READWRITE;
	WY_pTablePtr->WY_LDT[0].WY_ulDescDPL = SEGMENT_RING0;
	WY_pTablePtr->WY_LDT[0].WY_ulGranularity = 1;
	WY_pTablePtr->WY_LDT[0].WY_ulD = 1;
	WY_pTablePtr->WY_LDT[0].WY_ulSoftUse = 0;
	WY_pTablePtr->WY_LDT[0].WY_ulReserved = 0;

	WY_pTablePtr->WY_LDT[0].WY_ulLowSegLimit = 0xFFFF;
	WY_pTablePtr->WY_LDT[0].WY_ulHighSegLimit = 0xF;
	WY_pTablePtr->WY_LDT[0].WY_ulLowSegBase = 0;
	WY_pTablePtr->WY_LDT[0].WY_ulHighSegBase = 0;

	//进程代码段
	WY_pTablePtr->WY_LDT[1].WY_ulPresent = 1;
	WY_pTablePtr->WY_LDT[1].WY_ulDescType = 1;
	WY_pTablePtr->WY_LDT[1].WY_ulSegTYPE = SEGMENT_EXECUTEREAD;
	WY_pTablePtr->WY_LDT[1].WY_ulDescDPL = SEGMENT_RING3;
	WY_pTablePtr->WY_LDT[1].WY_ulGranularity = 1;
	WY_pTablePtr->WY_LDT[1].WY_ulD = 1;
	WY_pTablePtr->WY_LDT[1].WY_ulSoftUse = 0;
	WY_pTablePtr->WY_LDT[1].WY_ulReserved = 0;

	WY_pTablePtr->WY_LDT[1].WY_ulLowSegLimit = 0xFFFF;
	WY_pTablePtr->WY_LDT[1].WY_ulHighSegLimit = 0xF;
	WY_pTablePtr->WY_LDT[1].WY_ulLowSegBase = 0;
	WY_pTablePtr->WY_LDT[1].WY_ulHighSegBase = 0;

	//进程数据段
	WY_pTablePtr->WY_LDT[2].WY_ulPresent = 1;
	WY_pTablePtr->WY_LDT[2].WY_ulDescType = 1;
	WY_pTablePtr->WY_LDT[2].WY_ulSegTYPE = SEGMENT_READWRITE;
	WY_pTablePtr->WY_LDT[2].WY_ulDescDPL = SEGMENT_RING3;
	WY_pTablePtr->WY_LDT[2].WY_ulGranularity = 1;
	WY_pTablePtr->WY_LDT[2].WY_ulD = 1;
	WY_pTablePtr->WY_LDT[2].WY_ulSoftUse = 0;
	WY_pTablePtr->WY_LDT[2].WY_ulReserved = 0;

	WY_pTablePtr->WY_LDT[2].WY_ulLowSegLimit = 0xFFFF;
	WY_pTablePtr->WY_LDT[2].WY_ulHighSegLimit = 0xF;
	WY_pTablePtr->WY_LDT[2].WY_ulLowSegBase = 0;
	WY_pTablePtr->WY_LDT[2].WY_ulHighSegBase = 0;
	
	//设置消息管理部分
	WY_pTablePtr->WY_ulEarlyPos = -1;
	WY_pTablePtr->WY_ulLastPos = -1;
	//设置线程
	WY_pTablePtr->WY_pThreadCtr = (WY_pTHREAD)mallocs(sizeof(WY_THREAD) * MAX_THREAD_NUM + 1);
	memset((char *)WY_pTablePtr->WY_pThreadCtr,0,sizeof(WY_THREAD) * MAX_THREAD_NUM + 1);
	WY_pTablePtr->WY_pThreadCtr[0].WY_ulUseFlag = 1;
	WY_pTablePtr->WY_pThreadCtr[0].WY_usThreadState = WYOS_THREAD_RUN;
	WY_pTablePtr->WY_pThreadCtr[0].WY_usUseTime = THREAD_RUN_TIME;
	//设置主线程TSS段描述符
	WY_pProcDesc++;
	WY_pProcDesc->WY_ulPresent = 1;
	WY_pProcDesc->WY_ulDescType = 0;
	WY_pProcDesc->WY_ulSegTYPE = SYSTEM_BUSY_386TSS;
	WY_pProcDesc->WY_ulDescDPL = SEGMENT_RING0;
	WY_pProcDesc->WY_ulGranularity = 0;
	WY_pProcDesc->WY_ulD = 0;
	WY_pProcDesc->WY_ulSoftUse = 0;
	WY_pProcDesc->WY_ulReserved = 0;
	//设置TSS段基址及段限
	WY_pProcDesc->WY_ulLowSegLimit = 0x69;			//105个字节，长度应该包括I/O MAP结束标志
	WY_pProcDesc->WY_ulHighSegLimit = 0;
	WY_pProcDesc->WY_ulLowSegBase = ((ulong)&WY_pTablePtr->WY_pThreadCtr[0].WY_ThreadTss) & 0xFFFFFF;
	WY_pProcDesc->WY_ulHighSegBase = (((ulong)&WY_pTablePtr->WY_pThreadCtr[0].WY_ThreadTss) & 0xFF000000) >> 24;
	
	WY_pTablePtr->WY_pThreadCtr[0].WY_ThreadTss.WY_ulESP0 = WY_ulKnlStackLine + 0x10000;
	WY_pTablePtr->WY_pThreadCtr[0].WY_ThreadTss.WY_ulSS0 = 0x4;
	WY_pTablePtr->WY_pThreadCtr[0].WY_ThreadTss.WY_ulCR3 = WY_ulPhyAddr;
	WY_pTablePtr->WY_pThreadCtr[0].WY_ThreadTss.WY_ulEIP = WYOS_PROC_ENTRY_POINT;
	WY_pTablePtr->WY_pThreadCtr[0].WY_ThreadTss.WY_ulEFLAGS = 0x202;
	WY_pTablePtr->WY_pThreadCtr[0].WY_ThreadTss.WY_ulESP = WYOS_PROC_STACK_ADDR + 0x100000;
	WY_pTablePtr->WY_pThreadCtr[0].WY_ThreadTss.WY_ulES = 0x17;
	WY_pTablePtr->WY_pThreadCtr[0].WY_ThreadTss.WY_ulCS =0xF;
	WY_pTablePtr->WY_pThreadCtr[0].WY_ThreadTss.WY_ulSS = 0x17;
	WY_pTablePtr->WY_pThreadCtr[0].WY_ThreadTss.WY_ulDS = 0x17;
	WY_pTablePtr->WY_pThreadCtr[0].WY_ThreadTss.WY_ulFS = 0x17;
	WY_pTablePtr->WY_pThreadCtr[0].WY_ThreadTss.WY_ulGS = 0x17;
	WY_pTablePtr->WY_pThreadCtr[0].WY_ThreadTss.WY_ulLDT = WY_pTablePtr->WY_ulLDTSel;
	WY_pTablePtr->WY_pThreadCtr[0].WY_ThreadTss.WY_usBitmapOffset = sizeof(WY_TSS);
	//设置I/O MAP结束标志
	WY_pTablePtr->WY_pThreadCtr[0].WY_ulIOMapEnd = 0xFF;

	WY_pTablePtr->WY_pThreadCtr[0].WY_usTSSSel = (ushort)((ulong)WY_pProcDesc - WYOS_GDT_BASE);;
	WY_pTablePtr->WY_ulCurTID = 0;
	WY_pTablePtr->WY_ulThreadNum = 1;

}

ushort MasterScheudler()
{
	int	i;
	ushort				WY_usCurPID = WY_usCurrentPID;
	WY_pProcTable		WY_pProc;

	if(WY_PROCTABLE[WY_usCurPID].WY_usProcState != WYOS_PROC_STATE_RUN)
	{
		WY_usCurPID = 0;
	}
	
	for(i = WYOS_PROC_PRI_REALTIME;i < WYOS_PROC_PRI_USEUP;i++)
	{
		WY_pProc = WY_PRIQueue[i];
//		printk("This Proc %x\n",WY_pProc);
		//取优先级队列头进程，如果该进程不能剥夺正在运行的进程，
		//那么该队列其它进程也不能
		if(WY_pProc)
		{

//			printk("This Proc %x PID %d Pri %x curr pri %x \n",WY_pProc,WY_pProc->WY_ulPID,WY_pProc->WY_ulPriority,WY_pProc->WY_ulCurPriQueue);

			//如果优先级高于正在运行的进程，那么将运行进程剥夺
			//因为优先级0最高，6最小，所以数值越小优先级越高
			if(WY_pProc->WY_ulCurPriQueue< WY_PROCTABLE[WY_usCurPID].WY_ulCurPriQueue)
			{
				//剥夺
				WY_PROCTABLE[WY_usCurPID].WY_usProcState = WYOS_PROC_STATE_READY;
				WY_pProc->WY_usProcState = WYOS_PROC_STATE_RUN;
				WY_usCurPID = WY_pProc->WY_ulPID;
				break;
			}

		}
	}

	//如果没有找到合适的进程，重新计算所有的进程
	if(WY_usCurPID == 0)
	{
		do
		{
			WY_pProc = WY_PRIQueue[WYOS_PROC_PRI_USEUP];
			if(WY_pProc)
			{
				WY_PRIQueue[WYOS_PROC_PRI_USEUP] = WY_pProc->WY_pNextTable;
				WY_pProc->WY_pNextTable = WY_PRIQueue[WY_pProc->WY_ulPriority];
				WY_PRIQueue[WY_pProc->WY_ulPriority] = WY_pProc;
				WY_pProc->WY_ulCurPriQueue = WY_pProc->WY_ulPriority;
			}
		}while(WY_pProc);
		//重新找一个优先级最高的进程
		//取每个优先级队列头的进程，
		//让优先级较高的进程执行
		for(i = WYOS_PROC_PRI_REALTIME;i < WYOS_PROC_PRI_USEUP;i++)
		{
			WY_pProc = WY_PRIQueue[i];
			if(WY_pProc)
			{
				WY_pProc->WY_usProcState = WYOS_PROC_STATE_RUN;
				WY_usCurPID = (ushort)WY_pProc->WY_ulPID;
				break;
			}
		}
//		printk("Final PID %d\n",WY_usCurPID);
	}
	return	WY_usCurPID;
	
}

ulong TaskScheudler(ushort WY_usPID,ulong *WY_ulUseableTID)
{
///*
	int	i;

	//如果当前线程数只有一个，那么只返回主线程
	if(WY_PROCTABLE[WY_usPID].WY_ulThreadNum == 1)
	{
		*WY_ulUseableTID = 0;
		return (ulong)WY_PROCTABLE[WY_usPID].WY_pThreadCtr[0].WY_usTSSSel;
	}
	
	//寻找一个可用的线程
	i = WY_PROCTABLE[WY_usPID].WY_ulCurTID;
	//当前线程依然可用，则返回该线程
	if((WY_PROCTABLE[WY_usPID].WY_pThreadCtr[i].WY_usThreadState  == WYOS_THREAD_RUN) && WY_PROCTABLE[WY_usPID].WY_pThreadCtr[i].WY_usUseTime)
	{
		WY_PROCTABLE[WY_usPID].WY_pThreadCtr[i].WY_usUseTime --;
		*WY_ulUseableTID = i;
		return (ulong)WY_PROCTABLE[WY_usPID].WY_pThreadCtr[i].WY_usTSSSel;
	}
	else
	{
		if((WY_PROCTABLE[WY_usPID].WY_pThreadCtr[i].WY_usThreadState  == WYOS_THREAD_RUN) 
			&& (WY_PROCTABLE[WY_usPID].WY_pThreadCtr[i].WY_usUseTime == 0))
		{
			WY_PROCTABLE[WY_usPID].WY_pThreadCtr[i].WY_usUseTime  = 3;
			WY_PROCTABLE[WY_usPID].WY_pThreadCtr[i].WY_usThreadState = WYOS_THREAD_RUNABLE;
		}
		i = (i + 1) % MAX_THREAD_NUM; 
	}
	
	while(i != WY_PROCTABLE[WY_usPID].WY_ulCurTID)
	{
		if(WY_PROCTABLE[WY_usPID].WY_pThreadCtr[i].WY_ulUseFlag)
		{
			//如果该线程就绪，就让它执行
			if(WY_PROCTABLE[WY_usPID].WY_pThreadCtr[i].WY_usThreadState == WYOS_THREAD_RUNABLE)
			{
				WY_PROCTABLE[WY_usPID].WY_pThreadCtr[i].WY_usThreadState = WYOS_THREAD_RUN;
				*WY_ulUseableTID =  i;
				return (ulong)WY_PROCTABLE[WY_usPID].WY_pThreadCtr[i].WY_usTSSSel; 
			}
		}

		i = (i + 1) % MAX_THREAD_NUM; 

	}
	
	//如果找到最后还是找到一开始的那一个线程,就让它执行
	if(WY_PROCTABLE[WY_usPID].WY_pThreadCtr[i].WY_usThreadState == WYOS_THREAD_RUNABLE)
	{
		WY_PROCTABLE[WY_usPID].WY_pThreadCtr[i].WY_usThreadState =  WYOS_THREAD_RUN;
		*WY_ulUseableTID =  i;
		return (ulong)WY_PROCTABLE[WY_usPID].WY_pThreadCtr[i].WY_usTSSSel;
	}
	else
	{
		return 0;
	}
//*/
}


//newline

