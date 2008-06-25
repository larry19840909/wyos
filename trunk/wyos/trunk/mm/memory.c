/***************************************************************************
			WYOS Memory.c
			�ڴ����Դ�ļ�
						����:WY.lslrt				editor	 :WY.lslrt
						����:2005/12/2			date	 	 :2005/12/2
						��Ȩ:WY �� WY.lslrt����   copyright  :WY and WY.lslrt
						URL:http://www.wyos.net http://wylslrt.go.3322.org
***************************************************************************/
/**************************************************************************
			�޸�2006-6-6
			ȥ�����������Ļ���Ϊ�����ж�
			����Ч�ʸ��� 
***************************************************************************/
//��Ҫ�޸�,��Ҫ���л������
#include "..\include\WYOSType.h"
#include "..\include\syscall.h"
#include "..\include\cpu.h"
#include "..\video\video.h"
#include "..\include\string.h"
#include "..\include\memory.h"
#include "..\include\process.h"

extern WY_ProcTable	WY_PROCTABLE[MAX_PROC_NUM];
extern ushort			WY_usCurrentPID;


unsigned	int			WY_nUseableMemory = 0;		//�����ڴ��������ֽ�Ϊ��λ
unsigned int			WY_nPhyMemPagesNum = 0;		//���õ�����ҳ����Ŀ��
WY_pFreeMemRec		WY_pFreeMemFirstRec = NULL,WY_pFreeMemLastRec = NULL;//�����ڴ������ͷ��β
//�����ֱ�Ϊ����ҳ���ڴ����ʱ��ҳ��������ε�ַ��
//ҳ���ڴ���ʱ���ε�ַ
static unsigned long	*WY_ulTmpPE = NULL,*WY_ulTmpPELine = NULL;			

void	meminit()
{
	ulong *				WY_pulRangeAddr = (ulong *)0x94000;			//int15�ж�e802�Ź��ܷ��ؿ�ĸ�����ַ
	WY_pE802MM			WY_pMMAddr = (WY_pE802MM)0x94004;			//int15�ж�e802�Ź��ܷ��ؿ����ʼ��ַ
	WY_pMemRecord		WY_pMemUseRecord = MEMORY_USE_RECORD_START,WY_pTmpPERec = NULL;
	WY_pFreeMemRec		WY_pFreeMemRecord = MEMORY_FREE_RECORD_START;
	int					WY_i,WY_nTmp;					//WY_nTmp�ڳ�ʼ�������ʱ���ʾ��һ����¼
	ulong				*WY_nTest,*WY_nTest1,*WY_nTest2;


	//����ȫ�ֱ���
	WY_nUseableMemory = 0;		//�����ڴ��������ֽ�Ϊ��λ
	WY_nPhyMemPagesNum = 0;		//���õ�����ҳ����Ŀ��
	WY_pFreeMemFirstRec = NULL,WY_pFreeMemLastRec = NULL;//�����ڴ������ͷ��β
	//�����ֱ�Ϊ����ҳ���ڴ����ʱ��ҳ��������ε�ַ��
	//ҳ���ڴ���ʱ���ε�ַ
	WY_ulTmpPE = NULL;
	WY_ulTmpPELine = NULL;	

	printk("Scaning Physical Memory...\n");
	printk("Primary Memory Infomation:\n");
	printk("AddrBaseLow  AddrBaseHigh  AddrLengthLow  AddrLengthHigh  AddressType\n");

	//�����ǳ�ʼ���ڴ�ʹ������Ϳ�������
	//����ĸ���
	for(WY_i = 0x4000;WY_i < 0x8000;WY_i++)
		*(char *)WY_i = 0;
	WY_pMemUseRecord[0].WY_ulLinearAddress = 0x4000;
	WY_pMemUseRecord[0].WY_ulRangeSize = 0x1000;
	WY_pMemUseRecord[0].WY_ulPhysicalPage = 0x4;
	WY_pMemUseRecord[0].WY_ulRecordType = 1;
	WY_pMemUseRecord[0].WY_ulIdentity = 0;
	WY_pMemUseRecord[0].WY_Use.WY_TableCharacter.WY_ulNextTable = 0x5;
	WY_pMemUseRecord[0].WY_Use.WY_TableCharacter.WY_ulTableUsed = 2;

	//��¼��0x00000000 --- 0x00007FFF ���ڴ�ʹ��
	//����������
	ConstructUseRecord(&WY_pMemUseRecord[1],0x0,0x8000,0,1,0x4,2,1,TRUE);
	WY_PROCTABLE[0].WY_pMemFirstRec = &WY_pMemUseRecord[1];
	
	//��¼��0x00100000 --- 0x001FFFFF ���ں˴�����ռ�ռ�
	ConstructUseRecord(&WY_pMemUseRecord[2],0x100000,0x100000,0x100,2,0,0,0,TRUE);
	WY_PROCTABLE[0].WY_pMemLastRec = &WY_pMemUseRecord[2];
	
	WY_pMemUseRecord += 0x1000;
	
	WY_pMemUseRecord[0].WY_ulLinearAddress = 0x5000;
	WY_pMemUseRecord[0].WY_ulRangeSize = 0x1000;
	WY_pMemUseRecord[0].WY_ulPhysicalPage = 0x5;
	WY_pMemUseRecord[0].WY_ulRecordType = 1;
	WY_pMemUseRecord[0].WY_ulIdentity = 0;
	WY_pMemUseRecord[0].WY_Use.WY_TableCharacter.WY_ulNextTable = 0;
	WY_pMemUseRecord[0].WY_Use.WY_TableCharacter.WY_ulTableUsed = 0;

	WY_pMemUseRecord = MEMORY_USE_RECORD_START;
	
	WY_pFreeMemRecord[0].WY_Free.WY_Table.WY_ulThisTableId = 0x6;
	WY_pFreeMemRecord[0].WY_Free.WY_Table.WY_ulNextTable = 0x7;
	WY_pFreeMemRecord[0].WY_Free.WY_Table.WY_ulRecordUsed = 0;

	WY_pFreeMemRecord += 0x1000;
	
	WY_pFreeMemRecord[0].WY_Free.WY_Table.WY_ulThisTableId = 0x7;
	WY_pFreeMemRecord[0].WY_Free.WY_Table.WY_ulNextTable = 0;
	WY_pFreeMemRecord[0].WY_Free.WY_Table.WY_ulRecordUsed = 0;

	WY_pFreeMemRecord = MEMORY_FREE_RECORD_START;

	//��ȡint15 E802  �ṹ��Ϣ�������ں��Ѿ�ռ�õ��ڴ�
	//�����ڴ��������
	for(WY_i = 0;WY_i<*WY_pulRangeAddr;WY_i++)
	{
		
		if(WY_pMMAddr[WY_i].WY_ulAddrRangeType == ADDRESS_RANGE_MEMORY)
		{
			printk("%x    %x     %x     %x     %d  OS can use\n",WY_pMMAddr[WY_i].WY_ulBaseAddrLow,
									  WY_pMMAddr[WY_i].WY_ulBaseAddrHigh,
									  WY_pMMAddr[WY_i].WY_ulLengthLow,
									  WY_pMMAddr[WY_i].WY_ulLengthHigh,
									  WY_pMMAddr[WY_i].WY_ulAddrRangeType);
			if(WY_nUseableMemory < (WY_pMMAddr[WY_i].WY_ulBaseAddrLow + 
									  WY_pMMAddr[WY_i].WY_ulLengthLow))
				WY_nUseableMemory = WY_pMMAddr[WY_i].WY_ulBaseAddrLow + WY_pMMAddr[WY_i].WY_ulLengthLow;
			if(WY_pMMAddr[WY_i].WY_ulBaseAddrLow == 0)
			{
				ConstructFreeRecord(&WY_pFreeMemRecord[1],0x8000,WY_pMMAddr[WY_i].WY_ulLengthLow - 0x0008000,0x6,1,2,1);
				WY_nTmp = 2;
			}
			else if(WY_pMMAddr[WY_i].WY_ulBaseAddrLow == 0x00100000)
			{
				//�����������������
				//&WY_pFreeMemRecord[WY_nTmp],0x190000,WY_pMMAddr[WY_i].WY_ulLengthLow - 0x00090000,0x6,WY_nTmp,++WY_nTmp,0
				//��һ��������ȡ�����¼��ָ�룬�����ڶ�����������һ����¼��λ�ã�
				//������C   ���������������ջ�����������ڶ��������ı��ʽ����ִ����
				//���µ�һ��������ָ���������һ���������Ϊʲô���ڵ�һ�������Լ���
				ConstructFreeRecord(&WY_pFreeMemRecord[WY_nTmp++],0x200000,WY_pMMAddr[WY_i].WY_ulLengthLow - 0x00100000,0x6,WY_nTmp,WY_nTmp+1,0);
			}
			else
			{
				ConstructFreeRecord(&WY_pFreeMemRecord[WY_nTmp++],WY_pMMAddr[WY_i].WY_ulBaseAddrLow,WY_pMMAddr[WY_i].WY_ulLengthLow,0x6,WY_nTmp,WY_nTmp+1,0);
			}
		}
		else
		{
			printk("%x    %x     %x     %x     %d\n",WY_pMMAddr[WY_i].WY_ulBaseAddrLow,
									  WY_pMMAddr[WY_i].WY_ulBaseAddrHigh,
									  WY_pMMAddr[WY_i].WY_ulLengthLow,
									  WY_pMMAddr[WY_i].WY_ulLengthHigh,
									  WY_pMMAddr[WY_i].WY_ulAddrRangeType);
		}
	}
	WY_pFreeMemRecord[--WY_nTmp].WY_Free.WY_Record.WY_ulTablePosOfNextRecord = 0;
	WY_pFreeMemRecord[WY_nTmp].WY_Free.WY_Record.WY_ulTableOfNextRecord = 0;
	WY_pFreeMemRecord->WY_Free.WY_Table.WY_ulRecordUsed = WY_nTmp;
	WY_pFreeMemFirstRec = &WY_pFreeMemRecord[1];
	WY_pFreeMemLastRec = &WY_pFreeMemRecord[WY_nTmp];
	//����ڴ���Ϣ
	WY_nPhyMemPagesNum = WY_nUseableMemory >> 12;
	printk("Primary Memory Size : %dMB,Physical Memory Pages Number : %d\n",WY_nUseableMemory/(1024*1024),WY_nPhyMemPagesNum);
	
	//Ϊ����ҳ�����ʱ��ַ��������ҳ����
	WY_ulTmpPELine = (ulong*)0xF000;
	WY_ulTmpPE = (ulong *)(0x1000 + ((((ulong)WY_ulTmpPELine) & 0x3FF000) >> 12) * sizeof(ulong));
	*WY_ulTmpPE = 0x1;

	//����ҳ���쳣������
	SetInterrupt((INT_PROC)(ulong)PageException,0x8, SEGMENT_RING3,GATE_386INTERRUPT,0xE);
	//ע��ϵͳ����
	SetupSyscall((SysCallProc)SyscallGetMmInfo, 20);
	SetupSyscall((SysCallProc)SyscallMalloc, 21);
	SetupSyscall((SysCallProc)SyscallFree, 22);
	
}

void ConstructUseRecord(WY_pMemRecord WY_pMemUseRec, ulong WY_ulLinearAddr, ulong WY_ulBlockSize, ulong WY_ulPhyPageNum, ulong WY_ulRecNum, ulong WY_ulNextRecTable, ulong WY_ulNextRecNum, ulong WY_ulFirstFlag,BOOL WY_bKnl)
{
	WY_pMemUseRec->WY_ulLinearAddress = WY_ulLinearAddr;
	WY_pMemUseRec->WY_ulRangeSize = WY_ulBlockSize;
	WY_pMemUseRec->WY_ulPhysicalPage = WY_ulPhyPageNum;
	WY_pMemUseRec->WY_ulRecordType = 0;
	WY_pMemUseRec->WY_ulIdentity = WY_ulRecNum;
	WY_pMemUseRec->WY_Use.WY_RecordCharacter.WY_ulTableOfNextRecord = WY_ulNextRecTable;
	WY_pMemUseRec->WY_Use.WY_RecordCharacter.WY_ulTablePosOfNextRecord = WY_ulNextRecNum;
	WY_pMemUseRec->WY_Use.WY_RecordCharacter.WY_ulFlag = 1;
	WY_pMemUseRec->WY_Use.WY_RecordCharacter.WY_ulFirst = WY_ulFirstFlag;
	WY_pMemUseRec->WY_ulKNLFlag = WY_bKnl;

}

void ConstructFreeRecord(WY_pFreeMemRec WY_pFreeRecord, ulong WY_ulPhysicalAddress, ulong WY_ulBlockSize, ulong WY_ulTableOfNextRec, ulong WY_ulThisRecNum, ulong WY_ulNextRecNum,ulong WY_ulIsFirst)
{
	WY_pFreeRecord->WY_Free.WY_Record.WY_ulPhysicalAddress = WY_ulPhysicalAddress;
	WY_pFreeRecord->WY_Free.WY_Record.WY_ulRangeSize = WY_ulBlockSize;
	WY_pFreeRecord->WY_Free.WY_Record.WY_ulTableOfNextRecord = WY_ulTableOfNextRec;
	WY_pFreeRecord->WY_Free.WY_Record.WY_ulThisRecNum = WY_ulThisRecNum;
	WY_pFreeRecord->WY_Free.WY_Record.WY_ulTablePosOfNextRecord = WY_ulNextRecNum;
	WY_pFreeRecord->WY_Free.WY_Record.WY_ulUseFlag = 1;
	WY_pFreeRecord->WY_Free.WY_Record.WY_ulFirstFlag = WY_ulIsFirst;
	
}

//ҳ����ռ�õ��ڴ治��¼��ʹ�������ڣ�����
//ͨ��ҳ�����ε�ַ�������š���Ȼ�������
//ֻ����˵�ַ������ҳ���С�ǹ̶��ģ�����
//����ȷ��ʹ���˶����ڴ档
//�����ƴ���������PDT��ʱ,��������������
//ҳĿ¼���и���
//��Ϊÿ��ҳ��ĵ�һ��ָ�����ҳ��,����ÿ��
//����������4MB-4kB
ulong PhyToLinear(ulong WY_ulPhysicalAddress,int WY_nPageNum,BOOL WY_bKnl)
{
	WY_pFreeMemRec		WY_pFreeRec = WY_pFreeMemFirstRec;
	int					i,j,cnt = 0,sp =0 ,ep = 0;
	ulong				*WY_ulPTE = NULL,*WY_ulPDT = NULL,WY_ulPhyAddr;
	ushort				WY_usCurPID = GetCurrentPID();
	//��������ַ���ڵ�4M  ֮�£���ô��ֱ�ӷ��ظõ�ַ
	//��Ϊ�ں˵ĵ�һ��ҳ���ӳ������һ������
	__asm__("pushf");
	wyos_close_int();

	//������Ҫ���ĸ�ҳ��ʼ����
	//�ں������1 - 255��ҳ���û���256 - 1023��ҳ��
	if(WY_bKnl)
	{
		if(WY_ulPhysicalAddress + (ulong)(WY_nPageNum * 0x1000)< 0x400000)
		{
			__asm__("popf");
			return WY_ulPhysicalAddress;
		}
		sp = 1;
		ep = 256;
	}
	else
	{
		sp = 256;
		ep = 1024;
	}

	
	//Ѱ�ҿ��õ������ַ�ռ�
	//�����ҵ�һ�����õ�ҳ����
	for(i = sp;i < ep;i++)
	{
		WY_ulPTE = (ulong *)WY_PROCTABLE[WY_usCurPID].WY_ulPTELine[i];
//		printk("PID %d  KNL %d PTE Num %d PTE Line %x\n",WY_usCurPID,WY_bKnl,i,WY_ulPTE);

///*			
		if(WY_ulPTE)
		{
			for(j = 0;j < 1024;j++)
			{
				//��ҳ����δ��,��ô�Ϳ���������������ҳ�����Ƿ�Ҳδ��
				if(!WY_ulPTE[j])
				{
					cnt = 1;
					while(cnt < WY_nPageNum)
					{
						if(j + cnt < 1024)
						{
							if(WY_ulPTE[j + cnt])
								break;		//�Ѿ�ʹ��
							cnt++;
						}
						else
						{
							break;
						}
					}
					//����,�ڴ��Ѿ�ʹ�õ���һ����ʼ�鿴
					if(cnt < WY_nPageNum)
					{
						j = j + cnt ;
						continue;
					}
					//����ռ��㹻,ӳ��
					cnt = 0;
					while(cnt < WY_nPageNum)
					{
						//��0λ����λ��1,��2λ��дλ��Ϊ�ɶ�д1,��3λϵͳ�û�λȡWY_bKnl�ķ�
						WY_ulPTE[j + cnt] = (ulong)(((WY_ulPhysicalAddress + cnt * 0x1000)& 0xFFFFF000) | (!WY_bKnl << 2) | 0x3);
						cnt++;
					}
					//ӳ�����,���������ַ
					//ҳĿ¼������Ϊ�����ַ�ĸ�10λ
					//ҳ���������Ϊ�����ַ���м�10λ
					__asm__("popf");
					return (ulong)(((i << 22) & 0xFFC00000) + ((j << 12) & 0x003FF000));
				}
			}
		}
//	/*
		else
		{
			//��ҳĿ¼��δӳ��
			//��Ϊ�����ַ�ռ������������
			//�����Ŀ¼������ӳ��,��ôΪ�����ӳ�䡣
			//�����ڿ���������Ѱ��һ�����ÿռ䡣
				while(WY_pFreeRec)
				{
					if(WY_pFreeRec->WY_Free.WY_Record.WY_ulRangeSize >= 0x1000)
					{
						if(WY_pFreeRec->WY_Free.WY_Record.WY_ulPhysicalAddress == WY_ulPhysicalAddress)
						{
							//�ҵ��ĺ��ʵ��ڴ����Ҫ����ĳ�ͻ
							//����һ���ڴ����
							WY_pFreeRec = (WY_pFreeMemRec)(ulong)((WY_pFreeRec->WY_Free.WY_Record.WY_ulTableOfNextRecord << 12) + WY_pFreeRec->WY_Free.WY_Record.WY_ulTablePosOfNextRecord * MEM_FREE_RECORD_SIZE);
							continue;
						}
						if(WY_pFreeRec->WY_Free.WY_Record.WY_ulRangeSize > 0x1000)
						{
							WY_pFreeRec->WY_Free.WY_Record.WY_ulRangeSize -= 0x1000;
							WY_ulPhyAddr = WY_pFreeRec->WY_Free.WY_Record.WY_ulPhysicalAddress;
							WY_pFreeRec->WY_Free.WY_Record.WY_ulPhysicalAddress += 0x1000;
						}
						else
						{
							WY_ulPhyAddr = WY_pFreeRec->WY_Free.WY_Record.WY_ulPhysicalAddress >> 12;
							DeleteRec_free(WY_pFreeRec);
						}
						WY_nUseableMemory -= 0x1000;
						break;
					}
					WY_pFreeRec = (WY_pFreeMemRec)(ulong)((WY_pFreeRec->WY_Free.WY_Record.WY_ulTableOfNextRecord << 12) + WY_pFreeRec->WY_Free.WY_Record.WY_ulTablePosOfNextRecord * MEM_FREE_RECORD_SIZE);
				}
				
				if(WY_pFreeRec)//�ҵ�һ�������ڴ�
				{
					
					if(WY_bKnl)
					{
						//�ҵ�һ������ҳ��
						//��ӵ�ҳĿ¼����ȥ
						for(j = 0;j < MAX_PROC_NUM;j++)
						{
							if(WY_PROCTABLE[j].WY_usTableUse)
							{
								WY_PROCTABLE[j].WY_ulPDTLine[i] = WY_ulPhyAddr | 0x3;
							}
						}
						//�޸���ʱҳ����ָ���ҳ��
						*WY_ulTmpPE = WY_ulPhyAddr | 0x1;
						//�����������Ŀ�������WY_ulTmpPELine�����ַ��TLB
						//һ��ʼʹ���������TLB����,������Ӱ��Ч��,��
						//ֱ�����һ��,��������,��ĳЩ�����,������ʽ��
						//x86��������,���ڸ�����WY_ulTmpPE���ҳ�����е�����ҳ��
						//�ͻ������ΪTLBû��ˢ��,��WY_ulTmpPELine
						//�򻹻ᱻӳ�䵽��һ�ε�����ҳ����,����,�����
						//Ϊ�µ�ҳ����޸ľͻ������,������ǰ������ҳ��
						//���ᱻ�޸�
//						__asm__("mov %cr3,%eax\n\t"
//								"mov %eax,%cr3\n\t");
						__asm__("invlpg %0"::"m"(*WY_ulTmpPELine));
						//��ոñ�
						for(j = 0;j < 1024;j++)
						{
							WY_ulTmpPELine[j] = 0;
						}
						//�޸����ҳ��ĵ�һ�����ָ���Լ�
						*WY_ulTmpPELine = *WY_ulTmpPE;
						//����ҳ���µ����ε�ַ���뵽ҳ���ַ������
						//��Ϊ���ҳ�����ں˿ռ�,������н��̱����޸�
						for(j = 0;j < MAX_PROC_NUM;j++)
						{
							if(WY_PROCTABLE[j].WY_usTableUse)
							{
								WY_PROCTABLE[j].WY_ulPTELine[i] = (ulong)((ulong)i << 22);
							}
						}
						 
					}
					else
					{
						//����ҳĿ¼��
						WY_PROCTABLE[WY_usCurPID].WY_ulPDTLine[i] = WY_ulPhyAddr | 0x7;
						WY_PROCTABLE[WY_usCurPID].WY_ulPTELine[i] = PhyToLinear(WY_ulPhyAddr, 1, TRUE);
						memset((char*)(ulong*)WY_PROCTABLE[WY_usCurPID].WY_ulPTELine[i],0,4096);
					}
					//����ѭ��,��ʼ���²���ҳ����
					i--;
					continue;
				}
				//���пռ䲻��,����ӳ��
				__asm__("popf");
				return 0;
		}
		//*/
	}
	//����ռ�����,����ӳ��
	__asm__("popf");
	return 0;

}

ulong UnmappedLinear(ulong WY_ulLinearAddress, int WY_nPageNum,BOOL WY_bKnl)
{
	int			WY_i,WY_nPEIndex;
	ulong		*WY_ulPE;
	ushort		WY_usCurPID = GetCurrentPID();

	__asm__("pushf");
	wyos_close_int();
	//�ҵ�ҳ�����ε�ַ
	WY_ulPE = (ulong*)(WY_PROCTABLE[WY_usCurPID].WY_ulPTELine[(WY_ulLinearAddress & 0xFFC00000) >> 22]);
//	printk("unmap pid %d PTE %x",WY_usCurPID,WY_ulPE);
	//�ҵ�ҳ���е�ҳ��������
	WY_nPEIndex = (int)((WY_ulLinearAddress & 0x003FF000) >> 12);
	//����Щ�����ַ���ڵ���0x400000ʱ����Щҳ������Ϊ0;
	//��Ϊ��С��0x400000ʱ�����ַ�������ַ��һһ��Ӧ��;
	if((WY_ulLinearAddress + WY_nPageNum * 0x1000) >= 0x400000)
	{
		for(WY_i = 0;WY_i < WY_nPageNum;WY_i++)
		{
			WY_ulPE[WY_nPEIndex + WY_i] = 0;
		}
	}
	//ʹ��Щ�����ַ��TLB����Ч
	for(WY_i = 0;WY_i < WY_nPageNum;WY_i++)
	{
		WY_ulLinearAddress += WY_i * 0x1000;
		__asm__("invlpg %0"::"m"(*((ulong *)WY_ulLinearAddress)));
	}
	__asm__("popf");
}

ulong LineartToPhy(ulong WY_ulLinearAddress)
{
	ulong *		WY_pPTE = NULL;
	ushort		WY_usCurPID = GetCurrentPID();

	WY_pPTE = (ulong*)WY_PROCTABLE[WY_usCurPID].WY_ulPTELine[(WY_ulLinearAddress & 0xFFC00000) >> 22];
	return (((WY_pPTE[(WY_ulLinearAddress & 0x003FF000) >> 12]) & 0xFFFFF000) + (WY_ulLinearAddress & 0xFFF));
	
}

WY_pMemRecord allocrec_use(BOOL WY_bSystem)
{
	WY_pMemRecord		WY_pUseRecTable,WY_pTmpUse;
	WY_pFreeMemRec		WY_pFreeRecord = WY_pFreeMemFirstRec;//,WY_pTmpFree,WY_pTmpFreeTable;
	int					WY_i;
	ulong				WY_ulPhyAddr;

	//�����еļ�¼����Ѱ�ҿ��еļ�¼
	if(WY_bSystem)
	{
		WY_pUseRecTable = WY_PROCTABLE[0].WY_FirstRecTalbe;
	}
	else
	{
		WY_pUseRecTable = WY_PROCTABLE[GetCurrentPID()].WY_FirstRecTalbe;
	}
	__asm__("pushf");
	wyos_close_int();
	while(1)
	{
		if(WY_pUseRecTable->WY_Use.WY_TableCharacter.WY_ulTableUsed<255)
		{
			for(WY_i = 1;WY_i < NUM_USEREC_TABLE;WY_i++)
			{
				if(WY_pUseRecTable[WY_i].WY_Use.WY_RecordCharacter.WY_ulFlag == 0)
				{
					WY_pUseRecTable->WY_Use.WY_TableCharacter.WY_ulTableUsed++;
					__asm__("popf");
					return &WY_pUseRecTable[WY_i];
				}
			}
		}
		if(WY_pUseRecTable->WY_Use.WY_TableCharacter.WY_ulNextTable == 0) break;
		WY_pUseRecTable = (WY_pMemRecord)((ulong)WY_pUseRecTable->WY_Use.WY_TableCharacter.WY_ulNextTable << 12);
	}
	//��¼������,����������һ����¼��
	if(WY_pFreeRecord == NULL)
	{
		//���жϣ��������������ȡ
		__asm__("popf");
		return NULL;
	}
	//�ڿ��е������ڴ����ҵ�һ����¼���С�Ŀռ�
	while(WY_pFreeRecord->WY_Free.WY_Record.WY_ulTablePosOfNextRecord != 0)
	{
		if(WY_pFreeRecord->WY_Free.WY_Record.WY_ulRangeSize >= 0x1000)
		{
			//�ҵ���������һ�飬���޸���Щ��¼
			WY_ulPhyAddr = WY_pFreeRecord->WY_Free.WY_Record.WY_ulPhysicalAddress;
			WY_pTmpUse = (WY_pMemRecord)PhyToLinear(WY_ulPhyAddr,1,TRUE);
			if(WY_pFreeRecord->WY_Free.WY_Record.WY_ulRangeSize > 0x1000)
			{
				WY_pFreeRecord->WY_Free.WY_Record.WY_ulPhysicalAddress += 0x1000;
				WY_pFreeRecord->WY_Free.WY_Record.WY_ulRangeSize -= 0x1000;
			}
			else
			{
				//����ü�¼�Ŀ����ڴ�����Ϊһ����¼��
				//��ɾ�������¼
				DeleteRec_free(WY_pFreeRecord);
			}
			//����ü�¼��
			//�������һ����¼����¼�������ڴ�
			//���صڶ�����¼���Թ�����ʹ��
			WY_pTmpUse->WY_ulLinearAddress = (ulong)WY_pTmpUse;
			WY_pTmpUse->WY_ulRangeSize = 0x1000;
			WY_pTmpUse->WY_ulPhysicalPage = WY_ulPhyAddr >> 12;
			WY_pTmpUse->WY_ulRecordType = 1;
			WY_pTmpUse->WY_ulIdentity = 0;
			WY_pTmpUse->WY_Use.WY_TableCharacter.WY_ulNextTable = 0;
			WY_pTmpUse->WY_Use.WY_TableCharacter.WY_ulTableUsed = 2;
			WY_pUseRecTable->WY_Use.WY_TableCharacter.WY_ulNextTable = (ulong)(WY_pTmpUse )>> 12;
			ConstructUseRecord(&WY_pTmpUse[1],(ulong)WY_pTmpUse,0x1000,WY_ulPhyAddr>>12,1,0,0,0,TRUE);
			InsertRec_use(&WY_pTmpUse[1],WY_bSystem);

			WY_nUseableMemory -= 0x1000;
			__asm__("popf");
			return &WY_pTmpUse[2];
		}
		WY_pFreeRecord = (WY_pFreeMemRec)(ulong)((WY_pFreeRecord->WY_Free.WY_Record.WY_ulTableOfNextRecord << 12) + WY_pFreeRecord->WY_Free.WY_Record.WY_ulTablePosOfNextRecord * MEM_FREE_RECORD_SIZE);
	}
}

WY_pFreeMemRec allocrec_free()
{
	WY_pFreeMemRec		WY_pFreeTable = MEMORY_FREE_RECORD_START,WY_pNewTable;
	WY_pFreeMemRec		WY_pFreeRec = WY_pFreeMemFirstRec;
	WY_pMemRecord		WY_pUseRec;
	int					WY_i;
	ulong				WY_ulPhyAddr;

	//�����еı���ȥ����
	__asm__("pushf");
	wyos_close_int();
	while(1)//WY_pFreeTable->WY_Table.WY_ulNextTable != 0
	{
		if(WY_pFreeTable->WY_Free.WY_Table.WY_ulRecordUsed < 255)
		{
			for(WY_i = 1;WY_i < NUM_FREEREC_TABLE;WY_i++)
			{
				if(WY_pFreeTable[WY_i].WY_Free.WY_Record.WY_ulUseFlag == 0)
				{
					WY_pFreeTable->WY_Free.WY_Table.WY_ulRecordUsed++;

					__asm__("popf");
					return &WY_pFreeTable[WY_i];
				}
			}
		}
		if(WY_pFreeTable->WY_Free.WY_Table.WY_ulNextTable == 0) break;
		WY_pFreeTable = (WY_pFreeMemRec)(WY_pFreeTable->WY_Free.WY_Table.WY_ulNextTable << 12);
	}
	//���еı��Ѿ������꣬���ڴӿ��е������ڴ��з���һ��
	if(WY_pFreeRec == NULL) 
	{
		__asm__("popf");
		return NULL;	//��Ϊû�п��м�¼�ˣ���ô�ڴ�϶�������
	}
	while(WY_pFreeRec)
	{
		if(WY_pFreeRec->WY_Free.WY_Record.WY_ulRangeSize >= 0x1000)
		{
			WY_ulPhyAddr = WY_pFreeRec->WY_Free.WY_Record.WY_ulPhysicalAddress;
			WY_pNewTable = (WY_pFreeMemRec)PhyToLinear(WY_ulPhyAddr,1,TRUE);
			if(WY_pFreeRec->WY_Free.WY_Record.WY_ulRangeSize > 0x1000)
			{
				WY_pFreeRec->WY_Free.WY_Record.WY_ulRangeSize -= 0x1000;
				WY_pFreeRec->WY_Free.WY_Record.WY_ulPhysicalAddress += 0x1000;
			}
			else 
			{
				DeleteRec_free(WY_pFreeRec);	
			}
			WY_pNewTable->WY_Free.WY_Table.WY_ulThisTableId = (ulong)WY_pNewTable >> 12;
			WY_pNewTable->WY_Free.WY_Table.WY_ulRecordUsed = 1;
			WY_pNewTable->WY_Free.WY_Table.WY_ulNextTable = 0;
			WY_pFreeTable->WY_Free.WY_Table.WY_ulNextTable = WY_pNewTable->WY_Free.WY_Table.WY_ulThisTableId;

			WY_pUseRec = allocrec_use(TRUE);
			ConstructUseRecord(WY_pUseRec, (ulong)WY_pNewTable,0x1000,WY_ulPhyAddr >> 12, ((ulong)WY_pUseRec & 0xFFF) >> 4, 0,0,0,TRUE);
			InsertRec_use(WY_pUseRec,TRUE);
			WY_nUseableMemory -= 0x1000;
			__asm__("popf");
			return &WY_pNewTable[1];
		}
		WY_pFreeRec = (WY_pFreeMemRec)(ulong)((WY_pFreeRec->WY_Free.WY_Record.WY_ulTableOfNextRecord << 12) + WY_pFreeRec->WY_Free.WY_Record.WY_ulTablePosOfNextRecord * MEM_FREE_RECORD_SIZE);
	}
	__asm__("popf");
	return NULL;
}

void InsertRec_use(WY_pMemRecord WY_pMemUseRec,BOOL WY_bSystem)
{
	ushort				WY_usCurPID;
	WY_pMemRecord		WY_pMemRec,WY_pTmpRec;

	//�����¼����Ϊ�գ���ô��ʼ����
	if(WY_bSystem)
	{
		WY_usCurPID = 0;
	}
	else
	{
		WY_usCurPID = GetCurrentPID();
	}
	WY_pMemRec = WY_PROCTABLE[WY_usCurPID].WY_pMemFirstRec;
	WY_pTmpRec = WY_PROCTABLE[WY_usCurPID].WY_pMemFirstRec;

	__asm__("pushf");
	wyos_close_int();
	if(WY_PROCTABLE[WY_usCurPID].WY_pMemFirstRec == NULL)
	{
		WY_PROCTABLE[WY_usCurPID].WY_pMemFirstRec = WY_pMemUseRec;
		WY_PROCTABLE[WY_usCurPID].WY_pMemLastRec = WY_pMemUseRec;
		WY_pMemUseRec->WY_Use.WY_RecordCharacter.WY_ulFirst = 1;
		WY_pMemUseRec->WY_Use.WY_RecordCharacter.WY_ulTableOfNextRecord = 0;
		WY_pMemUseRec->WY_Use.WY_RecordCharacter.WY_ulTablePosOfNextRecord = 0;
		__asm__("popf");
		return;
	}
	//���������ַ˳�����ȣ�ҳ��ƫ�ƴ����ȵķ�ʽ
	//����ü�¼
	while(WY_pMemRec)
	{
		if(WY_pMemUseRec->WY_ulPhysicalPage <= WY_pMemRec->WY_ulPhysicalPage)
		{
			if(WY_pMemUseRec->WY_ulPhysicalPage == WY_pMemRec->WY_ulPhysicalPage)
			{
				if((WY_pMemUseRec->WY_ulLinearAddress & 0xFFF) > (WY_pMemRec->WY_ulLinearAddress & 0xFFF) )
				{
					WY_pTmpRec = WY_pMemRec;
					WY_pMemRec = (WY_pMemRecord)(ulong)((WY_pMemRec->WY_Use.WY_RecordCharacter.WY_ulTableOfNextRecord << 12) + WY_pMemRec->WY_Use.WY_RecordCharacter.WY_ulTablePosOfNextRecord * MEM_USE_RECORD_SIZE);
					continue;
				}
			}

			if(WY_pMemUseRec == WY_PROCTABLE[WY_usCurPID].WY_pMemFirstRec)
			{
				WY_pMemUseRec->WY_Use.WY_RecordCharacter.WY_ulTableOfNextRecord = (ulong)WY_PROCTABLE[WY_usCurPID].WY_pMemFirstRec >> 12;
				WY_pMemUseRec->WY_Use.WY_RecordCharacter.WY_ulTableOfNextRecord = WY_PROCTABLE[WY_usCurPID].WY_pMemFirstRec->WY_ulIdentity;
				WY_PROCTABLE[WY_usCurPID].WY_pMemFirstRec->WY_Use.WY_RecordCharacter.WY_ulFirst = 0;
				WY_pMemUseRec->WY_Use.WY_RecordCharacter.WY_ulFirst = 1;
				WY_PROCTABLE[WY_usCurPID].WY_pMemFirstRec = WY_pMemUseRec;
				__asm__("popf");
				return;
			}
			else
			{
				WY_pMemUseRec->WY_Use.WY_RecordCharacter.WY_ulTableOfNextRecord = (ulong)WY_pMemRec >> 12;
				WY_pMemUseRec->WY_Use.WY_RecordCharacter.WY_ulTablePosOfNextRecord = WY_pMemRec->WY_ulIdentity;
				WY_pTmpRec->WY_Use.WY_RecordCharacter.WY_ulTableOfNextRecord = (ulong)WY_pMemUseRec >> 12;
				WY_pTmpRec->WY_Use.WY_RecordCharacter.WY_ulTablePosOfNextRecord = WY_pMemUseRec->WY_ulIdentity;
				__asm__("popf");
				return;
			}
		}
		WY_pTmpRec = WY_pMemRec;
		WY_pMemRec = (WY_pMemRecord)(ulong)((WY_pMemRec->WY_Use.WY_RecordCharacter.WY_ulTableOfNextRecord << 12) + WY_pMemRec->WY_Use.WY_RecordCharacter.WY_ulTablePosOfNextRecord * MEM_USE_RECORD_SIZE);
	}
	//������ҳ��Ŵ����������еģ���ô������������β
	WY_PROCTABLE[WY_usCurPID].WY_pMemLastRec->WY_Use.WY_RecordCharacter.WY_ulTableOfNextRecord = (ulong)WY_pMemUseRec >> 12;
	WY_PROCTABLE[WY_usCurPID].WY_pMemLastRec->WY_Use.WY_RecordCharacter.WY_ulTablePosOfNextRecord = WY_pMemUseRec->WY_ulIdentity;
	WY_PROCTABLE[WY_usCurPID].WY_pMemLastRec = WY_pMemUseRec;
	WY_PROCTABLE[WY_usCurPID].WY_pMemLastRec->WY_Use.WY_RecordCharacter.WY_ulTableOfNextRecord = 0;
	WY_PROCTABLE[WY_usCurPID].WY_pMemLastRec->WY_Use.WY_RecordCharacter.WY_ulTablePosOfNextRecord = 0;
	__asm__("popf");
}

void InsertRec_free(WY_pFreeMemRec WY_pFreeMemRecord)
{
	WY_pFreeMemRec			WY_pFreeRec = WY_pFreeMemFirstRec,WY_pPreFreeRec = WY_pFreeMemFirstRec;

	__asm__("pushf");
	wyos_close_int();
	if(WY_pFreeRec == NULL)
	{
		WY_pFreeMemFirstRec = WY_pFreeMemRecord;
		WY_pFreeMemLastRec = WY_pFreeMemRecord;
		WY_pFreeMemRecord->WY_Free.WY_Record.WY_ulTableOfNextRecord = 0;
		WY_pFreeMemRecord->WY_Free.WY_Record.WY_ulTablePosOfNextRecord = 0;
		WY_pFreeMemRecord->WY_Free.WY_Record.WY_ulFirstFlag = 1;
		__asm__("popf");
		return;
	}

	while(WY_pFreeRec)
	{
		if(WY_pFreeMemRecord->WY_Free.WY_Record.WY_ulPhysicalAddress < WY_pFreeRec->WY_Free.WY_Record.WY_ulPhysicalAddress)
		{
			if(WY_pFreeRec == WY_pFreeMemFirstRec)
			{
				WY_pFreeMemRecord->WY_Free.WY_Record.WY_ulTableOfNextRecord = (ulong)WY_pFreeRec >> 12;
				WY_pFreeMemRecord->WY_Free.WY_Record.WY_ulTablePosOfNextRecord = WY_pFreeRec->WY_Free.WY_Record.WY_ulThisRecNum;
				WY_pFreeMemRecord->WY_Free.WY_Record.WY_ulFirstFlag = 1;
				WY_pFreeRec->WY_Free.WY_Record.WY_ulFirstFlag = 0;
				WY_pFreeMemFirstRec = WY_pFreeMemRecord;
				__asm__("popf");
				return;
			}
			else
			{
				WY_pFreeMemRecord->WY_Free.WY_Record.WY_ulTableOfNextRecord = WY_pPreFreeRec->WY_Free.WY_Record.WY_ulTableOfNextRecord;
				WY_pFreeMemRecord->WY_Free.WY_Record.WY_ulTablePosOfNextRecord = WY_pPreFreeRec->WY_Free.WY_Record.WY_ulTablePosOfNextRecord;
				WY_pPreFreeRec->WY_Free.WY_Record.WY_ulTableOfNextRecord = (ulong)WY_pFreeMemRecord >> 12;
				WY_pPreFreeRec->WY_Free.WY_Record.WY_ulTablePosOfNextRecord = WY_pFreeMemRecord->WY_Free.WY_Record.WY_ulThisRecNum;
				__asm__("popf");
				return;
			}
		}
		WY_pPreFreeRec = WY_pFreeRec;
		WY_pFreeRec = (WY_pFreeMemRec)((WY_pFreeRec->WY_Free.WY_Record.WY_ulTableOfNextRecord << 12) + WY_pFreeRec->WY_Free.WY_Record.WY_ulTablePosOfNextRecord * MEM_FREE_RECORD_SIZE);
	}

	WY_pFreeMemLastRec->WY_Free.WY_Record.WY_ulTableOfNextRecord = (ulong)WY_pFreeMemRecord;
	WY_pFreeMemLastRec->WY_Free.WY_Record.WY_ulTablePosOfNextRecord = WY_pFreeMemRecord->WY_Free.WY_Record.WY_ulThisRecNum;
	WY_pFreeMemRecord->WY_Free.WY_Record.WY_ulTableOfNextRecord = 0;
	WY_pFreeMemRecord->WY_Free.WY_Record.WY_ulTablePosOfNextRecord = 0;
	WY_pFreeMemLastRec = WY_pFreeMemRecord;
	__asm__("popf");
}

void DeleteRec_use(WY_pMemRecord WY_pMemUseRec,BOOL WY_bSystem)
{
	ushort				WY_usCurPID;
	WY_pMemRecord		WY_pMemUseRecord;

	if(WY_bSystem)
	{
		WY_usCurPID = 0;
	}
	else
	{
		WY_usCurPID = GetCurrentPID();
	}
	WY_pMemUseRecord = WY_PROCTABLE[WY_usCurPID].WY_pMemFirstRec;

	__asm__("pushf");
	wyos_close_int();
	if(WY_pMemUseRec == WY_PROCTABLE[WY_usCurPID].WY_pMemFirstRec)
	{
		WY_PROCTABLE[WY_usCurPID].WY_pMemFirstRec = (WY_pMemRecord)((WY_pMemUseRec->WY_Use.WY_RecordCharacter.WY_ulTableOfNextRecord << 12) + WY_pMemUseRec->WY_Use.WY_RecordCharacter.WY_ulTablePosOfNextRecord * MEM_USE_RECORD_SIZE);
		WY_PROCTABLE[WY_usCurPID].WY_pMemFirstRec->WY_Use.WY_RecordCharacter.WY_ulFirst = 1;
		WY_pMemUseRec->WY_Use.WY_RecordCharacter.WY_ulFlag = 0;
	}
	else if(WY_pMemUseRec == WY_PROCTABLE[WY_usCurPID].WY_pMemLastRec)
	{
		while(WY_pMemUseRecord->WY_Use.WY_RecordCharacter.WY_ulTableOfNextRecord != (ulong)WY_pMemUseRec >> 12 ||
			   WY_pMemUseRecord->WY_Use.WY_RecordCharacter.WY_ulTablePosOfNextRecord != WY_pMemUseRec->WY_ulIdentity)
			   WY_pMemUseRecord = (WY_pMemRecord)((WY_pMemUseRecord->WY_Use.WY_RecordCharacter.WY_ulTableOfNextRecord << 12) + WY_pMemUseRecord->WY_Use.WY_RecordCharacter.WY_ulTablePosOfNextRecord * MEM_USE_RECORD_SIZE);
		WY_PROCTABLE[WY_usCurPID].WY_pMemLastRec = WY_pMemUseRecord;
		WY_pMemUseRecord->WY_Use.WY_RecordCharacter.WY_ulTableOfNextRecord = 0;
		WY_pMemUseRecord->WY_Use.WY_RecordCharacter.WY_ulTablePosOfNextRecord = 0;
		WY_pMemUseRec->WY_Use.WY_RecordCharacter.WY_ulFlag = 0;
	}
	else
	{
		while(WY_pMemUseRecord->WY_Use.WY_RecordCharacter.WY_ulTableOfNextRecord != (ulong)WY_pMemUseRec >> 12 ||
			   WY_pMemUseRecord->WY_Use.WY_RecordCharacter.WY_ulTablePosOfNextRecord != WY_pMemUseRec->WY_ulIdentity)
			   WY_pMemUseRecord = (WY_pMemRecord)((WY_pMemUseRecord->WY_Use.WY_RecordCharacter.WY_ulTableOfNextRecord << 12) + WY_pMemUseRecord->WY_Use.WY_RecordCharacter.WY_ulTablePosOfNextRecord * MEM_USE_RECORD_SIZE);
		WY_pMemUseRecord->WY_Use.WY_RecordCharacter.WY_ulTableOfNextRecord = WY_pMemUseRec->WY_Use.WY_RecordCharacter.WY_ulTableOfNextRecord;
		WY_pMemUseRecord->WY_Use.WY_RecordCharacter.WY_ulTablePosOfNextRecord = WY_pMemUseRec->WY_Use.WY_RecordCharacter.WY_ulTablePosOfNextRecord;
		WY_pMemUseRec->WY_Use.WY_RecordCharacter.WY_ulFlag = 0;
	}
	WY_pMemUseRec = (WY_pMemRecord)(((ulong)WY_pMemUseRec >> 12) << 12);
	WY_pMemUseRec->WY_Use.WY_TableCharacter.WY_ulTableUsed --;
	__asm__("popf");
}

void DeleteRec_free(WY_pFreeMemRec WY_pFreeRec)
{
	WY_pFreeMemRec		WY_pFreePreRec = WY_pFreeMemFirstRec;


	__asm__("pushf");
	wyos_close_int();
	if(WY_pFreeRec == WY_pFreeMemFirstRec)
	{
		WY_pFreeMemFirstRec = (WY_pFreeMemRec)(ulong)((WY_pFreeRec->WY_Free.WY_Record.WY_ulTableOfNextRecord << 12) + WY_pFreeRec->WY_Free.WY_Record.WY_ulTablePosOfNextRecord * MEM_FREE_RECORD_SIZE);
		WY_pFreeMemFirstRec->WY_Free.WY_Record.WY_ulFirstFlag = 1;
		WY_pFreeRec->WY_Free.WY_Record.WY_ulUseFlag = 0;
	}
	else if(WY_pFreeRec == WY_pFreeMemLastRec)
	{
		while(WY_pFreePreRec->WY_Free.WY_Record.WY_ulTableOfNextRecord != ((ulong)WY_pFreeRec >> 12) || 
			   WY_pFreePreRec->WY_Free.WY_Record.WY_ulTablePosOfNextRecord != (ulong)(WY_pFreeRec->WY_Free.WY_Record.WY_ulThisRecNum))
			   WY_pFreePreRec = (WY_pFreeMemRec)(ulong)((WY_pFreePreRec->WY_Free.WY_Record.WY_ulTableOfNextRecord << 12) + WY_pFreePreRec->WY_Free.WY_Record.WY_ulTablePosOfNextRecord * MEM_FREE_RECORD_SIZE);
		
		WY_pFreeMemLastRec = WY_pFreePreRec;
		WY_pFreeMemLastRec->WY_Free.WY_Record.WY_ulTableOfNextRecord = 0;
		WY_pFreeMemLastRec->WY_Free.WY_Record.WY_ulTablePosOfNextRecord = 0;
		WY_pFreeRec->WY_Free.WY_Record.WY_ulUseFlag = 0;
	}
	else
	{
		while(WY_pFreePreRec->WY_Free.WY_Record.WY_ulTableOfNextRecord != ((ulong)WY_pFreeRec >> 12) || 
			   WY_pFreePreRec->WY_Free.WY_Record.WY_ulTablePosOfNextRecord != (ulong)(WY_pFreeRec->WY_Free.WY_Record.WY_ulThisRecNum))
			   WY_pFreePreRec = (WY_pFreeMemRec)(ulong)((WY_pFreePreRec->WY_Free.WY_Record.WY_ulTableOfNextRecord << 12) + WY_pFreePreRec->WY_Free.WY_Record.WY_ulTablePosOfNextRecord * MEM_FREE_RECORD_SIZE);

		WY_pFreePreRec->WY_Free.WY_Record.WY_ulTableOfNextRecord = WY_pFreeRec->WY_Free.WY_Record.WY_ulTableOfNextRecord;
		WY_pFreePreRec->WY_Free.WY_Record.WY_ulTablePosOfNextRecord = WY_pFreeRec->WY_Free.WY_Record.WY_ulTablePosOfNextRecord;
		WY_pFreeRec->WY_Free.WY_Record.WY_ulUseFlag = 0;
	}
	WY_pFreeRec = (WY_pFreeMemRec)(((ulong)WY_pFreeRec>>12)<<12);
	WY_pFreeRec->WY_Free.WY_Table.WY_ulRecordUsed--;
	__asm__("popf");
}

WY_pMemRecord AllocPage(int WY_nPageNum,BOOL WY_bKnl,BOOL WY_bSystem)
{
	WY_pFreeMemRec		WY_pFreeRec = WY_pFreeMemFirstRec;
	WY_pMemRecord		WY_pMemUse;
	ulong				WY_ulRangeSize = WY_nPageNum * 0x1000,WY_ulPhyPageNum,WY_ulLinearAddr;
	ushort				WY_usCurPID;

	if(WY_bSystem)
	{
		WY_usCurPID = 0;
	}
	else
	{
		WY_usCurPID = GetCurrentPID();
	}
//	printk("CurPID %d\n",GetCurrentPID());
//	printk("page num %x KNLFLAG %x\n",WY_nPageNum,WY_bKnl);
	__asm__("pushf");
	wyos_close_int();
	while(WY_pFreeRec)
	{
		if(WY_pFreeRec->WY_Free.WY_Record.WY_ulRangeSize >= WY_ulRangeSize)
		{
			if(WY_pFreeRec->WY_Free.WY_Record.WY_ulRangeSize > WY_ulRangeSize)
			{
				WY_ulLinearAddr = PhyToLinear(WY_pFreeRec->WY_Free.WY_Record.WY_ulPhysicalAddress,WY_nPageNum,WY_bKnl);
//				printk("RS %x\n",WY_pFreeRec->WY_Free.WY_Record.WY_ulRangeSize);
//				printk("Linear %x\n",WY_ulLinearAddr);
				if(!WY_ulLinearAddr)
				{
					__asm__("popf");
					return NULL;
				}
				
				WY_pFreeRec->WY_Free.WY_Record.WY_ulRangeSize -= WY_ulRangeSize;
				WY_ulPhyPageNum = WY_pFreeRec->WY_Free.WY_Record.WY_ulPhysicalAddress >> 12;
				WY_pFreeRec->WY_Free.WY_Record.WY_ulPhysicalAddress += WY_ulRangeSize;

				WY_pMemUse = allocrec_use(WY_bSystem);
				ConstructUseRecord(WY_pMemUse,WY_ulLinearAddr,WY_ulRangeSize, WY_ulPhyPageNum,((ulong)WY_pMemUse & 0xFFF)>>4, 0, 0, 0,WY_bKnl);
				InsertRec_use(WY_pMemUse,WY_bSystem);
				__asm__("popf");
				return WY_pMemUse;
			}
			else
			{
				WY_ulLinearAddr = PhyToLinear(WY_pFreeRec->WY_Free.WY_Record.WY_ulPhysicalAddress,WY_nPageNum,WY_bKnl);
//				printk("Linear %x\n",WY_ulLinearAddr);
				if(!WY_ulLinearAddr)
				{
					__asm__("popf");
					return NULL;
				}
				
				WY_ulPhyPageNum = WY_pFreeRec->WY_Free.WY_Record.WY_ulPhysicalAddress >> 12;

				DeleteRec_free(WY_pFreeRec);
				
				WY_pMemUse = allocrec_use(WY_bSystem);
				wyos_close_int();
				ConstructUseRecord(WY_pMemUse,WY_ulLinearAddr,WY_ulRangeSize, WY_ulPhyPageNum,((ulong)WY_pMemUse & 0xFFF)>>4, 0, 0, 0,WY_bKnl);
				InsertRec_use(WY_pMemUse,WY_bSystem);
				__asm__("popf");
				return WY_pMemUse;
			}
		}
		WY_pFreeRec = (WY_pFreeMemRec)(ulong)((WY_pFreeRec->WY_Free.WY_Record.WY_ulTableOfNextRecord << 12) + WY_pFreeRec->WY_Free.WY_Record.WY_ulTablePosOfNextRecord * MEM_FREE_RECORD_SIZE);
	}
	__asm__("popf");
	return NULL;
}

ulong AllocPhyPage(int WY_nPageNum)
{
	WY_pFreeMemRec WY_pFreeRec = WY_pFreeMemFirstRec;
	ulong			WY_ulPhyAddr = 0;

	__asm__("pushf");
	wyos_close_int();
	while(WY_pFreeRec)
	{
		if(WY_pFreeRec->WY_Free.WY_Record.WY_ulRangeSize >= 0x1000 * WY_nPageNum)
		{
			WY_ulPhyAddr = WY_pFreeRec->WY_Free.WY_Record.WY_ulPhysicalAddress;
			
			if(WY_pFreeRec->WY_Free.WY_Record.WY_ulRangeSize > 0x1000 * WY_nPageNum)
			{
				WY_pFreeRec->WY_Free.WY_Record.WY_ulRangeSize -= 0x1000 * WY_nPageNum;
				WY_ulPhyAddr = WY_pFreeRec->WY_Free.WY_Record.WY_ulPhysicalAddress;
				WY_pFreeRec->WY_Free.WY_Record.WY_ulPhysicalAddress += 0x1000 * WY_nPageNum;
			}
			else
			{
				WY_ulPhyAddr = WY_pFreeRec->WY_Free.WY_Record.WY_ulPhysicalAddress >> 12;
				DeleteRec_free(WY_pFreeRec);
			}
			WY_nUseableMemory -= 0x1000 * WY_nPageNum;
			__asm__("popf");
			return WY_ulPhyAddr;
		}
		WY_pFreeRec = (WY_pFreeMemRec)(ulong)((WY_pFreeRec->WY_Free.WY_Record.WY_ulTableOfNextRecord << 12) + WY_pFreeRec->WY_Free.WY_Record.WY_ulTablePosOfNextRecord * MEM_FREE_RECORD_SIZE);
	}
	__asm__("popf");
	return WY_ulPhyAddr;
}

void FreePage(ulong WY_ulPhysicalAddress,int WY_nPageNum,BOOL WY_bKnl)
{
	WY_pFreeMemRec		WY_pFreeRec;
	WY_pFreeRec = allocrec_free();
	ConstructFreeRecord(WY_pFreeRec, WY_ulPhysicalAddress,WY_nPageNum * 0x1000,0,((ulong)WY_pFreeRec & 0xFFF) >> 4, 0,0);
	InsertRec_free(WY_pFreeRec);
	ClearupFreeChain();
}


void ClearupFreeChain()
{
	WY_pFreeMemRec		WY_pFreeRec = WY_pFreeMemFirstRec,WY_pNextRec = (WY_pFreeMemRec)((WY_pFreeMemFirstRec->WY_Free.WY_Record.WY_ulTableOfNextRecord << 12)  + WY_pFreeMemFirstRec->WY_Free.WY_Record.WY_ulTablePosOfNextRecord * MEM_FREE_RECORD_SIZE);


	__asm__("pushf");
	wyos_close_int();
	while(WY_pNextRec)
	{
		if(WY_pFreeRec->WY_Free.WY_Record.WY_ulPhysicalAddress + WY_pFreeRec->WY_Free.WY_Record.WY_ulRangeSize 
	           == WY_pNextRec->WY_Free.WY_Record.WY_ulPhysicalAddress)
		{
			WY_pFreeRec->WY_Free.WY_Record.WY_ulRangeSize += WY_pNextRec->WY_Free.WY_Record.WY_ulRangeSize;
			
			
			DeleteRec_free(WY_pNextRec);

			WY_pNextRec = (WY_pFreeMemRec)((WY_pFreeRec->WY_Free.WY_Record.WY_ulTableOfNextRecord << 12)  + WY_pFreeRec->WY_Free.WY_Record.WY_ulTablePosOfNextRecord * MEM_FREE_RECORD_SIZE);
		}
		else
		{
			WY_pFreeRec = WY_pNextRec;
			WY_pNextRec = (WY_pFreeMemRec)((WY_pNextRec->WY_Free.WY_Record.WY_ulTableOfNextRecord << 12) + WY_pNextRec->WY_Free.WY_Record.WY_ulTablePosOfNextRecord * MEM_FREE_RECORD_SIZE);
		}
	}

	__asm__("popf");
}

void * mallocmem(int WY_nSize,BOOL WY_bKnl,BOOL WY_bSystem)
{
	int					WY_nMemSize,WY_nPageFreeMem;
	WY_pMemRecord		WY_pPreUse,WY_pUseRec;
	WY_pMemRecord		WY_pMemUse;

//	printk("CurPID %d\n",GetCurrentPID());
	if(WY_bSystem)
	{
		WY_pPreUse = WY_PROCTABLE[0].WY_pMemFirstRec;
		WY_pUseRec = (WY_pMemRecord)((WY_PROCTABLE[0].WY_pMemFirstRec->WY_Use.WY_RecordCharacter.WY_ulTableOfNextRecord << 12) 
									 + WY_PROCTABLE[0].WY_pMemFirstRec->WY_Use.WY_RecordCharacter.WY_ulTablePosOfNextRecord 
									 * MEM_USE_RECORD_SIZE);
	}
	else
	{
		WY_pPreUse = WY_PROCTABLE[GetCurrentPID()].WY_pMemFirstRec;
		WY_pUseRec = (WY_pMemRecord)((WY_PROCTABLE[GetCurrentPID()].WY_pMemFirstRec->WY_Use.WY_RecordCharacter.WY_ulTableOfNextRecord << 12)
									 + WY_PROCTABLE[GetCurrentPID()].WY_pMemFirstRec->WY_Use.WY_RecordCharacter.WY_ulTablePosOfNextRecord 
									 * MEM_USE_RECORD_SIZE);
	}
	
//	printk("First %x Next %x \n",WY_pPreUse,WY_pUseRec);
	//��Ϊ��С���䵥λ��4  �ֽ�,���¼���һ�´�С,�����ֽڶ���
	if(WY_nSize % 4 == 0) WY_nMemSize = WY_nSize;
	else
	{
		WY_nMemSize = ((WY_nSize / 4) + 1) * 4;
	}

	if(WY_nMemSize % 0x1000 == 0)
	{
		//��4K  �ı���,ֱ�ӵ��� AllocPage  ������
		WY_pMemUse = AllocPage(WY_nMemSize / 0x1000,WY_bKnl,WY_bSystem);
		WY_nUseableMemory -= WY_nMemSize;
		return (PVOID)WY_pMemUse->WY_ulLinearAddress;
	}
	else
	{
		__asm__("pushf");
		wyos_close_int();
		while(WY_pUseRec)
		{
			//���ж���Ϊ��ȷ����һ����¼�ǲ���ĳ������ҳ��
			//�����һ��ʹ�õļ�¼����Ϊ�˼���ҳ����Ƭ
			//��Ҫ��ÿ���Ѿ������ҳ���ڽ��з��䡣������Ҫ
			//�ҵ�ÿ���Ѿ������ҳ�������һ��ʹ�ü�¼��
			//�����ʣ��ռ䡣��Ϊ�����ǰ�������ҳ������
			//���еģ����������ж�WY_pPreUse  �Ƿ�Ϊ��ҳ�����һ��
			//ʹ�ü�¼
			//WY_pPreUse->WY_ulKNLFlag == WY_bKnl�жϸ�ҳ���Ƿ��������Ȩ����ͬ
			//ֻ����ͬ�ſ��Է���ģ���������ҳ�����
			if((WY_pUseRec->WY_ulPhysicalPage > WY_pPreUse->WY_ulPhysicalPage) && (WY_pPreUse->WY_ulKNLFlag == WY_bKnl))
			{
				//����ж����ж���һ����¼�Ƿ��ǿ�ҳ���
				//����ǾͲ��ټ����,��Ϊ�϶��Ѿ�����,
				//�ж�����ѡ��ԭ��,һ��ʼ����0x1018�ֽ�,����0x00008
				//ҳ��,��ô������16�ֽ�,��ô�ʹ�0x00009����,
				//��ַ��0x00009018��������������жϵĻ����´����룬
				//���ǻ���0x00008�����¼������0x00008018���䣬�ͳ����˴���
				//����һ�жϣ������һ����¼�ļ�¼��ʼ��ַ���ϴ�С
				//�����������¼����ʼ��ַ�Ļ���Ҫô�ϸ���¼û�п�ҳ��
				//Ҫô�����¼���Ǹ��������Ǹ�����ġ�
				if((WY_pPreUse->WY_ulRangeSize + WY_pPreUse->WY_ulLinearAddress) != 
				     WY_pUseRec->WY_ulLinearAddress)
				{
					WY_nPageFreeMem = (0x1000 - ((WY_pPreUse->WY_ulLinearAddress & 0xFFF) + WY_pPreUse->WY_ulRangeSize % 0x1000))%0x1000;
					//ҳ��ʣ��ռ��Ƿ����Ҫ����Ŀռ䣬���������
					if(WY_nPageFreeMem >= WY_nMemSize)
					{

						WY_pMemUse = allocrec_use(WY_bSystem);
						ConstructUseRecord(WY_pMemUse,
										      WY_pPreUse->WY_ulLinearAddress + WY_pPreUse->WY_ulRangeSize,
										      WY_nMemSize,
							                           ((WY_pPreUse->WY_ulPhysicalPage << 12) + WY_pPreUse->WY_ulRangeSize)/0x1000, 
							                           ((ulong)WY_pMemUse & 0xFFF) >> 4,0,0,0,WY_bKnl);
						InsertRec_use(WY_pMemUse,WY_bSystem);
						WY_nUseableMemory -= WY_nMemSize;
						__asm__("popf");
						return (PVOID)WY_pMemUse->WY_ulLinearAddress;
					}
				}
			}
			WY_pPreUse = WY_pUseRec;
			WY_pUseRec = (WY_pMemRecord)((WY_pUseRec->WY_Use.WY_RecordCharacter.WY_ulTableOfNextRecord << 12) + WY_pUseRec->WY_Use.WY_RecordCharacter.WY_ulTablePosOfNextRecord * MEM_USE_RECORD_SIZE);
		}
		//�������ѭ���ˣ���ôWY_pPreUse  �϶�ָ��������һ����¼
		//��ô�Ϳ������һ����¼�Ƿ��������
		WY_nPageFreeMem = (0x1000 - ((WY_pPreUse->WY_ulLinearAddress & 0xFFF) + WY_pPreUse->WY_ulRangeSize % 0x1000))%0x1000;
		//ҳ��ʣ��ռ��Ƿ����Ҫ����Ŀռ䣬���������
		if((WY_nPageFreeMem >= WY_nMemSize) &&(WY_pPreUse->WY_ulKNLFlag == WY_bKnl))
		{
//			printk("       this a %x s %x\n",WY_pPreUse->WY_ulLinearAddress,WY_pPreUse->WY_ulRangeSize);
			WY_pMemUse = allocrec_use(WY_bSystem);
			ConstructUseRecord(WY_pMemUse,
							      WY_pPreUse->WY_ulLinearAddress + WY_pPreUse->WY_ulRangeSize,
							      WY_nMemSize,
				                           ((WY_pPreUse->WY_ulPhysicalPage << 12) + WY_pPreUse->WY_ulRangeSize)/0x1000, 
				                           ((ulong)WY_pMemUse & 0xFFF) >> 4,0,0,0,WY_bKnl);
			InsertRec_use(WY_pMemUse,WY_bSystem);
//			printk("     tihs\n");
			__asm__("popf");
			return (PVOID)WY_pMemUse->WY_ulLinearAddress;
		}
		else
		{
			//����һ��ҳ��,�����ҳ���ڽ��з���
			WY_pMemUse = AllocPage((WY_nMemSize / 0x1000) + 1,WY_bKnl,WY_bSystem);
			wyos_close_int();
			WY_pMemUse->WY_ulRangeSize = WY_nMemSize;
			WY_nUseableMemory -= WY_nMemSize;
			__asm__("popf");
//			printk("Alloc Page Return Size %x  %x\n",WY_pMemUse->WY_ulLinearAddress,WY_pMemUse);
//			printk("");
//			printk("");
			return (PVOID)WY_pMemUse->WY_ulLinearAddress;
		}
	}
	__asm__("popf");
	return (PVOID)0;
}

//��Ҫ���Ƶĵط���FreePage֮ǰ��ӳ��������ַ�������ַȡ����
void freemem(void * WY_pAddr,BOOL WY_bKnl,BOOL WY_bSystem)
{
	WY_pMemRecord		WY_pPreUse = NULL,WY_pUseRec,WY_pNextRec;
	int					WY_nPhyArea = 0,WY_nLastPage,WY_nTmpPage;
	ulong				WY_ulLinearAddr = 0,WY_ulMemSize = 0;

	if(WY_bSystem)
	{
		WY_pUseRec = WY_PROCTABLE[0].WY_pMemFirstRec;
		WY_pNextRec = (WY_pMemRecord)((WY_PROCTABLE[0].WY_pMemFirstRec->WY_Use.WY_RecordCharacter.WY_ulTableOfNextRecord << 12) 
									  + WY_PROCTABLE[0].WY_pMemFirstRec->WY_Use.WY_RecordCharacter.WY_ulTablePosOfNextRecord 
									  * MEM_USE_RECORD_SIZE);
	}
	else
	{
		WY_pUseRec = WY_PROCTABLE[GetCurrentPID()].WY_pMemFirstRec;
		WY_pNextRec = (WY_pMemRecord)((WY_PROCTABLE[GetCurrentPID()].WY_pMemFirstRec->WY_Use.WY_RecordCharacter.WY_ulTableOfNextRecord << 12) 
									  + WY_PROCTABLE[GetCurrentPID()].WY_pMemFirstRec->WY_Use.WY_RecordCharacter.WY_ulTablePosOfNextRecord 
									  * MEM_USE_RECORD_SIZE);
	}
//	printk("Freemem First %x Next %x \n",WY_pUseRec,WY_pNextRec);
	__asm__("pushf");
	wyos_close_int();
	while(WY_pUseRec)
	{
		//��������ĵ�ַ�Ƿ��ڸü�¼��Χ֮��
		if((ulong)WY_pAddr >= WY_pUseRec->WY_ulLinearAddress && (ulong)WY_pAddr < (WY_pUseRec->WY_ulLinearAddress + WY_pUseRec->WY_ulRangeSize))
		{
			//�ж�����ڴ��Ƿ񳬹�һ��ҳ��
//			printk("Loop First %x Next %x Linear %x Rec Line %xRS %x \n",WY_pUseRec,WY_pNextRec,WY_pAddr,WY_pUseRec->WY_ulLinearAddress,WY_pUseRec->WY_ulRangeSize);
			if(WY_pUseRec->WY_ulRangeSize >= 0x1000)
			{
				if(WY_pUseRec->WY_ulRangeSize % 0x1000 == 0) WY_nPhyArea = WY_pUseRec->WY_ulRangeSize / 0x1000;
				else WY_nPhyArea = WY_pUseRec->WY_ulRangeSize / 0x1000 + 1;
			}
			else
			{
				if(WY_pPreUse != NULL)
				{
					//����һ���ڴ�����Խ��ҳ������һ��ҳ���Ƿ������
					//��ͬ����ͬ���ҳ��Ͳ��ܱ��ͷŵ�
					if(WY_pPreUse->WY_ulRangeSize%0x1000 == 0) WY_nLastPage = WY_pPreUse->WY_ulPhysicalPage + WY_pPreUse->WY_ulRangeSize / 0x1000 - 1;
					else WY_nLastPage = WY_pPreUse->WY_ulPhysicalPage + WY_pPreUse->WY_ulRangeSize / 0x1000;

					if(WY_nLastPage == WY_pUseRec->WY_ulPhysicalPage)
					{
						WY_nUseableMemory += WY_pUseRec->WY_ulRangeSize;
						DeleteRec_use(WY_pUseRec,WY_bSystem);
						__asm__("popf");
						return;
					}
				}
				WY_nPhyArea = 1;
			}
			//�ж���һ����¼������ڴ���ռ�����һ��ҳ���Ƿ���ͬ
			if((WY_pUseRec->WY_ulPhysicalPage + WY_nPhyArea - 1) == WY_pNextRec->WY_ulPhysicalPage)
				WY_nPhyArea--;

			WY_nTmpPage = WY_pUseRec->WY_ulPhysicalPage << 12;
			WY_ulLinearAddr = WY_pUseRec->WY_ulLinearAddress;
			//��Ҫ���ȡ��ӳ��Ĵ���
			WY_nUseableMemory += WY_pUseRec->WY_ulRangeSize;
			WY_ulMemSize = WY_pUseRec->WY_ulRangeSize;
			DeleteRec_use(WY_pUseRec,WY_bSystem);
			//ֻ�������¼��ռ�˸��⼸��ҳ�棬�⼸��ҳ��ſ��Ա��ͷŵ�
			if(WY_nPhyArea)	
			{
	//			WY_nUseableMemory += WY_ulMemSize;
				FreePage(WY_nTmpPage,WY_nPhyArea,WY_bKnl);
	//			WY_nUseableMemory += WY_nPhyArea * 0x1000;
				UnmappedLinear(WY_ulLinearAddr,WY_nPhyArea,WY_bKnl);
			}
			__asm__("popf");
			return;
		}
		else		//���ڸ÷�Χ�ڣ�������һ����¼
		{
			WY_pPreUse = WY_pUseRec;
			WY_pUseRec = WY_pNextRec;
			WY_pNextRec = (WY_pMemRecord)((WY_pNextRec->WY_Use.WY_RecordCharacter.WY_ulTableOfNextRecord << 12) + WY_pNextRec->WY_Use.WY_RecordCharacter.WY_ulTablePosOfNextRecord * MEM_USE_RECORD_SIZE);
		}
	}
	__asm__("popf");
}

void * mallock(int WY_nSize)
{
	return mallocmem(WY_nSize,TRUE,FALSE);
}

void freek(void * WY_pAddr)
{
	return freemem(WY_pAddr,TRUE,FALSE);
}

void *mallocs(int WY_nSize)
{
//	printk("CurPID %d\n",GetCurrentPID());
	return mallocmem(WY_nSize,TRUE,TRUE);
}

void frees(void * WY_pAddr)
{
	return freemem(WY_pAddr,TRUE,TRUE);
}

void PageExpProc(ulong WY_ulErrCode,ulong WY_ulFaultLinearAddr)
{
	printk("Page Exception Error PID %x  TID %x Code %x Cr2:%x\n",WY_usCurrentPID,GetCurrentTID(WY_usCurrentPID),WY_ulErrCode,WY_ulFaultLinearAddr);
	while(1)
	{}
}

ulong SyscallGetMmInfo()
{
	return WY_nUseableMemory;
}

ulong SyscallMalloc(pSyscallParam WY_pInputParam)
{
	ulong	WY_ulRetValue = (ulong)mallocmem(WY_pInputParam->WY_ulParam0,FALSE,FALSE);

//	printk("mallocmem return %x\n",WY_ulRetValue);
	return WY_ulRetValue;
}

void SyscallFree(pSyscallParam WY_pInputParam)
{
	freemem((PVOID)WY_pInputParam->WY_ulParam0,FALSE,FALSE);
}
//new line
