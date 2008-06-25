/***************************************************************************
			WYOS userlib.h
			ϵͳ�����û��ӿ�ͷ�ļ�
						����:WY.lslrt				editor	 :WY.lslrt
						����:2006/6/29			date	 	 :2006/6/29
						��Ȩ:WY �� WY.lslrt���� 	copyright  :WY and WY.lslrt
						URL:http://www.wyos.net http://wylslrt.go.3322.org
***************************************************************************/
#ifndef	_WYOS_USERLIB_H
#define	_WYOS_USERLIB_H


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

//ϵͳ��������ӿ��ַ�
void userputc(char WY_cDisp);

//ϵͳ���ýӿ�����
void usercls();

//ϵͳ���ýӿڻ���Լ���PID
ulong GetPID();

//ϵͳ���ýӿڻ���Լ���TID
ulong GetTID();

//ϵͳ���ýӿڻ�ÿ����ڴ���Ϣ
ulong GetMemoryInfo();

//ϵͳ���������ڴ�
void *malloc(ulong WY_ulBlockSize);

//ϵͳ�����ͷ��ڴ�
void free(PVOID WY_pMMPTR);

//ϵͳ��������
void Sleep(ulong WY_ulWaitTime);

//ϵͳ����SendMessage
ulong SendMessage(ushort WY_usPID,ulong WY_ulMsg,WPARAM WY_wparam,LPARAM WY_lparam);

//ϵͳ����RecvMessage
ulong RecvMessage(WY_PMSG WY_pmsg);

void puts(char * WY_szDisp);

void putn(int WY_nDisp, BOOL WY_bFullPrn);

void putx(int WY_nDisp);

void printf(char * WY_pcfmt,...);

#endif
