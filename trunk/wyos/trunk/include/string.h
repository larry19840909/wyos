/***************************************************************************
			WYOS string.h
			字符串操作头文件
						编码:WY.lslrt				editor	 :WY.lslrt
						日期:2005/12/2			date	 :2005/12/2
						版权:WY 和 WY.lslrt所有 	copyright:WY and WY.lslrt
						URL:http://www.wyos.net http://wylslrt.go.3322.org
***************************************************************************/
#ifndef	_WYOS_STRING_H
#define	_WYOS_STRING_H
#include "..\include\stdarg.h"

//求字符串长度
//WY_szSource	欲求其长度的字符串
int strlen(char *WY_szSource);

//将一个数字转换为字符串, 完全根据字符串长度转化
//比如字符串长度是3,需要转化的数字是2,转化后的结果
//就是002
//WY_nSource	源数
//WY_szDest		转换为字符串后的字符串指针
//WY_nDestLen	目的字串的长度
void	itoaf(uint WY_nSource,char *WY_szDest,int WY_nDestLen);

//将一个数字转换为字符串, 完全根据数字转化
//比如字符串长度是3,需要转化的数字是2,转化后的结果
//就是2
//WY_nSource	源数
//WY_szDest		转换为字符串后的字符串指针
//WY_nDestLen	目的字串的长度
//返回值:  转换后的字符串长度,  如果为0  则表示给定
//字符串不够长
int itoap(uint WY_nSource,char *WY_szDest,int WY_nDestLen);

//将一个数字以0xXXXXXXXX 的方式存入到一个字符串里
int htoa(uint WY_nSource,char *WY_szDest,int WY_nDestLen);

//将一个表示数字的字符串转换为一个int 变量
//WY_szSource	源串
//返回值澹澹:	转换后的数据
uint atoi(char *WY_szSource);

//格式化一个字符串
int vsprintf(char * WY_pcbuf, char *WY_pcfmt, ...);

char * strcpy(char * WY_pcdest,const char *WY_pcsrc);


char * strncpy(char * WY_pcdest,const char *WY_pcsrc,int WY_nconunt);


char * strcat(char * WY_pcdest,const char * WY_pcsrc);


char * strncat(char * WY_pcdest,const char * WY_pcsrc,int WY_nconunt);


int strcmp(const char * WY_pccs,const char * WY_pcct);


int strncmp(const char * WY_pccs,const char * WY_pcct,int WY_nconunt);


void memset(char * WY_pcdest,int WY_n,int WY_nsize);

void memcpy(char * WY_pcdest,const char * WY_pcsource,int WY_ncount);

void Upper(char *WY_strsrc);
#endif

