/***************************************************************************
			WYOS video.c
			显示驱动代码文件
						编码:WY.lslrt			editor	 :WY.lslrt
						日期:2005/12/2			date	 :2005/12/2
						版权:WY 和 WY.lslrt所有 copyright:WY and WY.lslrt
						URL:http://www.wyos.net http://wylslrt.go.3322.org
***************************************************************************/
#include "..\include\WYOSType.h"
#include "..\include\syscall.h"
#include "..\include\string.h"
#include "video.h"

void wyos_video_int()
{
	WY_nVideoPos = VIDEO_TEXT_BASE_ADDR;
	SetCharCharacterEx(VIDEO_RGB_GREEN,TRUE,VIDEO_RGB_BLACK,TRUE);
	ClearVideo();
	SetupSyscall(sysputc,0);
	SetupSyscall(syscls,1);
}

void	ClearVideo()
{
	int	WY_i;
	for(WY_i = 0;WY_i < 2000;WY_i++)
	{
		ShowChar((char*)(WY_i*2 + VIDEO_TEXT_BASE_ADDR),0);
		ShowChar((char*)(WY_i*2 + 1 + VIDEO_TEXT_BASE_ADDR),WY_nCharChara);
	}
}

void ScrollVideo()
{
	char		*WY_pcVideoBuf = (char *)VIDEO_TEXT_BASE_ADDR;
	int		WY_i;
	for(WY_i = 0;WY_i<1920;WY_i++)
	{
		ShowChar((char*)(WY_i*2 + VIDEO_TEXT_BASE_ADDR),
				  *(char*)(WY_i*2 + 160 + VIDEO_TEXT_BASE_ADDR));
		ShowChar((char*)(WY_i*2 + 1 + VIDEO_TEXT_BASE_ADDR),
				  *(char*)(WY_i*2 + 1 + 160 + VIDEO_TEXT_BASE_ADDR));
	}
	WY_nVideoPos = 1920 * 2 + VIDEO_TEXT_BASE_ADDR;
	for(WY_i = 1920;WY_i < 2000;WY_i++)
	{
		ShowChar((char*)(WY_i*2 + VIDEO_TEXT_BASE_ADDR),0);
		ShowChar((char*)(WY_i*2 + 1 + VIDEO_TEXT_BASE_ADDR),WY_nCharChara);
	}
}

char	SetCharCharacterEx(ushort WY_usFRGB, BOOL WY_bHighLight, ushort WY_usBRGB, BOOL WY_bBlink)
{
	char		WY_cCharacter;
	WY_cCharacter = (char)WY_usFRGB | ((char)WY_bHighLight << 3) | ((char)WY_usBRGB << 4) | ((char)WY_bBlink << 7);
	SetCharaCHaracter(WY_cCharacter);
	return WY_cCharacter;
}

void	SetCharaCHaracter(char WY_cCharChara)
{
	WY_nCharChara = (int)WY_cCharChara;
}

void putc(char WY_cDisp)
{
	if((WY_nVideoPos -VIDEO_TEXT_BASE_ADDR) <= VIDEO_TEXT_MAX_ADDR)
	{
		if(WY_cDisp == '\n')
		{
			WY_nVideoPos += 160;
			WY_nVideoPos = WY_nVideoPos - (WY_nVideoPos - VIDEO_TEXT_BASE_ADDR)%160;
		}
		else
		{
			ShowChar((char *)WY_nVideoPos++, WY_cDisp);
			ShowChar((char *)WY_nVideoPos++,(char)WY_nCharChara);
		}
	}
	else
	{
		ScrollVideo();
		ShowChar((char *)WY_nVideoPos++, WY_cDisp);
		ShowChar((char *)WY_nVideoPos++,(char)WY_nCharChara);
	}
	
}

void puts(char * WY_szDisp)
{
	while(*WY_szDisp != 0)
	{
		putc(*WY_szDisp);
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

void printk(char * WY_pcfmt,...)
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
					putc(va_arg(WY_args,char));
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
		else		putc(*WY_pcfmt++);
	}
	va_end(WY_args);
}

ulong sysputc(pSyscallParam WY_pInputParam)
{
	putc(WY_pInputParam->WY_ulParam0);
	return 0;
}

ulong syscls(pSyscallParam WY_pInputParam)
{
	ClearVideo();
	return 0;
}


//new line

