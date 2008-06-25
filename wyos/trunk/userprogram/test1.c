#include "..\include\wyos.h"
#include "..\include\string.h"
#include "..\include\syscall.h"
#include "..\userlib\userlib.h"

int main(void)
{
	ulong	testvalue;
	PVOID	testp1,testp2,time = 2000;

	testvalue = 0;

	testvalue = GetPID();
	printf(" PID = %x\n",testvalue);
	testp1 = malloc(0x1005);
	if(testvalue % 2 == 0)
	{
		printf("will sleep %d\n",time);
		Sleep(time);
	}
	testp2 = malloc(0x10);
	printf("PID %d testp1 %x testp2 %x\n",testvalue,testp1,testp2);
	memset((char*)testp2,'Y',5);
	*((char*)testp2 + 5) = 0;
	printf("PID %d testp2 :%s\n",testvalue,testp2);
	free(testp1);
	free(testp2);

	return 0;
}
