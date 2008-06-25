/***************************************************************************
			WYOS fat12.h
			fat12�ļ�ϵͳ���ͷ�ļ�
						����:WY.lslrt				editor	 :WY.lslrt
						����:2006/10/8			date	 :2006/10/8
						��Ȩ:WY �� WY.lslrt���� 	copyright:WY and WY.lslrt
						URL:http://www.wyos.net http://wylslrt.go.3322.org
***************************************************************************/
#include "..\include\WYOSType.h"
#include "..\include\cpu.h"
#include "..\include\io.h"
#include "..\include\syscall.h"
#include "..\video\video.h"
#include "..\include\kernel.h"
#include "..\include\memory.h"
#include "..\include\process.h"
#include "..\include\driver.h"
#include "..\include\floppy.h"
#include "..\include\fat12.h"
#include "..\include\string.h"


extern WY_ProcTable	WY_PROCTABLE[MAX_PROC_NUM];

PVOID				WY_Clusterbuf;					//�ػ�����

FILE					WY_FileQueue;
FILE					WY_DirQueue;

void Fat12Init()
{
	WY_pFDRequest	WY_pFDReq;
	char				WY_strFullPath[256];
	wyos_save_flag();
	wyos_open_int();

	printk("     Fat12 Initializing.......");

	WY_Clusterbuf = mallocs(9 * 512);
	if(WY_Clusterbuf == NULL)
	{
		printk("Failed\n");
	}
	//��ȡ�ر�,�ܹ�9�Ÿ�����
	WY_pFDReq = (WY_pFDRequest)FDRWLBA(1,9,WY_Clusterbuf, FD_READ_DIR, FD_OPMODE_ASYN);
	
	while(WY_pFDReq->WY_ulState != FD_STATE_OK);
	
	frees(WY_pFDReq);

	strcpy(WY_PROCTABLE[0].WY_CurFilePathName,"A:\\KERNEL.BIN");

	WY_FileQueue.WY_pNext = NULL;
	WY_FileQueue.WY_pQueue = NULL;

	WY_DirQueue.WY_pNext = NULL;
	WY_DirQueue.WY_pQueue = NULL;
	
	printk("OK\n");
	printk("%d\n",CheckFileName("C:\\abc\\\\jjj.txt"));
	wyos_restore_flag();
}


ulong ReadCluster(ulong WY_ulCluster, void * WY_pBuf, ulong WY_ulOPMode)
{
	//���ȼ����߼�������
	ulong	WY_ulLBAAddr = WY_ulCluster - 2;		//����0,1���Ѿ���ϵͳռ��

	WY_ulLBAAddr += 33;								//����fat����Ŀ¼����������������

	return FDRWLBA(WY_ulLBAAddr, 1, WY_pBuf, FD_READ_DIR,WY_ulOPMode);
}

ulong GetNextCluster(ulong WY_ulCurCluster)
{
	ulong	WY_ulOffset;
	ushort	WY_ustmpData;
	ulong	WY_ulNextCluster;
	
	//�����ҵ���������ڵ�FAT���ƫ��,ȡ�ø�ƫ�Ƶ�����,��Ϊ��һ�صĴغ�
	if(WY_ulCurCluster >= 0xFF0)
	{
		//��Ч�غ�
		return 0xFFF;
	}

	WY_ulOffset  = (WY_ulCurCluster >> 1) + WY_ulCurCluster;
	//Ȼ��ȥ��,���ڸ�12λ,ż�ڵ�12λ
	WY_ustmpData =  (ushort)(*(ushort *)((ulong)WY_Clusterbuf + WY_ulOffset));
	if(WY_ulCurCluster & 0x1)
	{
		//���
		WY_ulNextCluster = WY_ustmpData >> 4;
	}
	else
	{
		//ż��
		WY_ulNextCluster = WY_ustmpData & 0xFFF;
	}
	return WY_ulNextCluster;
}

//���ļ�����Ϊ�䴴��������

ulong KCreateFile(char * WY_strFileName, ulong WY_ulCreateMode, ulong WY_ulFileOp, ulong WY_ulShare, ulong WY_ulFileType)
{
	char*			WY_strFullPath;						//�ļ�����·��
	char*			WY_strName;						//�ļ���
	char*			WY_strFileDir;						//�ļ�����Ŀ¼����·��
	FILE*			WY_fpFile;							//�ļ�������
	FILE*			WY_fpDir;							//Ŀ¼������
	FILE*			WY_fpMatch;						//��ƥ��Ŀ¼�ļ�������
	PVOID			WY_pFileMapping;					//��ƥ��Ŀ¼�ļ�ӳ���ڴ�
	ulong			WY_ulDirDeep;						//Ŀ¼���
	ulong			WY_ulMatchNum = 0;					//���ƶ�
	BOOL			WY_bExistFlag = FALSE;				//���ڱ�־

	//����ļ����ĺϷ���
	//���ȹ�������·����
	
}

/*
����һ���ļ����Ĺ����ķ�
�����Զ����·���������ܹ���ʶ��
���ļ��������ļ�·���ǺϷ���
�����Ǹ��ķ��ĵݹ��½�����ķǵݹ黯
���ķ�����,ÿ���ķ����������ע��
S->Disk:\E  | | .\E  | ..\E | \E  | E                       Disk = (A-Z);
E->N\E  |  N  | ^                                                 ^��ʾ��
N->dirname  | filename.suffixname                       dirname,filename = (A-Z)^(1 - 8)
                                                                           suffixname = (A-Z)^(1 - 3)
*/
//��������ֵ-1�Ƿ�,0���·��,1����·��
int CheckFileName(char * WY_strFileName)
{
	int			i = 0,j;	    //��һ��Ҫ�����ַ�λ��ָʾ��
	int			WY_nstrlen = strlen(WY_strFileName);
	BOOL		WY_bhave = FALSE;	//���������\��β
	BOOL		WY_bFullPath = FALSE;

	Upper(WY_strFileName);

	if(WY_strFileName[0] >= 'A' && WY_strFileName[0] <= 'Z')
	{
		//�����Ǿ���·��
		if(WY_strFileName[1] == ':')
		{
			i = 2;
			WY_bhave = TRUE;
			WY_bFullPath = TRUE;
		}
		
	}
	
	if(WY_strFileName[0] == '.')
	{
		//�п�����.\E����..\E����
		i = 1;
		if(WY_strFileName[i] == '.')
		{
			//..����
			//Ҳ����ֱ�Ӽ����һ���ǲ���\������ֻ��Ϊ�˴�������
			//��ʣ�µ�\��鹲����
			i = 2;
		}
		WY_bhave = TRUE;
	}

	//���������ַ���
	if(WY_strFileName[i] == '\\')	//����������������\name������ƥ��a:\�е����һ���ַ�
	{
		i++;
	}
	else
	{	//ֻ�о���·���������ַ���Ϊ\ʱΪ�Ƿ�,�������·��
		//��ʼ��Ϊ.,..,\\,����Ϊ�Ϸ���ĸ���е��ַ�ʱΥ��
		if(WY_bhave || (WY_strFileName[0] < 'A' || WY_strFileName[0] > 'Z'))
		{
			return -1;
		}
	}

	while(i < WY_nstrlen)
	{
		
		//Ӧ����file.suffix����dirname���͵Ľṹ
		for(j = 0;j < 8;j++)
		{
			if(WY_strFileName[i] < 'A' || WY_strFileName[i] > 'Z')
			{
				return -1;
			}
			
			if(WY_strFileName[i+ 1] == '.' || WY_strFileName[i + 1] == '\\')
			{
				i++;
				break;
			}
			else if( i == (WY_nstrlen - 1))
			{
				break;
			}
			else
			{
				i++;
			}
			
		}

		if(WY_strFileName[i] == '.')		//���滹���ܸ��к�׺��
		{
			i++;
			for(j = 0;j < 3;j++)
			{
				if(WY_strFileName[i] < 'A' || WY_strFileName[i] > 'Z')
				{
					return -1;
				}
				if(i == (WY_nstrlen - 1))
				{
					break;
				}

				i++;
			}

		}

		if(WY_strFileName[i] == '\\')
		{
			i++;
		}
		else
		{
			//ֻ�е���·����βʱ���Բ�ʹ��'\\'
			if(i == (WY_nstrlen - 1))
			{
				if(WY_bFullPath)
				{
					return 1;
				} 
				return 0;
			}
			else
			{
				return -1;
			}
		}
	}
}

void ConstructFullPath(char * WY_strFullPath, char * WY_strDirpath, char * WY_strFileName)
{
	int			i = 0,j = strlen(WY_strDirpath) - 1;
	char			*WY_strTmpDir,*WY_strTmpName;

	
	WY_strTmpDir = mallocs(FILE_MAX_PATH);
	WY_strTmpName = mallocs(FILE_MAX_PATH);
	memset(WY_strTmpDir,0,FILE_MAX_PATH);
	memset(WY_strTmpName,0,FILE_MAX_PATH);

	if(WY_strFileName[i] == '.')
	{
		i++;
		if(WY_strFileName[i] == '.')
		{
			i++;
			//��Ҫ��Ŀ¼·����ǰ��һ��
			if(WY_strDirpath[j] == '\\')
			{
				j--;
			}
			while(WY_strDirpath[j] != '\\' && j >= 0)
			{
				j--;
			}
			if(j < 0)
			{
				//�������·��������
				WY_strFullPath[0] = 0;
				return;
			}
		}
		
	}
	//ȥ��\\�����������ֱ�Ӵ��ģ�Ҳ������.\\����..\\����
	if(WY_strFileName[i] == '\\')
	{
		i++;
	}
	//�Ѿ��������������ִ���Ŀ¼�ִ�,���Ƶ�������
	memcpy(WY_strTmpName,WY_strFileName + i,strlen(WY_strFileName) - i);
	memcpy(WY_strTmpDir,WY_strDirpath,j + 1);
	//��ʼ����Ŀ¼�ִ�
	//���Ƶ�Ŀ���ִ���
	if(WY_strTmpDir[strlen(WY_strTmpDir) - 1] == '\\')
	{
		vsprintf(WY_strFullPath,"%s%s",WY_strTmpDir,WY_strTmpName);
	}
	else
	{
		vsprintf(WY_strFullPath,"%s\\%s",WY_strTmpDir,WY_strTmpName);
	}
	printk("%s\n",WY_strFullPath);
}

void GetFileDirector(char *WY_strFullPath,char *WY_strFileDirector)
{
	int	i = strlen(WY_strFullPath) - 1;

	if(WY_strFullPath[i] == '\\')
	{
		i --;
	}

	while(WY_strFullPath[i] != '\\')
	{
		i--;
	}
	memcpy(WY_strFileDirector,WY_strFullPath,i + 1);
}

ulong GetDirDeep(char *WY_strFullpath)
{
	int	WY_nDeep = -1;
	
	while(*WY_strFullpath)
	{
		if (*WY_strFullpath == '\\')
		{
			WY_nDeep ++;
		}
		WY_strFullpath++;
	}

	return WY_nDeep;
}

ulong match(char *WY_strFirstPath,char *WY_strSecondPath)
{
	int	WY_nMatchNum = 0;			//���Ƴ̶�

	while(*WY_strFirstPath && *WY_strSecondPath)
	{
		if(*WY_strFirstPath == *WY_strSecondPath)
		{
			if(*WY_strFirstPath == '\\')
			{
				WY_nMatchNum++;
			}
			WY_strFirstPath++;
			WY_strSecondPath++;
		}
		else
		{
			return WY_nMatchNum;
		}
	}
	return WY_nMatchNum;
}


