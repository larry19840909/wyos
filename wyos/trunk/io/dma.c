/***************************************************************************
			WYOS dma.c
			ISA BUS DMA���Դ�ļ�
						����:WY.lslrt				editor	 :WY.lslrt
						����:2006/9/1				date	 	 :2006/9/1
						��Ȩ:WY �� WY.lslrt���� 	copyright  :WY and WY.lslrt
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

/*-----------------------DMA����������START---------------------------------*/
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
	//���μĴ�������ַ�Ĵ����������Ĵ�����ҳ��Ĵ�����ģʽ�Ĵ���,�Ⱥ󴥷���
	ushort	WY_usMaskPort,WY_usAddrPort,WY_usCntPort,WY_usPagePort,WY_usModePort,WY_usFFPort;
	ushort	WY_usOffset;
	BYTE	WY_bytePage;

	//������־�Ĵ���
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

	//���ȹرո�ͨ��
	CloseDMAChannel(WY_ulDmaChn);

	//����ģʽ��
	WritePort(WY_usModePort,WY_ucTransferMode|WY_ucAddrDir | WY_ucCntInit | WY_ucDirect | WY_ulDmaChn % 4);
	//���õ�ַ�ͼ�����
	WY_usOffset = WY_ulPhyAddress & 0xFFFF;
	WY_bytePage = (WY_ulPhyAddress & 0xFF0000) >> 16;

	ClearFF(WY_usFFPort);
	WritePort(WY_usAddrPort,WY_usOffset & 0xFF);	//����ƫ�ƵͰ�λ
	WritePort(WY_usAddrPort,(WY_usOffset >> 8) & 0xFF);	//����ƫ����
	ClearFF(WY_usFFPort);
	WritePort(WY_usCntPort,(WY_ulDataSize - 1) & 0xFF); //���ͼ����Ͱ�λ
	WritePort(WY_usCntPort,((WY_ulDataSize - 1) >> 8) & 0xFF);//���ͼ����߰�λ

	//����ҳ���ַ
	WritePort(WY_usPagePort,WY_bytePage);
	//����ͨ��
	OpenDMAChannel(WY_ulDmaChn);

	//�ָ���־�Ĵ���
	__asm__("popf");
}

void OpenDMAChannel(ulong WY_ulDmaChn)
{
	ushort	WY_usMaskPort;					//���μĴ���

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
	ushort	WY_usMaskPort;					//���μĴ���

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
/*-----------------------DMA����������END-----------------------------------*/

