/***************************************************************************
			WYOS fat12.h
			fat12文件系统相关头文件
						编码:WY.lslrt				editor	 :WY.lslrt
						日期:2006/10/8			date	 :2006/10/8
						版权:WY 和 WY.lslrt所有 	copyright:WY and WY.lslrt
						URL:http://www.wyos.net http://wylslrt.go.3322.org
***************************************************************************/
#ifndef	_WYOS_FAT12_H
#define	_WYOS_FAT12_H

#define	FILE_MAX_PATH		(unsigned int)256

typedef	struct	_FILE_DESCRIPTION
{
	ulong			WY_ulPID;						//进程标示
	ulong			WY_ulOpMode;					//操作方式，读或者写
	ulong			WY_ulOpenMode;				//打开方式，普通方式(使用cache)或者使用文件影射
	ulong			WY_ulShare;						//共享标志。
	ulong *			WY_ulShareSpinLock;			//共享操作自选锁
	char				WY_chLogicName[FILE_MAX_PATH];	//文件逻辑名，例如A:\test.txt
	ulong			WY_ulStartCluster;				//文件起始簇号
	ulong			WY_ulFileSize;					//文件大小
	void *			WY_Filebuffer;					//File cache
	ulong			WY_ulBufSize;					//cache大小，如果是文件影射则为文件大小
	ulong			WY_ulBlockInCache;				//在cache中的第一个块号
	ulong			WY_ulCurCluster;				//当前读入的最后一个簇号
	ulong			WY_ulFileOffset;					//文件偏移 
	ulong			WY_ulOpenCount;				//文件被打开计数器，文件结构有两个，一个是在队列中，一个不在队列中
													//当一个文件被创建，但是打开队列中并没有该文件，则创建两个温家结构
													//一个放入插入队列作计数使用。
	struct _FILE_INFO * WY_pQueue;					//指向在队列中的文件结构，
	struct _FILE_INFO * WY_pNext;						//下一个结构地址
}FILE;

void Fat12Init();

ulong ReadCluster(ulong WY_ulCluster,void* WY_pBuf,ulong WY_ulOPMode);

ulong GetNextCluster(ulong	WY_ulCurCluster);

ulong KCreateFile(char *WY_strFileName,ulong WY_ulCreateMode,ulong WY_ulFileOp,ulong WY_ulShare,ulong WY_ulFileType);

int CheckFileName(char *WY_strFileName);

void ConstructFullPath(char * WY_strFullPath,char *WY_strDirpath,char *WY_strFileName);

void GetFileDirector(char *WY_strDirpath,char *WY_strFullpath);

ulong GetDirDeep(char *WY_strFullpath);

ulong match(char *WY_strFirstPath,char *WY_strSecondPath);

#endif
