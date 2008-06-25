/***************************************************************************
			WYOS message.h
			消息头文件
						编码:WY.lslrt				editor	 :WY.lslrt
						日期:2006/8/8				date	 :2006/8/8
						版权:WY 和 WY.lslrt所有 	copyright:WY and WY.lslrt
						URL:http://www.wyos.net http://wylslrt.go.3322.org
***************************************************************************/
#ifndef	WYOS_MSG_H
#define	WYOS_MSG_H

#include "syscall.h"

typedef	ulong	WPARAM;		//消息数字量参数
typedef	PVOID	LPARAM;		//消息指针参数

typedef	struct	_MSG
{
	ulong		WY_ulMsg;
	WPARAM		WY_wparam;
	LPARAM		WY_lparam;
	ulong		WY_time;
	ulong		WY_ulReserved[4];
}WY_MSG,*WY_PMSG;

#define		MSG_USER		0x100;		//用户自定义消息

ulong KSendMessage(ushort WY_usPID,ulong WY_ulMsg,WPARAM WY_wparam,LPARAM WY_lparam);

ulong KReciveMessage(pSyscallParam WY_pInputParam);

ulong SendMsgSyscall(pSyscallParam WY_pInputParam);
#endif
//newline
