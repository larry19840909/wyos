/***************************************************************************
			WYOS driver.c
			driver相关源文件
						编码:WY.lslrt				editor	 :WY.lslrt
						日期:2006/09/2			date	 :2006/09/2
						版权:WY 和 WY.lslrt所有 	copyright:WY and WY.lslrt
						URL:http://www.wyos.net http://wylslrt.go.3322.org
***************************************************************************/
#include "..\include\WYOSType.h"
#include "..\include\syscall.h"
#include "..\include\cpu.h"
#include "..\video\video.h"
#include "..\include\string.h"
#include "..\include\memory.h"
#include "..\include\math.h"
#include "..\include\mutex.h"
#include "..\include\process.h"
#include "..\include\driver.h"
#include "..\include\floppy.h"

void DriverInit()
{
	printk("Driver Initializing...\n");
	FloppyInit();
}

//newline
