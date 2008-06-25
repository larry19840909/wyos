/***************************************************************************
			WYOS io.h
			����������ͷ�ļ�
						����:WY.lslrt			editor	 :WY.lslrt
						����:2005/12/2			date	 :2005/12/2
						��Ȩ:WY �� WY.lslrt���� copyright:WY and WY.lslrt
						URL:http://www.wyos.net http://wylslrt.go.3322.org
***************************************************************************/
#ifndef	_WYOS_IO_H
#define	_WYOS_IO_H

//
//�������ݵ�ָ���˿�
//
void WritePort(ushort WY_usPort,BYTE	WY_bData);
void WritePortW(ushort WY_usPort,ulong WY_ulData);
//
//��ָ���˿ڶ�������
//
BYTE ReadPort(ushort WY_usPort);
ulong ReadPortW(ushort WY_usPort);
//
//������һЩӲ���豸�ĳ�ʼ���Ͳ�������
//

//
//8259��ʼ������
//
void Init8259();

//
//����ĳ��Ӳ���ж�
//
BOOL OpenHardInt(ulong WY_ulIRQL);

//
//�ر�ĳ��Ӳ���ж�
//
BOOL CloseHardInt(ulong WY_ulIRQL);

//
//��ʼ����ʱ��
//
void InitTimer();

//ISA DMA����������
//ģʽ�Ĵ�����λ����
//ģʽѡ�� 7:6
#define DMA_ISA_DEMANDMODE 0 		//00
#define DMA_ISA_SINGLEMODE 0x40 		//01
#define DMA_ISA_BLOCKMODE 0x80 		//10
#define DMA_ISA_CASCADEMODE 0xC0 	//11 (����ģʽ����ʹ��)

//��ַ���� bit 5
#define DMA_ISA_ADDRDEC 32	// 1
#define DMA_ISA_ADDRINC 0 	// 0

//�Զ���ʼ�������� bit 4
#define DMA_ISA_AUTOINIT  		 0x10 // 1
#define DMA_ISA_SINGLECYCLE  	 0 //0
//���䷽ʽ 3:2
#define DMA_ISA_VERIFY 			 0 //00
#define DMA_ISA_WRITE 			 4//01		//��дΪI/O -> MM
#define DMA_ISA_READ			 8 //10		//�ö�ΪMM->I/O
//
//��ʼ��DMA������
//
void InitDMA();

//
//����DMAͨ��
//
//WY_ulPhyAddress�����������ڴ�Ͷ�16M֮��
//WY_ulDataSize��1-3ͨ������С��64KB,5-7����С��128KB
//WY_ucTransferMode����ģʽ
//WY_ucDirect���䷽��
//WY_ucAddrDir��ַ������־
//WY_ucCntInit�����Զ���ʼ����־
void SetDMAChannel(ulong WY_ulDmaChn,ulong WY_ulPhyAddress,ushort WY_ulDataSize,
					     uchar WY_ucTransferMode,uchar WY_ucDirect,uchar WY_ucAddrDir,uchar WY_ucCntInit);

//
//��DMAͨ��
//
void OpenDMAChannel(ulong WY_ulDmaChn);

//
//�ر�DMAͨ��
//
void CloseDMAChannel(ulong WY_ulDmaChn);

//
//����Ⱥ󴥷���
//
#define	FFPORT_DMA1	0xC
#define	FFPORT_DMA2	0xD8
#define	ClearFF(FFPORT)	__asm__("out %%al,%%dx"::"a"(0),"d"(FFPORT))
#endif
