/***************************************************************************
			WYOS message.h
			��Ϣͷ�ļ�
						����:WY.lslrt				editor	 :WY.lslrt
						����:2006/8/8				date	 :2006/8/8
						��Ȩ:WY �� WY.lslrt���� 	copyright:WY and WY.lslrt
						URL:http://www.wyos.net http://wylslrt.go.3322.org
***************************************************************************/
#ifndef	WYOS_MSG_H
#define	WYOS_MSG_H

#include "syscall.h"

typedef	ulong	WPARAM;		//��Ϣ����������
typedef	PVOID	LPARAM;		//��Ϣָ�����

typedef	struct	_MSG
{
	ulong		WY_ulMsg;
	WPARAM		WY_wparam;
	LPARAM		WY_lparam;
	ulong		WY_time;
	ulong		WY_ulReserved[4];
}WY_MSG,*WY_PMSG;

#define		MSG_USER		0x100;		//�û��Զ�����Ϣ

ulong KSendMessage(ushort WY_usPID,ulong WY_ulMsg,WPARAM WY_wparam,LPARAM WY_lparam);

ulong KReciveMessage(pSyscallParam WY_pInputParam);

ulong SendMsgSyscall(pSyscallParam WY_pInputParam);
#endif
//newline
