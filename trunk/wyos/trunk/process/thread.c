/***************************************************************************
			WYOS thread.c
			线程相关源文件
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

extern WY_ProcTable	WY_PROCTABLE[MAX_PROC_NUM];

ulong KCreateThread(THREAD_ROUTINE ThreadRoutine, PVOID WY_pParam, BOOL WY_bKnl)
{
	WY_pSystemDesc		WY_pTSSDesc = NULL;
	PVOID				WY_pThreadStack = NULL,WY_pKnlStack = NULL;
	ushort				WY_usCurPID = GetCurrentPID();
	int					i;

	//申请任务段描述符
	WY_pTSSDesc = allocGlobalDesc(TRUE);
	if(WY_pTSSDesc == NULL)
	{
		return -1;
	}
	//申请线程堆栈
	if(WY_bKnl)
	{
		//内核线程
		WY_pThreadStack = mallocs(0x800);
		if(WY_pThreadStack == NULL)
		{
			freeGlobalDesc(((ulong)WY_pTSSDesc - WYOS_GDT_BASE));
			return -1;
		}

	}
	else
	{
		//用户线程，申请用户堆栈
		WY_pThreadStack = mallocmem(0x800,FALSE,FALSE);
		if(WY_pThreadStack == NULL)
		{
			freeGlobalDesc(((ulong)WY_pTSSDesc - WYOS_GDT_BASE));
			return -1;
		}
		//申请内核堆栈
		WY_pKnlStack = mallock(0x800);
		if(WY_pKnlStack == NULL)
		{
			freeGlobalDesc(((ulong)WY_pTSSDesc - WYOS_GDT_BASE));
			freek(WY_pThreadStack);
			return -1;
		}
	}
	
	//申请线程表,因为第0号线程是主线程，因此从1开始
	for(i = 1;i <= MAX_THREAD_NUM;i++)
	{
		//查看是否使用
		if(!WY_PROCTABLE[WY_usCurPID].WY_pThreadCtr[i].WY_ulUseFlag)
		{
			//未使用,选用此表
			break;
		}
	}
	if(i > MAX_THREAD_NUM)
	{
		//没有可用线程表
		freeGlobalDesc(((ulong)WY_pTSSDesc - WYOS_GDT_BASE));
		if(WY_bKnl)
		{
			//内核线程，释放内核堆栈
			frees(WY_pThreadStack);
		}
		else
		{
			//用户线程，释放用户堆栈
			freemem(WY_pThreadStack,FALSE,FALSE);
			freek(WY_pKnlStack);
		}
		return -1;
	}
	else
	{
		//设置TSS段
		memset((char*)&WY_PROCTABLE[WY_usCurPID].WY_pThreadCtr[i].WY_ThreadTss,0,sizeof(WY_TSS));

		if(!WY_bKnl)
		{
			memcpy((char*)&WY_PROCTABLE[WY_usCurPID].WY_pThreadCtr[i].WY_ThreadTss,(char*)&WY_PROCTABLE[WY_usCurPID].WY_pThreadCtr[0].WY_ThreadTss,sizeof(WY_TSS));
			WY_PROCTABLE[WY_usCurPID].WY_pThreadCtr[i].WY_ThreadTss.WY_ulESP0 = (ulong)WY_pKnlStack + 0x800;
		}
		else
		{
			WY_PROCTABLE[WY_usCurPID].WY_pThreadCtr[i].WY_ThreadTss.WY_ulEFLAGS = 0x202;
			WY_PROCTABLE[WY_usCurPID].WY_pThreadCtr[i].WY_ThreadTss.WY_ulCS = 0x8;
			WY_PROCTABLE[WY_usCurPID].WY_pThreadCtr[i].WY_ThreadTss.WY_ulSS = 0x10;
			WY_PROCTABLE[WY_usCurPID].WY_pThreadCtr[i].WY_ThreadTss.WY_ulDS = 0x10;
			WY_PROCTABLE[WY_usCurPID].WY_pThreadCtr[i].WY_ThreadTss.WY_ulES = 0x10;
			WY_PROCTABLE[WY_usCurPID].WY_pThreadCtr[i].WY_ThreadTss.WY_ulFS = 0x10;
			WY_PROCTABLE[WY_usCurPID].WY_pThreadCtr[i].WY_ThreadTss.WY_ulGS = 0x10;
			WY_PROCTABLE[WY_usCurPID].WY_pThreadCtr[i].WY_ThreadTss.WY_ulLDT = 0;
		}
		WY_PROCTABLE[WY_usCurPID].WY_pThreadCtr[i].WY_ThreadTss.WY_ulCR3 = WY_PROCTABLE[WY_usCurPID].WY_pThreadCtr[0].WY_ThreadTss.WY_ulCR3;
		WY_PROCTABLE[WY_usCurPID].WY_pThreadCtr[i].WY_ThreadTss.WY_ulEIP = (ulong)ThreadRoutine;
		WY_PROCTABLE[WY_usCurPID].WY_pThreadCtr[i].WY_ThreadTss.WY_ulESP = (ulong)WY_pThreadStack + 0x800;
		WY_PROCTABLE[WY_usCurPID].WY_pThreadCtr[i].WY_ThreadTss.WY_usBitmapOffset = sizeof(WY_TSS);
		//设置I/O MAP结束标志
		WY_PROCTABLE[WY_usCurPID].WY_pThreadCtr[i].WY_ulIOMapEnd = 0xFF;
		//设置任务状态段描述符
		WY_pTSSDesc->WY_ulPresent = 1;
		WY_pTSSDesc->WY_ulDescType = 0;
		WY_pTSSDesc->WY_ulSegTYPE = SYSTEM_BUSY_386TSS;
		WY_pTSSDesc->WY_ulDescDPL = SEGMENT_RING0;
		WY_pTSSDesc->WY_ulGranularity = 0;
		WY_pTSSDesc->WY_ulD = 0;
		WY_pTSSDesc->WY_ulSoftUse = 0;
		WY_pTSSDesc->WY_ulReserved = 0;
		//设置TSS段基址及段限
		WY_pTSSDesc->WY_ulLowSegLimit = 0x69;			//105个字节，长度应该包括I/O MAP结束标志
		WY_pTSSDesc->WY_ulHighSegLimit = 0;
		WY_pTSSDesc->WY_ulLowSegBase = ((ulong)&WY_PROCTABLE[WY_usCurPID].WY_pThreadCtr[i].WY_ThreadTss) & 0xFFFFFF;
		WY_pTSSDesc->WY_ulHighSegBase = (((ulong)&WY_PROCTABLE[WY_usCurPID].WY_pThreadCtr[i].WY_ThreadTss)  & 0xFF000000) >> 24;

		WY_PROCTABLE[WY_usCurPID].WY_pThreadCtr[i].WY_ulUseFlag = 1;
		WY_PROCTABLE[WY_usCurPID].WY_pThreadCtr[i].WY_usThreadState = WYOS_THREAD_RUNABLE;
		WY_PROCTABLE[WY_usCurPID].WY_pThreadCtr[i].WY_usUseTime = THREAD_RUN_TIME;
		WY_PROCTABLE[WY_usCurPID].WY_pThreadCtr[i].WY_usTSSSel = (ulong)WY_pTSSDesc - WYOS_GDT_BASE;
		WY_PROCTABLE[WY_usCurPID].WY_ulThreadNum++;

		return i;
	}
}

ulong GetTIDSyscall()
{
	return GetCurrentTID(GetCurrentPID());
}

//new line
