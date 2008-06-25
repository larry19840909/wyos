/***************************************************************************
			WYOS Memory.c
			内存管理源文件
						编码:WY.lslrt				editor	 :WY.lslrt
						日期:2005/12/2			date	 	 :2005/12/2
						版权:WY 和 WY.lslrt所有   copyright  :WY and WY.lslrt
						URL:http://www.wyos.net http://wylslrt.go.3322.org
***************************************************************************/
/**************************************************************************
			修改2006-6-6
			去掉自旋锁，改互斥为开关中断
			这样效率更高 
***************************************************************************/
//需要修改,需要进行互斥操作
#include "..\include\WYOSType.h"
#include "..\include\syscall.h"
#include "..\include\cpu.h"
#include "..\video\video.h"
#include "..\include\string.h"
#include "..\include\memory.h"
#include "..\include\process.h"

extern WY_ProcTable	WY_PROCTABLE[MAX_PROC_NUM];
extern ushort			WY_usCurrentPID;


unsigned	int			WY_nUseableMemory = 0;		//可用内存总数以字节为单位
unsigned int			WY_nPhyMemPagesNum = 0;		//可用的物理页框数目。
WY_pFreeMemRec		WY_pFreeMemFirstRec = NULL,WY_pFreeMemLastRec = NULL;//空闲内存链表的头和尾
//变量分别为申请页表内存的临时的页表项的线形地址，
//页表内存临时线形地址
static unsigned long	*WY_ulTmpPE = NULL,*WY_ulTmpPELine = NULL;			

void	meminit()
{
	ulong *				WY_pulRangeAddr = (ulong *)0x94000;			//int15中断e802号功能返回块的个数地址
	WY_pE802MM			WY_pMMAddr = (WY_pE802MM)0x94004;			//int15中断e802号功能返回块的起始地址
	WY_pMemRecord		WY_pMemUseRecord = MEMORY_USE_RECORD_START,WY_pTmpPERec = NULL;
	WY_pFreeMemRec		WY_pFreeMemRecord = MEMORY_FREE_RECORD_START;
	int					WY_i,WY_nTmp;					//WY_nTmp在初始化链表的时候表示下一个记录
	ulong				*WY_nTest,*WY_nTest1,*WY_nTest2;


	//设置全局变量
	WY_nUseableMemory = 0;		//可用内存总数以字节为单位
	WY_nPhyMemPagesNum = 0;		//可用的物理页框数目。
	WY_pFreeMemFirstRec = NULL,WY_pFreeMemLastRec = NULL;//空闲内存链表的头和尾
	//变量分别为申请页表内存的临时的页表项的线形地址，
	//页表内存临时线形地址
	WY_ulTmpPE = NULL;
	WY_ulTmpPELine = NULL;	

	printk("Scaning Physical Memory...\n");
	printk("Primary Memory Infomation:\n");
	printk("AddrBaseLow  AddrBaseHigh  AddrLengthLow  AddrLengthHigh  AddressType\n");

	//以下是初始化内存使用链表和空闲链表
	//清空四个表
	for(WY_i = 0x4000;WY_i < 0x8000;WY_i++)
		*(char *)WY_i = 0;
	WY_pMemUseRecord[0].WY_ulLinearAddress = 0x4000;
	WY_pMemUseRecord[0].WY_ulRangeSize = 0x1000;
	WY_pMemUseRecord[0].WY_ulPhysicalPage = 0x4;
	WY_pMemUseRecord[0].WY_ulRecordType = 1;
	WY_pMemUseRecord[0].WY_ulIdentity = 0;
	WY_pMemUseRecord[0].WY_Use.WY_TableCharacter.WY_ulNextTable = 0x5;
	WY_pMemUseRecord[0].WY_Use.WY_TableCharacter.WY_ulTableUsed = 2;

	//记录从0x00000000 --- 0x00007FFF 的内存使用
	//并构造链表
	ConstructUseRecord(&WY_pMemUseRecord[1],0x0,0x8000,0,1,0x4,2,1,TRUE);
	WY_PROCTABLE[0].WY_pMemFirstRec = &WY_pMemUseRecord[1];
	
	//记录从0x00100000 --- 0x001FFFFF 的内核代码所占空间
	ConstructUseRecord(&WY_pMemUseRecord[2],0x100000,0x100000,0x100,2,0,0,0,TRUE);
	WY_PROCTABLE[0].WY_pMemLastRec = &WY_pMemUseRecord[2];
	
	WY_pMemUseRecord += 0x1000;
	
	WY_pMemUseRecord[0].WY_ulLinearAddress = 0x5000;
	WY_pMemUseRecord[0].WY_ulRangeSize = 0x1000;
	WY_pMemUseRecord[0].WY_ulPhysicalPage = 0x5;
	WY_pMemUseRecord[0].WY_ulRecordType = 1;
	WY_pMemUseRecord[0].WY_ulIdentity = 0;
	WY_pMemUseRecord[0].WY_Use.WY_TableCharacter.WY_ulNextTable = 0;
	WY_pMemUseRecord[0].WY_Use.WY_TableCharacter.WY_ulTableUsed = 0;

	WY_pMemUseRecord = MEMORY_USE_RECORD_START;
	
	WY_pFreeMemRecord[0].WY_Free.WY_Table.WY_ulThisTableId = 0x6;
	WY_pFreeMemRecord[0].WY_Free.WY_Table.WY_ulNextTable = 0x7;
	WY_pFreeMemRecord[0].WY_Free.WY_Table.WY_ulRecordUsed = 0;

	WY_pFreeMemRecord += 0x1000;
	
	WY_pFreeMemRecord[0].WY_Free.WY_Table.WY_ulThisTableId = 0x7;
	WY_pFreeMemRecord[0].WY_Free.WY_Table.WY_ulNextTable = 0;
	WY_pFreeMemRecord[0].WY_Free.WY_Table.WY_ulRecordUsed = 0;

	WY_pFreeMemRecord = MEMORY_FREE_RECORD_START;

	//读取int15 E802  结构信息并根据内核已经占用的内存
	//构造内存空闲链表
	for(WY_i = 0;WY_i<*WY_pulRangeAddr;WY_i++)
	{
		
		if(WY_pMMAddr[WY_i].WY_ulAddrRangeType == ADDRESS_RANGE_MEMORY)
		{
			printk("%x    %x     %x     %x     %d  OS can use\n",WY_pMMAddr[WY_i].WY_ulBaseAddrLow,
									  WY_pMMAddr[WY_i].WY_ulBaseAddrHigh,
									  WY_pMMAddr[WY_i].WY_ulLengthLow,
									  WY_pMMAddr[WY_i].WY_ulLengthHigh,
									  WY_pMMAddr[WY_i].WY_ulAddrRangeType);
			if(WY_nUseableMemory < (WY_pMMAddr[WY_i].WY_ulBaseAddrLow + 
									  WY_pMMAddr[WY_i].WY_ulLengthLow))
				WY_nUseableMemory = WY_pMMAddr[WY_i].WY_ulBaseAddrLow + WY_pMMAddr[WY_i].WY_ulLengthLow;
			if(WY_pMMAddr[WY_i].WY_ulBaseAddrLow == 0)
			{
				ConstructFreeRecord(&WY_pFreeMemRecord[1],0x8000,WY_pMMAddr[WY_i].WY_ulLengthLow - 0x0008000,0x6,1,2,1);
				WY_nTmp = 2;
			}
			else if(WY_pMMAddr[WY_i].WY_ulBaseAddrLow == 0x00100000)
			{
				//本来的入参是这样的
				//&WY_pFreeMemRecord[WY_nTmp],0x190000,WY_pMMAddr[WY_i].WY_ulLengthLow - 0x00090000,0x6,WY_nTmp,++WY_nTmp,0
				//第一个参数是取这个记录的指针，倒数第二个参数是下一个记录的位置，
				//但由于C   入参是最后参数先入栈，这样倒数第二个参数的表达式就先执行了
				//导致第一个参数的指针向后跳了一个。这就是为什么现在第一个参数自加了
				ConstructFreeRecord(&WY_pFreeMemRecord[WY_nTmp++],0x200000,WY_pMMAddr[WY_i].WY_ulLengthLow - 0x00100000,0x6,WY_nTmp,WY_nTmp+1,0);
			}
			else
			{
				ConstructFreeRecord(&WY_pFreeMemRecord[WY_nTmp++],WY_pMMAddr[WY_i].WY_ulBaseAddrLow,WY_pMMAddr[WY_i].WY_ulLengthLow,0x6,WY_nTmp,WY_nTmp+1,0);
			}
		}
		else
		{
			printk("%x    %x     %x     %x     %d\n",WY_pMMAddr[WY_i].WY_ulBaseAddrLow,
									  WY_pMMAddr[WY_i].WY_ulBaseAddrHigh,
									  WY_pMMAddr[WY_i].WY_ulLengthLow,
									  WY_pMMAddr[WY_i].WY_ulLengthHigh,
									  WY_pMMAddr[WY_i].WY_ulAddrRangeType);
		}
	}
	WY_pFreeMemRecord[--WY_nTmp].WY_Free.WY_Record.WY_ulTablePosOfNextRecord = 0;
	WY_pFreeMemRecord[WY_nTmp].WY_Free.WY_Record.WY_ulTableOfNextRecord = 0;
	WY_pFreeMemRecord->WY_Free.WY_Table.WY_ulRecordUsed = WY_nTmp;
	WY_pFreeMemFirstRec = &WY_pFreeMemRecord[1];
	WY_pFreeMemLastRec = &WY_pFreeMemRecord[WY_nTmp];
	//输出内存信息
	WY_nPhyMemPagesNum = WY_nUseableMemory >> 12;
	printk("Primary Memory Size : %dMB,Physical Memory Pages Number : %d\n",WY_nUseableMemory/(1024*1024),WY_nPhyMemPagesNum);
	
	//为申请页表的临时地址变量申请页表项
	WY_ulTmpPELine = (ulong*)0xF000;
	WY_ulTmpPE = (ulong *)(0x1000 + ((((ulong)WY_ulTmpPELine) & 0x3FF000) >> 12) * sizeof(ulong));
	*WY_ulTmpPE = 0x1;

	//设置页面异常处理函数
	SetInterrupt((INT_PROC)(ulong)PageException,0x8, SEGMENT_RING3,GATE_386INTERRUPT,0xE);
	//注册系统调用
	SetupSyscall((SysCallProc)SyscallGetMmInfo, 20);
	SetupSyscall((SysCallProc)SyscallMalloc, 21);
	SetupSyscall((SysCallProc)SyscallFree, 22);
	
}

void ConstructUseRecord(WY_pMemRecord WY_pMemUseRec, ulong WY_ulLinearAddr, ulong WY_ulBlockSize, ulong WY_ulPhyPageNum, ulong WY_ulRecNum, ulong WY_ulNextRecTable, ulong WY_ulNextRecNum, ulong WY_ulFirstFlag,BOOL WY_bKnl)
{
	WY_pMemUseRec->WY_ulLinearAddress = WY_ulLinearAddr;
	WY_pMemUseRec->WY_ulRangeSize = WY_ulBlockSize;
	WY_pMemUseRec->WY_ulPhysicalPage = WY_ulPhyPageNum;
	WY_pMemUseRec->WY_ulRecordType = 0;
	WY_pMemUseRec->WY_ulIdentity = WY_ulRecNum;
	WY_pMemUseRec->WY_Use.WY_RecordCharacter.WY_ulTableOfNextRecord = WY_ulNextRecTable;
	WY_pMemUseRec->WY_Use.WY_RecordCharacter.WY_ulTablePosOfNextRecord = WY_ulNextRecNum;
	WY_pMemUseRec->WY_Use.WY_RecordCharacter.WY_ulFlag = 1;
	WY_pMemUseRec->WY_Use.WY_RecordCharacter.WY_ulFirst = WY_ulFirstFlag;
	WY_pMemUseRec->WY_ulKNLFlag = WY_bKnl;

}

void ConstructFreeRecord(WY_pFreeMemRec WY_pFreeRecord, ulong WY_ulPhysicalAddress, ulong WY_ulBlockSize, ulong WY_ulTableOfNextRec, ulong WY_ulThisRecNum, ulong WY_ulNextRecNum,ulong WY_ulIsFirst)
{
	WY_pFreeRecord->WY_Free.WY_Record.WY_ulPhysicalAddress = WY_ulPhysicalAddress;
	WY_pFreeRecord->WY_Free.WY_Record.WY_ulRangeSize = WY_ulBlockSize;
	WY_pFreeRecord->WY_Free.WY_Record.WY_ulTableOfNextRecord = WY_ulTableOfNextRec;
	WY_pFreeRecord->WY_Free.WY_Record.WY_ulThisRecNum = WY_ulThisRecNum;
	WY_pFreeRecord->WY_Free.WY_Record.WY_ulTablePosOfNextRecord = WY_ulNextRecNum;
	WY_pFreeRecord->WY_Free.WY_Record.WY_ulUseFlag = 1;
	WY_pFreeRecord->WY_Free.WY_Record.WY_ulFirstFlag = WY_ulIsFirst;
	
}

//页表所占用的内存不记录在使用链表内，而是
//通过页表线形地址数组存放着。虽然这个数组
//只存放了地址，但是页表大小是固定的，所以
//可以确定使用了多少内存。
//不完善处，如果添加PDT项时,必须对所有任务的
//页目录进行更新
//因为每个页表的第一项指向这个页表,所以每次
//最大可以申请4MB-4kB
ulong PhyToLinear(ulong WY_ulPhysicalAddress,int WY_nPageNum,BOOL WY_bKnl)
{
	WY_pFreeMemRec		WY_pFreeRec = WY_pFreeMemFirstRec;
	int					i,j,cnt = 0,sp =0 ,ep = 0;
	ulong				*WY_ulPTE = NULL,*WY_ulPDT = NULL,WY_ulPhyAddr;
	ushort				WY_usCurPID = GetCurrentPID();
	//如果物理地址是在低4M  之下，那么就直接返回该地址
	//因为内核的第一个页表就映射了这一块区域
	__asm__("pushf");
	wyos_close_int();

	//计算需要从哪个页表开始查找
	//内核申请从1 - 255号页表，用户从256 - 1023号页表
	if(WY_bKnl)
	{
		if(WY_ulPhysicalAddress + (ulong)(WY_nPageNum * 0x1000)< 0x400000)
		{
			__asm__("popf");
			return WY_ulPhysicalAddress;
		}
		sp = 1;
		ep = 256;
	}
	else
	{
		sp = 256;
		ep = 1024;
	}

	
	//寻找可用的虚拟地址空间
	//首先找到一个可用的页表项
	for(i = sp;i < ep;i++)
	{
		WY_ulPTE = (ulong *)WY_PROCTABLE[WY_usCurPID].WY_ulPTELine[i];
//		printk("PID %d  KNL %d PTE Num %d PTE Line %x\n",WY_usCurPID,WY_bKnl,i,WY_ulPTE);

///*			
		if(WY_ulPTE)
		{
			for(j = 0;j < 1024;j++)
			{
				//此页表项未用,那么就看看它下面连续的页表项是否也未用
				if(!WY_ulPTE[j])
				{
					cnt = 1;
					while(cnt < WY_nPageNum)
					{
						if(j + cnt < 1024)
						{
							if(WY_ulPTE[j + cnt])
								break;		//已经使用
							cnt++;
						}
						else
						{
							break;
						}
					}
					//不够,在从已经使用的下一个开始查看
					if(cnt < WY_nPageNum)
					{
						j = j + cnt ;
						continue;
					}
					//虚拟空间足够,映射
					cnt = 0;
					while(cnt < WY_nPageNum)
					{
						//第0位存在位置1,第2位读写位置为可读写1,第3位系统用户位取WY_bKnl的反
						WY_ulPTE[j + cnt] = (ulong)(((WY_ulPhysicalAddress + cnt * 0x1000)& 0xFFFFF000) | (!WY_bKnl << 2) | 0x3);
						cnt++;
					}
					//映射完成,返回虚拟地址
					//页目录的索引为虚拟地址的高10位
					//页表项的索引为虚拟地址的中间10位
					__asm__("popf");
					return (ulong)(((i << 22) & 0xFFC00000) + ((j << 12) & 0x003FF000));
				}
			}
		}
//	/*
		else
		{
			//该页目录项未映射
			//因为虚拟地址空间分配是连续的
			//如果该目录项美有映射,那么为它添加映射。
			//首先在空闲链表中寻找一个可用空间。
				while(WY_pFreeRec)
				{
					if(WY_pFreeRec->WY_Free.WY_Record.WY_ulRangeSize >= 0x1000)
					{
						if(WY_pFreeRec->WY_Free.WY_Record.WY_ulPhysicalAddress == WY_ulPhysicalAddress)
						{
							//找到的合适的内存块与要申请的冲突
							//到下一块内存块里
							WY_pFreeRec = (WY_pFreeMemRec)(ulong)((WY_pFreeRec->WY_Free.WY_Record.WY_ulTableOfNextRecord << 12) + WY_pFreeRec->WY_Free.WY_Record.WY_ulTablePosOfNextRecord * MEM_FREE_RECORD_SIZE);
							continue;
						}
						if(WY_pFreeRec->WY_Free.WY_Record.WY_ulRangeSize > 0x1000)
						{
							WY_pFreeRec->WY_Free.WY_Record.WY_ulRangeSize -= 0x1000;
							WY_ulPhyAddr = WY_pFreeRec->WY_Free.WY_Record.WY_ulPhysicalAddress;
							WY_pFreeRec->WY_Free.WY_Record.WY_ulPhysicalAddress += 0x1000;
						}
						else
						{
							WY_ulPhyAddr = WY_pFreeRec->WY_Free.WY_Record.WY_ulPhysicalAddress >> 12;
							DeleteRec_free(WY_pFreeRec);
						}
						WY_nUseableMemory -= 0x1000;
						break;
					}
					WY_pFreeRec = (WY_pFreeMemRec)(ulong)((WY_pFreeRec->WY_Free.WY_Record.WY_ulTableOfNextRecord << 12) + WY_pFreeRec->WY_Free.WY_Record.WY_ulTablePosOfNextRecord * MEM_FREE_RECORD_SIZE);
				}
				
				if(WY_pFreeRec)//找到一个物理内存
				{
					
					if(WY_bKnl)
					{
						//找到一个物理页面
						//添加到页目录项中去
						for(j = 0;j < MAX_PROC_NUM;j++)
						{
							if(WY_PROCTABLE[j].WY_usTableUse)
							{
								WY_PROCTABLE[j].WY_ulPDTLine[i] = WY_ulPhyAddr | 0x3;
							}
						}
						//修改临时页表项指向该页表
						*WY_ulTmpPE = WY_ulPhyAddr | 0x1;
						//以下内联汇编目的是清除WY_ulTmpPELine这个地址的TLB
						//一开始使用清除所有TLB内容,这样会影响效率,就
						//直接清除一项,如果不清除,在某些情况下,比如老式的
						//x86处理器上,由于更新了WY_ulTmpPE这个页表项中的物理页框
						//就会出现因为TLB没有刷新,而WY_ulTmpPELine
						//则还会被映射到上一次的物理页面上,这样,下面的
						//为新的页表的修改就会出错了,被更换前的物理页面
						//将会被修改
//						__asm__("mov %cr3,%eax\n\t"
//								"mov %eax,%cr3\n\t");
						__asm__("invlpg %0"::"m"(*WY_ulTmpPELine));
						//清空该表
						for(j = 0;j < 1024;j++)
						{
							WY_ulTmpPELine[j] = 0;
						}
						//修改这个页表的第一项，让它指向自己
						*WY_ulTmpPELine = *WY_ulTmpPE;
						//将该页表新的线形地址放入到页表地址数组中
						//因为这个页面在内核空间,因此所有进程表都得修改
						for(j = 0;j < MAX_PROC_NUM;j++)
						{
							if(WY_PROCTABLE[j].WY_usTableUse)
							{
								WY_PROCTABLE[j].WY_ulPTELine[i] = (ulong)((ulong)i << 22);
							}
						}
						 
					}
					else
					{
						//加入页目录项
						WY_PROCTABLE[WY_usCurPID].WY_ulPDTLine[i] = WY_ulPhyAddr | 0x7;
						WY_PROCTABLE[WY_usCurPID].WY_ulPTELine[i] = PhyToLinear(WY_ulPhyAddr, 1, TRUE);
						memset((char*)(ulong*)WY_PROCTABLE[WY_usCurPID].WY_ulPTELine[i],0,4096);
					}
					//返回循环,开始重新查找页表项
					i--;
					continue;
				}
				//空闲空间不够,不能映射
				__asm__("popf");
				return 0;
		}
		//*/
	}
	//虚拟空间已满,不能映射
	__asm__("popf");
	return 0;

}

ulong UnmappedLinear(ulong WY_ulLinearAddress, int WY_nPageNum,BOOL WY_bKnl)
{
	int			WY_i,WY_nPEIndex;
	ulong		*WY_ulPE;
	ushort		WY_usCurPID = GetCurrentPID();

	__asm__("pushf");
	wyos_close_int();
	//找到页表线形地址
	WY_ulPE = (ulong*)(WY_PROCTABLE[WY_usCurPID].WY_ulPTELine[(WY_ulLinearAddress & 0xFFC00000) >> 22]);
//	printk("unmap pid %d PTE %x",WY_usCurPID,WY_ulPE);
	//找到页表中的页表项索引
	WY_nPEIndex = (int)((WY_ulLinearAddress & 0x003FF000) >> 12);
	//当这些虚拟地址大于等于0x400000时将这些页表项置为0;
	//因为在小于0x400000时物理地址和虚拟地址是一一对应的;
	if((WY_ulLinearAddress + WY_nPageNum * 0x1000) >= 0x400000)
	{
		for(WY_i = 0;WY_i < WY_nPageNum;WY_i++)
		{
			WY_ulPE[WY_nPEIndex + WY_i] = 0;
		}
	}
	//使这些虚拟地址在TLB中无效
	for(WY_i = 0;WY_i < WY_nPageNum;WY_i++)
	{
		WY_ulLinearAddress += WY_i * 0x1000;
		__asm__("invlpg %0"::"m"(*((ulong *)WY_ulLinearAddress)));
	}
	__asm__("popf");
}

ulong LineartToPhy(ulong WY_ulLinearAddress)
{
	ulong *		WY_pPTE = NULL;
	ushort		WY_usCurPID = GetCurrentPID();

	WY_pPTE = (ulong*)WY_PROCTABLE[WY_usCurPID].WY_ulPTELine[(WY_ulLinearAddress & 0xFFC00000) >> 22];
	return (((WY_pPTE[(WY_ulLinearAddress & 0x003FF000) >> 12]) & 0xFFFFF000) + (WY_ulLinearAddress & 0xFFF));
	
}

WY_pMemRecord allocrec_use(BOOL WY_bSystem)
{
	WY_pMemRecord		WY_pUseRecTable,WY_pTmpUse;
	WY_pFreeMemRec		WY_pFreeRecord = WY_pFreeMemFirstRec;//,WY_pTmpFree,WY_pTmpFreeTable;
	int					WY_i;
	ulong				WY_ulPhyAddr;

	//在以有的记录表中寻找空闲的记录
	if(WY_bSystem)
	{
		WY_pUseRecTable = WY_PROCTABLE[0].WY_FirstRecTalbe;
	}
	else
	{
		WY_pUseRecTable = WY_PROCTABLE[GetCurrentPID()].WY_FirstRecTalbe;
	}
	__asm__("pushf");
	wyos_close_int();
	while(1)
	{
		if(WY_pUseRecTable->WY_Use.WY_TableCharacter.WY_ulTableUsed<255)
		{
			for(WY_i = 1;WY_i < NUM_USEREC_TABLE;WY_i++)
			{
				if(WY_pUseRecTable[WY_i].WY_Use.WY_RecordCharacter.WY_ulFlag == 0)
				{
					WY_pUseRecTable->WY_Use.WY_TableCharacter.WY_ulTableUsed++;
					__asm__("popf");
					return &WY_pUseRecTable[WY_i];
				}
			}
		}
		if(WY_pUseRecTable->WY_Use.WY_TableCharacter.WY_ulNextTable == 0) break;
		WY_pUseRecTable = (WY_pMemRecord)((ulong)WY_pUseRecTable->WY_Use.WY_TableCharacter.WY_ulNextTable << 12);
	}
	//记录表已满,则重新申请一个记录表
	if(WY_pFreeRecord == NULL)
	{
		//开中断，允许其它任务读取
		__asm__("popf");
		return NULL;
	}
	//在空闲的物理内存中找到一个记录表大小的空间
	while(WY_pFreeRecord->WY_Free.WY_Record.WY_ulTablePosOfNextRecord != 0)
	{
		if(WY_pFreeRecord->WY_Free.WY_Record.WY_ulRangeSize >= 0x1000)
		{
			//找到，申请这一块，并修改这些记录
			WY_ulPhyAddr = WY_pFreeRecord->WY_Free.WY_Record.WY_ulPhysicalAddress;
			WY_pTmpUse = (WY_pMemRecord)PhyToLinear(WY_ulPhyAddr,1,TRUE);
			if(WY_pFreeRecord->WY_Free.WY_Record.WY_ulRangeSize > 0x1000)
			{
				WY_pFreeRecord->WY_Free.WY_Record.WY_ulPhysicalAddress += 0x1000;
				WY_pFreeRecord->WY_Free.WY_Record.WY_ulRangeSize -= 0x1000;
			}
			else
			{
				//如果该记录的空闲内存正好为一个记录表，
				//则删除这个记录
				DeleteRec_free(WY_pFreeRecord);
			}
			//构造该记录表
			//并构造第一个记录，记录这个表的内存
			//返回第二个记录，以供程序使用
			WY_pTmpUse->WY_ulLinearAddress = (ulong)WY_pTmpUse;
			WY_pTmpUse->WY_ulRangeSize = 0x1000;
			WY_pTmpUse->WY_ulPhysicalPage = WY_ulPhyAddr >> 12;
			WY_pTmpUse->WY_ulRecordType = 1;
			WY_pTmpUse->WY_ulIdentity = 0;
			WY_pTmpUse->WY_Use.WY_TableCharacter.WY_ulNextTable = 0;
			WY_pTmpUse->WY_Use.WY_TableCharacter.WY_ulTableUsed = 2;
			WY_pUseRecTable->WY_Use.WY_TableCharacter.WY_ulNextTable = (ulong)(WY_pTmpUse )>> 12;
			ConstructUseRecord(&WY_pTmpUse[1],(ulong)WY_pTmpUse,0x1000,WY_ulPhyAddr>>12,1,0,0,0,TRUE);
			InsertRec_use(&WY_pTmpUse[1],WY_bSystem);

			WY_nUseableMemory -= 0x1000;
			__asm__("popf");
			return &WY_pTmpUse[2];
		}
		WY_pFreeRecord = (WY_pFreeMemRec)(ulong)((WY_pFreeRecord->WY_Free.WY_Record.WY_ulTableOfNextRecord << 12) + WY_pFreeRecord->WY_Free.WY_Record.WY_ulTablePosOfNextRecord * MEM_FREE_RECORD_SIZE);
	}
}

WY_pFreeMemRec allocrec_free()
{
	WY_pFreeMemRec		WY_pFreeTable = MEMORY_FREE_RECORD_START,WY_pNewTable;
	WY_pFreeMemRec		WY_pFreeRec = WY_pFreeMemFirstRec;
	WY_pMemRecord		WY_pUseRec;
	int					WY_i;
	ulong				WY_ulPhyAddr;

	//在已有的表中去申请
	__asm__("pushf");
	wyos_close_int();
	while(1)//WY_pFreeTable->WY_Table.WY_ulNextTable != 0
	{
		if(WY_pFreeTable->WY_Free.WY_Table.WY_ulRecordUsed < 255)
		{
			for(WY_i = 1;WY_i < NUM_FREEREC_TABLE;WY_i++)
			{
				if(WY_pFreeTable[WY_i].WY_Free.WY_Record.WY_ulUseFlag == 0)
				{
					WY_pFreeTable->WY_Free.WY_Table.WY_ulRecordUsed++;

					__asm__("popf");
					return &WY_pFreeTable[WY_i];
				}
			}
		}
		if(WY_pFreeTable->WY_Free.WY_Table.WY_ulNextTable == 0) break;
		WY_pFreeTable = (WY_pFreeMemRec)(WY_pFreeTable->WY_Free.WY_Table.WY_ulNextTable << 12);
	}
	//所有的表已经分配完，则在从空闲的物理内存中分配一块
	if(WY_pFreeRec == NULL) 
	{
		__asm__("popf");
		return NULL;	//因为没有空闲记录了，那么内存肯定用完了
	}
	while(WY_pFreeRec)
	{
		if(WY_pFreeRec->WY_Free.WY_Record.WY_ulRangeSize >= 0x1000)
		{
			WY_ulPhyAddr = WY_pFreeRec->WY_Free.WY_Record.WY_ulPhysicalAddress;
			WY_pNewTable = (WY_pFreeMemRec)PhyToLinear(WY_ulPhyAddr,1,TRUE);
			if(WY_pFreeRec->WY_Free.WY_Record.WY_ulRangeSize > 0x1000)
			{
				WY_pFreeRec->WY_Free.WY_Record.WY_ulRangeSize -= 0x1000;
				WY_pFreeRec->WY_Free.WY_Record.WY_ulPhysicalAddress += 0x1000;
			}
			else 
			{
				DeleteRec_free(WY_pFreeRec);	
			}
			WY_pNewTable->WY_Free.WY_Table.WY_ulThisTableId = (ulong)WY_pNewTable >> 12;
			WY_pNewTable->WY_Free.WY_Table.WY_ulRecordUsed = 1;
			WY_pNewTable->WY_Free.WY_Table.WY_ulNextTable = 0;
			WY_pFreeTable->WY_Free.WY_Table.WY_ulNextTable = WY_pNewTable->WY_Free.WY_Table.WY_ulThisTableId;

			WY_pUseRec = allocrec_use(TRUE);
			ConstructUseRecord(WY_pUseRec, (ulong)WY_pNewTable,0x1000,WY_ulPhyAddr >> 12, ((ulong)WY_pUseRec & 0xFFF) >> 4, 0,0,0,TRUE);
			InsertRec_use(WY_pUseRec,TRUE);
			WY_nUseableMemory -= 0x1000;
			__asm__("popf");
			return &WY_pNewTable[1];
		}
		WY_pFreeRec = (WY_pFreeMemRec)(ulong)((WY_pFreeRec->WY_Free.WY_Record.WY_ulTableOfNextRecord << 12) + WY_pFreeRec->WY_Free.WY_Record.WY_ulTablePosOfNextRecord * MEM_FREE_RECORD_SIZE);
	}
	__asm__("popf");
	return NULL;
}

void InsertRec_use(WY_pMemRecord WY_pMemUseRec,BOOL WY_bSystem)
{
	ushort				WY_usCurPID;
	WY_pMemRecord		WY_pMemRec,WY_pTmpRec;

	//如果记录链表为空，那么初始化它
	if(WY_bSystem)
	{
		WY_usCurPID = 0;
	}
	else
	{
		WY_usCurPID = GetCurrentPID();
	}
	WY_pMemRec = WY_PROCTABLE[WY_usCurPID].WY_pMemFirstRec;
	WY_pTmpRec = WY_PROCTABLE[WY_usCurPID].WY_pMemFirstRec;

	__asm__("pushf");
	wyos_close_int();
	if(WY_PROCTABLE[WY_usCurPID].WY_pMemFirstRec == NULL)
	{
		WY_PROCTABLE[WY_usCurPID].WY_pMemFirstRec = WY_pMemUseRec;
		WY_PROCTABLE[WY_usCurPID].WY_pMemLastRec = WY_pMemUseRec;
		WY_pMemUseRec->WY_Use.WY_RecordCharacter.WY_ulFirst = 1;
		WY_pMemUseRec->WY_Use.WY_RecordCharacter.WY_ulTableOfNextRecord = 0;
		WY_pMemUseRec->WY_Use.WY_RecordCharacter.WY_ulTablePosOfNextRecord = 0;
		__asm__("popf");
		return;
	}
	//按照物理地址顺序优先，页内偏移次优先的方式
	//插入该记录
	while(WY_pMemRec)
	{
		if(WY_pMemUseRec->WY_ulPhysicalPage <= WY_pMemRec->WY_ulPhysicalPage)
		{
			if(WY_pMemUseRec->WY_ulPhysicalPage == WY_pMemRec->WY_ulPhysicalPage)
			{
				if((WY_pMemUseRec->WY_ulLinearAddress & 0xFFF) > (WY_pMemRec->WY_ulLinearAddress & 0xFFF) )
				{
					WY_pTmpRec = WY_pMemRec;
					WY_pMemRec = (WY_pMemRecord)(ulong)((WY_pMemRec->WY_Use.WY_RecordCharacter.WY_ulTableOfNextRecord << 12) + WY_pMemRec->WY_Use.WY_RecordCharacter.WY_ulTablePosOfNextRecord * MEM_USE_RECORD_SIZE);
					continue;
				}
			}

			if(WY_pMemUseRec == WY_PROCTABLE[WY_usCurPID].WY_pMemFirstRec)
			{
				WY_pMemUseRec->WY_Use.WY_RecordCharacter.WY_ulTableOfNextRecord = (ulong)WY_PROCTABLE[WY_usCurPID].WY_pMemFirstRec >> 12;
				WY_pMemUseRec->WY_Use.WY_RecordCharacter.WY_ulTableOfNextRecord = WY_PROCTABLE[WY_usCurPID].WY_pMemFirstRec->WY_ulIdentity;
				WY_PROCTABLE[WY_usCurPID].WY_pMemFirstRec->WY_Use.WY_RecordCharacter.WY_ulFirst = 0;
				WY_pMemUseRec->WY_Use.WY_RecordCharacter.WY_ulFirst = 1;
				WY_PROCTABLE[WY_usCurPID].WY_pMemFirstRec = WY_pMemUseRec;
				__asm__("popf");
				return;
			}
			else
			{
				WY_pMemUseRec->WY_Use.WY_RecordCharacter.WY_ulTableOfNextRecord = (ulong)WY_pMemRec >> 12;
				WY_pMemUseRec->WY_Use.WY_RecordCharacter.WY_ulTablePosOfNextRecord = WY_pMemRec->WY_ulIdentity;
				WY_pTmpRec->WY_Use.WY_RecordCharacter.WY_ulTableOfNextRecord = (ulong)WY_pMemUseRec >> 12;
				WY_pTmpRec->WY_Use.WY_RecordCharacter.WY_ulTablePosOfNextRecord = WY_pMemUseRec->WY_ulIdentity;
				__asm__("popf");
				return;
			}
		}
		WY_pTmpRec = WY_pMemRec;
		WY_pMemRec = (WY_pMemRecord)(ulong)((WY_pMemRec->WY_Use.WY_RecordCharacter.WY_ulTableOfNextRecord << 12) + WY_pMemRec->WY_Use.WY_RecordCharacter.WY_ulTablePosOfNextRecord * MEM_USE_RECORD_SIZE);
	}
	//该物理页框号大于上面所有的，那么将它放在链表尾
	WY_PROCTABLE[WY_usCurPID].WY_pMemLastRec->WY_Use.WY_RecordCharacter.WY_ulTableOfNextRecord = (ulong)WY_pMemUseRec >> 12;
	WY_PROCTABLE[WY_usCurPID].WY_pMemLastRec->WY_Use.WY_RecordCharacter.WY_ulTablePosOfNextRecord = WY_pMemUseRec->WY_ulIdentity;
	WY_PROCTABLE[WY_usCurPID].WY_pMemLastRec = WY_pMemUseRec;
	WY_PROCTABLE[WY_usCurPID].WY_pMemLastRec->WY_Use.WY_RecordCharacter.WY_ulTableOfNextRecord = 0;
	WY_PROCTABLE[WY_usCurPID].WY_pMemLastRec->WY_Use.WY_RecordCharacter.WY_ulTablePosOfNextRecord = 0;
	__asm__("popf");
}

void InsertRec_free(WY_pFreeMemRec WY_pFreeMemRecord)
{
	WY_pFreeMemRec			WY_pFreeRec = WY_pFreeMemFirstRec,WY_pPreFreeRec = WY_pFreeMemFirstRec;

	__asm__("pushf");
	wyos_close_int();
	if(WY_pFreeRec == NULL)
	{
		WY_pFreeMemFirstRec = WY_pFreeMemRecord;
		WY_pFreeMemLastRec = WY_pFreeMemRecord;
		WY_pFreeMemRecord->WY_Free.WY_Record.WY_ulTableOfNextRecord = 0;
		WY_pFreeMemRecord->WY_Free.WY_Record.WY_ulTablePosOfNextRecord = 0;
		WY_pFreeMemRecord->WY_Free.WY_Record.WY_ulFirstFlag = 1;
		__asm__("popf");
		return;
	}

	while(WY_pFreeRec)
	{
		if(WY_pFreeMemRecord->WY_Free.WY_Record.WY_ulPhysicalAddress < WY_pFreeRec->WY_Free.WY_Record.WY_ulPhysicalAddress)
		{
			if(WY_pFreeRec == WY_pFreeMemFirstRec)
			{
				WY_pFreeMemRecord->WY_Free.WY_Record.WY_ulTableOfNextRecord = (ulong)WY_pFreeRec >> 12;
				WY_pFreeMemRecord->WY_Free.WY_Record.WY_ulTablePosOfNextRecord = WY_pFreeRec->WY_Free.WY_Record.WY_ulThisRecNum;
				WY_pFreeMemRecord->WY_Free.WY_Record.WY_ulFirstFlag = 1;
				WY_pFreeRec->WY_Free.WY_Record.WY_ulFirstFlag = 0;
				WY_pFreeMemFirstRec = WY_pFreeMemRecord;
				__asm__("popf");
				return;
			}
			else
			{
				WY_pFreeMemRecord->WY_Free.WY_Record.WY_ulTableOfNextRecord = WY_pPreFreeRec->WY_Free.WY_Record.WY_ulTableOfNextRecord;
				WY_pFreeMemRecord->WY_Free.WY_Record.WY_ulTablePosOfNextRecord = WY_pPreFreeRec->WY_Free.WY_Record.WY_ulTablePosOfNextRecord;
				WY_pPreFreeRec->WY_Free.WY_Record.WY_ulTableOfNextRecord = (ulong)WY_pFreeMemRecord >> 12;
				WY_pPreFreeRec->WY_Free.WY_Record.WY_ulTablePosOfNextRecord = WY_pFreeMemRecord->WY_Free.WY_Record.WY_ulThisRecNum;
				__asm__("popf");
				return;
			}
		}
		WY_pPreFreeRec = WY_pFreeRec;
		WY_pFreeRec = (WY_pFreeMemRec)((WY_pFreeRec->WY_Free.WY_Record.WY_ulTableOfNextRecord << 12) + WY_pFreeRec->WY_Free.WY_Record.WY_ulTablePosOfNextRecord * MEM_FREE_RECORD_SIZE);
	}

	WY_pFreeMemLastRec->WY_Free.WY_Record.WY_ulTableOfNextRecord = (ulong)WY_pFreeMemRecord;
	WY_pFreeMemLastRec->WY_Free.WY_Record.WY_ulTablePosOfNextRecord = WY_pFreeMemRecord->WY_Free.WY_Record.WY_ulThisRecNum;
	WY_pFreeMemRecord->WY_Free.WY_Record.WY_ulTableOfNextRecord = 0;
	WY_pFreeMemRecord->WY_Free.WY_Record.WY_ulTablePosOfNextRecord = 0;
	WY_pFreeMemLastRec = WY_pFreeMemRecord;
	__asm__("popf");
}

void DeleteRec_use(WY_pMemRecord WY_pMemUseRec,BOOL WY_bSystem)
{
	ushort				WY_usCurPID;
	WY_pMemRecord		WY_pMemUseRecord;

	if(WY_bSystem)
	{
		WY_usCurPID = 0;
	}
	else
	{
		WY_usCurPID = GetCurrentPID();
	}
	WY_pMemUseRecord = WY_PROCTABLE[WY_usCurPID].WY_pMemFirstRec;

	__asm__("pushf");
	wyos_close_int();
	if(WY_pMemUseRec == WY_PROCTABLE[WY_usCurPID].WY_pMemFirstRec)
	{
		WY_PROCTABLE[WY_usCurPID].WY_pMemFirstRec = (WY_pMemRecord)((WY_pMemUseRec->WY_Use.WY_RecordCharacter.WY_ulTableOfNextRecord << 12) + WY_pMemUseRec->WY_Use.WY_RecordCharacter.WY_ulTablePosOfNextRecord * MEM_USE_RECORD_SIZE);
		WY_PROCTABLE[WY_usCurPID].WY_pMemFirstRec->WY_Use.WY_RecordCharacter.WY_ulFirst = 1;
		WY_pMemUseRec->WY_Use.WY_RecordCharacter.WY_ulFlag = 0;
	}
	else if(WY_pMemUseRec == WY_PROCTABLE[WY_usCurPID].WY_pMemLastRec)
	{
		while(WY_pMemUseRecord->WY_Use.WY_RecordCharacter.WY_ulTableOfNextRecord != (ulong)WY_pMemUseRec >> 12 ||
			   WY_pMemUseRecord->WY_Use.WY_RecordCharacter.WY_ulTablePosOfNextRecord != WY_pMemUseRec->WY_ulIdentity)
			   WY_pMemUseRecord = (WY_pMemRecord)((WY_pMemUseRecord->WY_Use.WY_RecordCharacter.WY_ulTableOfNextRecord << 12) + WY_pMemUseRecord->WY_Use.WY_RecordCharacter.WY_ulTablePosOfNextRecord * MEM_USE_RECORD_SIZE);
		WY_PROCTABLE[WY_usCurPID].WY_pMemLastRec = WY_pMemUseRecord;
		WY_pMemUseRecord->WY_Use.WY_RecordCharacter.WY_ulTableOfNextRecord = 0;
		WY_pMemUseRecord->WY_Use.WY_RecordCharacter.WY_ulTablePosOfNextRecord = 0;
		WY_pMemUseRec->WY_Use.WY_RecordCharacter.WY_ulFlag = 0;
	}
	else
	{
		while(WY_pMemUseRecord->WY_Use.WY_RecordCharacter.WY_ulTableOfNextRecord != (ulong)WY_pMemUseRec >> 12 ||
			   WY_pMemUseRecord->WY_Use.WY_RecordCharacter.WY_ulTablePosOfNextRecord != WY_pMemUseRec->WY_ulIdentity)
			   WY_pMemUseRecord = (WY_pMemRecord)((WY_pMemUseRecord->WY_Use.WY_RecordCharacter.WY_ulTableOfNextRecord << 12) + WY_pMemUseRecord->WY_Use.WY_RecordCharacter.WY_ulTablePosOfNextRecord * MEM_USE_RECORD_SIZE);
		WY_pMemUseRecord->WY_Use.WY_RecordCharacter.WY_ulTableOfNextRecord = WY_pMemUseRec->WY_Use.WY_RecordCharacter.WY_ulTableOfNextRecord;
		WY_pMemUseRecord->WY_Use.WY_RecordCharacter.WY_ulTablePosOfNextRecord = WY_pMemUseRec->WY_Use.WY_RecordCharacter.WY_ulTablePosOfNextRecord;
		WY_pMemUseRec->WY_Use.WY_RecordCharacter.WY_ulFlag = 0;
	}
	WY_pMemUseRec = (WY_pMemRecord)(((ulong)WY_pMemUseRec >> 12) << 12);
	WY_pMemUseRec->WY_Use.WY_TableCharacter.WY_ulTableUsed --;
	__asm__("popf");
}

void DeleteRec_free(WY_pFreeMemRec WY_pFreeRec)
{
	WY_pFreeMemRec		WY_pFreePreRec = WY_pFreeMemFirstRec;


	__asm__("pushf");
	wyos_close_int();
	if(WY_pFreeRec == WY_pFreeMemFirstRec)
	{
		WY_pFreeMemFirstRec = (WY_pFreeMemRec)(ulong)((WY_pFreeRec->WY_Free.WY_Record.WY_ulTableOfNextRecord << 12) + WY_pFreeRec->WY_Free.WY_Record.WY_ulTablePosOfNextRecord * MEM_FREE_RECORD_SIZE);
		WY_pFreeMemFirstRec->WY_Free.WY_Record.WY_ulFirstFlag = 1;
		WY_pFreeRec->WY_Free.WY_Record.WY_ulUseFlag = 0;
	}
	else if(WY_pFreeRec == WY_pFreeMemLastRec)
	{
		while(WY_pFreePreRec->WY_Free.WY_Record.WY_ulTableOfNextRecord != ((ulong)WY_pFreeRec >> 12) || 
			   WY_pFreePreRec->WY_Free.WY_Record.WY_ulTablePosOfNextRecord != (ulong)(WY_pFreeRec->WY_Free.WY_Record.WY_ulThisRecNum))
			   WY_pFreePreRec = (WY_pFreeMemRec)(ulong)((WY_pFreePreRec->WY_Free.WY_Record.WY_ulTableOfNextRecord << 12) + WY_pFreePreRec->WY_Free.WY_Record.WY_ulTablePosOfNextRecord * MEM_FREE_RECORD_SIZE);
		
		WY_pFreeMemLastRec = WY_pFreePreRec;
		WY_pFreeMemLastRec->WY_Free.WY_Record.WY_ulTableOfNextRecord = 0;
		WY_pFreeMemLastRec->WY_Free.WY_Record.WY_ulTablePosOfNextRecord = 0;
		WY_pFreeRec->WY_Free.WY_Record.WY_ulUseFlag = 0;
	}
	else
	{
		while(WY_pFreePreRec->WY_Free.WY_Record.WY_ulTableOfNextRecord != ((ulong)WY_pFreeRec >> 12) || 
			   WY_pFreePreRec->WY_Free.WY_Record.WY_ulTablePosOfNextRecord != (ulong)(WY_pFreeRec->WY_Free.WY_Record.WY_ulThisRecNum))
			   WY_pFreePreRec = (WY_pFreeMemRec)(ulong)((WY_pFreePreRec->WY_Free.WY_Record.WY_ulTableOfNextRecord << 12) + WY_pFreePreRec->WY_Free.WY_Record.WY_ulTablePosOfNextRecord * MEM_FREE_RECORD_SIZE);

		WY_pFreePreRec->WY_Free.WY_Record.WY_ulTableOfNextRecord = WY_pFreeRec->WY_Free.WY_Record.WY_ulTableOfNextRecord;
		WY_pFreePreRec->WY_Free.WY_Record.WY_ulTablePosOfNextRecord = WY_pFreeRec->WY_Free.WY_Record.WY_ulTablePosOfNextRecord;
		WY_pFreeRec->WY_Free.WY_Record.WY_ulUseFlag = 0;
	}
	WY_pFreeRec = (WY_pFreeMemRec)(((ulong)WY_pFreeRec>>12)<<12);
	WY_pFreeRec->WY_Free.WY_Table.WY_ulRecordUsed--;
	__asm__("popf");
}

WY_pMemRecord AllocPage(int WY_nPageNum,BOOL WY_bKnl,BOOL WY_bSystem)
{
	WY_pFreeMemRec		WY_pFreeRec = WY_pFreeMemFirstRec;
	WY_pMemRecord		WY_pMemUse;
	ulong				WY_ulRangeSize = WY_nPageNum * 0x1000,WY_ulPhyPageNum,WY_ulLinearAddr;
	ushort				WY_usCurPID;

	if(WY_bSystem)
	{
		WY_usCurPID = 0;
	}
	else
	{
		WY_usCurPID = GetCurrentPID();
	}
//	printk("CurPID %d\n",GetCurrentPID());
//	printk("page num %x KNLFLAG %x\n",WY_nPageNum,WY_bKnl);
	__asm__("pushf");
	wyos_close_int();
	while(WY_pFreeRec)
	{
		if(WY_pFreeRec->WY_Free.WY_Record.WY_ulRangeSize >= WY_ulRangeSize)
		{
			if(WY_pFreeRec->WY_Free.WY_Record.WY_ulRangeSize > WY_ulRangeSize)
			{
				WY_ulLinearAddr = PhyToLinear(WY_pFreeRec->WY_Free.WY_Record.WY_ulPhysicalAddress,WY_nPageNum,WY_bKnl);
//				printk("RS %x\n",WY_pFreeRec->WY_Free.WY_Record.WY_ulRangeSize);
//				printk("Linear %x\n",WY_ulLinearAddr);
				if(!WY_ulLinearAddr)
				{
					__asm__("popf");
					return NULL;
				}
				
				WY_pFreeRec->WY_Free.WY_Record.WY_ulRangeSize -= WY_ulRangeSize;
				WY_ulPhyPageNum = WY_pFreeRec->WY_Free.WY_Record.WY_ulPhysicalAddress >> 12;
				WY_pFreeRec->WY_Free.WY_Record.WY_ulPhysicalAddress += WY_ulRangeSize;

				WY_pMemUse = allocrec_use(WY_bSystem);
				ConstructUseRecord(WY_pMemUse,WY_ulLinearAddr,WY_ulRangeSize, WY_ulPhyPageNum,((ulong)WY_pMemUse & 0xFFF)>>4, 0, 0, 0,WY_bKnl);
				InsertRec_use(WY_pMemUse,WY_bSystem);
				__asm__("popf");
				return WY_pMemUse;
			}
			else
			{
				WY_ulLinearAddr = PhyToLinear(WY_pFreeRec->WY_Free.WY_Record.WY_ulPhysicalAddress,WY_nPageNum,WY_bKnl);
//				printk("Linear %x\n",WY_ulLinearAddr);
				if(!WY_ulLinearAddr)
				{
					__asm__("popf");
					return NULL;
				}
				
				WY_ulPhyPageNum = WY_pFreeRec->WY_Free.WY_Record.WY_ulPhysicalAddress >> 12;

				DeleteRec_free(WY_pFreeRec);
				
				WY_pMemUse = allocrec_use(WY_bSystem);
				wyos_close_int();
				ConstructUseRecord(WY_pMemUse,WY_ulLinearAddr,WY_ulRangeSize, WY_ulPhyPageNum,((ulong)WY_pMemUse & 0xFFF)>>4, 0, 0, 0,WY_bKnl);
				InsertRec_use(WY_pMemUse,WY_bSystem);
				__asm__("popf");
				return WY_pMemUse;
			}
		}
		WY_pFreeRec = (WY_pFreeMemRec)(ulong)((WY_pFreeRec->WY_Free.WY_Record.WY_ulTableOfNextRecord << 12) + WY_pFreeRec->WY_Free.WY_Record.WY_ulTablePosOfNextRecord * MEM_FREE_RECORD_SIZE);
	}
	__asm__("popf");
	return NULL;
}

ulong AllocPhyPage(int WY_nPageNum)
{
	WY_pFreeMemRec WY_pFreeRec = WY_pFreeMemFirstRec;
	ulong			WY_ulPhyAddr = 0;

	__asm__("pushf");
	wyos_close_int();
	while(WY_pFreeRec)
	{
		if(WY_pFreeRec->WY_Free.WY_Record.WY_ulRangeSize >= 0x1000 * WY_nPageNum)
		{
			WY_ulPhyAddr = WY_pFreeRec->WY_Free.WY_Record.WY_ulPhysicalAddress;
			
			if(WY_pFreeRec->WY_Free.WY_Record.WY_ulRangeSize > 0x1000 * WY_nPageNum)
			{
				WY_pFreeRec->WY_Free.WY_Record.WY_ulRangeSize -= 0x1000 * WY_nPageNum;
				WY_ulPhyAddr = WY_pFreeRec->WY_Free.WY_Record.WY_ulPhysicalAddress;
				WY_pFreeRec->WY_Free.WY_Record.WY_ulPhysicalAddress += 0x1000 * WY_nPageNum;
			}
			else
			{
				WY_ulPhyAddr = WY_pFreeRec->WY_Free.WY_Record.WY_ulPhysicalAddress >> 12;
				DeleteRec_free(WY_pFreeRec);
			}
			WY_nUseableMemory -= 0x1000 * WY_nPageNum;
			__asm__("popf");
			return WY_ulPhyAddr;
		}
		WY_pFreeRec = (WY_pFreeMemRec)(ulong)((WY_pFreeRec->WY_Free.WY_Record.WY_ulTableOfNextRecord << 12) + WY_pFreeRec->WY_Free.WY_Record.WY_ulTablePosOfNextRecord * MEM_FREE_RECORD_SIZE);
	}
	__asm__("popf");
	return WY_ulPhyAddr;
}

void FreePage(ulong WY_ulPhysicalAddress,int WY_nPageNum,BOOL WY_bKnl)
{
	WY_pFreeMemRec		WY_pFreeRec;
	WY_pFreeRec = allocrec_free();
	ConstructFreeRecord(WY_pFreeRec, WY_ulPhysicalAddress,WY_nPageNum * 0x1000,0,((ulong)WY_pFreeRec & 0xFFF) >> 4, 0,0);
	InsertRec_free(WY_pFreeRec);
	ClearupFreeChain();
}


void ClearupFreeChain()
{
	WY_pFreeMemRec		WY_pFreeRec = WY_pFreeMemFirstRec,WY_pNextRec = (WY_pFreeMemRec)((WY_pFreeMemFirstRec->WY_Free.WY_Record.WY_ulTableOfNextRecord << 12)  + WY_pFreeMemFirstRec->WY_Free.WY_Record.WY_ulTablePosOfNextRecord * MEM_FREE_RECORD_SIZE);


	__asm__("pushf");
	wyos_close_int();
	while(WY_pNextRec)
	{
		if(WY_pFreeRec->WY_Free.WY_Record.WY_ulPhysicalAddress + WY_pFreeRec->WY_Free.WY_Record.WY_ulRangeSize 
	           == WY_pNextRec->WY_Free.WY_Record.WY_ulPhysicalAddress)
		{
			WY_pFreeRec->WY_Free.WY_Record.WY_ulRangeSize += WY_pNextRec->WY_Free.WY_Record.WY_ulRangeSize;
			
			
			DeleteRec_free(WY_pNextRec);

			WY_pNextRec = (WY_pFreeMemRec)((WY_pFreeRec->WY_Free.WY_Record.WY_ulTableOfNextRecord << 12)  + WY_pFreeRec->WY_Free.WY_Record.WY_ulTablePosOfNextRecord * MEM_FREE_RECORD_SIZE);
		}
		else
		{
			WY_pFreeRec = WY_pNextRec;
			WY_pNextRec = (WY_pFreeMemRec)((WY_pNextRec->WY_Free.WY_Record.WY_ulTableOfNextRecord << 12) + WY_pNextRec->WY_Free.WY_Record.WY_ulTablePosOfNextRecord * MEM_FREE_RECORD_SIZE);
		}
	}

	__asm__("popf");
}

void * mallocmem(int WY_nSize,BOOL WY_bKnl,BOOL WY_bSystem)
{
	int					WY_nMemSize,WY_nPageFreeMem;
	WY_pMemRecord		WY_pPreUse,WY_pUseRec;
	WY_pMemRecord		WY_pMemUse;

//	printk("CurPID %d\n",GetCurrentPID());
	if(WY_bSystem)
	{
		WY_pPreUse = WY_PROCTABLE[0].WY_pMemFirstRec;
		WY_pUseRec = (WY_pMemRecord)((WY_PROCTABLE[0].WY_pMemFirstRec->WY_Use.WY_RecordCharacter.WY_ulTableOfNextRecord << 12) 
									 + WY_PROCTABLE[0].WY_pMemFirstRec->WY_Use.WY_RecordCharacter.WY_ulTablePosOfNextRecord 
									 * MEM_USE_RECORD_SIZE);
	}
	else
	{
		WY_pPreUse = WY_PROCTABLE[GetCurrentPID()].WY_pMemFirstRec;
		WY_pUseRec = (WY_pMemRecord)((WY_PROCTABLE[GetCurrentPID()].WY_pMemFirstRec->WY_Use.WY_RecordCharacter.WY_ulTableOfNextRecord << 12)
									 + WY_PROCTABLE[GetCurrentPID()].WY_pMemFirstRec->WY_Use.WY_RecordCharacter.WY_ulTablePosOfNextRecord 
									 * MEM_USE_RECORD_SIZE);
	}
	
//	printk("First %x Next %x \n",WY_pPreUse,WY_pUseRec);
	//因为最小分配单位是4  字节,重新计算一下大小,按四字节对齐
	if(WY_nSize % 4 == 0) WY_nMemSize = WY_nSize;
	else
	{
		WY_nMemSize = ((WY_nSize / 4) + 1) * 4;
	}

	if(WY_nMemSize % 0x1000 == 0)
	{
		//是4K  的倍数,直接调用 AllocPage  就行了
		WY_pMemUse = AllocPage(WY_nMemSize / 0x1000,WY_bKnl,WY_bSystem);
		WY_nUseableMemory -= WY_nMemSize;
		return (PVOID)WY_pMemUse->WY_ulLinearAddress;
	}
	else
	{
		__asm__("pushf");
		wyos_close_int();
		while(WY_pUseRec)
		{
			//该判断是为了确定上一个记录是不是某个物理页面
			//中最后一个使用的记录，是为了减少页内碎片
			//需要在每个已经申请的页面内进行分配。所以需要
			//找到每个已经分配的页面中最后一个使用记录，
			//来检查剩余空间。因为链表是按照物理页框升序
			//排列的，所以这样判断WY_pPreUse  是否为该页面最后一个
			//使用记录
			//WY_pPreUse->WY_ulKNLFlag == WY_bKnl判断该页面是否与调用特权级相同
			//只有相同才可以分配的，否则会产生页面错误
			if((WY_pUseRec->WY_ulPhysicalPage > WY_pPreUse->WY_ulPhysicalPage) && (WY_pPreUse->WY_ulKNLFlag == WY_bKnl))
			{
				//这个判断是判断上一个记录是否是跨页面的
				//如果是就不再检查了,因为肯定已经满了,
				//判断条件选择原因,一开始申请0x1018字节,分配0x00008
				//页框,那么再审请16字节,那么就从0x00009分配,
				//地址是0x00009018，但是如果，不判断的话，下次申请，
				//还是会检查0x00008这个记录，还从0x00008018分配，就出现了错误
				//这样一判断，如果上一个记录的记录开始地址加上大小
				//不等于这个记录的起始地址的话，要么上个记录没有跨页面
				//要么这个记录不是跟着上面那个分配的。
				if((WY_pPreUse->WY_ulRangeSize + WY_pPreUse->WY_ulLinearAddress) != 
				     WY_pUseRec->WY_ulLinearAddress)
				{
					WY_nPageFreeMem = (0x1000 - ((WY_pPreUse->WY_ulLinearAddress & 0xFFF) + WY_pPreUse->WY_ulRangeSize % 0x1000))%0x1000;
					//页内剩余空间是否大于要申请的空间，大于则分配
					if(WY_nPageFreeMem >= WY_nMemSize)
					{

						WY_pMemUse = allocrec_use(WY_bSystem);
						ConstructUseRecord(WY_pMemUse,
										      WY_pPreUse->WY_ulLinearAddress + WY_pPreUse->WY_ulRangeSize,
										      WY_nMemSize,
							                           ((WY_pPreUse->WY_ulPhysicalPage << 12) + WY_pPreUse->WY_ulRangeSize)/0x1000, 
							                           ((ulong)WY_pMemUse & 0xFFF) >> 4,0,0,0,WY_bKnl);
						InsertRec_use(WY_pMemUse,WY_bSystem);
						WY_nUseableMemory -= WY_nMemSize;
						__asm__("popf");
						return (PVOID)WY_pMemUse->WY_ulLinearAddress;
					}
				}
			}
			WY_pPreUse = WY_pUseRec;
			WY_pUseRec = (WY_pMemRecord)((WY_pUseRec->WY_Use.WY_RecordCharacter.WY_ulTableOfNextRecord << 12) + WY_pUseRec->WY_Use.WY_RecordCharacter.WY_ulTablePosOfNextRecord * MEM_USE_RECORD_SIZE);
		}
		//如果跳出循环了，那么WY_pPreUse  肯定指向的是最后一个记录
		//那么就看看最后一个记录是否符合条件
		WY_nPageFreeMem = (0x1000 - ((WY_pPreUse->WY_ulLinearAddress & 0xFFF) + WY_pPreUse->WY_ulRangeSize % 0x1000))%0x1000;
		//页内剩余空间是否大于要申请的空间，大于则分配
		if((WY_nPageFreeMem >= WY_nMemSize) &&(WY_pPreUse->WY_ulKNLFlag == WY_bKnl))
		{
//			printk("       this a %x s %x\n",WY_pPreUse->WY_ulLinearAddress,WY_pPreUse->WY_ulRangeSize);
			WY_pMemUse = allocrec_use(WY_bSystem);
			ConstructUseRecord(WY_pMemUse,
							      WY_pPreUse->WY_ulLinearAddress + WY_pPreUse->WY_ulRangeSize,
							      WY_nMemSize,
				                           ((WY_pPreUse->WY_ulPhysicalPage << 12) + WY_pPreUse->WY_ulRangeSize)/0x1000, 
				                           ((ulong)WY_pMemUse & 0xFFF) >> 4,0,0,0,WY_bKnl);
			InsertRec_use(WY_pMemUse,WY_bSystem);
//			printk("     tihs\n");
			__asm__("popf");
			return (PVOID)WY_pMemUse->WY_ulLinearAddress;
		}
		else
		{
			//申请一个页面,在这个页面内进行分配
			WY_pMemUse = AllocPage((WY_nMemSize / 0x1000) + 1,WY_bKnl,WY_bSystem);
			wyos_close_int();
			WY_pMemUse->WY_ulRangeSize = WY_nMemSize;
			WY_nUseableMemory -= WY_nMemSize;
			__asm__("popf");
//			printk("Alloc Page Return Size %x  %x\n",WY_pMemUse->WY_ulLinearAddress,WY_pMemUse);
//			printk("");
//			printk("");
			return (PVOID)WY_pMemUse->WY_ulLinearAddress;
		}
	}
	__asm__("popf");
	return (PVOID)0;
}

//需要完善的地方，FreePage之前将映射该物理地址的虚拟地址取消。
void freemem(void * WY_pAddr,BOOL WY_bKnl,BOOL WY_bSystem)
{
	WY_pMemRecord		WY_pPreUse = NULL,WY_pUseRec,WY_pNextRec;
	int					WY_nPhyArea = 0,WY_nLastPage,WY_nTmpPage;
	ulong				WY_ulLinearAddr = 0,WY_ulMemSize = 0;

	if(WY_bSystem)
	{
		WY_pUseRec = WY_PROCTABLE[0].WY_pMemFirstRec;
		WY_pNextRec = (WY_pMemRecord)((WY_PROCTABLE[0].WY_pMemFirstRec->WY_Use.WY_RecordCharacter.WY_ulTableOfNextRecord << 12) 
									  + WY_PROCTABLE[0].WY_pMemFirstRec->WY_Use.WY_RecordCharacter.WY_ulTablePosOfNextRecord 
									  * MEM_USE_RECORD_SIZE);
	}
	else
	{
		WY_pUseRec = WY_PROCTABLE[GetCurrentPID()].WY_pMemFirstRec;
		WY_pNextRec = (WY_pMemRecord)((WY_PROCTABLE[GetCurrentPID()].WY_pMemFirstRec->WY_Use.WY_RecordCharacter.WY_ulTableOfNextRecord << 12) 
									  + WY_PROCTABLE[GetCurrentPID()].WY_pMemFirstRec->WY_Use.WY_RecordCharacter.WY_ulTablePosOfNextRecord 
									  * MEM_USE_RECORD_SIZE);
	}
//	printk("Freemem First %x Next %x \n",WY_pUseRec,WY_pNextRec);
	__asm__("pushf");
	wyos_close_int();
	while(WY_pUseRec)
	{
		//看所传入的地址是否在该记录范围之内
		if((ulong)WY_pAddr >= WY_pUseRec->WY_ulLinearAddress && (ulong)WY_pAddr < (WY_pUseRec->WY_ulLinearAddress + WY_pUseRec->WY_ulRangeSize))
		{
			//判断这块内存是否超过一个页面
//			printk("Loop First %x Next %x Linear %x Rec Line %xRS %x \n",WY_pUseRec,WY_pNextRec,WY_pAddr,WY_pUseRec->WY_ulLinearAddress,WY_pUseRec->WY_ulRangeSize);
			if(WY_pUseRec->WY_ulRangeSize >= 0x1000)
			{
				if(WY_pUseRec->WY_ulRangeSize % 0x1000 == 0) WY_nPhyArea = WY_pUseRec->WY_ulRangeSize / 0x1000;
				else WY_nPhyArea = WY_pUseRec->WY_ulRangeSize / 0x1000 + 1;
			}
			else
			{
				if(WY_pPreUse != NULL)
				{
					//看上一个内存所跨越的页面的最后一个页面是否与这个
					//相同，相同这个页面就不能被释放掉
					if(WY_pPreUse->WY_ulRangeSize%0x1000 == 0) WY_nLastPage = WY_pPreUse->WY_ulPhysicalPage + WY_pPreUse->WY_ulRangeSize / 0x1000 - 1;
					else WY_nLastPage = WY_pPreUse->WY_ulPhysicalPage + WY_pPreUse->WY_ulRangeSize / 0x1000;

					if(WY_nLastPage == WY_pUseRec->WY_ulPhysicalPage)
					{
						WY_nUseableMemory += WY_pUseRec->WY_ulRangeSize;
						DeleteRec_use(WY_pUseRec,WY_bSystem);
						__asm__("popf");
						return;
					}
				}
				WY_nPhyArea = 1;
			}
			//判断下一个记录于这块内存所占的最后一块页面是否相同
			if((WY_pUseRec->WY_ulPhysicalPage + WY_nPhyArea - 1) == WY_pNextRec->WY_ulPhysicalPage)
				WY_nPhyArea--;

			WY_nTmpPage = WY_pUseRec->WY_ulPhysicalPage << 12;
			WY_ulLinearAddr = WY_pUseRec->WY_ulLinearAddress;
			//需要添加取消映射的代码
			WY_nUseableMemory += WY_pUseRec->WY_ulRangeSize;
			WY_ulMemSize = WY_pUseRec->WY_ulRangeSize;
			DeleteRec_use(WY_pUseRec,WY_bSystem);
			//只有这个记录独占了个这几个页面，这几个页面才可以被释放掉
			if(WY_nPhyArea)	
			{
	//			WY_nUseableMemory += WY_ulMemSize;
				FreePage(WY_nTmpPage,WY_nPhyArea,WY_bKnl);
	//			WY_nUseableMemory += WY_nPhyArea * 0x1000;
				UnmappedLinear(WY_ulLinearAddr,WY_nPhyArea,WY_bKnl);
			}
			__asm__("popf");
			return;
		}
		else		//不在该范围内，则检查下一个记录
		{
			WY_pPreUse = WY_pUseRec;
			WY_pUseRec = WY_pNextRec;
			WY_pNextRec = (WY_pMemRecord)((WY_pNextRec->WY_Use.WY_RecordCharacter.WY_ulTableOfNextRecord << 12) + WY_pNextRec->WY_Use.WY_RecordCharacter.WY_ulTablePosOfNextRecord * MEM_USE_RECORD_SIZE);
		}
	}
	__asm__("popf");
}

void * mallock(int WY_nSize)
{
	return mallocmem(WY_nSize,TRUE,FALSE);
}

void freek(void * WY_pAddr)
{
	return freemem(WY_pAddr,TRUE,FALSE);
}

void *mallocs(int WY_nSize)
{
//	printk("CurPID %d\n",GetCurrentPID());
	return mallocmem(WY_nSize,TRUE,TRUE);
}

void frees(void * WY_pAddr)
{
	return freemem(WY_pAddr,TRUE,TRUE);
}

void PageExpProc(ulong WY_ulErrCode,ulong WY_ulFaultLinearAddr)
{
	printk("Page Exception Error PID %x  TID %x Code %x Cr2:%x\n",WY_usCurrentPID,GetCurrentTID(WY_usCurrentPID),WY_ulErrCode,WY_ulFaultLinearAddr);
	while(1)
	{}
}

ulong SyscallGetMmInfo()
{
	return WY_nUseableMemory;
}

ulong SyscallMalloc(pSyscallParam WY_pInputParam)
{
	ulong	WY_ulRetValue = (ulong)mallocmem(WY_pInputParam->WY_ulParam0,FALSE,FALSE);

//	printk("mallocmem return %x\n",WY_ulRetValue);
	return WY_ulRetValue;
}

void SyscallFree(pSyscallParam WY_pInputParam)
{
	freemem((PVOID)WY_pInputParam->WY_ulParam0,FALSE,FALSE);
}
//new line
