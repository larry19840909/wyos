;/***************************************************************************
;			WYOS exp.asm
;			cpu�쳣�����һЩ�жϴ������ļ�
;		����:WY.lslrt			editor	 :WY.lslrt
;		����:2006/3/5			date	 :2005/12/2
;		����:
;		��Ȩ:WY �� WY.lslrt����		copyright:WY and WY.lslrt
;		URL:http://www.wyos.net http://wylslrt.go.3322.org
;***************************************************************************/
[BITS 32]
[EXTERN _puts]					;����ַ�������
[EXTERN _putx]
[EXTERN _TimeInt]				;ʱ���жϴ�����
[EXTERN _SysCall_C_Func]			;ϵͳ����c�ӿ�
[EXTERN _PageExpProc]				;ҳ���쳣������
[EXTERN _WY_bFDInit]				;���̳�ʼ���ñ�־	
[EXTERN _WY_ulFDOpFun]				;�����жϴ�����
[EXTERN _WY_bFDInt]				;�жϷ�����־
[GLOBAL _DivError]				;�����쳣�жϴ���ƫ��
[GLOBAL _TimeInterrupt]				;ʱ���жϴ���ƫ��
[GLOBAL _SysCall]				;ϵͳ���ô���ƫ��
[GLOBAL _PageException]				;ҳ���쳣����ƫ�� 
[GLOBAL _FloppyInterrupt]
;[GLOBAL	_TestDiv]
[SECTION .text]
_TimeInterrupt:
	cli
	push	eax
	call	_TimeInt
	mov al , 0x20	;����EOI��Ϣ
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

	;��Ϊû�г����룬���Ժ���ȷ���Ǻ��ִ���
	;�����޸ķ��ص�ַ����ָ���ֲ�divָ����
	;x86�еĳ���ָ�����2�ֽڣ�����޸ķ��ص�ַ
	pop	eax
	add	eax,2	;�޸ķ��ص�ַ
	push	eax
	iret
_PageException:
	push	ebp
	mov	ebp,esp

	push	eax
	mov	eax,cr2			;cr2�Ĵ�����ջ������ҳ���쳣�����Ե�ַ
	push	eax
	mov	eax,dword [ebp + 4]
	push	eax			;���������ջ
	call	_PageExpProc
	pop	eax			;��������
	pop	eax			;��������
	
	pop	eax			;�ָ�eax
	pop	ebp			;�ָ�ebp
	add	esp,4			;����������
	iret
;
;ϵͳ���û�����
;

;SysCall�����Ĳ���Ϊ
;ulong WY_ulSyscallIndex,ulong WY_p0,ulong WY_p1,ulong WY_p2,
;ulong WY_p3,ulong WY_p4,ulong WY_p5,ulong WY_p6
;��ִ����push ebp,֮���ջ���
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
	;ebp����12��ԭ���ǣ�ջ��Ϊebp,Ȼ����ret addr��ret cs selector��������12���ֽڡ�
	add	ebp,12
	push	ebp
	call	_SysCall_C_Func
	pop	ebp
	pop	ebp
	;���Բ���������ȷ���ã�����ret����retf
	retf	32
_FloppyInterrupt:
	push	eax
	
	mov al , 0x20	;����EOI��Ϣ
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
;���Գ��������ж�
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