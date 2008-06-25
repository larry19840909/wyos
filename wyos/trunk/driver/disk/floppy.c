/***************************************************************************
			WYOS floppy.c
			软盘相关源文件
						编码:WY.lslrt				editor	 :WY.lslrt
						日期:2006/9/1				date	 :2006/9/1
						版权:WY 和 WY.lslrt所有 	copyright:WY and WY.lslrt
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

BOOL	WY_bFDInit =FALSE;	//软盘初始化完成标志
ulong	WY_ulFDOpFun = 0;	//控制器中断处理函数地址
BOOL	WY_bFDInt = FALSE;	//中断标志，表示是否发生中断了

//当前执行请求
WY_pFDRequest	WY_pCurrRQ = NULL;

//上升下降队列
WY_FDRequest	WY_UPQueue = {0,0,0,0,0,0,0,0,0,0,0,0};
WY_FDRequest	WY_DOWNQueue = {0,0,0,0,0,0,0,0,0,0,0,0};
//磁头方向，TRUE上升,FALSE下降，默认下降
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
static	BOOL	WY_bDirect = TRUE;	//TRUE 表示LBA上升，FALSE表示下降，默认上升开始
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
	ulong	WY_ulPageOffset;	//此页面偏移相对于DMA来说,DMA1控制器64K一个页面
	WY_FDRequest	WY_test;
	
	printk("     Floppy Detecing........");

	//初始化全局变量
	WY_bFDInit =FALSE;	//软盘初始化完成标志
	WY_ulFDOpFun = 0;	//控制器中断处理函数地址
	WY_bFDInt = FALSE;	//中断标志，表示是否发生中断了
	//当前执行请求
	WY_pCurrRQ = NULL;

	//上升下降队列
	memset((char*)&WY_UPQueue,0,sizeof(WY_FDRequest));
	memset((char*)&WY_DOWNQueue,0,sizeof(WY_FDRequest));
	
	//磁头方向，TRUE上升,FALSE下降，默认下降
	WY_bFDHDir = TRUE;	
	WY_ulRDQLock  = 1;

	WY_ulDMABufAddr;
	WY_ulDMABuf;
	WY_bDirect = TRUE;	//TRUE 表示LBA上升，FALSE表示下降，默认上升开始
	//检测软驱信息
	//通过读取CMOS中的数据判断
	WritePort(0x70,0x10);

	WY_byteCMOS = ReadPort(0x71);
	printk("                  Floppy A is %s \n                  Floppy B is %s\n",drive_type[WY_byteCMOS >> 4],drive_type[WY_byteCMOS & 0xF]);

	printk("     Floppy Initializing............................");
	
	//保护标志寄存器，并关闭中断
	__asm__("pushf");
	wyos_close_int();

	SetInterrupt((INT_PROC)FloppyInterrupt, 0x8, 0, GATE_386INTERRUPT, 0x26);

	OpenHardInt(FD_IRQL);
	//恢复标志寄存器
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

	//为DMA申请内存
	WY_ulDMABuf = (ulong)mallocs(0x10000);
	WY_ulDMABufAddr = LineartToPhy(WY_ulDMABuf);
	WY_ulPageOffset = (WY_ulDMABufAddr - (WY_ulDMABufAddr & 0xFF0000));//计算页内偏移
	//重新申请空间。
	frees((PVOID)WY_ulDMABuf);
	WY_ulDMABuf = (ulong)mallocs(0x10000 + WY_ulPageOffset);
	//计算新的缓冲区地址，保证在一个页面的开始
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
		//驱动器未准备好，等待
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
		//驱动器未准备好，等待
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
	//设置复位标志
	WritePort(FD_DOR,0);
	WritePort(FD_CCR,0);
	//复位结束
	WritePort(FD_DOR,0xC);
	__asm__("pushf");
	wyos_open_int();
	while(!WY_bFDInt)
	{
	}
	WY_bFDInt = FALSE;
	__asm__("popf");


	//结束命令，并读取结果信息
	for(i = 0;i < 4;i++)
	{
		SendByte(FD_SIS);
		GetByte(&ST0);
//		printk("         ST0: %x",ST0);
		GetByte(&Cylinder);
//		printk("  Cylinder: %x\n",Cylinder);
	}

	//发送SPECIFY命令
	SendByte(FD_SPECIFY);
	//设置三个时钟和启用DMA模式
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

	//恢复中断，并读取结果。发送SENSE INTERRUPT STATUS Command
	SendByte(FD_SIS);
	GetByte(&ST0);
	GetByte(&Cylinder);

	//判断执行正确与否
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

	//读取DOR寄存器，保护其它位不变
	WY_byteDOR = ReadPort(FD_DOR);

	WY_byteDOR = WY_byteDOR & 0xFC;
	WY_byteDOR = ((WY_byteDOR | (BYTE)(DriveNum & 0xFF)) | (1 << (DriveNum + 4)));

	WritePort(FD_DOR,WY_byteDOR);

}

void DisableDrive(int DriveNum)
{
	BYTE	WY_byteDOR;

	//读取DOR寄存器，保护其它位不变
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
	
	//更改当前操作请求

	WY_ulFDOpFun = (ulong)FDRWIntProc;

	EnableDrive(0);
//	FDRecalibrate();
	SetDMAChannel(2,WY_ulDMABufAddr,WY_pRWRQ->WY_ulSectorNum * 512, DMA_ISA_SINGLEMODE, WY_pRWRQ->WY_usDirect, DMA_ISA_ADDRINC, DMA_ISA_SINGLECYCLE);
	OpenHardInt(FD_IRQL);
	//发送读写命令
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
	//发送参数
	SendByte(WY_pRWRQ->WY_ulHead << 2);					//驱动器和磁头选择
	SendByte(WY_pRWRQ->WY_ulCylinder);						//发送柱面，磁头，扇区号码
	SendByte(WY_pRWRQ->WY_ulHead);
	SendByte(WY_pRWRQ->WY_ulSector);
	SendByte(2);												//磁盘扇区大小，2 = 512B,标准用法
	SendByte(18);											//磁道尾最后一个扇区号，3.5"1.44MB软盘标准
	SendByte(0x1B);											// 3.5"1.44MB软盘标准
	SendByte(0xFF);											//PC机均为0xFF
//	printk("Send Command End\n");
}

void FDRWIntProc()
{
	//读取结果字节
	GetByte(&ST0);
	GetByte(&ST1);
	GetByte(&ST2);
	GetByte(&Cylinder);
	GetByte(&Head);
	GetByte(&Sector);
	GetByte(&N);
	//判断结果,首先判断ST0
	if((ST0 & FD_ST0_IC_INV) == FD_ST0_IC_INV)
	{
		//无效命令
		WY_pCurrRQ->WY_ulState = FD_STATE_FAILD;
	}
	else if(((ST0 & FD_ST0_IC_ABN) == FD_ST0_IC_ABN) | ((ST0 & FD_ST0_IC_ABNP) == FD_ST0_IC_ABNP))
	{
		//重试
		if(WY_pCurrRQ->WY_ulState == 3)
		{
			//超过重试次数
			WY_pCurrRQ->WY_ulState = FD_STATE_FAILD;
		}
		else
		{
			//重试
			WY_pCurrRQ->WY_ulState++;
			FDRWSector(WY_pCurrRQ);
			return;
		}
		//以下可以获得更详细的出错信息
/*		//指令非正常退出
		if((ST0 & FD_ST0_EC) == FD_ST0_EC)
		{
			//设备检测错误
			WY_pCurrRQ->WY_ulState = FD_STATE_FAILD;
		}
		else
		{
			//判断ST1
			if((ST1 & FD_ST1_EN) == FD_ST1_EN)
			{
				WY_pCurrRQ->WY_ulState = FD_STATE_FAILD;
			}
			else if((ST1 & FD_ST1_DE) == FD_ST1_DE)
			{
				//数据CRC错误
				WY_pCurrRQ->WY_ulState = FD_STATE_FAILD;
			}
			else if((ST1 & FD_ST1_OR) = FD_ST1_OR)
			{
				//数据传输超时
				WY_pCurrRQ->WY_ulState = FD_STATE_FAILD;
			}
			else if((ST1 & FD_ST1_NW) == FD_ST1_NW)
			{
				//写保护
				WY_pCurrRQ->WY_ulState = FD_STATE_FAILD;
			}
			else
			{
				//检查ST2
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
		//启动请求处理工作线程
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
		//当前是否有读写操作在处理
//		if(WY_pCurrRQ != NULL)
//		{
//			//有，操作是否完成，未完成返回
//			if(WY_pCurrRQ->WY_ulState != FD_STATE_OK)
//			{
//				BlockThread(WYOS_PROC_BLOCK_TYPE_SOFTWARE,GetCurrentTID(0),WYOS_BLOCK_WAITTIME_INFINITY, 0, 0);
//			}
//		}
		//新操作，或操作已经完成
		WY_pNextRQ = GetFromFDRQ();
		WY_pCurrRQ = WY_pNextRQ;
//		printk("WY_pNextRQ %x next %x\n",WY_pNextRQ,WY_pNextRQ->WY_pNext);
		if(WY_pNextRQ == NULL)
		{
			//所有请求已经处理完
//			//关闭驱动器马达，并
			//等待输入请求
//			DisableDrive(0);
			BlockThread(WYOS_PROC_BLOCK_TYPE_SOFTWARE,WY_ulWorkThread,WYOS_BLOCK_WAITTIME_INFINITY, 0, 0);
		}
		else
		{
			//发送读写请求，然后等待操作完成
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
		//唤醒工作线程
		UnblockThread(0, WY_ulWorkThread,WYOS_PROC_BLOCK_TYPE_SOFTWARE, WYOS_BLOCK_STATE_COMPLETE);
		return (ulong)WY_pRQ;
	}
	else
	{
		//同步，则等待
		UnblockThread(0, WY_ulWorkThread,WYOS_PROC_BLOCK_TYPE_SOFTWARE, WYOS_BLOCK_STATE_COMPLETE);
		WY_ulRes = BlockThread(WYOS_PROC_BLOCk_TYPE_FLOPPY,WY_pRQ->WY_ulTID,5000,0,0);
		//返回
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
//磁盘请求队列操作               ///
//                               ///
////////////////////////////////////
void InsertToFDRQ(WY_pFDRequest WY_pFDOPR)
{
	WY_pFDRequest	WY_pRequest,WY_pPreRQ;
//	BOOL			WY_bUP;
	ulong			WY_ulSpinKey = 0;//用于互斥操作
	
	//如果没有操作，默认放入上升队列
	//该算法默认LBA  上升优先。
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
			//为其找到一个合适的位置
			WY_pRequest = WY_UPQueue.WY_pNext;
			WY_pPreRQ = &WY_UPQueue;
			
			while(WY_pRequest)
			{
				if(WY_pFDOPR->WY_ulLBAAddr < WY_pRequest->WY_ulLBAAddr)
				{
					//插入在此
					WY_pFDOPR->WY_pNext = WY_pRequest;
					WY_pPreRQ->WY_pNext = WY_pFDOPR;
					ReleaseSpinLock(&WY_ulSpinKey,&WY_ulRDQLock);
					return;
				}
				else
				{
					//比较下一个
					WY_pPreRQ = WY_pRequest;
					WY_pRequest = WY_pRequest->WY_pNext;
				}
			}
			//大于其中所有的，则将其插入队列尾
			WY_pPreRQ->WY_pNext = WY_pFDOPR;
			WY_pFDOPR->WY_pNext = NULL;
		}
	}
	else
	{
		//如果有操作则根据当前操作的LBA地址决定插入上升还是下降队列
		if(WY_pFDOPR->WY_ulLBAAddr >= WY_pCurrRQ->WY_ulLBAAddr)
		{
			if(WY_UPQueue.WY_pNext == NULL)
			{
				WY_UPQueue.WY_pNext = WY_pFDOPR;
			}
			else
			{
				//为其找到一个合适的位置
				WY_pRequest = WY_UPQueue.WY_pNext;
				WY_pPreRQ = &WY_UPQueue;
				
				while(WY_pRequest)
				{
					if(WY_pFDOPR->WY_ulLBAAddr < WY_pRequest->WY_ulLBAAddr)
					{
						//插入在此
						WY_pFDOPR->WY_pNext = WY_pRequest;
						WY_pPreRQ->WY_pNext = WY_pFDOPR;
						ReleaseSpinLock(&WY_ulSpinKey,&WY_ulRDQLock);
						return;
					}
					else
					{
						//比较下一个
						WY_pPreRQ = WY_pRequest;
						WY_pRequest = WY_pRequest->WY_pNext;
					}
				}
				//大于其中所有的，则将其插入队列尾
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

				//降序排列
				while(WY_pRequest)
				{
					if(WY_pFDOPR->WY_ulLBAAddr > WY_pRequest->WY_ulLBAAddr)
					{
						//插入在此
						WY_pFDOPR->WY_pNext = WY_pRequest;
						WY_pPreRQ->WY_pNext = WY_pFDOPR;
						ReleaseSpinLock(&WY_ulSpinKey,&WY_ulRDQLock);
						return;
					}
					else
					{
						//比较下一个
						WY_pPreRQ = WY_pRequest;
						WY_pRequest = WY_pRequest->WY_pNext;
					}
				}
				//小于其中所有的，则将其插入队列尾
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

	//根据当前驱动器旋转方向，选择读取队列
	if(WY_bDirect)
	{
		//上升队列
		//如果上升队列为空则改变旋转方向，并读取下降队列
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
		//下降队列
		//如果下降队列为空则改变旋转方向，并读取上升队列
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

	//取出队头节点
	WY_pFDR = WY_pFDRQ->WY_pNext;
	WY_pFDRQ->WY_pNext = WY_pFDR->WY_pNext;
	
	ReleaseSpinLock(&WY_ulSpinKey,&WY_ulRDQLock);
	return WY_pFDR;
}

