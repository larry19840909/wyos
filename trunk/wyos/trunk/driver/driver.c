/***************************************************************************
			WYOS driver.c
			driver���Դ�ļ�
						����:WY.lslrt				editor	 :WY.lslrt
						����:2006/09/2			date	 :2006/09/2
						��Ȩ:WY �� WY.lslrt���� 	copyright:WY and WY.lslrt
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
