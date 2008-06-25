/***************************************************************************
			WYOS message.c
			��ϢԴ�ļ�
						����:WY.lslrt				editor	 :WY.lslrt
						����:2005/12/2			date	 :2005/12/2
						��Ȩ:WY �� WY.lslrt���� 	copyright:WY and WY.lslrt
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
#include "..\include\message.h"

extern WY_ProcTable			WY_PROCTABLE[MAX_PROC_NUM];

ulong KSendMessage(ushort WY_usPID,ulong WY_ulMsg,WPARAM WY_wparam,LPARAM WY_lparam)
{
	ulong		e,l;

	if((!WY_PROCTABLE[WY_usPID].WY_usTableUse) || (!WY_usPID))		//�˽��̱�δʹ��,���ý��̲�����,�ں˽��̲�������Ϣ
	{
		return 0;
	}

	__asm__("pushf");
	wyos_close_int();
	e = WY_PROCTABLE[WY_usPID].WY_ulEarlyPos;
	l = WY_PROCTABLE[WY_usPID].WY_ulLastPos;

	if(e == l)	//��Ϣ����Ϊ�գ���ô���̿϶��������ˣ���д����Ϣ�󣬽�������ý���
	{
		l = (l + 1) % 30;
		
		WY_PROCTABLE[WY_usPID].WY_Msgbuf[l].WY_ulMsg = WY_ulMsg;
		WY_PROCTABLE[WY_usPID].WY_Msgbuf[l].WY_wparam = WY_wparam;
		WY_PROCTABLE[WY_usPID].WY_Msgbuf[l].WY_lparam = WY_lparam;
		WY_PROCTABLE[WY_usPID].WY_Msgbuf[l].WY_time = GetCurrentTick();
		
		WY_PROCTABLE[WY_usPID].WY_ulLastPos = l;
		//����Ŀ����̵��߳�
		//��Ϊ0���߳������̣߳�ֻ�����̲߳Ÿ�����Ϣ�Ĵ���
		UnblockThread(WY_usPID, 0,WYOS_PROC_BLOCK_TYPE_MSG,WYOS_BLOCK_STATE_COMPLETE);
	}
	else if(e == (l + 1) % 30)	//���ж����Ѿ����ˣ���ô����ľͱ����ǵ�
	{
		e = (e + 1) % 30;
		l = (l + 1) % 30;

		WY_PROCTABLE[WY_usPID].WY_ulEarlyPos = e;

		WY_PROCTABLE[WY_usPID].WY_Msgbuf[l].WY_ulMsg = WY_ulMsg;
		WY_PROCTABLE[WY_usPID].WY_Msgbuf[l].WY_wparam = WY_wparam;
		WY_PROCTABLE[WY_usPID].WY_Msgbuf[l].WY_lparam = WY_lparam;
		WY_PROCTABLE[WY_usPID].WY_Msgbuf[l].WY_time = GetCurrentTick();
		
		WY_PROCTABLE[WY_usPID].WY_ulLastPos = l;
	}
	else
	{
		l = (l + 1) % 30;
		
		WY_PROCTABLE[WY_usPID].WY_Msgbuf[l].WY_ulMsg = WY_ulMsg;
		WY_PROCTABLE[WY_usPID].WY_Msgbuf[l].WY_wparam = WY_wparam;
		WY_PROCTABLE[WY_usPID].WY_Msgbuf[l].WY_lparam = WY_lparam;
		WY_PROCTABLE[WY_usPID].WY_Msgbuf[l].WY_time = GetCurrentTick();
		
		WY_PROCTABLE[WY_usPID].WY_ulLastPos = l;
	}
	__asm__("popf");
	return 1;
}

ulong KReciveMessage(pSyscallParam WY_pInputParam)
{
	ulong		e,l;
	ushort		WY_usPID = GetCurrentPID();
	WY_PMSG	WY_pmsg = (WY_PMSG)WY_pInputParam->WY_ulParam0;
	
	__asm__("pushf");
	wyos_close_int();
	e = WY_PROCTABLE[WY_usPID].WY_ulEarlyPos;
	l = WY_PROCTABLE[WY_usPID].WY_ulLastPos;
	if(e == l)		//��Ϣ����Ϊ�գ��ȴ�
	{
		//�����߳�
		//��Ϊ0���߳������̣߳�ֻ�����̲߳Ÿ�����Ϣ�Ĵ���
		BlockThread(WYOS_PROC_BLOCK_TYPE_MSG,0,WYOS_BLOCK_WAITTIME_INFINITY, 0, 0);
	}
	//���д�ʱ��Ϊ��
	e = (e + 1) % 30;
	WY_PROCTABLE[WY_usPID].WY_ulEarlyPos = e;
	
	WY_pmsg->WY_ulMsg = WY_PROCTABLE[WY_usPID].WY_Msgbuf[e].WY_ulMsg;
	WY_pmsg->WY_wparam= WY_PROCTABLE[WY_usPID].WY_Msgbuf[e].WY_wparam;
	WY_pmsg->WY_lparam= WY_PROCTABLE[WY_usPID].WY_Msgbuf[e].WY_lparam;
	WY_pmsg->WY_time= WY_PROCTABLE[WY_usPID].WY_Msgbuf[e].WY_time;

	__asm__("popf");
	return 1;
	
}

ulong SendMsgSyscall(pSyscallParam WY_pInputParam)
{
	return KSendMessage(WY_pInputParam->WY_ulParam0, WY_pInputParam->WY_ulParam1,(WPARAM)WY_pInputParam->WY_ulParam2, (LPARAM)WY_pInputParam->WY_ulParam3);
}

//newline
