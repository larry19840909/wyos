;/***************************************************************************
;			WYOS exp.asm
;			cpu异常处理和一些中断处理汇编文件
;		编码:WY.lslrt			editor	 :WY.lslrt
;		日期:2006/3/5			date	 :2005/12/2
;		更新:
;		版权:WY 和 WY.lslrt所有		copyright:WY and WY.lslrt
;		URL:http://www.wyos.net http://wylslrt.go.3322.org
;***************************************************************************/
[BITS 32]
[EXTERN _puts]					;输出字符串函数
[EXTERN _putx]
[EXTERN _TimeInt]				;时间中断处理函数
[EXTERN _SysCall_C_Func]			;系统调用c接口
[EXTERN _PageExpProc]				;页面异常处理函数
[EXTERN _WY_bFDInit]				;软盘初始化好标志	
[EXTERN _WY_ulFDOpFun]				;软盘中断处理函数
[EXTERN _WY_bFDInt]				;中断发生标志
[GLOBAL _DivError]				;除法异常中断代码偏移
[GLOBAL _TimeInterrupt]				;时间中断代码偏移
[GLOBAL _SysCall]				;系统调用代码偏移
[GLOBAL _PageException]				;页面异常代码偏移 
[GLOBAL _FloppyInterrupt]
;[GLOBAL	_TestDiv]
[SECTION .text]
_TimeInterrupt:
	cli
	push	eax
	call	_TimeInt
	mov al , 0x20	;发送EOI消息
	out 0x20 , al
	out 0xa0 , al
	pop	eax
	sti
	iret
	jmp	_TimeInterrupt
_DivError:
	push	esi

	;begin
	mov	esi,DivErrMsg
	push	esi
	call	_puts
	pop	esi

	pop	esi

	;因为没有出错码，所以很难确定是何种错误
	;所以修改返回地址，查指令手册div指令在
	;x86中的除法指令长度是2字节，因此修改返回地址
	pop	eax
	add	eax,2	;修改返回地址
	push	eax
	iret
_PageException:
	push	ebp
	mov	ebp,esp

	push	eax
	mov	eax,cr2			;cr2寄存器入栈，引起页面异常的线性地址
	push	eax
	mov	eax,dword [ebp + 4]
	push	eax			;出错代码入栈
	call	_PageExpProc
	pop	eax			;弹出参数
	pop	eax			;弹出参数
	
	pop	eax			;恢复eax
	pop	ebp			;恢复ebp
	add	esp,4			;跳过错误码
	iret
;
;系统调用汇编入口
;

;SysCall函数的参数为
;ulong WY_ulSyscallIndex,ulong WY_p0,ulong WY_p1,ulong WY_p2,
;ulong WY_p3,ulong WY_p4,ulong WY_p5,ulong WY_p6
;在执行完push ebp,之后堆栈情况
;--------ebp
;--------cs selector
;--------ret addr
;--------WY_ulSyscallIndex
;--------WY_p0
;--------WY_p1
;--------WY_p2
;--------WY_p3
;--------WY_p4
;--------WY_p5
;--------WY_p6
_SysCall:
	push	ebp
	mov	ebp,esp
	;ebp增加12的原因是，栈顶为ebp,然后是ret addr和ret cs selector，所以是12个字节。
	add	ebp,12
	push	ebp
	call	_SysCall_C_Func
	pop	ebp
	pop	ebp
	;测试参数传递正确性用，先用ret代替retf
	retf	32
_FloppyInterrupt:
	push	eax
	
	mov al , 0x20	;发送EOI消息
	out 0x20 , al
	out 0xa0 , al

	mov dword [_WY_bFDInt],1
	
	mov eax,dword [_WY_ulFDOpFun]
	cmp eax,0
	jz  FDEnd
	call eax
FDEnd:	
	pop eax;
	iret
;测试除法错误中断
;_TestDiv:
;	push	esi
;	push	edx
;	push	ebx
;	push	eax
;
;	;begin
;	mov	esi,TestDivMsg
;	push	esi
;	call	_puts
;	pop	esi
;
;	mov	edx,0x1
;	mov	eax,0x100
;	mov	ebx,0
;	div	ebx
;
;	;exit
;	pop	eax
;	pop	ebx
;	pop	edx
;	pop	esi
;	ret
DivErrMsg	db	"Div Error.Please Check divisor or quotient to large",10,0
;TestDivMsg	db	"Test Div Exception Process",10,0