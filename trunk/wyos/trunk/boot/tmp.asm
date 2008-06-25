;
;该代码只负责将内核代码搬运到0x100000处
;
;
bits	32
org	2000h

start:
	;移动测试程序代码
	mov	esi,0x4000
	mov	edi,0x1000000
	mov	ecx,0x1000
	rep	movsd

	;移动内核代码
	mov	esi,0x5000
	mov	edi,0x100000
	mov	ecx,0x20000
	rep	movsd
	jmp	0x100000
