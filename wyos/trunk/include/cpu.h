/***************************************************************************
			WYOS cpu.h
			cpu相关头文件
						编码:WY.lslrt				editor	 :WY.lslrt
						日期:2005/12/2			date	 :2005/12/2
						版权:WY 和 WY.lslrt所有 	copyright:WY and WY.lslrt
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
	ulong	WY_ulLowSegLimit:16;		//段界限低16 位
	ulong	WY_ulLowSegBase:24;		//段基址低24 位
	ulong	WY_ulSegTYPE:4;			//所描述段的具体属性
	ulong	WY_ulDescType:1;			//描述符的类型，1存储段,0者系统段
	ulong	WY_ulDescDPL:2;			//描述符特权级
	ulong	WY_ulPresent:1;				//存在位
	ulong	WY_ulHighSegLimit:4;		//段界限高4位
	ulong	WY_ulSoftUse:1;				//软件利用位
	ulong	WY_ulReserved:1;			//必须为0
	ulong	WY_ulD:1;				//特殊的位
	                                                        //可执行段的描述符中:
	                                                        //D位决定了指令使用的地址及操作数所默认的大小。
	                                                        //D=1表示默认情况下指令使用32位地址及32位或8位操作数，
	                                                        //这样的代码段也称为32位代码段；D=0 表示默认情况下，
	                                                        //使用16位地址及16位或8位操作数，这样的代码段也称为16位代码段，
	                                                        //它与80286兼容。可以使用地址大小前缀和操作数大小前缀
	                                                        //分别改变默认的地址或操作数的大小
	                                                        //在向下扩展数据段的描述符中，D位决定段的上部边界。
	                                                        //D=1表示段的上部界限为4G；D=0表示段的上部界限为64K，
	                                                        //这是为了与80286兼容。
	                                                        //在描述由SS寄存器寻址的段描述符中，D位决定隐式的堆栈
	                                                        //访问指令(如PUSH和POP指令)使用何种堆栈指针寄存器。
	                                                        //D=1表示使用32位堆栈指针寄存器ESP；D=0表示使用16位堆栈
	                                                        //指针寄存器SP，这与80286兼容。 
	ulong	WY_ulGranularity:1;		//段界限粒度0粒度为字节，1粒度为4KB
	ulong	WY_ulHighSegBase:8;	//段基址高8位
}WY_StorageDesc,*WY_pStorageDesc,WY_SystemDesc,*WY_pSystemDesc;

typedef	struct	_GATE_DESC
{
	ulong	WY_ulLowOffset:16;			//偏移低16位
	ulong	WY_ulDescSelector:16;		//代码段选择子
	ulong	WY_ulUsParamCnt:5;			//参数计数器
	ulong	WY_ulReserved:3;			//保留必须为0
	ulong	WY_ulSegType:4;			//段类型
	ulong	WY_ulDescType:1;			//描述符类型
	ulong	WY_ulDescDPL:2;			//描述符特权级
	ulong	WY_ulPresent:1;				//存在位
	ulong	WY_ulHighOffset:16;			//偏移高16位
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
	ulong	WY_ulBLink;							//高16 位必须为0
	ulong	WY_ulESP0;
	ulong	WY_ulSS0;							//高16 位必须为0
	ulong	WY_ulESP1;
	ulong	WY_ulSS1;							//高16 位必须为0
	ulong	WY_ulESP2;
	ulong	WY_ulSS2;							//高16 位必须为0
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
	ulong	WY_ulES;							//高16 位必须为0
	ulong	WY_ulCS;							//高16 位必须为0
	ulong	WY_ulSS;							//高16 位必须为0
	ulong	WY_ulDS;							//高16 位必须为0
	ulong	WY_ulFS;							//高16 位必须为0
	ulong	WY_ulGS;							//高16 位必须为0
	ulong	WY_ulLDT;							//高16 位必须为0
	ushort	WY_usTrace:1;
	ushort	WY_usReserved:15;
	ushort	WY_usBitmapOffset;
}WY_TSS,*WY_pTSS;

#define	TSS_END_FLAG				0xFF
//
//用来初始化处理器状态，错误处理中断，和一些cpu外围芯片
//
void cpuinit();

//
//开中断
//
#define	wyos_open_int()		__asm__("sti")

//
//关中断
//
#define	wyos_close_int()		__asm__("cli")

//中断处理函数句柄
#define	INT_PROC	PVOID
//
//安装一个中断
//如果不是任务门选择子只需要填写0x8  ，
//如果是任务门，需要填写任务状态段选择子。
//
ulong SetInterrupt(INT_PROC WY_pIntProc,ulong WY_ulSelector,ulong WY_ulDPL,ulong WY_ulSegType,ulong WY_ulIntNum);

//
//卸载一个中断
//
ulong UninsInterrupt(ulong WY_ulIntNum);

//定义错误返回码
#define	WYOS_IDT_ERR_OK					0x0
#define	WYOS_IDT_ERR_NO_PARAM			0x1
#define	WYOS_IDT_ERR_INTNUM_LARGE		0x2
#define	WYOS_IDT_ERR_GATE_EXIST			0x4
#define	WYOS_IDT_ERR_GATE_NOTEXIST		0x5
void		TimeInt();

//申请全局描述符
void * allocGlobalDesc(BOOL WY_isSys);

//释放全局描述符
void	freeGlobalDesc(int	WY_nSelector);

//申请中断描述符
void * allocInterruptGate();

//释放中断描述符
void freeInterruptGate(WY_pGATEDesc WY_pIntGate);

//获得当前时间tick
ulong GetCurrentTick();
#endif

