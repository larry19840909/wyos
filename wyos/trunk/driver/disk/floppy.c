/***************************************************************************
			WYOS floppy.c
			�������Դ�ļ�
						����:WY.lslrt				editor	 :WY.lslrt
						����:2006/9/1				date	 :2006/9/1
						��Ȩ:WY �� WY.lslrt���� 	copyright:WY and WY.lslrt
						URL:http://www.wyos.net http://wylslrt.go.3322.org
***************************************************************************/
#include "..\..\include\WYOSType.h"
#include "..\..\include\syscall.h"
#include "..\..\include\cpu.h"
#include "..\..\include\io.h"
#include "..\..\video\video.h"
#include "..\..\include\string.h"
#include "..\..\include\memory.h"
#include "..\..\include\math.h"
#include "..\..\include\mutex.h"
#include "..\..\include\process.h"
#include "..\..\include\driver.h"
#include "..\..\include\floppy.h"

extern WY_ProcTable			WY_PROCTABLE[MAX_PROC_NUM];

BOOL	WY_bFDInit =FALSE;	//���̳�ʼ����ɱ�־
ulong	WY_ulFDOpFun = 0;	//�������жϴ�������ַ
BOOL	WY_bFDInt = FALSE;	//�жϱ�־����ʾ�Ƿ����ж���

//��ǰִ������
WY_pFDRequest	WY_pCurrRQ = NULL;

//�����½�����
WY_FDRequest	WY_UPQueue = {0,0,0,0,0,0,0,0,0,0,0,0};
WY_FDRequest	WY_DOWNQueue = {0,0,0,0,0,0,0,0,0,0,0,0};
//��ͷ����TRUE����,FALSE�½���Ĭ���½�
BOOL			WY_bFDHDir = TRUE;		

static unsigned char reply_buffer[7];
#define ST0				(reply_buffer[0])
#define ST1				(reply_buffer[1])
#define ST2				(reply_buffer[2])
#define Cylinder			(reply_buffer[3])
#define Head				(reply_buffer[4])
#define Sector			(reply_buffer[5])
#define N				(reply_buffer[6])

static	ulong	WY_ulRDQLock  = 1;
//static	ulong	WY_ulCRQLock = 1;


extern	void FloppyInterrupt();
//extern     DriverFun DriverFunction[32];

static 	ulong	WY_ulDMABufAddr;
static	ulong	WY_ulDMABuf;
static	BOOL	WY_bDirect = TRUE;	//TRUE ��ʾLBA������FALSE��ʾ�½���Ĭ��������ʼ
static	ulong	WY_ulWorkThread = 0;

void FloppyInit()
{
	char drive_type[][50] = {
                                "no floppy drive",
                                "360kb 5.25in floppy drive",
                                "1.2mb 5.25in floppy drive",
                                "720kb 3.5in",
                                "1.44mb 3.5in",
                                "2.88mb 3.5in" };
	
	BYTE	WY_byteCMOS;
	ulong	WY_ulPageOffset;	//��ҳ��ƫ�������DMA��˵,DMA1������64Kһ��ҳ��
	WY_FDRequest	WY_test;
	
	printk("     Floppy Detecing........");

	//��ʼ��ȫ�ֱ���
	WY_bFDInit =FALSE;	//���̳�ʼ����ɱ�־
	WY_ulFDOpFun = 0;	//�������жϴ�������ַ
	WY_bFDInt = FALSE;	//�жϱ�־����ʾ�Ƿ����ж���
	//��ǰִ������
	WY_pCurrRQ = NULL;

	//�����½�����
	memset((char*)&WY_UPQueue,0,sizeof(WY_FDRequest));
	memset((char*)&WY_DOWNQueue,0,sizeof(WY_FDRequest));
	
	//��ͷ����TRUE����,FALSE�½���Ĭ���½�
	WY_bFDHDir = TRUE;	
	WY_ulRDQLock  = 1;

	WY_ulDMABufAddr;
	WY_ulDMABuf;
	WY_bDirect = TRUE;	//TRUE ��ʾLBA������FALSE��ʾ�½���Ĭ��������ʼ
	//���������Ϣ
	//ͨ����ȡCMOS�е������ж�
	WritePort(0x70,0x10);

	WY_byteCMOS = ReadPort(0x71);
	printk("                  Floppy A is %s \n                  Floppy B is %s\n",drive_type[WY_byteCMOS >> 4],drive_type[WY_byteCMOS & 0xF]);

	printk("     Floppy Initializing............................");
	
	//������־�Ĵ��������ر��ж�
	__asm__("pushf");
	wyos_close_int();

	SetInterrupt((INT_PROC)FloppyInterrupt, 0x8, 0, GATE_386INTERRUPT, 0x26);

	OpenHardInt(FD_IRQL);
	//�ָ���־�Ĵ���
	__asm__("popf");

	if(!FDReset())
	{
		printk("FAILED\n");
		return;
	}
	if(!FDRecalibrate())
	{
		printk("FAILED\n");
		return;
	}

	printk("SUCCESSFUL\n");
	CloseHardInt(FD_IRQL);

	//ΪDMA�����ڴ�
	WY_ulDMABuf = (ulong)mallocs(0x10000);
	WY_ulDMABufAddr = LineartToPhy(WY_ulDMABuf);
	WY_ulPageOffset = (WY_ulDMABufAddr - (WY_ulDMABufAddr & 0xFF0000));//����ҳ��ƫ��
	//��������ռ䡣
	frees((PVOID)WY_ulDMABuf);
	WY_ulDMABuf = (ulong)mallocs(0x10000 + WY_ulPageOffset);
	//�����µĻ�������ַ����֤��һ��ҳ��Ŀ�ʼ
	WY_ulDMABufAddr = LineartToPhy(WY_ulDMABuf) + (0x10000 - WY_ulPageOffset);
	WY_ulDMABuf = PhyToLinear(WY_ulDMABufAddr,16,TRUE);
	WY_ulWorkThread = KCreateThread((THREAD_ROUTINE)RWProc,NULL,TRUE);

}

BOOL SendByte(BYTE WY_byteData)
{
	BYTE	WY_byteMSR;
	ulong	WY_TimeoutCnt = 0;

	WY_byteMSR = ReadPort(FD_MSR);
	while( (WY_byteMSR  & FD_WRITEABLE) != FD_WRITEABLE)
	{
		//������δ׼���ã��ȴ�
		WY_TimeoutCnt++;
		WY_byteMSR = ReadPort(FD_MSR);
		if(WY_TimeoutCnt > 1000)
		{
			return FALSE;
		}
	}

	WritePort(FD_FIFO,WY_byteData);
	return TRUE;
	
}

BOOL GetByte(BYTE *WY_byteData)
{
	BYTE WY_byteMSR;
	ulong WY_TimeoutCnt = 0;

	WY_byteMSR = ReadPort(FD_MSR);
	while( (WY_byteMSR & FD_READABLE) != FD_READABLE)
	{
		//������δ׼���ã��ȴ�
		WY_TimeoutCnt++;
		WY_byteMSR = ReadPort(FD_MSR);
		if(WY_TimeoutCnt >1000)
		{
			return FALSE;
		}
	}
	
	*WY_byteData = ReadPort(FD_FIFO);
	return TRUE;
}

inline void LBAtoCHS(ulong WY_ulLBAAddr, ulong * WY_ulCylinder, ulong * WY_ulHead, ulong * WY_ulSector)
{
	*WY_ulSector = WY_ulLBAAddr % FD_SNUM + 1;
	WY_ulLBAAddr /= FD_SNUM;
       *WY_ulHead = WY_ulLBAAddr % FD_HNUM;
	*WY_ulCylinder = WY_ulLBAAddr / FD_HNUM;
}

inline ulong CHStoLBA(ulong WY_ulCylinder, ulong WY_ulHead, ulong WY_ulSector)
{
	return (WY_ulCylinder * FD_HNUM * FD_SNUM + WY_ulHead * FD_SNUM + WY_ulSector - 1);
}

BOOL FDReset()
{
	int		i;
	printk(" %x\n",WY_ulFDOpFun);
	//���ø�λ��־
	WritePort(FD_DOR,0);
	WritePort(FD_CCR,0);
	//��λ����
	WritePort(FD_DOR,0xC);
	__asm__("pushf");
	wyos_open_int();
	while(!WY_bFDInt)
	{
	}
	WY_bFDInt = FALSE;
	__asm__("popf");


	//�����������ȡ�����Ϣ
	for(i = 0;i < 4;i++)
	{
		SendByte(FD_SIS);
		GetByte(&ST0);
//		printk("         ST0: %x",ST0);
		GetByte(&Cylinder);
//		printk("  Cylinder: %x\n",Cylinder);
	}

	//����SPECIFY����
	SendByte(FD_SPECIFY);
	//��������ʱ�Ӻ�����DMAģʽ
	SendByte(0xDF);
	SendByte(2);
	return TRUE;
	
}

BOOL FDRecalibrate()
{
	EnableDrive(0);
	
	SendByte(FD_RECALIBRATE);
	SendByte(0);

	__asm__("pushf");
	wyos_open_int();
	while(!WY_bFDInt)
	{
	}
	WY_bFDInit = FALSE;
	__asm__("popf");

	//�ָ��жϣ�����ȡ���������SENSE INTERRUPT STATUS Command
	SendByte(FD_SIS);
	GetByte(&ST0);
	GetByte(&Cylinder);

	//�ж�ִ����ȷ���
	if(((ST0 & 0xE0) >> 5) == 1)
	{
;		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

/*
b7    b6   b5   b4        b3          b2    b1     b0
ME3 ME2 ME1 ME0 DMAGATE RESET DS1 DS2
*/
void EnableDrive(int DriveNum)
{
	BYTE	WY_byteDOR;

	//��ȡDOR�Ĵ�������������λ����
	WY_byteDOR = ReadPort(FD_DOR);

	WY_byteDOR = WY_byteDOR & 0xFC;
	WY_byteDOR = ((WY_byteDOR | (BYTE)(DriveNum & 0xFF)) | (1 << (DriveNum + 4)));

	WritePort(FD_DOR,WY_byteDOR);

}

void DisableDrive(int DriveNum)
{
	BYTE	WY_byteDOR;

	//��ȡDOR�Ĵ�������������λ����
	WY_byteDOR = ReadPort(FD_DOR);

	WY_byteDOR = WY_byteDOR & 0xFC;
	WY_byteDOR = (WY_byteDOR | (BYTE)(DriveNum & 0xFF));
	
	switch(DriveNum)
	{
	case 0:
		WY_byteDOR &= 0xEF;
		break;
	case 1:
		WY_byteDOR &= 0xDF;
		break;
	case 2:
		WY_byteDOR &= 0xBF;
		break;
	case 3:
		WY_byteDOR &= 0x7F;
		break;
	default:
		break;
	}

	WritePort(FD_DOR,WY_byteDOR);
}

void FDRWSector(WY_pFDRequest WY_pRWRQ)
{
	WY_pFDRequest		WY_pSeparateRQ = NULL;
	
	//���ĵ�ǰ��������

	WY_ulFDOpFun = (ulong)FDRWIntProc;

	EnableDrive(0);
//	FDRecalibrate();
	SetDMAChannel(2,WY_ulDMABufAddr,WY_pRWRQ->WY_ulSectorNum * 512, DMA_ISA_SINGLEMODE, WY_pRWRQ->WY_usDirect, DMA_ISA_ADDRINC, DMA_ISA_SINGLECYCLE);
	OpenHardInt(FD_IRQL);
	//���Ͷ�д����
	if(WY_pRWRQ->WY_usDirect == FD_READ_DIR)
	{
//		printk("Begin RW OP %x \n",FD_READDATA(1,1));
		SendByte(FD_READDATA(1, 1));
	}
	else
	{
//		printk("Begin RW OP %x \n",FD_WRITEDATA(1));
		memcpy((char*)WY_ulDMABuf,WY_pRWRQ->WY_ulUserBuff,WY_pRWRQ->WY_ulSectorNum * 512);
		SendByte(FD_WRITEDATA(1));
	}
	//���Ͳ���
	SendByte(WY_pRWRQ->WY_ulHead << 2);					//�������ʹ�ͷѡ��
	SendByte(WY_pRWRQ->WY_ulCylinder);						//�������棬��ͷ����������
	SendByte(WY_pRWRQ->WY_ulHead);
	SendByte(WY_pRWRQ->WY_ulSector);
	SendByte(2);												//����������С��2 = 512B,��׼�÷�
	SendByte(18);											//�ŵ�β���һ�������ţ�3.5"1.44MB���̱�׼
	SendByte(0x1B);											// 3.5"1.44MB���̱�׼
	SendByte(0xFF);											//PC����Ϊ0xFF
//	printk("Send Command End\n");
}

void FDRWIntProc()
{
	//��ȡ����ֽ�
	GetByte(&ST0);
	GetByte(&ST1);
	GetByte(&ST2);
	GetByte(&Cylinder);
	GetByte(&Head);
	GetByte(&Sector);
	GetByte(&N);
	//�жϽ��,�����ж�ST0
	if((ST0 & FD_ST0_IC_INV) == FD_ST0_IC_INV)
	{
		//��Ч����
		WY_pCurrRQ->WY_ulState = FD_STATE_FAILD;
	}
	else if(((ST0 & FD_ST0_IC_ABN) == FD_ST0_IC_ABN) | ((ST0 & FD_ST0_IC_ABNP) == FD_ST0_IC_ABNP))
	{
		//����
		if(WY_pCurrRQ->WY_ulState == 3)
		{
			//�������Դ���
			WY_pCurrRQ->WY_ulState = FD_STATE_FAILD;
		}
		else
		{
			//����
			WY_pCurrRQ->WY_ulState++;
			FDRWSector(WY_pCurrRQ);
			return;
		}
		//���¿��Ի�ø���ϸ�ĳ�����Ϣ
/*		//ָ��������˳�
		if((ST0 & FD_ST0_EC) == FD_ST0_EC)
		{
			//�豸������
			WY_pCurrRQ->WY_ulState = FD_STATE_FAILD;
		}
		else
		{
			//�ж�ST1
			if((ST1 & FD_ST1_EN) == FD_ST1_EN)
			{
				WY_pCurrRQ->WY_ulState = FD_STATE_FAILD;
			}
			else if((ST1 & FD_ST1_DE) == FD_ST1_DE)
			{
				//����CRC����
				WY_pCurrRQ->WY_ulState = FD_STATE_FAILD;
			}
			else if((ST1 & FD_ST1_OR) = FD_ST1_OR)
			{
				//���ݴ��䳬ʱ
				WY_pCurrRQ->WY_ulState = FD_STATE_FAILD;
			}
			else if((ST1 & FD_ST1_NW) == FD_ST1_NW)
			{
				//д����
				WY_pCurrRQ->WY_ulState = FD_STATE_FAILD;
			}
			else
			{
				//���ST2
			}
		}*/
	}
	else
	{
//		printk("     ST0:%x ST1:%x ST2:%x C:%d H:%d S:%d\n",ST0,ST1,ST2,Cylinder,Head,Sector);
		WY_pCurrRQ->WY_ulState = FD_STATE_OK;
		if(WY_pCurrRQ->WY_usDirect == FD_READ_DIR)
		{
			memcpy((char*)WY_pCurrRQ->WY_ulUserBuff,(char*)WY_ulDMABuf,WY_pCurrRQ->WY_ulSectorNum * 512);
		}
		//�������������߳�
		UnblockThread(0, WY_ulWorkThread,WYOS_PROC_BLOCK_TYPE_SOFTWARE, WYOS_BLOCK_STATE_COMPLETE);
	}

	if(WY_pCurrRQ->WY_usOpMode == FD_OPMODE_SYN)
	{
		UnblockThread(WY_pCurrRQ->WY_ulPID,WY_pCurrRQ->WY_ulTID,WYOS_PROC_BLOCk_TYPE_FLOPPY,WYOS_BLOCK_STATE_COMPLETE);
	}
	WY_ulFDOpFun = 0;
}

ulong RWProc(PVOID WY_pParam)
{
	WY_pFDRequest	WY_pNextRQ;

//	printk("     Floppy Work Thread Running...\n");
	while(1)
	{
		//��ǰ�Ƿ��ж�д�����ڴ���
//		if(WY_pCurrRQ != NULL)
//		{
//			//�У������Ƿ���ɣ�δ��ɷ���
//			if(WY_pCurrRQ->WY_ulState != FD_STATE_OK)
//			{
//				BlockThread(WYOS_PROC_BLOCK_TYPE_SOFTWARE,GetCurrentTID(0),WYOS_BLOCK_WAITTIME_INFINITY, 0, 0);
//			}
//		}
		//�²�����������Ѿ����
		WY_pNextRQ = GetFromFDRQ();
		WY_pCurrRQ = WY_pNextRQ;
//		printk("WY_pNextRQ %x next %x\n",WY_pNextRQ,WY_pNextRQ->WY_pNext);
		if(WY_pNextRQ == NULL)
		{
			//���������Ѿ�������
//			//�ر�����������
			//�ȴ���������
//			DisableDrive(0);
			BlockThread(WYOS_PROC_BLOCK_TYPE_SOFTWARE,WY_ulWorkThread,WYOS_BLOCK_WAITTIME_INFINITY, 0, 0);
		}
		else
		{
			//���Ͷ�д����Ȼ��ȴ��������
			FDRWSector(WY_pNextRQ);
			BlockThread(WYOS_PROC_BLOCK_TYPE_SOFTWARE,WY_ulWorkThread,WYOS_BLOCK_WAITTIME_INFINITY, 0, 0);
		}
	}

}

ulong FDRWCHS(ulong WY_ulCylinder, ulong WY_ulHead, ulong WY_ulSector, ulong WY_ulSecNum, void * WY_pBuf, ulong WY_ulOPDir, ulong WY_ulOPMode)
{
	WY_pFDRequest		WY_pRQ;
	ulong				WY_ulRes;

	WY_pRQ = (WY_pFDRequest)mallocs(sizeof(WY_FDRequest));
	if(WY_pRQ == NULL)
	{
		return 0;
	}
	WY_pRQ->WY_ulCylinder = WY_ulCylinder;
	WY_pRQ->WY_ulHead	= WY_ulHead;
	WY_pRQ->WY_ulSector = WY_ulSector;
	WY_pRQ->WY_ulSectorNum = WY_ulSecNum;
	
	WY_pRQ->WY_ulPID = GetCurrentPID();
	WY_pRQ->WY_ulTID = GetCurrentTID(WY_pRQ->WY_ulPID);
	WY_pRQ->WY_ulUserBuff = WY_pBuf;
	WY_pRQ->WY_ulLBAAddr = CHStoLBA(WY_ulCylinder,WY_ulHead, WY_ulSector);
	WY_pRQ->WY_usDirect = (ushort)WY_ulOPDir;
	WY_pRQ->WY_usOpMode = (ushort)WY_ulOPMode;
	WY_pRQ->WY_ulState = FD_STATE_WAIT;

	InsertToFDRQ(WY_pRQ);
	if(WY_pRQ->WY_usOpMode == FD_OPMODE_ASYN)
	{
		//���ѹ����߳�
		UnblockThread(0, WY_ulWorkThread,WYOS_PROC_BLOCK_TYPE_SOFTWARE, WYOS_BLOCK_STATE_COMPLETE);
		return (ulong)WY_pRQ;
	}
	else
	{
		//ͬ������ȴ�
		UnblockThread(0, WY_ulWorkThread,WYOS_PROC_BLOCK_TYPE_SOFTWARE, WYOS_BLOCK_STATE_COMPLETE);
		WY_ulRes = BlockThread(WYOS_PROC_BLOCk_TYPE_FLOPPY,WY_pRQ->WY_ulTID,5000,0,0);
		//����
		WY_ulRes = WY_pRQ->WY_ulState;
		frees(WY_pRQ);
		return WY_ulRes;
	}
}

ulong FDRWLBA(ulong WY_ulLBAAddr, ulong WY_ulSecNum, void * WY_pBuf, ulong WY_ulOPDir, ulong WY_ulOPMode)
{
	ulong	WY_ulCylinder,WY_ulHead,WY_ulSector;

	LBAtoCHS(WY_ulLBAAddr, &WY_ulCylinder,&WY_ulHead, &WY_ulSector);
	return FDRWCHS(WY_ulCylinder,WY_ulHead, WY_ulSector,WY_ulSecNum, WY_pBuf, WY_ulOPDir,WY_ulOPMode);
}
////////////////////////////////////
//                               ///
//����������в���               ///
//                               ///
////////////////////////////////////
void InsertToFDRQ(WY_pFDRequest WY_pFDOPR)
{
	WY_pFDRequest	WY_pRequest,WY_pPreRQ;
//	BOOL			WY_bUP;
	ulong			WY_ulSpinKey = 0;//���ڻ������
	
	//���û�в�����Ĭ�Ϸ�����������
	//���㷨Ĭ��LBA  �������ȡ�
	SpinLock(&WY_ulSpinKey,&WY_ulRDQLock);
	if(WY_pCurrRQ == NULL)
	{
		if(WY_UPQueue.WY_pNext == NULL)
		{
			WY_UPQueue.WY_pNext = WY_pFDOPR;
			WY_pFDOPR->WY_pNext = NULL;
		}
		else
		{
			//Ϊ���ҵ�һ�����ʵ�λ��
			WY_pRequest = WY_UPQueue.WY_pNext;
			WY_pPreRQ = &WY_UPQueue;
			
			while(WY_pRequest)
			{
				if(WY_pFDOPR->WY_ulLBAAddr < WY_pRequest->WY_ulLBAAddr)
				{
					//�����ڴ�
					WY_pFDOPR->WY_pNext = WY_pRequest;
					WY_pPreRQ->WY_pNext = WY_pFDOPR;
					ReleaseSpinLock(&WY_ulSpinKey,&WY_ulRDQLock);
					return;
				}
				else
				{
					//�Ƚ���һ��
					WY_pPreRQ = WY_pRequest;
					WY_pRequest = WY_pRequest->WY_pNext;
				}
			}
			//�����������еģ�����������β
			WY_pPreRQ->WY_pNext = WY_pFDOPR;
			WY_pFDOPR->WY_pNext = NULL;
		}
	}
	else
	{
		//����в�������ݵ�ǰ������LBA��ַ�����������������½�����
		if(WY_pFDOPR->WY_ulLBAAddr >= WY_pCurrRQ->WY_ulLBAAddr)
		{
			if(WY_UPQueue.WY_pNext == NULL)
			{
				WY_UPQueue.WY_pNext = WY_pFDOPR;
			}
			else
			{
				//Ϊ���ҵ�һ�����ʵ�λ��
				WY_pRequest = WY_UPQueue.WY_pNext;
				WY_pPreRQ = &WY_UPQueue;
				
				while(WY_pRequest)
				{
					if(WY_pFDOPR->WY_ulLBAAddr < WY_pRequest->WY_ulLBAAddr)
					{
						//�����ڴ�
						WY_pFDOPR->WY_pNext = WY_pRequest;
						WY_pPreRQ->WY_pNext = WY_pFDOPR;
						ReleaseSpinLock(&WY_ulSpinKey,&WY_ulRDQLock);
						return;
					}
					else
					{
						//�Ƚ���һ��
						WY_pPreRQ = WY_pRequest;
						WY_pRequest = WY_pRequest->WY_pNext;
					}
				}
				//�����������еģ�����������β
				WY_pPreRQ->WY_pNext = WY_pFDOPR;
				WY_pFDOPR->WY_pNext = NULL;
			}
		}
		else
		{
			if(WY_DOWNQueue.WY_pNext == NULL)
			{
				WY_DOWNQueue.WY_pNext = WY_pFDOPR;
			}
			else
			{
				WY_pRequest = WY_DOWNQueue.WY_pNext;
				WY_pPreRQ = &WY_DOWNQueue;

				//��������
				while(WY_pRequest)
				{
					if(WY_pFDOPR->WY_ulLBAAddr > WY_pRequest->WY_ulLBAAddr)
					{
						//�����ڴ�
						WY_pFDOPR->WY_pNext = WY_pRequest;
						WY_pPreRQ->WY_pNext = WY_pFDOPR;
						ReleaseSpinLock(&WY_ulSpinKey,&WY_ulRDQLock);
						return;
					}
					else
					{
						//�Ƚ���һ��
						WY_pPreRQ = WY_pRequest;
						WY_pRequest = WY_pRequest->WY_pNext;
					}
				}
				//С���������еģ�����������β
				WY_pPreRQ->WY_pNext = WY_pFDOPR;
				WY_pFDOPR->WY_pNext = NULL;
			}
			
		}
		
	}
	ReleaseSpinLock(&WY_ulSpinKey,&WY_ulRDQLock);
}

WY_pFDRequest GetFromFDRQ()
{
	WY_pFDRequest	WY_pFDRQ,WY_pFDR;
	ulong			WY_ulSpinKey = 0;


	SpinLock(&WY_ulSpinKey,&WY_ulRDQLock);

	//���ݵ�ǰ��������ת����ѡ���ȡ����
	if(WY_bDirect)
	{
		//��������
		//�����������Ϊ����ı���ת���򣬲���ȡ�½�����
		if(WY_UPQueue.WY_pNext)
		{
			WY_pFDRQ = &WY_UPQueue;
		}
		else
		{
			WY_pFDRQ = &WY_DOWNQueue;
			WY_bDirect = FALSE;
		}
	}
	else
	{
		//�½�����
		//����½�����Ϊ����ı���ת���򣬲���ȡ��������
		if(WY_DOWNQueue.WY_pNext)
		{
			WY_pFDRQ = &WY_DOWNQueue;
		}
		else
		{
			WY_pFDRQ = &WY_UPQueue;
			WY_bDirect = TRUE;
		}
	}

	//ȡ����ͷ�ڵ�
	WY_pFDR = WY_pFDRQ->WY_pNext;
	WY_pFDRQ->WY_pNext = WY_pFDR->WY_pNext;
	
	ReleaseSpinLock(&WY_ulSpinKey,&WY_ulRDQLock);
	return WY_pFDR;
}

