/***************************************************************************
			WYOS io.c
			输入输出源文件
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
#include "..\include\process.h"


extern WY_ProcTable	WY_PROCTABLE[MAX_PROC_NUM];

extern void TimeInterrupt();

WY_TSS				WY_TimeTSS;		//时钟TSS段

void WritePort(ushort WY_usPort, BYTE WY_bData)
{
	__asm__("out %%al,%%dx" : : "a"(WY_bData) , "d"(WY_usPort));
}

void WritePortW(ushort WY_usPort, ulong WY_ulData)
{
	__asm__("outl %%eax,%%dx"::"a"(WY_ulData),"d"(WY_usPort));
}

BYTE ReadPort(ushort WY_usPort)
{
	BYTE	WY_bResult;

	__asm__("in %%dx,%%al" : "=a"(WY_bResult) : "d"(WY_usPort));

	return WY_bResult;
}

ulong ReadPortW(ushort WY_usPort)
{
	ulong WY_ulResult;

	__asm__("inl %%dx,%%eax":"=a"(WY_ulResult):"d"(WY_usPort));

	return WY_ulResult;
}
//
//下面是一些硬件设备的初始化和操作函数
//

/*-----------------------8259可编程中断控制器操作Start-----------------*/
void Init8259()
{
	//发送ICW1  命令字，级联工作方式
	WritePort(0x20,0x11);
	WritePort(0xA0,0x11);
	__asm("nop");

	//发送ICW2  命令字，设置中断类型码（从0x20  开始）
	WritePort(0x21,0x20);
	WritePort(0xA1,0x28);
	__asm("nop");

	//发送ICW3  命令字，设置级联引脚
	WritePort(0x21,0x4);
	WritePort(0xA1,0x2);
	__asm("nop");

	//发送ICW4  命令字，设置嵌套方式，处理器字长（1  表示为16  或者32  位系统）
	//主片从片都处于普通全嵌套方式，因为不允许
	//一个硬件设备处理中，被其它硬件给中断
	WritePort(0x21,0x1);
	WritePort(0xA1,0x1);
	__asm("nop");

	//发送OCW1  命令字，屏蔽所有中断
	WritePort(0x21,0xFF);
	WritePort(0xA1,0xFF);
}

BOOL OpenHardInt(ulong WY_ulIRQL)
{
	ushort		WY_usPort;
	BYTE		WY_bCommand;
	if(WY_ulIRQL == 2 || WY_ulIRQL > 15) return FALSE;

	WY_usPort = (WY_ulIRQL / 8) * 0x80 + 0x21;
	WY_ulIRQL = WY_ulIRQL % 8;
	WY_bCommand = ReadPort(WY_usPort);
	switch(WY_ulIRQL)
	{
		case	0:
			WY_bCommand = WY_bCommand & 0xFE;
			break;
		case	1:
			WY_bCommand = WY_bCommand & 0xFD;
			break;
		case	2:
			WY_bCommand = WY_bCommand & 0xFB;
			break;
		case	3:
			WY_bCommand = WY_bCommand & 0xF7;
			break;
		case	4:
			WY_bCommand = WY_bCommand & 0xEF;
			break;
		case	5:
			WY_bCommand = WY_bCommand & 0xDF;
			break;
		case	6:
			WY_bCommand = WY_bCommand & 0xBF;
			break;
		case	7:
			WY_bCommand = WY_bCommand & 0x7F;
			break;
		default:
			break;
	}
	WritePort(WY_usPort, WY_bCommand);
	return TRUE;
}

BOOL CloseHardInt(ulong WY_ulIRQL)
{
	ushort	WY_usPort;
	BYTE	WY_bCommand;

	if(WY_ulIRQL == 2 || WY_ulIRQL > 15) return FALSE;

	WY_usPort = (WY_ulIRQL / 8) * 0x80 + 0x21;
	WY_ulIRQL = WY_ulIRQL % 8;
	WY_bCommand = ReadPort(WY_usPort);

	switch(WY_ulIRQL)
	{
		case	0:
			WY_bCommand = WY_bCommand | 0x1;
			break;
		case	1:
			WY_bCommand = WY_bCommand | 0x2;
			break;
		case	2:
			WY_bCommand = WY_bCommand | 0x4;
			break;
		case	3:
			WY_bCommand = WY_bCommand | 0x8;
			break;
		case	4:
			WY_bCommand = WY_bCommand | 0x16;
			break;
		case	5:
			WY_bCommand = WY_bCommand | 0x32;
			break;
		case	6:
			WY_bCommand = WY_bCommand | 0x64;
			break;
		case	7:
			WY_bCommand = WY_bCommand | 0x128;
			break;
		default:
			break;
	}
	WritePort(WY_usPort,WY_bCommand);
	return TRUE;
}
/*-----------------------8259可编程中断控制器操作End-------------------*/
/*-----------------------可编程定时器操作START---------------------------*/

void InitTimer()
{
	WY_pSystemDesc		WY_pKnlDesc;
	PVOID				WY_pStack;
	
	WY_pKnlDesc = (WY_pSystemDesc)0x2020;	
	WY_pKnlDesc->WY_ulLowSegLimit = 0x68;
	WY_pKnlDesc->WY_ulLowSegBase = (ulong)(&WY_TimeTSS) & 0xFFFFFF;
	WY_pKnlDesc->WY_ulSegTYPE = 0x9;
	WY_pKnlDesc->WY_ulDescType = 0;
	WY_pKnlDesc->WY_ulDescDPL = 0;
	WY_pKnlDesc->WY_ulPresent = 1;
	WY_pKnlDesc->WY_ulHighSegLimit = 0;
	WY_pKnlDesc->WY_ulSoftUse = 0;
	WY_pKnlDesc->WY_ulReserved = 0;
	WY_pKnlDesc->WY_ulD = 0;
	WY_pKnlDesc->WY_ulGranularity = 0;
	WY_pKnlDesc->WY_ulHighSegBase = (ulong)(&WY_TimeTSS) >> 24;

	WY_pStack  = mallock(0x100000); //申请时钟中断堆栈
	wyos_close_int();
	
	WY_TimeTSS.WY_ulCR3 = WY_PROCTABLE[0].WY_ulPDTPhy;
	WY_TimeTSS.WY_ulEIP = (ulong)TimeInterrupt;
	WY_TimeTSS.WY_ulEFLAGS = 0x202;
	WY_TimeTSS.WY_ulES = 0x10;
	WY_TimeTSS.WY_ulCS = 0x8;
	WY_TimeTSS.WY_ulDS = 0x10;
	WY_TimeTSS.WY_ulFS = 0x10;
	WY_TimeTSS.WY_ulGS = 0x10;
	WY_TimeTSS.WY_ulSS = 0x10;
	WY_TimeTSS.WY_ulESP = (ulong)WY_pStack + 0x100000;

	SetInterrupt(0, 0x20,0, GATE_TASK, 0x20);
	OpenHardInt(0);
}
/*-----------------------可编程定时器操作END-----------------------------*/

