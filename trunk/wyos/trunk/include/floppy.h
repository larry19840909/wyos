/***************************************************************************
			WYOS floppy.h
			软盘相关头文件
						编码:WY.lslrt				editor	 :WY.lslrt
						日期:2005/12/2			date	 :2005/12/2
						版权:WY 和 WY.lslrt所有 	copyright:WY and WY.lslrt
						URL:http://www.wyos.net http://wylslrt.go.3322.org
***************************************************************************/
#ifndef	WYOS_FLOPPY_H
#define	WYOS_FLOPPY_H

#define	FD_IRQL			0x6
//驱动器寄存器地址
#define	FD_SRA			0x3F0
#define	SD_SRB			0x3F1
#define	FD_DOR			0x3F2
#define	FD_TDR			0x3F3
#define	FD_MSR			0x3F4
#define	FD_DSR			0x3F4
#define	FD_FIFO		0x3F5
#define	FD_DIR			0x3F7
#define	FD_CCR			0x3F7

//MSR寄存器中指示驱动器状态
#define	FD_READABLE	0xC0
#define	FD_WRITEABLE	0x80

//驱动器命令
//数据传输命令
#define	FD_READDATA(MT,SK)			(((MT << 7) | 0x40 | (SK << 5) | 0x6) & 0xFF)
#define	FD_READDELETEDDATA(MT,SK)	(((MT << 7) | 0x40 | (SK << 5) | 0xC) & 0xFF)
#define   FD_WRITEDATA(MT)				(((MT << 7) | 0x40 |  0x5) & 0xFF)
#define   FD_WRITEDELETEDDATA(MT)		(((MT << 7) | 0x40 |  0x9) & 0xFF)
#define	FD_READTRACK					0x42
#define	FD_VERIFY(MT,SK)				(((MT << 7) | 0x40 | (SK << 5) | 0x16) & 0xFF)
#define	FD_FORMATTRACK				0x4D
//控制命令
#define	FD_RECALIBRATE				0x7
#define	FD_SIS							0x8	//SENSE INTERRUPT STATUS,only issued immediately after SEEK ,RELATIVE SEEK and RECALIBRATE
											//to terminal them and to provide verification of the head position(PCN). 
											//The H (Head Address) bit in ST0 will alwaysreturn a "0''.
#define	FD_SPECIFY						0x3
#define	FD_SDS							0x4	//SENSE DRIVER STATUS
#define	FD_SEEK						0xF
#define	FD_CONFIGURE					0x13
#define	FD_READID						0x4A

//磁盘执行命令返回结果
#define	FD_ST0_IC_ABN					0x40			//非正常终止
#define	FD_ST0_IC_INV					0x80			//无效命令
#define	FD_ST0_IC_ABNP				0xC0
#define	FD_ST0_EC						0x10			//设备检测错误

#define	FD_ST1_EN						0x80			//越过该磁道最后一个扇区
#define	FD_ST1_DE						0x20			//CRC错误
#define	FD_ST1_OR						0x10			//超时或者过载，发生数据丢失
#define	FD_ST1_ND						0x4				//没有数据
#define	FD_ST1_NW						0x2				//写保护
#define	FD_ST1_MA						0x1				//丢失地址标签

#define	FD_ST2_CM						0x40			//丢失控制标签
#define	FD_ST2_DD						0x20			//数据域CRC错误
#define	FD_ST2_WC						0x10			//错误柱面号
#define	FD_ST2_BC						0x2				//坏柱面
#define	FD_ST2_MD						0x1				//丢失地址标签


//软磁盘物理数据
#define	FD_CNUM						80	//总柱面数
#define	FD_HNUM						2	//每柱面磁头数
#define	FD_SNUM						18	//每磁头扇区数

typedef	struct	_FD_REQUEST
{
	ulong		WY_ulPID;				//发出请求的进程
	ulong		WY_ulTID;				//发出请求的线程
	ushort		WY_usDirect;			//读、写请求
	ushort		WY_usOpMode;			//同步、异步请求
	ulong		WY_ulLBAAddr;			//磁盘LBA地址
	ulong		WY_ulSectorNum;		//读取扇区数目 
	PVOID		WY_ulUserBuff;			//进程欲放置区域
	ulong		WY_ulCylinder;			//柱面
	ulong		WY_ulHead;				//磁头
	ulong		WY_ulSector;			//起始扇区号 
	ulong		WY_ulState;				//状态
	struct _FD_REQUEST	*WY_pNext;		//下一个请求块
}WY_FDRequest,*WY_pFDRequest;
//定义WY_usDirect的取值，DMA一致

#define	FD_WRITE_DIR					8	//MM->I/O	DMA READ
#define	FD_READ_DIR					4	//I/O->MM	DMA WRITE

#define	FD_STATE_WAIT					0
// 1-3 表示重试次数.
#define	FD_STATE_FAILD				4	//失败
#define	FD_STATE_OK					6

#define	FD_OPMODE_SYN					0	//同步
#define	FD_OPMODE_ASYN				1	//异步

void	FloppyInit();

BOOL SendByte(BYTE WY_byteData);

BOOL GetByte(BYTE *WY_byteData);

inline void  LBAtoCHS(ulong WY_ulLBAAddr,ulong *WY_ulCylinder,ulong *WY_ulHead,ulong *WY_ulSector);

inline ulong CHStoLBA(ulong WY_ulCylinder,ulong WY_ulHead,ulong WY_ulSector);
//
//复位操作
//
BOOL FDReset();

//
//重校中断处理
//
BOOL FDRecalibrate();

//
//驱动使能
//
void EnableDrive(int DriveNum);

//
//禁用驱动
//
void DisableDrive(int DriveNum);

//
//读写磁盘
//
void FDRWSector(WY_pFDRequest WY_pRWRQ);

//
//读写磁盘中断处理
//
void FDRWIntProc();

ulong  RWProc(PVOID WY_pParam);

ulong FDRWCHS(ulong WY_ulCylinder,ulong WY_ulHead,ulong WY_ulSector,ulong WY_ulSecNum,void* WY_pBuf,ulong WY_ulOPDir,ulong WY_ulOPMode);
ulong FDRWLBA(ulong WY_ulLBAAddr,ulong WY_ulSecNum,void* WY_pBuf,ulong WY_ulOPDir,ulong WY_ulOPMode);

////////////////////////////////////
//                               ///
//磁盘请求队列操作               ///
//                               ///
////////////////////////////////////
//
//放入队列
//
void InsertToFDRQ(WY_pFDRequest WY_pFDOPR);

//
//从队列中取出，取出后在队列中删除此数据
//
WY_pFDRequest GetFromFDRQ();
#endif
