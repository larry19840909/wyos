/***************************************************************************
			WYOS block.c
			�������Դ�ļ�
						����:WY.lslrt				editor	 :WY.lslrt
						����:2006/6/29			date	 	 :2006/6/29
						��Ȩ:WY �� WY.lslrt���� 	copyright  :WY and WY.lslrt
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


extern WY_ProcTable		WY_PROCTABLE[MAX_PROC_NUM];
extern ushort				WY_usCurrentPID;
extern WY_pProcTable		WY_PRIQueue[WYOS_PROC_PRI_KIND_NUM];


WY_pBlockHead			WY_BlockQueue[WYOS_PROC_BLOCK_TYPE_NUM];



ulong BlockThread(ulong WY_BlockType, ulong WY_ulTID, ulong WY_ulWaitTime, ulong WY_ulWaitParam1, ulong WY_ulWaitParam2)
{
	ushort			WY_usPID = GetCurrentPID();
	ulong			WY_ulResult,WY_usTID = GetCurrentTID(WY_usPID),WY_ulUsealbeTID;
	WY_pBlockHead	WY_pNode;
	WY_pSystemDesc  WY_pUTTssDesc;			//usable thread : UT

	__asm__("pushf");
	wyos_close_int();

	if(WY_PROCTABLE[WY_usPID].WY_pThreadCtr[WY_ulTID].WY_usThreadState  == WYOS_THREAD_WAIT)
	{
		//���߳��Ѿ���������
		return 0;
	}
	//��������ڵ�ռ�
	WY_pNode = (WY_pBlockHead)mallocs(sizeof(WY_BlockNode));
//	printk("Node : %x\n",WY_pNode);
	if(WY_pNode == NULL)			//����ʧ��
	{
		__asm__("popf");
		return 0;
	}

	//������뵽��Ӧ�ĵȴ�������
	WY_pNode->WY_ulPID = WY_usPID;
	WY_pNode->WY_ulWaitTID = WY_ulTID;
	WY_pNode->WY_ulWaitTime = WY_ulWaitTime;
	WY_pNode->WY_ulWaitParam1 = 0;					//0��ʾ������߳�����
	WY_pNode->WY_ulWaitParam2 = WY_ulWaitParam2;
	WY_pNode->WY_ulCompleteState = WYOS_BLOCK_STATE_WAIT;
	WY_pNode->WY_pNextNode = WY_BlockQueue[WY_BlockType];
	WY_BlockQueue[WY_BlockType] = WY_pNode;
	//�����߳�״̬
	WY_PROCTABLE[WY_usPID].WY_pThreadCtr[WY_ulTID].WY_usThreadState = WYOS_THREAD_WAIT;

	WY_PROCTABLE[WY_usPID].WY_ulThreadNum--;

	
	if(WY_ulTID == WY_usTID)		//����������Լ�����ôת���¸��߳�ִ�С�
	{
		WY_pNode->WY_ulWaitParam1 = 1;	//��ʾ���Լ�����
		
		if(WY_PROCTABLE[WY_usPID].WY_ulThreadNum == 0)
		{
			//û�п���ִ�е��̣߳�ת���¸����̽���
			//���ȴӾ���������ɾ���ý���,���ҵ�һ�������õĽ���
			WY_PRIQueue[WY_PROCTABLE[WY_usPID].WY_ulCurPriQueue] = WY_PROCTABLE[WY_usPID].WY_pNextTable;
			WY_PROCTABLE[WY_usPID].WY_pNextTable = NULL;
			WY_PROCTABLE[WY_usPID].WY_ulUseableTime = WY_PROCTABLE[WY_usPID].WY_ulTimeslice;
			WY_PROCTABLE[WY_usPID].WY_usProcState = WYOS_PROC_STATE_BLOCK;
			WY_PROCTABLE[WY_usPID].WY_ulCurPriQueue = WY_PROCTABLE[WY_usPID].WY_ulPriority;
			
			WY_usCurrentPID = MasterScheudler();
//			printk("PriQ %x WY_usPID %d CurPID %d\n",WY_PRIQueue[WY_PROCTABLE[WY_usPID].WY_ulCurPriQueue],WY_usPID,WY_usCurrentPID);
		}
		//��Ŀ�����״̬Ϊ��ִ��
		WY_PROCTABLE[WY_usCurrentPID].WY_usProcState = WYOS_PROC_STATE_RUN;
		//ת���¸��߳�ִ��
		WY_PROCTABLE[WY_usPID].WY_pThreadCtr[WY_ulTID].WY_ThreadTss.WY_ulBLink = TaskScheudler(WY_usCurrentPID,&WY_PROCTABLE[WY_usCurrentPID].WY_ulCurTID);
		
		WY_ulUsealbeTID = WY_PROCTABLE[WY_usCurrentPID].WY_ulCurTID;
		//�����������̵߳�TSS��ΪBUSY
		WY_pUTTssDesc = (WY_pSystemDesc)((ulong)WY_PROCTABLE[WY_usCurrentPID].WY_pThreadCtr[WY_ulUsealbeTID].WY_usTSSSel + WYOS_GDT_BASE);
		WY_pUTTssDesc->WY_ulSegTYPE = SYSTEM_BUSY_386TSS;

//		printk("next TID %d\n",WY_PROCTABLE[WY_usCurrentPID].WY_ulCurTID);
		__asm__("pushl %ebp\n\t"
		"mov  %esp,%ebp\n\t"
		"pushf\n\t"
		"orl $0x4000,-4(%ebp)\n\t"
		"popf\n\t"
		"popl %ebp\n\t"
		"iret");
		__asm__("popf");
		//���̱߳�����
		WY_ulResult = WY_pNode->WY_ulCompleteState;
		frees(WY_pNode);

		return WY_ulResult;
	}
	wyos_restore_flag();
	return 0;
	
}


ulong UnblockThread(ushort WY_usPID, ulong WY_ulTID, ulong WY_BlockType, ulong WY_ulUnblockReason)
{
	WY_pBlockHead	WY_pNode = WY_BlockQueue[WY_BlockType],WY_pPreNode = WY_BlockQueue[WY_BlockType];
	WY_pProcTable	WY_pPreTable;
	WY_pSystemDesc	WY_pThreadTssDesc;
	int				i;

	if(WY_PROCTABLE[WY_usPID].WY_pThreadCtr[WY_ulTID].WY_usThreadState != WYOS_THREAD_WAIT)
	{
		//���߳�û�б�����
		return 0;
	}
	
	while(WY_pNode)
	{
		if(WY_pNode->WY_ulPID == WY_usPID && WY_pNode->WY_ulWaitTID == WY_ulTID)
		{
			WY_pNode->WY_ulCompleteState = WY_ulUnblockReason;
			//�Ӹö�����ɾ��
			if(WY_pNode == WY_BlockQueue[WY_BlockType])
			{
				WY_BlockQueue[WY_BlockType] = WY_pNode->WY_pNextNode;
			}
			else
			{
				WY_pPreNode->WY_pNextNode = WY_pNode->WY_pNextNode;
			}
			
			if(WY_pNode->WY_ulWaitParam1 == 0)
			{
				//��������������ô��Ӧ��Ϊ���ͷŸýڵ�
				frees((void *)WY_pNode);
			}
			

			wyos_save_flag();
			wyos_close_int();
//			printk("PID %d state %d\n",WY_usPID,WY_PROCTABLE[WY_usPID].WY_usProcState);
			//����ý��̴���BLOCK ״̬��
			//������̷����������
			if(WY_PROCTABLE[WY_usPID].WY_usProcState == WYOS_PROC_STATE_BLOCK)
			{
				WY_PROCTABLE[WY_usPID].WY_usProcState = WYOS_PROC_STATE_READY;
				if(WY_PRIQueue[WY_PROCTABLE[WY_usPID].WY_ulCurPriQueue] == NULL)
				{
					WY_PROCTABLE[WY_usPID].WY_pNextTable = WY_PRIQueue[WY_PROCTABLE[WY_usPID].WY_ulCurPriQueue];
					WY_PRIQueue[WY_PROCTABLE[WY_usPID].WY_ulCurPriQueue] = &WY_PROCTABLE[WY_usPID];
				}
				else
				{
					WY_pPreTable = WY_PRIQueue[WY_PROCTABLE[WY_usPID].WY_ulCurPriQueue];
					while(WY_pPreTable->WY_pNextTable)
					{
						WY_pPreTable = WY_pPreTable->WY_pNextTable;
					}
					WY_pPreTable->WY_pNextTable = &WY_PROCTABLE[WY_usPID];
					WY_PROCTABLE[WY_usPID].WY_pNextTable = NULL;
				}
			}
			WY_PROCTABLE[WY_usPID].WY_ulThreadNum++;
			
			//�޸��߳�״̬
			WY_PROCTABLE[WY_usPID].WY_pThreadCtr[WY_ulTID].WY_usThreadState = WYOS_THREAD_RUNABLE;
			//�޸�����Ϊæ����
			WY_pThreadTssDesc = (WY_pSystemDesc)((ulong)WY_PROCTABLE[WY_usPID].WY_pThreadCtr[WY_ulTID].WY_usTSSSel+ WYOS_GDT_BASE);
			WY_pThreadTssDesc->WY_ulSegTYPE = SYSTEM_BUSY_386TSS;			
			wyos_restore_flag();
			return 1;
		}
		else
		{
			WY_pPreNode = WY_pNode;
			WY_pNode = WY_pNode->WY_pNextNode;
		}
	}
	return 0;
}
	
void CheckBlockQueue()
{
	int i;
	WY_pBlockHead	WY_pNode,WY_pPreNode;

	for(i = WYOS_PROC_BLOCK_TYPE_KEYBOARD;i < WYOS_PROC_BLOCK_TYPE_NUM;i++)
	{
		WY_pNode = WY_BlockQueue[i];
		while(WY_pNode)
		{
			if(WY_pNode->WY_ulWaitTime != WYOS_BLOCK_WAITTIME_INFINITY)
			{
				WY_pNode->WY_ulWaitTime -= 55;
//				printk("wait pid %d tid %d lasttime %d\n",WY_pNode->WY_ulPID,WY_pNode->WY_ulWaitTID,WY_pNode->WY_ulWaitTime);
//				while(1);
				if(WY_pNode->WY_ulWaitTime <= 0)		//��ʱ���������
				{
					WY_pNode->WY_ulCompleteState = WYOS_BLOCK_STATE_TIMEOUT;
					WY_pPreNode = WY_pNode;
					WY_pNode = WY_pNode->WY_pNextNode;
					UnblockThread(WY_pPreNode->WY_ulPID,WY_pPreNode->WY_ulWaitTID,i, WYOS_BLOCK_STATE_TIMEOUT);
					continue;
				}
			}
			WY_pNode = WY_pNode->WY_pNextNode;
		}
	}
}

ulong SleepKnl(pSyscallParam WY_pInputParam)
{
	BlockThread(WYOS_PROC_BLOCK_TYPE_SLEEP,GetCurrentTID(GetCurrentPID()),WY_pInputParam->WY_ulParam0,0,0);
	return 0;
}
//newline
