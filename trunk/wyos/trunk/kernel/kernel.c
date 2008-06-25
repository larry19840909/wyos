/***************************************************************************
			WYOS Kernel.c
			内核文件
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
#include "..\include\string.h"
#include "..\include\memory.h"
#include "..\include\kernel.h"
#include "..\include\process.h"
#include "..\include\math.h"
#include "..\include\driver.h"
#include "..\include\floppy.h"
#include "..\include\fat12.h"

extern unsigned long	WY_ulPTEBase[1024];
extern WY_ProcTable	WY_PROCTABLE[MAX_PROC_NUM];
extern ushort			WY_usCurrentPID;

//DriverFun	DriverFunction[32];

//ulong	TestThread(PVOID WY_pParam);

int main(void)
{
	WY_pSystemDesc				WY_pKnlDesc;		//内核LDT  和TSS  描述符的指针
	WY_pSystemDesc				WY_pTaskDesc;		//任务用来建立TSS  描述符
	int							WY_nloop = 0;
	void *						WY_pStack;
	ulong						WY_ulTmp = 1;
	ulong *						WY_nTaskPDT;
	WY_pProcTable				WY_testProc;
//	WY_pMemRecord				WY_ptest;
//	char							WY_testbuf[512];
	//清空IDT表
	memset((char*)WYOS_IDT_BASE,0,8 * 256);
	wyos_init_syscall();
	wyos_video_int();
	printk("WYOS Initializing.Please wait......\n");
	meminit();
//	printk("CurPID %d\n",GetCurrentPID());
	InitProc();
//	printk("CurPID %d\n",GetCurrentPID());
	//重新申请内核堆栈3M
	//原因是先前的堆栈
	//并不知其所占用大小，
	//而且未记录，为防止
	//将来发生被内核数据
	//给覆盖或者堆栈被数据
	//覆盖，则重新建立堆栈
	
	WY_pStack = mallocs(0x300000);
	printk("Rebuild System Stack....");
	if(!WY_pStack)
	{
		//内核堆栈申请出错，输出出错语句，死机
		printk("Rebuild Kernel Stack Error,please restart\n");
		while(1)
		{}
	}
	//申请成功，重设esp
	//因为内核堆栈开始设置在4MB的，随着
	//内核的不断增长，有可能会覆盖堆栈
	//因此重新设置堆栈，防止堆栈被覆盖

	printk("Successful!New Stack %x - %x\n",WY_pStack,WY_pStack + 0x300000);
	__asm__("movl %1,%%esp\n\t"
			"add $0x300000,%%esp\n\t"
			"movl %%esp,%%eax":"=a"(WY_ulTmp) :"m"(WY_pStack));

	cpuinit();
//	memset((char *)DriverFunction,0,sizeof(DriverFun)*32);
	DriverInit();
	Fat12Init();
//	printk("sizeof PROC %x\n",sizeof(WY_ProcTable));

/*	//测试创建第一个进程
	WY_testProc = AllocProcTable(&WY_ulTmp);
	printk("first P %x \n",WY_testProc);
	InitProcCtrTable(WY_testProc, 0, WY_ulTmp,WYOS_PROC_PRI_MIDDLE,WYOS_PROC_TS_MIDDLE,WYOS_PROC_STATE_USER);
	ConstructProcTable(WY_testProc, WYOS_PROC_STATE_USER, 0x1000000, 4096);
	//测试创建第二个进程
	WY_testProc = AllocProcTable(&WY_ulTmp);
	printk("Second P %x \n",WY_testProc);
	InitProcCtrTable(WY_testProc, 0, WY_ulTmp,WYOS_PROC_PRI_MIDDLE,WYOS_PROC_TS_MIDDLE,WYOS_PROC_STATE_USER);
	ConstructProcTable(WY_testProc, WYOS_PROC_STATE_USER, 0x1000000, 4096);
///*	//测试创建第三个进程
	WY_testProc = AllocProcTable(&WY_ulTmp);
	printk("Third P %x \n",WY_testProc);
	InitProcCtrTable(WY_testProc, 0, WY_ulTmp,WYOS_PROC_PRI_REALTIME,WYOS_PROC_TS_MIDDLE,WYOS_PROC_STATE_USER);
	ConstructProcTable(WY_testProc, WYOS_PROC_STATE_USER, 0x1000000, 4096);
	//测试创建第四个进程
	WY_testProc = AllocProcTable(&WY_ulTmp);
	printk("Forth P %x \n",WY_testProc);
	InitProcCtrTable(WY_testProc, 0, WY_ulTmp,WYOS_PROC_PRI_HIGH,WYOS_PROC_TS_MIDDLE,WYOS_PROC_STATE_USER);
	ConstructProcTable(WY_testProc, WYOS_PROC_STATE_USER, 0x1000000, 4096);
	//测试创建第五个进程
	WY_testProc = AllocProcTable(&WY_ulTmp);
	printk("Fifty P %x \n",WY_testProc);
	InitProcCtrTable(WY_testProc, 0, WY_ulTmp,WYOS_PROC_PRI_LOW,WYOS_PROC_TS_MIDDLE,WYOS_PROC_STATE_USER);
	ConstructProcTable(WY_testProc, WYOS_PROC_STATE_USER, 0x1000000, 4096);
	//测试创建第六个进程
	WY_testProc = AllocProcTable(&WY_ulTmp);
	printk("sixty P %x \n",WY_testProc);
	InitProcCtrTable(WY_testProc, 0, WY_ulTmp,WYOS_PROC_PRI_LOW,WYOS_PROC_TS_MIDDLE,WYOS_PROC_STATE_USER);
	ConstructProcTable(WY_testProc, WYOS_PROC_STATE_USER, 0x1000000, 4096);
	//测试创建第七个进程
	WY_testProc = AllocProcTable(&WY_ulTmp);
	printk("seven P %x \n",WY_testProc);
	InitProcCtrTable(WY_testProc, 0, WY_ulTmp,WYOS_PROC_PRI_LOW,WYOS_PROC_TS_MIDDLE,WYOS_PROC_STATE_USER);
	ConstructProcTable(WY_testProc, WYOS_PROC_STATE_USER, 0x1000000, 4096);
	//测试创建第八个进程
	WY_testProc = AllocProcTable(&WY_ulTmp);
	printk("eighty P %x \n",WY_testProc);
	InitProcCtrTable(WY_testProc, 0, WY_ulTmp,WYOS_PROC_PRI_LOW,WYOS_PROC_TS_MIDDLE,WYOS_PROC_STATE_USER);
	ConstructProcTable(WY_testProc, WYOS_PROC_STATE_USER, 0x1000000, 4096);
	//测试创建第九个进程
	WY_testProc = AllocProcTable(&WY_ulTmp);
	printk("ninety P %x \n",WY_testProc);
	InitProcCtrTable(WY_testProc, 0, WY_ulTmp,WYOS_PROC_PRI_LOW,WYOS_PROC_TS_MIDDLE,WYOS_PROC_STATE_USER);
	ConstructProcTable(WY_testProc, WYOS_PROC_STATE_USER, 0x1000000, 4096);
	//测试创建第十个进程
	WY_testProc = AllocProcTable(&WY_ulTmp);
	printk("tenth P %x \n",WY_testProc);
	InitProcCtrTable(WY_testProc, 0, WY_ulTmp,WYOS_PROC_PRI_LOW,WYOS_PROC_TS_MIDDLE,WYOS_PROC_STATE_USER);
	ConstructProcTable(WY_testProc, WYOS_PROC_STATE_USER, 0x1000000, 4096);
	//测试创建第十一个进程
	WY_testProc = AllocProcTable(&WY_ulTmp);
	printk("11 P %x \n",WY_testProc);
	InitProcCtrTable(WY_testProc, 0, WY_ulTmp,WYOS_PROC_PRI_LOW,WYOS_PROC_TS_MIDDLE,WYOS_PROC_STATE_USER);
	ConstructProcTable(WY_testProc, WYOS_PROC_STATE_USER, 0x1000000, 4096);
	//测试创建第十二个进程
	WY_testProc = AllocProcTable(&WY_ulTmp);
	printk("12 P %x \n",WY_testProc);
	InitProcCtrTable(WY_testProc, 0, WY_ulTmp,WYOS_PROC_PRI_LOW,WYOS_PROC_TS_MIDDLE,WYOS_PROC_STATE_USER);
	ConstructProcTable(WY_testProc, WYOS_PROC_STATE_USER, 0x1000000, 4096);
	//测试创建第十三个进程
	WY_testProc = AllocProcTable(&WY_ulTmp);
	printk("13 P %x \n",WY_testProc);
	InitProcCtrTable(WY_testProc, 0, WY_ulTmp,WYOS_PROC_PRI_LOW,WYOS_PROC_TS_MIDDLE,WYOS_PROC_STATE_USER);
	ConstructProcTable(WY_testProc, WYOS_PROC_STATE_USER, 0x1000000, 4096);
//*/
////测试内存申请函数
//	WY_pStack = mallocmem(0x400000,FALSE);
//	memset(WY_pStack,0,0x400000);
//	memset(WY_pStack,'W',22);
//	printk("%x %s",WY_pStack,WY_pStack);

//	WY_ptest = AllocPage(1,TRUE);
//	WY_nTaskPDT = 0;
//	WY_nTaskPDT[512] = (WY_ptest->WY_ulPhysicalPage << 12) | 0x1;//(0x80000000 & 0xFFC00000) >> 22
//	WY_nTaskPDT = (ulong *)WY_ptest->WY_ulLinearAddress;
//	WY_nTaskPDT[0] = 0x1000000 | 0x1;
//	__asm__("call 0x80000000");

//	KCreateThread((THREAD_ROUTINE)TestThread,NULL,TRUE);
//	KCreateThread((THREAD_ROUTINE)TestThread,NULL,TRUE);
//	KCreateThread((THREAD_ROUTINE)TestThread,NULL,TRUE);
//	printk("");
	wyos_close_int();
	wyos_open_int();

	while(1)
	{
/*
		for(WY_nloop = 0;WY_nloop < 32;WY_nloop++)
		{
			if(DriverFunction[WY_nloop] != NULL)
			{
				DriverFunction[WY_nloop]();
			}
		}
//*/
/*	
//		printk("??");
		KSendMessage(1, 0x100,0,0);
		KSendMessage(2, 0x100,0,0);
		KSendMessage(3, 0x100,0,0);
		KSendMessage(4, 0x100,0,0);
		KSendMessage(5, 0x100,0,0);
		KSendMessage(6, 0x100,0,0);
		KSendMessage(7, 0x100,0,0);
		KSendMessage(8, 0x100,0,0);
		KSendMessage(9, 0x100,0,0);
		KSendMessage(10, 0x100,0,0);
		KSendMessage(11, 0x100,0,0);
		KSendMessage(12, 0x100,0,0);
		KSendMessage(13, 0x100,0,0);
//*/		
//		printk("idle\n");
	};
}

/*
ulong	TestThread(PVOID WY_pParam)
{
	ulong	tid = GetCurrentTID(0);
	while(1)
	{
		if(tid == 1)
		{
			printk("test thread %d \n",tid);
			BlockThread(WYOS_PROC_BLOCK_TYPE_SLEEP, tid,4000,0,0);
//			UnblockThread(GetCurrentPID(), 3,WYOS_PROC_BLOCK_TYPE_SLEEP,WYOS_BLOCK_STATE_COMPLETE);
//			UnblockThread(GetCurrentPID(), 2,WYOS_PROC_BLOCK_TYPE_SLEEP,WYOS_BLOCK_STATE_COMPLETE);

		}
		else
		{
			printk("I can run,Hoooooooooooooooooooooooo tid %d\n",tid);
			BlockThread(WYOS_PROC_BLOCK_TYPE_SLEEP, tid,tid * 2000,0,0);
		}
	}
}
*/

