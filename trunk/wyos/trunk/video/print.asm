;/***************************************************************************
;			WYOS print.asm
;			��ʾ��������ļ�
;		����:WY.lslrt			editor	 :WY.lslrt
;		����:2005/12/2			date	 :2005/12/2
;		����:
;		��Ȩ:WY �� WY.lslrt����		copyright:WY and WY.lslrt
;		URL:http://www.wyos.net http://wylslrt.go.3322.org
;***************************************************************************/
[BITS 32]
[GLOBAL _ShowChar]
[SECTION .text]
;=============================================================================
;ShowChar(char *WY_pDisplay,char WY_chDisp);
;��ʾ�ַ�
;WY_pDisplay	��ʾ������ָ�롣
;WY_chDisp	Ҫ��ʾ���ַ���
;����: ��
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