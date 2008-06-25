/***************************************************************************
			WYOS cpu.h
			cpu���ͷ�ļ�
						����:WY.lslrt				editor	 :WY.lslrt
						����:2005/12/2			date	 :2005/12/2
						��Ȩ:WY �� WY.lslrt���� 	copyright:WY and WY.lslrt
						URL:http://www.wyos.net http://wylslrt.go.3322.org
***************************************************************************/
#ifndef	_WYOS_CPU_H
#define	_WYOS_CPU_H

extern void DivError();
extern void TestInterrupt();

#define	wyos_save_flag() 		__asm__("pushf")
#define	wyos_restore_flag() 		__asm__("popf")

#define	WYOS_GDT_BASE				0x2000
#define	WYOS_USERDESC_BASE			0x3000
#define	WYOS_IDT_BASE					0x3800


typedef	struct _SEG_DESC
{
	ulong	WY_ulLowSegLimit:16;		//�ν��޵�16 λ
	ulong	WY_ulLowSegBase:24;		//�λ�ַ��24 λ
	ulong	WY_ulSegTYPE:4;			//�������εľ�������
	ulong	WY_ulDescType:1;			//�����������ͣ�1�洢��,0��ϵͳ��
	ulong	WY_ulDescDPL:2;			//��������Ȩ��
	ulong	WY_ulPresent:1;				//����λ
	ulong	WY_ulHighSegLimit:4;		//�ν��޸�4λ
	ulong	WY_ulSoftUse:1;				//�������λ
	ulong	WY_ulReserved:1;			//����Ϊ0
	ulong	WY_ulD:1;				//�����λ
	                                                        //��ִ�жε���������:
	                                                        //Dλ������ָ��ʹ�õĵ�ַ����������Ĭ�ϵĴ�С��
	                                                        //D=1��ʾĬ�������ָ��ʹ��32λ��ַ��32λ��8λ��������
	                                                        //�����Ĵ����Ҳ��Ϊ32λ����Σ�D=0 ��ʾĬ������£�
	                                                        //ʹ��16λ��ַ��16λ��8λ�������������Ĵ����Ҳ��Ϊ16λ����Σ�
	                                                        //����80286���ݡ�����ʹ�õ�ַ��Сǰ׺�Ͳ�������Сǰ׺
	                                                        //�ֱ�ı�Ĭ�ϵĵ�ַ��������Ĵ�С
	                                                        //��������չ���ݶε��������У�Dλ�����ε��ϲ��߽硣
	                                                        //D=1��ʾ�ε��ϲ�����Ϊ4G��D=0��ʾ�ε��ϲ�����Ϊ64K��
	                                                        //����Ϊ����80286���ݡ�
	                                                        //��������SS�Ĵ���Ѱַ�Ķ��������У�Dλ������ʽ�Ķ�ջ
	                                                        //����ָ��(��PUSH��POPָ��)ʹ�ú��ֶ�ջָ��Ĵ�����
	                                                        //D=1��ʾʹ��32λ��ջָ��Ĵ���ESP��D=0��ʾʹ��16λ��ջ
	                                                        //ָ��Ĵ���SP������80286���ݡ� 
	ulong	WY_ulGranularity:1;		//�ν�������0����Ϊ�ֽڣ�1����Ϊ4KB
	ulong	WY_ulHighSegBase:8;	//�λ�ַ��8λ
}WY_StorageDesc,*WY_pStorageDesc,WY_SystemDesc,*WY_pSystemDesc;

typedef	struct	_GATE_DESC
{
	ulong	WY_ulLowOffset:16;			//ƫ�Ƶ�16λ
	ulong	WY_ulDescSelector:16;		//�����ѡ����
	ulong	WY_ulUsParamCnt:5;			//����������
	ulong	WY_ulReserved:3;			//��������Ϊ0
	ulong	WY_ulSegType:4;			//������
	ulong	WY_ulDescType:1;			//����������
	ulong	WY_ulDescDPL:2;			//��������Ȩ��
	ulong	WY_ulPresent:1;				//����λ
	ulong	WY_ulHighOffset:16;			//ƫ�Ƹ�16λ
}WY_GATEDesc,*WY_pGATEDesc;

#define	STORAGE_DESC										0x1
#define	SYSTEM_DESC										0x0
#define	GATE_DESC											0x0

#define	SEGMENT_READONLY									0x0
#define	SEGMENT_READONLY_VISITED						0x1
#define	SEGMENT_READWRITE								0x2
#define	SEGMENT_READWRITE_VISITED						0x3
#define	SEGMENT_READONLY_LOWEXTEND						0x4
#define	SEGMENT_READONLY_VISITED_LOWEXTEND			0x5
#define 	SEGMENT_READWRITE_LOWEXTEND					0x6
#define	SEGMENT_READWRITE_LOWEXTEND_VISITED			0x7
#define	SEGMENT_EXECUTEONLY								0x8
#define	SEGMENT_EXECUTEONLY_VISITED						0x9
#define	SEGMENT_EXECUTEREAD								0xA
#define	SEGMENT_EXECUTEREAD_VISITED						0xB
#define	SEGMENT_EXECUTEONLY_CONSISTENTCODE			0xC
#define	SEGMENT_EXECUTEONLY_CONSISTENDCODE_VISITED	0xD
#define	SEGMENT_EXECUTEREAD_CONSISTENDCODE			0xE
#define	SEGMENT_EXECUTEREAD_CONSISTENDCODE_VISTIED	0xF

#define	SYSTEM_USEABLE_286TSS							0x1
#define	SYSTEM_LDT											0x2
#define	SYSTEM_BUSY_286TSS								0x3
#define	GATE_286CALL										0x4
#define	GATE_TASK											0x5
#define	GATE_286INTERRUPT									0x6
#define	GATE_286TRAP										0x7
#define	SYSTEM_USEABLE_386TSS							0x9
#define	SYSTEM_BUSY_386TSS								0xB
#define	GATE_386CALL										0xC
#define	GATE_386INTERRUPT									0xE
#define	GATE_386TRAP										0xF

#define	SEGMENT_RING0										0x0
#define	SEGMENT_RING1										0x1
#define	SEGMENT_RING2										0x2
#define	SEGMENT_RING3										0x3

typedef	struct	_TSS
{
	ulong	WY_ulBLink;							//��16 λ����Ϊ0
	ulong	WY_ulESP0;
	ulong	WY_ulSS0;							//��16 λ����Ϊ0
	ulong	WY_ulESP1;
	ulong	WY_ulSS1;							//��16 λ����Ϊ0
	ulong	WY_ulESP2;
	ulong	WY_ulSS2;							//��16 λ����Ϊ0
	ulong	WY_ulCR3;
	ulong	WY_ulEIP;
	ulong	WY_ulEFLAGS;
	ulong	WY_ulEAX;
	ulong	WY_ulECX;
	ulong	WY_ulEDX;
	ulong	WY_ulEBX;
	ulong	WY_ulESP;
	ulong	WY_ulEBP;
	ulong	WY_ulESI;
	ulong	WY_ulEDI;
	ulong	WY_ulES;							//��16 λ����Ϊ0
	ulong	WY_ulCS;							//��16 λ����Ϊ0
	ulong	WY_ulSS;							//��16 λ����Ϊ0
	ulong	WY_ulDS;							//��16 λ����Ϊ0
	ulong	WY_ulFS;							//��16 λ����Ϊ0
	ulong	WY_ulGS;							//��16 λ����Ϊ0
	ulong	WY_ulLDT;							//��16 λ����Ϊ0
	ushort	WY_usTrace:1;
	ushort	WY_usReserved:15;
	ushort	WY_usBitmapOffset;
}WY_TSS,*WY_pTSS;

#define	TSS_END_FLAG				0xFF
//
//������ʼ��������״̬���������жϣ���һЩcpu��ΧоƬ
//
void cpuinit();

//
//���ж�
//
#define	wyos_open_int()		__asm__("sti")

//
//���ж�
//
#define	wyos_close_int()		__asm__("cli")

//�жϴ��������
#define	INT_PROC	PVOID
//
//��װһ���ж�
//�������������ѡ����ֻ��Ҫ��д0x8  ��
//����������ţ���Ҫ��д����״̬��ѡ���ӡ�
//
ulong SetInterrupt(INT_PROC WY_pIntProc,ulong WY_ulSelector,ulong WY_ulDPL,ulong WY_ulSegType,ulong WY_ulIntNum);

//
//ж��һ���ж�
//
ulong UninsInterrupt(ulong WY_ulIntNum);

//������󷵻���
#define	WYOS_IDT_ERR_OK					0x0
#define	WYOS_IDT_ERR_NO_PARAM			0x1
#define	WYOS_IDT_ERR_INTNUM_LARGE		0x2
#define	WYOS_IDT_ERR_GATE_EXIST			0x4
#define	WYOS_IDT_ERR_GATE_NOTEXIST		0x5
void		TimeInt();

//����ȫ��������
void * allocGlobalDesc(BOOL WY_isSys);

//�ͷ�ȫ��������
void	freeGlobalDesc(int	WY_nSelector);

//�����ж�������
void * allocInterruptGate();

//�ͷ��ж�������
void freeInterruptGate(WY_pGATEDesc WY_pIntGate);

//��õ�ǰʱ��tick
ulong GetCurrentTick();
#endif

