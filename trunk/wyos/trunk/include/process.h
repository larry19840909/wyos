/***************************************************************************
			WYOS process.h
			�������ͷ�ļ�
						����:WY.lslrt				editor	 :WY.lslrt
						����:2006/6/29			date	 	 :2006/6/29
						��Ȩ:WY �� WY.lslrt���� 	copyright  :WY and WY.lslrt
						URL:http://www.wyos.net http://wylslrt.go.3322.org
***************************************************************************/
#ifndef	_WYOS_PROCESS_H
#define	_WYOS_PROCESS_H

#include "message.h"
#include "thread.h"

#define	MAX_PROC_NUM	32

#define	WYOS_PROC_ENTRY_POINT	0x80000000		//ҳĿ¼����512
#define	WYOS_PROC_STACK_ADDR	0x7FC00000		//ҳĿ¼����511

typedef	struct _PROCESS_TABLE
{
/*--���̿��Ʋ���--*/
	ulong					WY_ulPID;					//����PID
	ulong					WY_ulFID;					//������PID
	ulong					WY_ulPriority;				//�������ȼ�
	ulong					WY_ulCurPriQueue;			//��ǰ�������ȼ�����
	ulong					WY_ulTimeslice;				//����ʱ��Ƭ
	ulong					WY_ulUseableTime;			//����ʱ��Ƭ
	ushort					WY_usProcState:8;			//����״̬
	ushort					WY_usProcProperty:8;		//��������
	ushort					WY_usTableUse;				//���Ƿ�ʹ��,0δ�ã���0ʹ��
	struct _PROCESS_TABLE *	WY_pNextTable;				//��һ�����̱�,�����ȼ�������ʹ��
/*--����״̬����--*/
	ulong					WY_ulForceState;			//����״̬�����̺������Ϣֻ�ᷢ��ӵ�н���Ľ���
	ulong					WY_ulLDTSel;				//LDTѡ����
//	ulong					WY_ulTSSSel;				//TSSѡ����
	WY_SystemDesc			WY_LDT[16];					//LDT��
//	WY_TSS					WY_MainTSS;				//�������߳�
//	ulong					WY_ulIOMapEnd:8;			//TSS I/Oλͼ������־
//	ulong					WY_ulReserved:24;			//Ϊ�˱�֤���ֽڶ���,Ϊһ���ֽڵ�I/O������3�ֽ�
/*--�߳̿��Ʋ���--*/	
	ulong					WY_ulThreadNum;
	ulong					WY_ulCurTID;
	WY_pTHREAD				WY_pThreadCtr;				//�߳̿��Ʊ�
/*--�ڴ���Ʋ���--*/	
	WY_pMemRecord			WY_pMemFirstRec;			//�ڴ�ʹ��������ʼ���Ե�ַ
	ulong					WY_ulFirstUsePhy;			//........................................����.......
	WY_pMemRecord			WY_pMemLastRec;			//..............................�������Ե�ַ
	ulong					WY_ulLastUsePhy;			//........................................����.......
	WY_pMemRecord			WY_FirstRecTalbe;			//��һ�����ü�¼�����Ե�ַ
	ulong					WY_ulPDTPhy;				//ҳĿ¼�����ַ
	ulong*					WY_ulPDTLine;				//ҳĿ¼���Ե�ַ
	ulong					WY_ulPTELine[1024];			//1024��ҳ������Ե�ַ
/*--��Ϣ������--*/	
	WY_MSG					WY_Msgbuf[30];				//��Ϣ������
	ulong					WY_ulEarlyPos;				//�������Ϣ�ڻ������е�λ��
	ulong					WY_ulLastPos;				//���µ���Ϣ�ڻ������е�λ��
/*--�ļ�ϵͳ����--*/
	ulong					WY_ulVolumeType;			//������
	ulong					WY_ulFileOpenNum;			//�ļ�����Ŀ
	char						WY_CurFilePathName[256];	//��ǰ��������·��
	PVOID					WY_pFileQueue;
	PVOID					WY_pDirQueue;
	ulong					reserved[662];
}WY_ProcTable,*WY_pProcTable;


//�������ȼ�
#define	WYOS_PROC_PRI_KIND_NUM	6				//���ȼ�����

#define	WYOS_PROC_PRI_REALTIME	0
#define	WYOS_PROC_PRI_HIGH		1
#define	WYOS_PROC_PRI_MIDDLE		2
#define	WYOS_PROC_PRI_LOW		3
#define	WYOS_PROC_PRI_USEUP		4				//ʱ��Ƭ����
#define	WYOS_PROC_PRI_IDLE		5				//�����ȼ�����CPU����ʱ�ŵ���
													//����4�����ȼ��Ľ���û���ˣ�
													//�Ż�ȥ����
//����ʱ��Ƭ����				
#define	WYOS_PROC_TS_LONG		6
#define	WYOS_PROC_TS_MIDDLE		4
#define	WYOS_PROC_TS_SHORT		2
//����״̬
#define	WYOS_PROC_STATE_RUN		0x1			//����״̬
#define	WYOS_PROC_STATE_BLOCK	0x2			//����״̬,�����̶߳��������Ż��Ϊ��״̬
#define	WYOS_PROC_STATE_READY	0x4			//����״̬
//#define	WYOS_PROC_STATE_SLEEP	0x8			//����״̬�������˹�����
//��������
#define	WYOS_PROC_STATE_USER	0x4				//�û�����
#define	WYOS_PROC_STATE_SYSTEM	0x2				//ϵͳ�߳�
#define	WYOS_PROC_STATE_KERNEL	0x1				//ϵͳ�ں�

//��������
#define	WYOS_PROC_BLOCK_TYPE_NUM	0x10
//��������
#define	WYOS_PROC_BLOCK_TYPE_KEYBOARD	0x0
#define	WYOS_PROC_BLOCk_TYPE_FLOPPY		0x1
#define	WYOS_PROC_BLOCK_TYPE_HARDDISK	0x2		
#define	WYOS_PROC_BLOCK_TYPE_CDROM		0x3
#define	WYOS_PROC_BLOCK_TYPE_PE			0x4		//�ⲿ�豸
#define	WYOS_PROC_BLOCK_TYPE_SLEEP		0x5
#define	WYOS_PROC_BLOCK_TYPE_SOFTWARE	0x6
#define	WYOS_PROC_BLOCK_TYPE_EVENT		0x7
#define	WYOS_PROC_BLOCK_TYPE_MUTEX		0x8
#define	WYOS_PROC_BLOCK_TYPE_MSG		0x9

//�������нڵ�ṹ
typedef	struct	_BLOCK_QUEUE_NODE
{
	ulong	WY_ulPID;
	ulong	WY_ulWaitTID;
	int		WY_ulWaitTime;
	ulong	WY_ulWaitParam1;
	ulong	WY_ulWaitParam2;
	ulong	WY_ulCompleteState;
	struct 	_BLOCK_QUEUE_NODE * WY_pNextNode;
}WY_BlockNode,*WY_pBlockHead;

#define	WYOS_BLOCK_STATE_WAIT			0x0
#define	WYOS_BLOCK_STATE_TIMEOUT		0x1
#define	WYOS_BLOCK_STATE_COMPLETE		0x2
//#define	WYOS_BLOCK_STATE_OUTSTANDING	0x3		//�ڴ�״̬�£�������Ȼ��Ҫ�ȴ�
													//��Ϊ��������ܱ��ֽ׶�ִ����
													//������ʴ��̣�����������������
													//��Խ�˴ŵ�����ʱ��������ָ����
													//������
#define	WYOS_BLOCK_STATE_ERROR			0x4		//���󣬵ȴ����¼�δ�ɹ�

#define	WYOS_BLOCK_WAITTIME_INFINITY	0xFFFFFFFF
#define	WYOS_BLOCK_WAITTIME_IMMEDIATE	0x0

//
//��ʼ������ģ�飬���ҳ�ʼ���ں˵Ľ��̱�
//
void InitProc();

//
//��õ�ǰ����PID
//
ushort GetCurrentPID();

//
//������̱�
//
WY_pProcTable AllocProcTable(ulong *WY_ulPID);

//
//��ʼ�����̿��Ʊ�
//
void	InitProcCtrTable(WY_pProcTable WY_pTablePtr,ulong WY_ulFID,ulong WY_ulPID,
					     ulong WY_ulPriority,ulong WY_ulTimeslice,ushort WY_usProcProperty);

//
//������̱�
//
BOOL ConstructProcTable(WY_pProcTable WY_pTablePtr,ushort WY_usProcProperty,
							ulong WY_ulTextPhyAddr,ulong WY_ulTextRange);

//
//�����Ⱥ�����������һ������ִ�еĽ���PID
//
ushort MasterScheudler(); 

//
//������Ⱥ�������������ִ�н��̵�һ���߳�
//����ѡ���Ӹ�������MasterScheudler��������ã�
//��֧���̵߳İ汾��ʵ�֡�
//
ulong TaskScheudler(ushort WY_usPID,ulong *WY_ulUseableTID);

//
//�����Լ��ĺ���
//
//ulong Block();
//
//�������ָ�����̵ĺ���
//
//ulong Unblock(ushort WY_usPID,ulong WY_BlockType,ulong WY_ulUnblockReason);


//
//ϵͳ����Sleep�ں˲���
//
ulong SleepKnl(pSyscallParam WY_pInputParam);

//
//�������ȼ�����
//
//void InserttoPri(ushort WY_usPID);

//
//�����ȼ�������ɾ��
//
//void DeleteinPri(ushort WY_usPID);
#endif

