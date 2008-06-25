;/***************************************************************************
;			WYOS print.asm
;			显示驱动汇编文件
;		编码:WY.lslrt			editor	 :WY.lslrt
;		日期:2005/12/2			date	 :2005/12/2
;		更新:
;		版权:WY 和 WY.lslrt所有		copyright:WY and WY.lslrt
;		URL:http://www.wyos.net http://wylslrt.go.3322.org
;***************************************************************************/
[BITS 32]
[GLOBAL _ShowChar]
[SECTION .text]
;=============================================================================
;ShowChar(char *WY_pDisplay,char WY_chDisp);
;显示字符
;WY_pDisplay	显示缓冲区指针。
;WY_chDisp	要显示的字符。
;返回: 无
;=============================================================================
_ShowChar:
	push	ebp
	push	eax
	push	ebx
	mov	ebp,esp
	add	ebp,16
	mov	eax,[ebp]
	mov	ebx,[ebp+4]
	mov	byte [DS:eax],bl
	pop	ebx
	pop	eax
	pop	ebp
	ret