/***************************************************************************
			WYOS floppy.h
			�������ͷ�ļ�
						����:WY.lslrt				editor	 :WY.lslrt
						����:2005/12/2			date	 :2005/12/2
						��Ȩ:WY �� WY.lslrt���� 	copyright:WY and WY.lslrt
						URL:http://www.wyos.net http://wylslrt.go.3322.org
***************************************************************************/
#ifndef	WYOS_FLOPPY_H
#define	WYOS_FLOPPY_H

#define	FD_IRQL			0x6
//�������Ĵ�����ַ
#define	FD_SRA			0x3F0
#define	SD_SRB			0x3F1
#define	FD_DOR			0x3F2
#define	FD_TDR			0x3F3
#define	FD_MSR			0x3F4
#define	FD_DSR			0x3F4
#define	FD_FIFO		0x3F5
#define	FD_DIR			0x3F7
#define	FD_CCR			0x3F7

//MSR�Ĵ�����ָʾ������״̬
#define	FD_READABLE	0xC0
#define	FD_WRITEABLE	0x80

//����������
//���ݴ�������
#define	FD_READDATA(MT,SK)			(((MT << 7) | 0x40 | (SK << 5) | 0x6) & 0xFF)
#define	FD_READDELETEDDATA(MT,SK)	(((MT << 7) | 0x40 | (SK << 5) | 0xC) & 0xFF)
#define   FD_WRITEDATA(MT)				(((MT << 7) | 0x40 |  0x5) & 0xFF)
#define   FD_WRITEDELETEDDATA(MT)		(((MT << 7) | 0x40 |  0x9) & 0xFF)
#define	FD_READTRACK					0x42
#define	FD_VERIFY(MT,SK)				(((MT << 7) | 0x40 | (SK << 5) | 0x16) & 0xFF)
#define	FD_FORMATTRACK				0x4D
//��������
#define	FD_RECALIBRATE				0x7
#define	FD_SIS							0x8	//SENSE INTERRUPT STATUS,only issued immediately after SEEK ,RELATIVE SEEK and RECALIBRATE
											//to terminal them and to provide verification of the head position(PCN). 
											//The H (Head Address) bit in ST0 will alwaysreturn a "0''.
#define	FD_SPECIFY						0x3
#define	FD_SDS							0x4	//SENSE DRIVER STATUS
#define	FD_SEEK						0xF
#define	FD_CONFIGURE					0x13
#define	FD_READID						0x4A

//����ִ������ؽ��
#define	FD_ST0_IC_ABN					0x40			//��������ֹ
#define	FD_ST0_IC_INV					0x80			//��Ч����
#define	FD_ST0_IC_ABNP				0xC0
#define	FD_ST0_EC						0x10			//�豸������

#define	FD_ST1_EN						0x80			//Խ���ôŵ����һ������
#define	FD_ST1_DE						0x20			//CRC����
#define	FD_ST1_OR						0x10			//��ʱ���߹��أ��������ݶ�ʧ
#define	FD_ST1_ND						0x4				//û������
#define	FD_ST1_NW						0x2				//д����
#define	FD_ST1_MA						0x1				//��ʧ��ַ��ǩ

#define	FD_ST2_CM						0x40			//��ʧ���Ʊ�ǩ
#define	FD_ST2_DD						0x20			//������CRC����
#define	FD_ST2_WC						0x10			//���������
#define	FD_ST2_BC						0x2				//������
#define	FD_ST2_MD						0x1				//��ʧ��ַ��ǩ


//�������������
#define	FD_CNUM						80	//��������
#define	FD_HNUM						2	//ÿ�����ͷ��
#define	FD_SNUM						18	//ÿ��ͷ������

typedef	struct	_FD_REQUEST
{
	ulong		WY_ulPID;				//��������Ľ���
	ulong		WY_ulTID;				//����������߳�
	ushort		WY_usDirect;			//����д����
	ushort		WY_usOpMode;			//ͬ�����첽����
	ulong		WY_ulLBAAddr;			//����LBA��ַ
	ulong		WY_ulSectorNum;		//��ȡ������Ŀ 
	PVOID		WY_ulUserBuff;			//��������������
	ulong		WY_ulCylinder;			//����
	ulong		WY_ulHead;				//��ͷ
	ulong		WY_ulSector;			//��ʼ������ 
	ulong		WY_ulState;				//״̬
	struct _FD_REQUEST	*WY_pNext;		//��һ�������
}WY_FDRequest,*WY_pFDRequest;
//����WY_usDirect��ȡֵ��DMAһ��

#define	FD_WRITE_DIR					8	//MM->I/O	DMA READ
#define	FD_READ_DIR					4	//I/O->MM	DMA WRITE

#define	FD_STATE_WAIT					0
// 1-3 ��ʾ���Դ���.
#define	FD_STATE_FAILD				4	//ʧ��
#define	FD_STATE_OK					6

#define	FD_OPMODE_SYN					0	//ͬ��
#define	FD_OPMODE_ASYN				1	//�첽

void	FloppyInit();

BOOL SendByte(BYTE WY_byteData);

BOOL GetByte(BYTE *WY_byteData);

inline void  LBAtoCHS(ulong WY_ulLBAAddr,ulong *WY_ulCylinder,ulong *WY_ulHead,ulong *WY_ulSector);

inline ulong CHStoLBA(ulong WY_ulCylinder,ulong WY_ulHead,ulong WY_ulSector);
//
//��λ����
//
BOOL FDReset();

//
//��У�жϴ���
//
BOOL FDRecalibrate();

//
//����ʹ��
//
void EnableDrive(int DriveNum);

//
//��������
//
void DisableDrive(int DriveNum);

//
//��д����
//
void FDRWSector(WY_pFDRequest WY_pRWRQ);

//
//��д�����жϴ���
//
void FDRWIntProc();

ulong  RWProc(PVOID WY_pParam);

ulong FDRWCHS(ulong WY_ulCylinder,ulong WY_ulHead,ulong WY_ulSector,ulong WY_ulSecNum,void* WY_pBuf,ulong WY_ulOPDir,ulong WY_ulOPMode);
ulong FDRWLBA(ulong WY_ulLBAAddr,ulong WY_ulSecNum,void* WY_pBuf,ulong WY_ulOPDir,ulong WY_ulOPMode);

////////////////////////////////////
//                               ///
//����������в���               ///
//                               ///
////////////////////////////////////
//
//�������
//
void InsertToFDRQ(WY_pFDRequest WY_pFDOPR);

//
//�Ӷ�����ȡ����ȡ�����ڶ�����ɾ��������
//
WY_pFDRequest GetFromFDRQ();
#endif
