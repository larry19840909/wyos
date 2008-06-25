/***************************************************************************
			WYOS syscall.c
			内核文件
						编码:WY.lslrt				editor	 :WY.lslrt
						日期:2006/6/29			date	 	:2006/6/29
						版权:WY 和 WY.lslrt所有 	copyright:WY and WY.lslrt
						URL:http://www.wyos.net http://wylslrt.go.3322.org
***************************************************************************/
#include "..\include\WYOSType.h"
#include "..\include\cpu.h"
#include "..\include\io.h"
#include "..\include\syscall.h"
#include "..\video\video.h"
//系统调用表
SysCallProc			wyos_syscall_func_table[WYOS_MAX_SYSCALL_NUM];

extern ulong SysCall(ulong WY_ulSyscallIndex,ulong WY_p0,ulong WY_p1,ulong WY_p2,
					   ulong WY_p3,ulong WY_p4,ulong WY_p5,ulong WY_p6);

void wyos_init_syscall()
{
	int				WY_i;
	WY_pGATEDesc	WY_pSyscallGate = (WY_pGATEDesc)(WYOS_GDT_BASE + WYOS_SYSCALL_GATE_SEL);
	
	for(WY_i = 0;WY_i < WYOS_MAX_SYSCALL_NUM;WY_i++)
	{
		wyos_syscall_func_table[WY_i] = NULL;
	}

	//添加调用门
	WY_pSyscallGate->WY_ulDescDPL = SEGMENT_RING3;
	WY_pSyscallGate->WY_ulDescType = GATE_DESC;
	WY_pSyscallGate->WY_ulPresent = 1;
	WY_pSyscallGate->WY_ulReserved = 0;
	WY_pSyscallGate->WY_ulSegType = GATE_386CALL;
	WY_pSyscallGate->WY_ulHighOffset = ((ulong)SysCall >> 16);
	WY_pSyscallGate->WY_ulLowOffset = ((ulong)SysCall & 0xFFFF);
	WY_pSyscallGate->WY_ulDescSelector = 0x8;
	WY_pSyscallGate->WY_ulUsParamCnt = 8;
}

ulong SysCall_C_Func(pSyscallParam WY_pInputParam)
{
	SyscallParam	WY_InputParam;

	WY_InputParam.WY_ulSysCallIndex = WY_pInputParam->WY_ulSysCallIndex;
	WY_InputParam.WY_ulParam0 = WY_pInputParam->WY_ulParam0;
	WY_InputParam.WY_ulParam1 = WY_pInputParam->WY_ulParam1;
	WY_InputParam.WY_ulParam2 = WY_pInputParam->WY_ulParam2;
	WY_InputParam.WY_ulParam3 = WY_pInputParam->WY_ulParam3;
	WY_InputParam.WY_ulParam4 = WY_pInputParam->WY_ulParam4;
	WY_InputParam.WY_ulParam5 = WY_pInputParam->WY_ulParam5;
	WY_InputParam.WY_ulParam6 = WY_pInputParam->WY_ulParam6;
	
	if(WY_pInputParam->WY_ulSysCallIndex < WYOS_MAX_SYSCALL_NUM)
		return	wyos_syscall_func_table[WY_pInputParam->WY_ulSysCallIndex](WY_pInputParam);

}

ulong SetupSyscall(SysCallProc WY_procInterProc, ulong WY_ulCallIndex)
{
	wyos_syscall_func_table[WY_ulCallIndex] = WY_procInterProc;
	return 0;
}
