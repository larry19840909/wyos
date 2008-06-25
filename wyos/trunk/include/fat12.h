/***************************************************************************
			WYOS fat12.h
			fat12�ļ�ϵͳ���ͷ�ļ�
						����:WY.lslrt				editor	 :WY.lslrt
						����:2006/10/8			date	 :2006/10/8
						��Ȩ:WY �� WY.lslrt���� 	copyright:WY and WY.lslrt
						URL:http://www.wyos.net http://wylslrt.go.3322.org
***************************************************************************/
#ifndef	_WYOS_FAT12_H
#define	_WYOS_FAT12_H

#define	FILE_MAX_PATH		(unsigned int)256

typedef	struct	_FILE_DESCRIPTION
{
	ulong			WY_ulPID;						//���̱�ʾ
	ulong			WY_ulOpMode;					//������ʽ��������д
	ulong			WY_ulOpenMode;				//�򿪷�ʽ����ͨ��ʽ(ʹ��cache)����ʹ���ļ�Ӱ��
	ulong			WY_ulShare;						//�����־��
	ulong *			WY_ulShareSpinLock;			//���������ѡ��
	char				WY_chLogicName[FILE_MAX_PATH];	//�ļ��߼���������A:\test.txt
	ulong			WY_ulStartCluster;				//�ļ���ʼ�غ�
	ulong			WY_ulFileSize;					//�ļ���С
	void *			WY_Filebuffer;					//File cache
	ulong			WY_ulBufSize;					//cache��С��������ļ�Ӱ����Ϊ�ļ���С
	ulong			WY_ulBlockInCache;				//��cache�еĵ�һ�����
	ulong			WY_ulCurCluster;				//��ǰ��������һ���غ�
	ulong			WY_ulFileOffset;					//�ļ�ƫ�� 
	ulong			WY_ulOpenCount;				//�ļ����򿪼��������ļ��ṹ��������һ�����ڶ����У�һ�����ڶ�����
													//��һ���ļ������������Ǵ򿪶����в�û�и��ļ����򴴽������¼ҽṹ
													//һ������������������ʹ�á�
	struct _FILE_INFO * WY_pQueue;					//ָ���ڶ����е��ļ��ṹ��
	struct _FILE_INFO * WY_pNext;						//��һ���ṹ��ַ
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
