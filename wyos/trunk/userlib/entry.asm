;***************************************************************************
;			WYOS Head.asm
;			�ں˼��س�ʼ�����룬����Setup.asm��ִ��	
;		����:WY.lslrt			editor	 :WY.lslrt
;		����:2005/12/2			date	 :2005/12/2
;		����:
;		��Ȩ:WY �� WY.lslrt����		copyright:WY and WY.lslrt
;		URL:http://www.wyos.net http://wylslrt.go.3322.org
;***************************************************************************

[BITS 32]
[EXTERN _main]
;[EXTERN _ExitProcess]
[GLOBAL _ASMENTRY]
[SECTION .text]
_ASMENTRY:
	start:
		;ѹ�����
		call	_main		;����c���
		;�˳�
	        ;call	_ExitProcess	;�����������������
	end:    
	        jmp	end
	        ret