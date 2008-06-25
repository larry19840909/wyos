/***************************************************************************
			WYOS dma.c
			ISA BUS DMA相关源文件
						编码:WY.lslrt				editor	 :WY.lslrt
						日期:2006/9/1				date	 	 :2006/9/1
						版权:WY 和 WY.lslrt所有 	copyright  :WY and WY.lslrt
						URL:http://www.wyos.net http://wylslrt.go.3322.org
***************************************************************************/
#include "..\include\WYOSType.h"
#include "..\include\cpu.h"
#include "..\include\io.h"
#include "..\include\syscall.h"
#include "..\video\video.h"
#include "..\include\string.h"
#include "..\include\memory.h"
#include "..\include\process.h"

/*-----------------------DMA控制器操作START---------------------------------*/
void InitDMA()
{
	int	i;
	ClearFF(FFPORT_DMA1);
	ClearFF(FFPORT_DMA2);

	for(i = 1;i < 7;i++)
	{
		if(i != 4)
		{
			CloseDMAChannel(i);
		};
	}
	
}

void SetDMAChannel(ulong WY_ulDmaChn,ulong WY_ulPhyAddress,ushort WY_ulDataSize,
					     uchar WY_ucTransferMode,uchar WY_ucDirect,uchar WY_ucAddrDir,uchar WY_ucCntInit)
{
	//屏蔽寄存器，地址寄存器，计数寄存器，页面寄存器，模式寄存器,先后触发器
	ushort	WY_usMaskPort,WY_usAddrPort,WY_usCntPort,WY_usPagePort,WY_usModePort,WY_usFFPort;
	ushort	WY_usOffset;
	BYTE	WY_bytePage;

	//保护标志寄存器
	__asm__("pushf");
	wyos_close_int();
	
	switch(WY_ulDmaChn)
	{
	case 1:
		WY_usMaskPort = 0xA;
		WY_usModePort = 0xB;
		WY_usAddrPort = 0x2;
		WY_usCntPort = 0x3;
		WY_usFFPort = FFPORT_DMA1;
		WY_usPagePort = 0x83;
		break;
	case 2:
		WY_usMaskPort = 0xA;
		WY_usModePort = 0xB;
		WY_usAddrPort = 0x4;
		WY_usCntPort = 0x5;
		WY_usFFPort = FFPORT_DMA1;
		WY_usPagePort = 0x81;
		break;
	case 3:
		WY_usMaskPort = 0xA;
		WY_usModePort = 0xB;
		WY_usAddrPort = 0x6;
		WY_usCntPort = 0x7;
		WY_usFFPort = FFPORT_DMA1;
		WY_usPagePort = 0x82;
		break;
	case 5:
		WY_usMaskPort = 0xD4;
		WY_usModePort = 0xD6;
		WY_usAddrPort = 0xC4;
		WY_usCntPort = 0xC6;
		WY_usFFPort = FFPORT_DMA2;
		WY_usPagePort = 0x8B;
		break;
	case 6:
		WY_usMaskPort = 0xD4;
		WY_usModePort = 0xD6;
		WY_usAddrPort = 0xC8;
		WY_usCntPort = 0xCA;
		WY_usFFPort = FFPORT_DMA2;
		WY_usPagePort = 0x89;
		break;
	case 7:
		WY_usMaskPort = 0xD4;
		WY_usModePort = 0xD6;
		WY_usAddrPort = 0xCC;
		WY_usCntPort = 0xCE;
		WY_usFFPort = FFPORT_DMA2;
		WY_usPagePort = 0x8A;
		break;
	default:
		return;
	}

	//首先关闭该通道
	CloseDMAChannel(WY_ulDmaChn);

	//设置模式字
	WritePort(WY_usModePort,WY_ucTransferMode|WY_ucAddrDir | WY_ucCntInit | WY_ucDirect | WY_ulDmaChn % 4);
	//设置地址和计数器
	WY_usOffset = WY_ulPhyAddress & 0xFFFF;
	WY_bytePage = (WY_ulPhyAddress & 0xFF0000) >> 16;

	ClearFF(WY_usFFPort);
	WritePort(WY_usAddrPort,WY_usOffset & 0xFF);	//发送偏移低八位
	WritePort(WY_usAddrPort,(WY_usOffset >> 8) & 0xFF);	//发送偏移量
	ClearFF(WY_usFFPort);
	WritePort(WY_usCntPort,(WY_ulDataSize - 1) & 0xFF); //发送计数低八位
	WritePort(WY_usCntPort,((WY_ulDataSize - 1) >> 8) & 0xFF);//发送计数高八位

	//设置页面地址
	WritePort(WY_usPagePort,WY_bytePage);
	//启用通道
	OpenDMAChannel(WY_ulDmaChn);

	//恢复标志寄存器
	__asm__("popf");
}

void OpenDMAChannel(ulong WY_ulDmaChn)
{
	ushort	WY_usMaskPort;					//屏蔽寄存器

	if(WY_ulDmaChn < 4)
		{
			WY_usMaskPort = 0xA;
		}
	else
		{
			WY_usMaskPort = 0xD4;
		}

	WritePort(WY_usMaskPort,WY_ulDmaChn % 4);
}

void CloseDMAChannel(ulong WY_ulDmaChn)
{
	ushort	WY_usMaskPort;					//屏蔽寄存器

	if(WY_ulDmaChn < 4)
		{
			WY_usMaskPort = 0xA;
		}
	else
		{
			WY_usMaskPort = 0xD4;
		}
	WritePort(WY_usMaskPort,(WY_ulDmaChn % 4) | 0x4);
}
/*-----------------------DMA控制器操作END-----------------------------------*/

