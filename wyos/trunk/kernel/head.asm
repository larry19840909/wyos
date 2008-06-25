;***************************************************************************
;			WYOS Head.asm
;			内核加载初始化代码，仅在Setup.asm后执行	
;		编码:WY.lslrt			editor	 :WY.lslrt
;		日期:2005/12/2			date	 :2005/12/2
;		更新:
;		版权:WY 和 WY.lslrt所有		copyright:WY and WY.lslrt
;		URL:http://www.wyos.net http://wylslrt.go.3322.org
;***************************************************************************
PDTBASE		equ	0x0
PETBASE		equ	0x1000
GDTBASE		equ	0x2000
USERDESC	equ	0x3000	;留给用户进程的全局描述符，
				;模式采用类似于linux的设置，
				;ldt0,tss0,ldt1,tss1如此类推，
				;总共可以支持128个用户任务
IDTBASE		equ	0x3800

[BITS 32]
[EXTERN _main]
[EXTERN _WY_nVideoPos]
[GLOBAL _WYOSEntry]
[SECTION .text]
_WYOSEntry:
	;低1M的地址空间用于存放页表、描述附表、
	;DMA高速缓存等其它一类系统信息
start:
	call	InitGdt
	call	InitIdt
	lgdt	[gdt_desc]
	lidt	[idt_desc]
	jmp	clear
clear:
	mov	ax,0x10
	mov	ds,ax
	mov	es,ax
	mov	fs,ax
	mov	gs,ax
	mov	ss,ax
	mov	esp,0x400000

	call	PagingMode
	call	_main
	jmp	$
;=======================================
;	初始化GDT
;=======================================
;                   high base GD A HLIM PDPd TYPE MIDDLE BASE                   Low Limit
;System Data
;0x00CF92000000FFFF 0000 0000 1100 1111 1001 0010 0000 0000 0000 0000 0000 0000 1111 1111 1111 1111
;System Code
;0x00CF9A000000FFFF 0000 0000 1100 1111 1001 1010 0000 0000 0000 0000 0000 0000 1111 1111 1111 1111
InitGdt:
	mov	esi,GDTBASE
	mov	dword [esi],0
	mov	dword [esi+4],0
	add	esi,8
InitSystemCode:
	mov	word [esi],0xFFFF
	mov	word [esi+2],0x0
	mov	word [esi+4],0x9A00
	mov	word [esi+6],0x00CF
	add	esi,8
InitSystemDate:
	mov	word [esi],0xFFFF
	mov	word [esi+2],0x0
	mov	word [esi+4],0x9200
	mov	word [esi+6],0x00CF
;	add	esi,8
;InitUserCode:
;	mov	word [esi],0xFFFF
;	mov	word [esi+2],0x0
;	mov	word [esi+4],0xFA00
;	mov	word [esi+6],0x00CF
;	add	esi,8
;InitUserData:
;	mov	word [esi],0xFFFF
;	mov	word [esi+2],0x0
;	mov	word [esi+4],0xF200
;	mov	word [esi+6],0x00CF
	ret
;=======================================
;	初始化IDT
;=======================================
InitIdt:
	ret
;=======================================
;	初始化页目录和页表，
;       并进入分页模式
;=======================================
PagingMode:
SetPDT:
	mov	edi,PDTBASE
	mov	eax,0
	mov	ecx,0x400
	rep	stosd
	setFirstPDT:
		mov	edi,PDTBASE
		mov	eax,0x1001
		stosd
SetPET:
	mov	edi,PETBASE
	mov	ecx,0x400
	mov	eax,3
	SetPETValue:
		stosd
		add	eax,0x1000
		loop	SetPETValue
		mov	edi,PETBASE
	;设置页表项完成，修改CR0进入分页模式
	mov	eax,0
	mov	cr3,eax	;将页目录基址放入到CR3
	mov	eax,cr0
	or	eax,0x80000000
	mov	cr0,eax
	jmp	clear_TLB
clear_TLB:
	xor	eax,eax
	ret
;=======================================
;	伪描述符
;=======================================
gdt_desc:
	dw	0x1800
gdtaddr dd	GDTBASE
idt_desc:
	dw	0x800
idtaddr dd	IDTBASE