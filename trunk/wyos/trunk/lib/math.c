/***************************************************************************
			WYOS math.c
			��ѧ��Դ�ļ�
						����:WY.lslrt				editor	 :WY.lslrt
						����:2006/5/2				date		 :2006/5/2
						��Ȩ:WY �� WY.lslrt���� 	copyright  :WY and WY.lslrt
						URL:http://www.wyos.net http://wylslrt.go.3322.org
***************************************************************************/
#include "..\include\WYOSType.h"
#include "..\include\string.h"
//#include "..\include\memory.h"
#include "..\include\math.h"

void swap(ulong * WY_nOp1, ulong * WY_nOp2)
{
	__asm__("movl (%%edi), %%eax\n\t"
			"xchg (%%esi),%%eax\n\t"
			"xchg (%%edi),%%eax "::"S"(WY_nOp1),"D"(WY_nOp2));
}
