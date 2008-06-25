/***************************************************************************
			WYOS userlib.h
			系统调用用户接口头文件
						编码:WY.lslrt				editor	 :WY.lslrt
						日期:2006/6/29			date	 	 :2006/6/29
						版权:WY 和 WY.lslrt所有 	copyright  :WY and WY.lslrt
						URL:http://www.wyos.net http://wylslrt.go.3322.org
***************************************************************************/
#ifndef	_WYOS_USERLIB_H
#define	_WYOS_USERLIB_H


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

//系统调用输出接口字符
void userputc(char WY_cDisp);

//系统调用接口清屏
void usercls();

//系统调用接口获得自己的PID
ulong GetPID();

//系统调用接口获得自己的TID
ulong GetTID();

//系统调用接口获得可用内存信息
ulong GetMemoryInfo();

//系统调用申请内存
void *malloc(ulong WY_ulBlockSize);

//系统调用释放内存
void free(PVOID WY_pMMPTR);

//系统调用休眠
void Sleep(ulong WY_ulWaitTime);

//系统调用SendMessage
ulong SendMessage(ushort WY_usPID,ulong WY_ulMsg,WPARAM WY_wparam,LPARAM WY_lparam);

//系统调用RecvMessage
ulong RecvMessage(WY_PMSG WY_pmsg);

void puts(char * WY_szDisp);

void putn(int WY_nDisp, BOOL WY_bFullPrn);

void putx(int WY_nDisp);

void printf(char * WY_pcfmt,...);

#endif
