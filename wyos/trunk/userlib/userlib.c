/***************************************************************************
			WYOS userlib.c
			内核文件
						编码:WY.lslrt				editor	 :WY.lslrt
						日期:2006/6/29			date	 	:2006/6/29
						版权:WY 和 WY.lslrt所有 	copyright:WY and WY.lslrt
						URL:http://www.wyos.net http://wylslrt.go.3322.org
***************************************************************************/
#include "..\include\WYOSType.h"
#include "..\include\syscall.h"
#include "..\include\string.h"
#include "userlib.h"


void userputc(char WY_cDisp)
{
	ulong	WY_ulRetValue = 0;

	UseSyscall(0, WY_cDisp,0, 0, 0, 0, 0, 0, WY_ulRetValue);
}

void usercls()
{
	ulong	WY_ulRetValue = 0;

	UseSyscall(1, 0, 0, 0, 0, 0, 0, 0, WY_ulRetValue);
}

ulong GetPID()
{
	ulong	WY_ulRetValue = 0;

	UseSyscall(10, 0, 0, 0, 0, 0, 0, 0,WY_ulRetValue);
	return WY_ulRetValue;
}

ulong GetTID()
{
	ulong	WY_ulRetValue = 0;
	UseSyscall(9,0,0,0,0,0,0,0,WY_ulRetValue);
	return WY_ulRetValue;
}
ulong GetMemoryInfo()
{
	ulong	WY_ulRetValue = 0;

	UseSyscall(20, 0, 0, 0, 0, 0, 0, 0,WY_ulRetValue);
	return WY_ulRetValue;
}

void *malloc(ulong WY_ulBlockSize)
{
	ulong	WY_ulRetValue = 0;

	UseSyscall(21, WY_ulBlockSize, 0, 0, 0, 0, 0, 0,WY_ulRetValue);
	return (PVOID)WY_ulRetValue;
}

void free(PVOID WY_pMMPTR)
{
	ulong	WY_ulRetValue = 0;

	UseSyscall(22, WY_pMMPTR, 0, 0, 0, 0, 0, 0,WY_ulRetValue);
}

void Sleep(ulong WY_ulWaitTime)
{
	ulong WY_ulRetValue;
	UseSyscall(16, WY_ulWaitTime,0, 0, 0, 0, 0, 0, WY_ulRetValue);
}

ulong SendMessage(ushort WY_usPID, ulong WY_ulMsg, WPARAM WY_wparam, LPARAM WY_lparam)
{
	ulong WY_ulRetValue;

	UseSyscall(17, WY_usPID, WY_ulMsg, WY_wparam, WY_lparam, 0, 0, 0, WY_ulRetValue);
	return WY_ulRetValue;
}

ulong RecvMessage(WY_PMSG WY_pmsg)
{
	ulong WY_ulRetValue;
	UseSyscall(18,WY_pmsg,0,0,0,0,0,0,WY_ulRetValue);
	return WY_ulRetValue;
}

void puts(char * WY_szDisp)
{
	while(*WY_szDisp != 0)
	{
		userputc(*WY_szDisp);
		WY_szDisp++;
	}
}

void putn(int WY_nDisp,BOOL WY_bFullPrn)
{
	char		WY_szDisp[11];

	if(WY_bFullPrn)
	{
		itoaf(WY_nDisp,WY_szDisp,11);
		puts(WY_szDisp);
	}
	else
	{
		itoap(WY_nDisp,WY_szDisp,11);
		puts(WY_szDisp);
	}
}


void putx(int WY_nDisp)
{
	char	WY_szDisp[11];
	htoa(WY_nDisp,WY_szDisp,11);
	puts(WY_szDisp);
}

void printf(char * WY_pcfmt,...)
{
	va_list	WY_args;
	char		WY_pcbuf[11];
	int		WY_nNum;
	
	va_start(WY_args,WY_pcfmt);
	while(*WY_pcfmt)
	{
		if(*WY_pcfmt == '%')
		{
			WY_pcfmt++;
			switch(*WY_pcfmt++)
			{
				case	'd':
					WY_nNum = va_arg(WY_args,int);
					putn(WY_nNum,FALSE);
					break;
				case	'c':
					userputc(va_arg(WY_args,char));
					break;
				case	's':
					puts(va_arg(WY_args,char *));
					break;
				case	'x':
					WY_nNum = va_arg(WY_args,int);
					putx(WY_nNum);
					break;
				case	'p':
					break;
				default:
					break;
				
			}
		}
		else		userputc(*WY_pcfmt++);
	}
	va_end(WY_args);
}
