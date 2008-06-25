/***************************************************************************
			WYOS WYOSType.h
			WYOS 数据类型头文件
						编码:WY.lslrt			editor	 :WY.lslrt
						日期:2005/12/2			date	 :2005/12/2
						版权:WY 和 WY.lslrt所有 copyright:WY and WY.lslrt
						URL:http://www.wyos.net http://wylslrt.go.3322.org
***************************************************************************/
#ifndef	_WYOS_WYOSTYPE_H
#define	_WYOS_WYOSTYPE_H

#include "kernel.h"

typedef	unsigned short 	ushort;

typedef 	unsigned int		uint;

typedef unsigned long 		ulong;

typedef	void *			PVOID;

#ifndef	NULL
#define	NULL	(PVOID)0
#endif

typedef	unsigned char		BYTE;
typedef unsigned char		uchar;

typedef	BYTE			BOOL;
#define	TRUE	1
#define	FALSE	0

#endif

