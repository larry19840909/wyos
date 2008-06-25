;***************************************************************************
;			WYOS Head.asm
;			内核加载初始化代码，仅在Setup.asm后执行	
;		编码:WY.lslrt			editor	 :WY.lslrt
;		日期:2005/12/2			date	 :2005/12/2
;		更新:
;		版权:WY 和 WY.lslrt所有		copyright:WY and WY.lslrt
;		URL:http://www.wyos.net http://wylslrt.go.3322.org
;***************************************************************************

[BITS 32]
[EXTERN _main]
;[EXTERN _ExitProcess]
[GLOBAL _ASMENTRY]
[SECTION .text]
_ASMENTRY:
	start:
		;压入参数
		call	_main		;调用c入口
		;退出
	        ;call	_ExitProcess	;程序结束，结束进程
	end:    
	        jmp	end
	        ret