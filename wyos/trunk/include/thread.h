/***************************************************************************
			WYOS thread.h
			�߳����ͷ�ļ�
						����:WY.lslrt				editor	 :WY.lslrt
						����:2006/6/29			date	 	 :2006/6/29
						��Ȩ:WY �� WY.lslrt���� 	copyright  :WY and WY.lslrt
						URL:http://www.wyos.net http://wylslrt.go.3322.org
***************************************************************************/

#ifndef	_WYOS_THREAD_H
#define	_WYOS_THREAD_H

#define	MAX_THREAD_NUM	11
#define	THREAD_RUN_TIME	3

typedef	ulong (* THREAD_ROUTINE)(PVOID WY_pParam);

typedef	struct	_THREAD_TABLE
{
	THREAD_ROUTINE 	WY_ThreadRoutin;
	ushort			WY_usUseTime;
	ushort			WY_usTSSSel;
	ushort			WY_usThreadState;
	ushort			WY_usWaitTime;
	WY_TSS			WY_ThreadTss;
	ulong			WY_ulIOMapEnd:8;			//TSS I/Oλͼ������־
	ulong			WY_ulUseFlag:8;				
	ulong			WY_ulReserved:16;
}WY_THREAD,*WY_pTHREAD;

#define	WYOS_THREAD_NO		0x0
#define	WYOS_THREAD_RUN		0x1
#define	WYOS_THREAD_RUNABLE	0x2
#define	WYOS_THREAD_WAIT		0x3

#define	GetCurrentTID(WY_PID) (WY_PROCTABLE[WY_PID].WY_ulCurTID)

ulong KCreateThread(THREAD_ROUTINE ThreadRoutine,PVOID WY_pParam,BOOL WY_bKnl);

//
//�����̵߳ĺ���
//
ulong BlockThread(ulong WY_BlockType,ulong WY_ulTID,ulong WY_ulWaitTime,ulong WY_ulWaitParam1,ulong WY_ulWaitParam2);

//
//�������ָ�����̵ĺ���
//
ulong UnblockThread(ushort WY_usPID,ulong WY_ulTID,ulong WY_BlockType,ulong WY_ulUnblockReason);

//
//����������п���û�г�ʱ�Ľ���
//
void CheckBlockQueue();

//
//��ȡ�߳�IDϵͳ����
//
ulong GetTIDSyscall();

#endif

