/***************************************************************************
			WYOS cpu.c
			处理器初始化和使用源文件
						编码:WY.lslrt			editor	 :WY.lslrt
						日期:2005/12/2			date	 :2005/12/2
						版权:WY 和 WY.lslrt所有 copyright:WY and WY.lslrt
						URL:http://www.wyos.net http://wylslrt.go.3322.org
***************************************************************************/
#include "..\include\WYOSType.h"
#include "..\include\cpu.h"
#include "..\include\io.h"
#include "..\include\syscall.h"
#include "..\video\video.h"
#include "..\include\kernel.h"
#include "..\include\memory.h"
#include "..\include\process.h"

//extern void TestDiv();
extern ushort			WY_usCurrentPID;
extern WY_TSS		WY_TimeTSS;
extern WY_ProcTable	WY_PROCTABLE[MAX_PROC_NUM];
extern WY_pProcTable	WY_PRIQueue[WYOS_PROC_PRI_KIND_NUM];
extern void 			CheckBlockQueue();

//可用的全局描述符起始地址
static WY_pStorageDesc			WY_pGDT = (WY_pStorageDesc)WYOS_GDT_BASE+0x8;
//进程可用的全局描述符表地址
static WY_pSystemDesc			WY_pUserDesc = (WY_pSystemDesc)WYOS_USERDESC_BASE;
//IDT起始地址
static WY_pGATEDesc			WY_pIDT = (WY_pGATEDesc)WYOS_IDT_BASE;
//系统时间tick
static ulong					WY_ulSystemTick = 0;

//int							count = 0;
void cpuinit()
{
	ulong	WY_ulRetValue;

	//设置全局变量
	//可用的全局描述符起始地址
	WY_pGDT = (WY_pStorageDesc)WYOS_GDT_BASE+0x8;
	//进程可用的全局描述符表地址
	WY_pUserDesc = (WY_pSystemDesc)WYOS_USERDESC_BASE;
	//IDT起始地址
	WY_pIDT = (WY_pGATEDesc)WYOS_IDT_BASE;
	//系统时间tick
	WY_ulSystemTick = 0;


	Init8259();
	InitTimer();
	//安装除法错误处理中断
	WY_ulRetValue = SetInterrupt((INT_PROC)(ulong)DivError,0x8,SEGMENT_RING3,GATE_386INTERRUPT,0);
	if(WY_ulRetValue != WYOS_IDT_ERR_OK)
		printk("Install Div Error Process Failed. Error Code = %d\n",WY_ulRetValue);

}

ulong SetInterrupt(INT_PROC WY_pIntProc, ulong WY_ulSelector,ulong WY_ulDPL, ulong WY_ulSegType,ulong WY_ulIntNum)
{
	WY_pGATEDesc		WY_pIDT = (WY_pGATEDesc)(0x3800 + WY_ulIntNum * 8);

	if(WY_ulIntNum > 255) return WYOS_IDT_ERR_INTNUM_LARGE;
	
	if(WY_pIDT->WY_ulPresent == 1) return WYOS_IDT_ERR_GATE_EXIST;
	
	if(WY_ulDPL > 3) return WYOS_IDT_ERR_NO_PARAM;

	if(WY_ulSegType > 0xF) return WYOS_IDT_ERR_NO_PARAM;

	

	WY_pIDT->WY_ulHighOffset = (ulong)WY_pIntProc >> 16;
	WY_pIDT->WY_ulLowOffset = (ulong)WY_pIntProc & 0xFFFF;
	WY_pIDT->WY_ulDescSelector = WY_ulSelector;
	WY_pIDT->WY_ulDescDPL = WY_ulDPL;
	WY_pIDT->WY_ulUsParamCnt = 0;
	WY_pIDT->WY_ulSegType = WY_ulSegType;
	WY_pIDT->WY_ulPresent = 1;
	WY_pIDT->WY_ulReserved = 0;
	WY_pIDT->WY_ulDescType = 0;

	return WYOS_IDT_ERR_OK;
}

ulong UninsInterrupt(ulong WY_ulIntNum)
{
	WY_pGATEDesc		WY_pIDT = (WY_pGATEDesc)(0x3800 + WY_ulIntNum * 8);

	if(WY_ulIntNum > 255) return WYOS_IDT_ERR_INTNUM_LARGE;
	if(WY_pIDT->WY_ulPresent == 0) return WYOS_IDT_ERR_GATE_NOTEXIST;
	WY_pIDT->WY_ulPresent = 0;
}
///*
void TimeInt()
{
	ushort	WY_usPID;
	int		i;
	ulong	WY_ulTSSSEL;
	WY_pSystemDesc	WY_proctssdesc;
	WY_pProcTable	WY_testtable;

//	printk("time int\n");
	WY_ulSystemTick += 55;
//	printk("Current PID : %d Usable Time %d\n",WY_usCurrentPID,WY_PROCTABLE[WY_usCurrentPID].WY_ulUseableTime);
	
//	printk("%x PID 0 Next %x PRI %d Queue %x\n",&WY_PROCTABLE[0],WY_PROCTABLE[0].WY_pNextTable,WY_PROCTABLE[0].WY_ulCurPriQueue,WY_PRIQueue[WY_PROCTABLE[0].WY_ulCurPriQueue]);
	
	WY_PROCTABLE[WY_usCurrentPID].WY_ulUseableTime--;
	if(WY_PROCTABLE[WY_usCurrentPID].WY_ulUseableTime == 0)
	{
		WY_PROCTABLE[WY_usCurrentPID].WY_ulUseableTime = WY_PROCTABLE[WY_usCurrentPID].WY_ulTimeslice;
		
		WY_PRIQueue[WY_PROCTABLE[WY_usCurrentPID].WY_ulCurPriQueue] = WY_PROCTABLE[WY_usCurrentPID].WY_pNextTable;
		
		WY_PROCTABLE[WY_usCurrentPID].WY_pNextTable = WY_PRIQueue[WY_PROCTABLE[WY_usCurrentPID].WY_ulCurPriQueue + 1];
		WY_PRIQueue[WY_PROCTABLE[WY_usCurrentPID].WY_ulCurPriQueue + 1] = &WY_PROCTABLE[WY_usCurrentPID];
		
		WY_PROCTABLE[WY_usCurrentPID].WY_ulCurPriQueue++;
		WY_PROCTABLE[WY_usCurrentPID].WY_usProcState = WYOS_PROC_STATE_READY;
	
	}

	CheckBlockQueue();
//	printk("%x PID 0 Next %x PRI %d Queue %x\n",&WY_PROCTABLE[0],WY_PROCTABLE[0].WY_pNextTable,WY_PROCTABLE[0].WY_ulCurPriQueue,WY_PRIQueue[WY_PROCTABLE[0].WY_ulCurPriQueue]);
/*	//
	for(i = WYOS_PROC_PRI_REALTIME;i < WYOS_PROC_PRI_IDLE;i++)
	{
		WY_testtable = WY_PRIQueue[i];
		printk("PRI %d : ",i);
		while(WY_testtable)
		{
			printk("PID %d ",WY_testtable->WY_ulPID);
			WY_testtable = WY_testtable->WY_pNextTable;
		}
		printk("\n");
		
	}*/

	//没有线程的调度方式
	WY_usPID = MasterScheudler();
/*	printk("-----------------------\n");
	for(i = WYOS_PROC_PRI_REALTIME;i < WYOS_PROC_PRI_IDLE;i++)
	{
		WY_testtable = WY_PRIQueue[i];
		printk("PRI %d : ",i);
		while(WY_testtable)
		{
			printk("PID %d ",WY_testtable->WY_ulPID);
			WY_testtable = WY_testtable->WY_pNextTable;
		}
		printk("\n");
		
	}*/
//	printk("Next PID %d ......................\n",WY_usPID);
//	if(WY_usPID == 0)
//	{
//		printk("Next Pid %d %d\n",WY_usPID,WY_ulSystemTick);
//		printk("PID 0\n");
//	}

	//如果该任务为可用认为，则置为忙		
	WY_usCurrentPID = WY_usPID;
	
	WY_ulTSSSEL = TaskScheudler(WY_usPID, &WY_PROCTABLE[WY_usPID].WY_ulCurTID);
	WY_proctssdesc = (WY_pSystemDesc)(WY_PROCTABLE[WY_usPID].WY_pThreadCtr[WY_PROCTABLE[WY_usPID].WY_ulCurTID].WY_usTSSSel+ WYOS_GDT_BASE);
	WY_proctssdesc->WY_ulSegTYPE = SYSTEM_BUSY_386TSS;
//	printk("PID %x TID %x TSS %x\n",WY_usPID,WY_PROCTABLE[WY_usPID].WY_ulCurTID,WY_ulTSSSEL);
	WY_TimeTSS.WY_ulBLink= WY_ulTSSSEL;

}


void * allocGlobalDesc(BOOL WY_isSys)
{
	WY_pStorageDesc			WY_pGDesc;				//指向描述符的指针
	
	if(WY_isSys)
	{
		WY_pGDesc = WY_pGDT;
		while((ulong)WY_pGDesc < WYOS_USERDESC_BASE)
		{
			if(WY_pGDesc->WY_ulPresent)
				WY_pGDesc++;
			else		return (PVOID)WY_pGDesc;
		}
		return NULL;
	}
	else
	{
		WY_pGDesc = WY_pUserDesc;
		while((ulong)WY_pGDesc < WYOS_IDT_BASE)
		{
			if(WY_pGDesc->WY_ulPresent)
				//因为这些描述符是给进程用的，所以一次
				//分配两个一个LDT  一个TSS 
				WY_pGDesc = (WY_pStorageDesc)((ulong)WY_pGDesc + 16);
			else		return (PVOID)WY_pGDesc;
		}
		return NULL;
	}
}

void freeGlobalDesc(int WY_nSelector)
{
	WY_pStorageDesc		WY_pGDesc;

	if(WY_nSelector > 0xFFFF) return;

	WY_nSelector = (WY_nSelector >> 3) << 3;
	WY_pGDesc = (WY_pStorageDesc)WYOS_GDT_BASE + (WY_nSelector - 1) * 8;
	WY_pGDesc->WY_ulPresent = 0;

	return;
}

void * allocInterruptGate()
{
	WY_pGATEDesc		WY_pIntGate = (WY_pGATEDesc)WYOS_IDT_BASE + 8*0x10;		//0x0 - 0xF 为异常处理
																					//0x20 - 0x2F 为硬件中断

	while((ulong)WY_pIntGate < WYOS_IDT_BASE + 0x20 * 8)
	{
		if(WY_pIntGate->WY_ulPresent)
			WY_pIntGate++;
		else return (PVOID)WY_pIntGate;
	}

	WY_pIntGate = (WY_pGATEDesc) WYOS_IDT_BASE + 0x30 * 8;
	while((ulong)WY_pIntGate <=  WYOS_IDT_BASE + 0xFF * 8)
	{
		if(WY_pIntGate->WY_ulPresent)
			WY_pIntGate++;
		else return (PVOID)WY_pIntGate;
	}
	return NULL;
}

void freeInterruptGate(WY_pGATEDesc WY_pIntGate)
{
	if((ulong)WY_pIntGate > WYOS_IDT_BASE + 0xF * 8 && (ulong)WY_pIntGate <= WYOS_IDT_BASE + 0xFF * 8)
	{
		WY_pIntGate->WY_ulPresent = 0;
	}
}

ulong GetCurrentTick()
{
	return WY_ulSystemTick;
}

