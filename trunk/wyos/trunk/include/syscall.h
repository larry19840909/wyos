/***************************************************************************
			WYOS syscall.h
			ϵͳ����ͷ�ļ�
						����:WY.lslrt				editor	 :WY.lslrt
						����:2006/6/29			date	 	 :2006/6/29
						��Ȩ:WY �� WY.lslrt���� 	copyright  :WY and WY.lslrt
						URL:http://www.wyos.net http://wylslrt.go.3322.org
***************************************************************************/
#ifndef	_WYOS_SYSCALL_H
#define	_WYOS_SYSCALL_H

//ϵͳ���ú��������ṹ
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
//ϵͳ����ע������ŵ�ѡ����
#define	WYOS_SYSCALL_GATE_SEL	0x80
//
//��ʼ��ϵͳ���ú���
//
void wyos_init_syscall();

//
//ϵͳ����C�����ӿڣ��ɵ����ŵ���
//
ulong SysCall_C_Func(pSyscallParam WY_pInputParam);

//
//ϵͳ���ð�װ�ӿ�
//
ulong SetupSyscall(SysCallProc WY_procInterProc,ulong WY_ulCallIndex);
//
//����ϵͳ���õĺ�
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

