#include "..\include\wyos.h"
#include "..\include\string.h"
#include "..\include\syscall.h"
#include "..\userlib\userlib.h"

int main(void)
{
	WY_MSG	WY_msg;
	ushort	WY_usPID = GetPID();
	ulong	WY_ulTID = GetTID();
	int		i = 0;
//	printf(" My PID = %d WY_msg = %x\n",WY_usPID,&WY_msg);
/*	while(RecvMessage(&WY_msg))
	{
		switch(WY_msg.WY_ulMsg)
		{
		case 0x100:
			printf("PID %d Recv MSG Time %d\n",WY_usPID,WY_msg.WY_time);
			break;
		default:
			break;
		}
	}
	printf("PID %d Unknown Exit\n",WY_usPID);
*/
	while(1)
	{
		printf("My PID = %d My TID = %d %d\n",WY_usPID,WY_ulTID,i);
		Sleep((14 - WY_usPID) * 1000);
		i++;
	}
	return 0;
}
