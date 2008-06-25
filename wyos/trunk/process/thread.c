/***************************************************************************
			WYOS thread.c
			�߳����Դ�ļ�
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

extern WY_ProcTable	WY_PROCTABLE[MAX_PROC_NUM];

ulong KCreateThread(THREAD_ROUTINE ThreadRoutine, PVOID WY_pParam, BOOL WY_bKnl)
{
	WY_pSystemDesc		WY_pTSSDesc = NULL;
	PVOID				WY_pThreadStack = NULL,WY_pKnlStack = NULL;
	ushort				WY_usCurPID = GetCurrentPID();
	int					i;

	//���������������
	WY_pTSSDesc = allocGlobalDesc(TRUE);
	if(WY_pTSSDesc == NULL)
	{
		return -1;
	}
	//�����̶߳�ջ
	if(WY_bKnl)
	{
		//�ں��߳�
		WY_pThreadStack = mallocs(0x800);
		if(WY_pThreadStack == NULL)
		{
			freeGlobalDesc(((ulong)WY_pTSSDesc - WYOS_GDT_BASE));
			return -1;
		}

	}
	else
	{
		//�û��̣߳������û���ջ
		WY_pThreadStack = mallocmem(0x800,FALSE,FALSE);
		if(WY_pThreadStack == NULL)
		{
			freeGlobalDesc(((ulong)WY_pTSSDesc - WYOS_GDT_BASE));
			return -1;
		}
		//�����ں˶�ջ
		WY_pKnlStack = mallock(0x800);
		if(WY_pKnlStack == NULL)
		{
			freeGlobalDesc(((ulong)WY_pTSSDesc - WYOS_GDT_BASE));
			freek(WY_pThreadStack);
			return -1;
		}
	}
	
	//�����̱߳�,��Ϊ��0���߳������̣߳���˴�1��ʼ
	for(i = 1;i <= MAX_THREAD_NUM;i++)
	{
		//�鿴�Ƿ�ʹ��
		if(!WY_PROCTABLE[WY_usCurPID].WY_pThreadCtr[i].WY_ulUseFlag)
		{
			//δʹ��,ѡ�ô˱�
			break;
		}
	}
	if(i > MAX_THREAD_NUM)
	{
		//û�п����̱߳�
		freeGlobalDesc(((ulong)WY_pTSSDesc - WYOS_GDT_BASE));
		if(WY_bKnl)
		{
			//�ں��̣߳��ͷ��ں˶�ջ
			frees(WY_pThreadStack);
		}
		else
		{
			//�û��̣߳��ͷ��û���ջ
			freemem(WY_pThreadStack,FALSE,FALSE);
			freek(WY_pKnlStack);
		}
		return -1;
	}
	else
	{
		//����TSS��
		memset((char*)&WY_PROCTABLE[WY_usCurPID].WY_pThreadCtr[i].WY_ThreadTss,0,sizeof(WY_TSS));

		if(!WY_bKnl)
		{
			memcpy((char*)&WY_PROCTABLE[WY_usCurPID].WY_pThreadCtr[i].WY_ThreadTss,(char*)&WY_PROCTABLE[WY_usCurPID].WY_pThreadCtr[0].WY_ThreadTss,sizeof(WY_TSS));
			WY_PROCTABLE[WY_usCurPID].WY_pThreadCtr[i].WY_ThreadTss.WY_ulESP0 = (ulong)WY_pKnlStack + 0x800;
		}
		else
		{
			WY_PROCTABLE[WY_usCurPID].WY_pThreadCtr[i].WY_ThreadTss.WY_ulEFLAGS = 0x202;
			WY_PROCTABLE[WY_usCurPID].WY_pThreadCtr[i].WY_ThreadTss.WY_ulCS = 0x8;
			WY_PROCTABLE[WY_usCurPID].WY_pThreadCtr[i].WY_ThreadTss.WY_ulSS = 0x10;
			WY_PROCTABLE[WY_usCurPID].WY_pThreadCtr[i].WY_ThreadTss.WY_ulDS = 0x10;
			WY_PROCTABLE[WY_usCurPID].WY_pThreadCtr[i].WY_ThreadTss.WY_ulES = 0x10;
			WY_PROCTABLE[WY_usCurPID].WY_pThreadCtr[i].WY_ThreadTss.WY_ulFS = 0x10;
			WY_PROCTABLE[WY_usCurPID].WY_pThreadCtr[i].WY_ThreadTss.WY_ulGS = 0x10;
			WY_PROCTABLE[WY_usCurPID].WY_pThreadCtr[i].WY_ThreadTss.WY_ulLDT = 0;
		}
		WY_PROCTABLE[WY_usCurPID].WY_pThreadCtr[i].WY_ThreadTss.WY_ulCR3 = WY_PROCTABLE[WY_usCurPID].WY_pThreadCtr[0].WY_ThreadTss.WY_ulCR3;
		WY_PROCTABLE[WY_usCurPID].WY_pThreadCtr[i].WY_ThreadTss.WY_ulEIP = (ulong)ThreadRoutine;
		WY_PROCTABLE[WY_usCurPID].WY_pThreadCtr[i].WY_ThreadTss.WY_ulESP = (ulong)WY_pThreadStack + 0x800;
		WY_PROCTABLE[WY_usCurPID].WY_pThreadCtr[i].WY_ThreadTss.WY_usBitmapOffset = sizeof(WY_TSS);
		//����I/O MAP������־
		WY_PROCTABLE[WY_usCurPID].WY_pThreadCtr[i].WY_ulIOMapEnd = 0xFF;
		//��������״̬��������
		WY_pTSSDesc->WY_ulPresent = 1;
		WY_pTSSDesc->WY_ulDescType = 0;
		WY_pTSSDesc->WY_ulSegTYPE = SYSTEM_BUSY_386TSS;
		WY_pTSSDesc->WY_ulDescDPL = SEGMENT_RING0;
		WY_pTSSDesc->WY_ulGranularity = 0;
		WY_pTSSDesc->WY_ulD = 0;
		WY_pTSSDesc->WY_ulSoftUse = 0;
		WY_pTSSDesc->WY_ulReserved = 0;
		//����TSS�λ�ַ������
		WY_pTSSDesc->WY_ulLowSegLimit = 0x69;			//105���ֽڣ�����Ӧ�ð���I/O MAP������־
		WY_pTSSDesc->WY_ulHighSegLimit = 0;
		WY_pTSSDesc->WY_ulLowSegBase = ((ulong)&WY_PROCTABLE[WY_usCurPID].WY_pThreadCtr[i].WY_ThreadTss) & 0xFFFFFF;
		WY_pTSSDesc->WY_ulHighSegBase = (((ulong)&WY_PROCTABLE[WY_usCurPID].WY_pThreadCtr[i].WY_ThreadTss)  & 0xFF000000) >> 24;

		WY_PROCTABLE[WY_usCurPID].WY_pThreadCtr[i].WY_ulUseFlag = 1;
		WY_PROCTABLE[WY_usCurPID].WY_pThreadCtr[i].WY_usThreadState = WYOS_THREAD_RUNABLE;
		WY_PROCTABLE[WY_usCurPID].WY_pThreadCtr[i].WY_usUseTime = THREAD_RUN_TIME;
		WY_PROCTABLE[WY_usCurPID].WY_pThreadCtr[i].WY_usTSSSel = (ulong)WY_pTSSDesc - WYOS_GDT_BASE;
		WY_PROCTABLE[WY_usCurPID].WY_ulThreadNum++;

		return i;
	}
}

ulong GetTIDSyscall()
{
	return GetCurrentTID(GetCurrentPID());
}

//new line
