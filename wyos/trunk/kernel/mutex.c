/***************************************************************************
			WYOS mutex.c
			�������Դ�ļ�
						����:WY.lslrt				editor	 :WY.lslrt
						����:2006/5/2				date		 :2006/5/2
						��Ȩ:WY �� WY.lslrt���� 	copyright  :WY and WY.lslrt
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
	//��֤������ȷ�Լ�
	//û�б�ʹ��ʱ����
	//����ʹ��ʱ��WY_ulKey
	//��ֵΪ��Ч
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
