/***************************************************************************
			WYOS mutex.c
			互斥操作源文件
						编码:WY.lslrt				editor	 :WY.lslrt
						日期:2006/5/2				date		 :2006/5/2
						版权:WY 和 WY.lslrt所有 	copyright  :WY and WY.lslrt
						URL:http://www.wyos.net http://wylslrt.go.3322.org
***************************************************************************/
#include "..\include\WYOSType.h"
#include "..\include\syscall.h"
//#include "..\include\cpu.h"
#include "..\video\video.h"
#include "..\include\string.h"
#include "..\include\math.h"
#include "..\include\memory.h"

void SpinLock(ulong * WY_ulKey,ulong *WY_ulLockHole)
{
	//保证操作正确性既
	//没有被使用时或已
	//经被使用时让WY_ulKey
	//的值为无效
	if(*WY_ulKey == 1)
	{
		if(*WY_ulLockHole == 0 || *WY_ulLockHole == 1)
			*WY_ulKey = 0;
	}

	while(*WY_ulKey != 1)
		swap(WY_ulKey,WY_ulLockHole);
}

void ReleaseSpinLock(ulong * WY_ulKey, ulong *WY_ulLockHole)
{
	if(*WY_ulLockHole)
		return;
	*WY_ulKey = 1;
	swap(WY_ulKey,WY_ulLockHole);
}
//new line
