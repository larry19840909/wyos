/***************************************************************************
			WYOS string.c
			字符串源文件
						编码:WY.lslrt			editor	 :WY.lslrt
						日期:2005/12/2			date	 :2005/12/2
						版权:WY 和 WY.lslrt所有 copyright:WY and WY.lslrt
						URL:http://www.wyos.net http://wylslrt.go.3322.org
***************************************************************************/
#include "..\include\WYOSType.h"
#include "..\include\string.h"

int strlen(char *WY_szSource)
{
	int	WY_nCnt = 0;
	while(*WY_szSource++ != 0)
	{
		WY_nCnt++;
	}
	return WY_nCnt;
}

void itoaf(uint WY_nSource, char * WY_szDest,int WY_nDestLen)
{
	int	WY_nAC;		//余数airthmetical compliment

	*(WY_szDest + WY_nDestLen -1) = 0;
	WY_nDestLen--;
	while(WY_nDestLen-- > 0)
	{
		WY_szDest[WY_nDestLen] = (char)(WY_nSource % 10 + 48);
		WY_nSource = WY_nSource / 10;
	}
}

int itoap(uint WY_nSource, char * WY_szDest, int WY_nDestLen)
{
	int	WY_nStackVertex = -1;
	int	WY_nStack[11];
	int	WY_i = 0;

	if(WY_nSource == 0)
	{
		WY_nStack[++WY_nStackVertex] = 48;
	}

	while(WY_nSource > 0)
	{
		WY_nStack[++WY_nStackVertex] = (WY_nSource % 10 + 48);
		WY_nSource = WY_nSource / 10;
	}

	while( WY_nStackVertex > -1)
	{
		WY_szDest[WY_i++] = (char)WY_nStack[WY_nStackVertex--];
	}
	for(;WY_i < WY_nDestLen;WY_i++)
	{
		WY_szDest[WY_i] = 0;
	}
}

int htoa(uint WY_nSource, char * WY_szDest, int WY_nDestLen)
{
	char	WY_cHex[16] = {'0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F'};
	int	WY_nAC;

	*(WY_szDest + WY_nDestLen -1) = 0;
	WY_nDestLen--;
	while(WY_nDestLen-- > 2)
	{
		WY_szDest[WY_nDestLen] = WY_cHex[WY_nSource % 16];
		WY_nSource = WY_nSource / 16;
	}
	WY_szDest[WY_nDestLen--] = 'x';
	WY_szDest[WY_nDestLen] = '0';
}
uint atoi(char * WY_szSource)
{
	int	WY_nNumPos = 0;	//字符串每一位数字的位置
	int	WY_nSourceLen = strlen(WY_szSource);
	uint	WY_nResult = 0;

	while(WY_nNumPos< WY_nSourceLen -1)
	{
		WY_nResult *= 10;
		WY_nResult += (char)WY_szSource[WY_nNumPos] - 48;
	}

	return WY_nResult;
}

int	vsprintf(char * WY_pcbuf, char * WY_pcfmt, ...)
{
	char 	WY_pctmp[11];
	char *	WY_pctmp1;
	char	*	WY_pcbuftmp = WY_pcbuf;
	va_list	WY_args;
	int		WY_nNum;
	int		WY_i;

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

					itoap(WY_nNum,WY_pctmp,11);
					for(WY_i = 0;WY_i < 11;WY_i++)
					{
						if(WY_pctmp[WY_i] != 0)
							*WY_pcbuftmp++ = WY_pctmp[WY_i];
						else break;
					}

					break;
				case	'c':
					*WY_pcbuftmp++ = va_arg(WY_args,char);
					break;
				case	'x':
					WY_nNum = va_arg(WY_args,int);
					htoa(WY_nNum,WY_pctmp,11);
					for(WY_i = 0;WY_i < 11;WY_i++)
					{
						if(WY_pctmp[WY_i] != 0)
							*WY_pcbuftmp++ = WY_pctmp[WY_i];
						else break;
					}
					break;
				case	's':
					WY_pctmp1 = va_arg(WY_args,char *);

					while(*WY_pctmp1) 
						*WY_pcbuftmp++ = *WY_pctmp1++;
					
					break;
				case	'p':
					break;
				default:
					break;
			}
		}
		else
		{
			*WY_pcbuftmp++ = *WY_pcfmt++;
		}
	}
	*WY_pcbuftmp = 0;
	va_end(WY_args);
	return 0;
}

char * strcpy(char * WY_pcdest,const char *WY_pcsrc)
{
__asm__("cld\n"
	"1:\tlodsb\n\t"
	"stosb\n\t"
	"testb %%al,%%al\n\t"
	"jne 1b"
	::"S" (WY_pcsrc),"D" (WY_pcdest));
return WY_pcdest;
}

char * strncpy(char * WY_pcdest,const char *WY_pcsrc,int WY_nconunt)
{
__asm__("cld\n"
	"1:\tdecl %2\n\t"
	"js 2f\n\t"
	"lodsb\n\t"
	"stosb\n\t"
	"testb %%al,%%al\n\t"
	"jne 1b\n\t"
	"rep\n\t"
	"stosb\n"
	"2:"
	::"S" (WY_pcsrc),"D" (WY_pcdest),"c" (WY_nconunt));
return WY_pcdest;
}

char * strcat(char * WY_pcdest,const char * WY_pcsrc)
{
__asm__("cld\n\t"
	"repne\n\t"
	"scasb\n\t"
	"decl %1\n"
	"1:\tlodsb\n\t"
	"stosb\n\t"
	"testb %%al,%%al\n\t"
	"jne 1b"
	::"S" (WY_pcsrc),"D" (WY_pcdest),"a" (0),"c" (0xffffffff));
return WY_pcdest;
}

char * strncat(char * WY_pcdest,const char * WY_pcsrc,int WY_nconunt)
{
__asm__("cld\n\t"
	"repne\n\t"
	"scasb\n\t"
	"decl %1\n\t"
	"movl %4,%3\n"
	"1:\tdecl %3\n\t"
	"js 2f\n\t"
	"lodsb\n\t"
	"stosb\n\t"
	"testb %%al,%%al\n\t"
	"jne 1b\n"
	"2:\txorl %2,%2\n\t"
	"stosb"
	::"S" (WY_pcsrc),"D" (WY_pcdest),"a" (0),"c" (0xffffffff),"g" (WY_nconunt)
	);
return WY_pcdest;
}

int strcmp(const char * WY_pccs,const char * WY_pcct)
{
register int __res ;
__asm__("cld\n"
	"1:\tlodsb\n\t"
	"scasb\n\t"
	"jne 2f\n\t"
	"testb %%al,%%al\n\t"
	"jne 1b\n\t"
	"xorl %%eax,%%eax\n\t"
	"jmp 3f\n"
	"2:\tmovl $1,%%eax\n\t"
	"jl 3f\n\t"
	"negl %%eax\n"
	"3:"
	:"=a" (__res):"D" (WY_pccs),"S" (WY_pcct));
return __res;
}

int strncmp(const char * WY_pccs,const char * WY_pcct,int WY_nconunt)
{
register int __res ;
__asm__("cld\n"
	"1:\tdecl %3\n\t"
	"js 2f\n\t"
	"lodsb\n\t"
	"scasb\n\t"
	"jne 3f\n\t"
	"testb %%al,%%al\n\t"
	"jne 1b\n"
	"2:\txorl %%eax,%%eax\n\t"
	"jmp 4f\n"
	"3:\tmovl $1,%%eax\n\t"
	"jl 4f\n\t"
	"negl %%eax\n"
	"4:"
	:"=a" (__res):"D" (WY_pccs),"S" (WY_pcct),"c" (WY_nconunt));
return __res;
}

void memset(char * WY_pcdest,int WY_n,int WY_nsize)
{
	__asm__("cld\n"
		"rep stosb\n\t"
		::"D"(WY_pcdest),"a"(WY_n),"c"(WY_nsize));
}

void memcpy(char * WY_pcdest, const char * WY_pcsource, int WY_ncount)
{
	__asm__("cld\n"
			"rep movsb\n\t"
			::"D"(WY_pcdest),"S"(WY_pcsource),"c"(WY_ncount));
}

void Upper(char * WY_strsrc)
{
	int		WY_nStrlen = strlen(WY_strsrc);
	int		i;

	for(i = 0;i < WY_nStrlen;i++)
	{
		if(WY_strsrc[i] >= 'a' && WY_strsrc[i] <= 'z')
		{
			WY_strsrc[i] = WY_strsrc[i] - 32;
		}
	}
}
