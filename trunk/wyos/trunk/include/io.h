/***************************************************************************
			WYOS io.h
			输入输出相关头文件
						编码:WY.lslrt			editor	 :WY.lslrt
						日期:2005/12/2			date	 :2005/12/2
						版权:WY 和 WY.lslrt所有 copyright:WY and WY.lslrt
						URL:http://www.wyos.net http://wylslrt.go.3322.org
***************************************************************************/
#ifndef	_WYOS_IO_H
#define	_WYOS_IO_H

//
//输入数据到指定端口
//
void WritePort(ushort WY_usPort,BYTE	WY_bData);
void WritePortW(ushort WY_usPort,ulong WY_ulData);
//
//从指定端口读入数据
//
BYTE ReadPort(ushort WY_usPort);
ulong ReadPortW(ushort WY_usPort);
//
//下面是一些硬件设备的初始化和操作函数
//

//
//8259初始化代码
//
void Init8259();

//
//开放某个硬件中断
//
BOOL OpenHardInt(ulong WY_ulIRQL);

//
//关闭某个硬件中断
//
BOOL CloseHardInt(ulong WY_ulIRQL);

//
//初始化定时器
//
void InitTimer();

//ISA DMA控制器定义
//模式寄存器各位定义
//模式选择 7:6
#define DMA_ISA_DEMANDMODE 0 		//00
#define DMA_ISA_SINGLEMODE 0x40 		//01
#define DMA_ISA_BLOCKMODE 0x80 		//10
#define DMA_ISA_CASCADEMODE 0xC0 	//11 (级联模式，不使用)

//地址增减 bit 5
#define DMA_ISA_ADDRDEC 32	// 1
#define DMA_ISA_ADDRINC 0 	// 0

//自动初始化计数器 bit 4
#define DMA_ISA_AUTOINIT  		 0x10 // 1
#define DMA_ISA_SINGLECYCLE  	 0 //0
//传输方式 3:2
#define DMA_ISA_VERIFY 			 0 //00
#define DMA_ISA_WRITE 			 4//01		//该写为I/O -> MM
#define DMA_ISA_READ			 8 //10		//该读为MM->I/O
//
//初始化DMA控制器
//
void InitDMA();

//
//设置DMA通道
//
//WY_ulPhyAddress必须在物理内存低端16M之内
//WY_ulDataSize在1-3通道必须小于64KB,5-7必须小于128KB
//WY_ucTransferMode传输模式
//WY_ucDirect传输方向
//WY_ucAddrDir地址增减标志
//WY_ucCntInit计数自动初始化标志
void SetDMAChannel(ulong WY_ulDmaChn,ulong WY_ulPhyAddress,ushort WY_ulDataSize,
					     uchar WY_ucTransferMode,uchar WY_ucDirect,uchar WY_ucAddrDir,uchar WY_ucCntInit);

//
//打开DMA通道
//
void OpenDMAChannel(ulong WY_ulDmaChn);

//
//关闭DMA通道
//
void CloseDMAChannel(ulong WY_ulDmaChn);

//
//清除先后触发器
//
#define	FFPORT_DMA1	0xC
#define	FFPORT_DMA2	0xD8
#define	ClearFF(FFPORT)	__asm__("out %%al,%%dx"::"a"(0),"d"(FFPORT))
#endif
