/***************************************************************************
			WYOS syscall.h
			系统调用头文件
						编码:WY.lslrt				editor	 :WY.lslrt
						日期:2006/6/29			date	 	 :2006/6/29
						版权:WY 和 WY.lslrt所有 	copyright  :WY and WY.lslrt
						URL:http://www.wyos.net http://wylslrt.go.3322.org
***************************************************************************/
#ifndef	_WYOS_SYSCALL_H
#define	_WYOS_SYSCALL_H

//系统调用函数参数结构
typedef struct 	_SYSCALL_PARAM
{
	ulong	WY_ulSysCallIndex;
	ulong	WY_ulParam0;
	ulong	WY_ulParam1;
	ulong 	WY_ulParam2;
	ulong	WY_ulParam3;
	ulong	WY_ulParam4;
	ulong	WY_ulParam5;
	ulong	WY_ulParam6;
}SyscallParam,*pSyscallParam;

typedef	ulong(* SysCallProc)(pSyscallParam WY_pInputParam);

#define	WYOS_MAX_SYSCALL_NUM	0x40
//系统调用注册调用门的选择子
#define	WYOS_SYSCALL_GATE_SEL	0x80
//
//初始化系统调用函数
//
void wyos_init_syscall();

//
//系统调用C函数接口，由调用门调用
//
ulong SysCall_C_Func(pSyscallParam WY_pInputParam);

//
//系统调用安装接口
//
ulong SetupSyscall(SysCallProc WY_procInterProc,ulong WY_ulCallIndex);
//
//调用系统调用的宏
//
#define	UseSyscall(SYSCALL_INDEX,P0,P1,P2,P3,P4,P5,P6,RET)	__asm__("pushl %8\n\t"		\
															    	"pushl %7\n\t"		\
															    	"pushl %6\n\t"		\
															    	"pushl %5\n\t"		\
															    	"pushl %4\n\t"		\
															    	"pushl %3\n\t"		\
															    	"pushl %2\n\t"		\
															    	"pushl %1\n\t"		\
															    	"lcall	   $0x80,$0\n\t"\
															    	"add	   $32,%%esp\n\t"	\
															    	:"=a"(RET):"g"(SYSCALL_INDEX),"g"(P0),"g"(P1),"g"(P2),"g"(P3),"g"(P4),"g"(P5),"g"(P6))



#endif

