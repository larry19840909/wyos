/***************************************************************************
			WYOS fat12.h
			fat12文件系统相关头文件
						编码:WY.lslrt				editor	 :WY.lslrt
						日期:2006/10/8			date	 :2006/10/8
						版权:WY 和 WY.lslrt所有 	copyright:WY and WY.lslrt
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

PVOID				WY_Clusterbuf;					//簇缓冲区

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
	//读取簇表,总共9九个扇区
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
	//首先计算逻辑扇区号
	ulong	WY_ulLBAAddr = WY_ulCluster - 2;		//首先0,1簇已经被系统占用

	WY_ulLBAAddr += 33;								//加上fat表，根目录表，和引导扇区个数

	return FDRWLBA(WY_ulLBAAddr, 1, WY_pBuf, FD_READ_DIR,WY_ulOPMode);
}

ulong GetNextCluster(ulong WY_ulCurCluster)
{
	ulong	WY_ulOffset;
	ushort	WY_ustmpData;
	ulong	WY_ulNextCluster;
	
	//首先找到这个簇所在的FAT表的偏移,取得该偏移的内容,即为下一簇的簇号
	if(WY_ulCurCluster >= 0xFF0)
	{
		//无效簇号
		return 0xFFF;
	}

	WY_ulOffset  = (WY_ulCurCluster >> 1) + WY_ulCurCluster;
	//然后去读,奇在高12位,偶在低12位
	WY_ustmpData =  (ushort)(*(ushort *)((ulong)WY_Clusterbuf + WY_ulOffset));
	if(WY_ulCurCluster & 0x1)
	{
		//奇簇
		WY_ulNextCluster = WY_ustmpData >> 4;
	}
	else
	{
		//偶簇
		WY_ulNextCluster = WY_ustmpData & 0xFFF;
	}
	return WY_ulNextCluster;
}

//打开文件，并为其创建描述符

ulong KCreateFile(char * WY_strFileName, ulong WY_ulCreateMode, ulong WY_ulFileOp, ulong WY_ulShare, ulong WY_ulFileType)
{
	char*			WY_strFullPath;						//文件完整路径
	char*			WY_strName;						//文件名
	char*			WY_strFileDir;						//文件所在目录完整路径
	FILE*			WY_fpFile;							//文件描述符
	FILE*			WY_fpDir;							//目录描述符
	FILE*			WY_fpMatch;						//最匹配目录文件描述符
	PVOID			WY_pFileMapping;					//最匹配目录文件映射内存
	ulong			WY_ulDirDeep;						//目录深度
	ulong			WY_ulMatchNum = 0;					//相似度
	BOOL			WY_bExistFlag = FALSE;				//存在标志

	//检查文件名的合法性
	//首先构建完整路径名
	
}

/*
根据一个文件名的构造文法
进行自定向下分析，如果能够被识别
则文件名或者文件路径是合法的
下面是该文法的递归下降程序的非递归化
该文法如下,每句文法后面跟的是注释
S->Disk:\E  | | .\E  | ..\E | \E  | E                       Disk = (A-Z);
E->N\E  |  N  | ^                                                 ^表示空
N->dirname  | filename.suffixname                       dirname,filename = (A-Z)^(1 - 8)
                                                                           suffixname = (A-Z)^(1 - 3)
*/
//函数返回值-1非法,0相对路径,1完整路径
int CheckFileName(char * WY_strFileName)
{
	int			i = 0,j;	    //下一个要检查的字符位置指示器
	int			WY_nstrlen = strlen(WY_strFileName);
	BOOL		WY_bhave = FALSE;	//必须得有以\结尾
	BOOL		WY_bFullPath = FALSE;

	Upper(WY_strFileName);

	if(WY_strFileName[0] >= 'A' && WY_strFileName[0] <= 'Z')
	{
		//可能是绝对路径
		if(WY_strFileName[1] == ':')
		{
			i = 2;
			WY_bhave = TRUE;
			WY_bFullPath = TRUE;
		}
		
	}
	
	if(WY_strFileName[0] == '.')
	{
		//有可能是.\E或者..\E类型
		i = 1;
		if(WY_strFileName[i] == '.')
		{
			//..类型
			//也可以直接检查下一个是不是\，这里只是为了代码简单起见
			//将剩下的\检查共用了
			i = 2;
		}
		WY_bhave = TRUE;
	}

	//检测下面的字符串
	if(WY_strFileName[i] == '\\')	//有形如这样的输入\name或者来匹配a:\中的最后一个字符
	{
		i++;
	}
	else
	{	//只有绝对路径第三个字符不为\时为非法,或者相对路径
		//开始不为.,..,\\,或者为合法字母表中的字符时违法
		if(WY_bhave || (WY_strFileName[0] < 'A' || WY_strFileName[0] > 'Z'))
		{
			return -1;
		}
	}

	while(i < WY_nstrlen)
	{
		
		//应该是file.suffix或者dirname类型的结构
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

		if(WY_strFileName[i] == '.')		//后面还可能跟有后缀名
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
			//只有当到路径结尾时可以不使用'\\'
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
			//需要将目录路径向前退一个
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
				//不能组成路径，返回
				WY_strFullPath[0] = 0;
				return;
			}
		}
		
	}
	//去掉\\，这个可能是直接带的，也可能是.\\或者..\\带的
	if(WY_strFileName[i] == '\\')
	{
		i++;
	}
	//已经处理完了名字字串和目录字串,复制到缓冲区
	memcpy(WY_strTmpName,WY_strFileName + i,strlen(WY_strFileName) - i);
	memcpy(WY_strTmpDir,WY_strDirpath,j + 1);
	//开始处理目录字串
	//复制到目标字串里
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
	int	WY_nMatchNum = 0;			//类似程度

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


