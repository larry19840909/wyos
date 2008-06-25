/***************************************************************************
			WYOS string.h
			�ַ�������ͷ�ļ�
						����:WY.lslrt				editor	 :WY.lslrt
						����:2005/12/2			date	 :2005/12/2
						��Ȩ:WY �� WY.lslrt���� 	copyright:WY and WY.lslrt
						URL:http://www.wyos.net http://wylslrt.go.3322.org
***************************************************************************/
#ifndef	_WYOS_STRING_H
#define	_WYOS_STRING_H
#include "..\include\stdarg.h"

//���ַ�������
//WY_szSource	�����䳤�ȵ��ַ���
int strlen(char *WY_szSource);

//��һ������ת��Ϊ�ַ���, ��ȫ�����ַ�������ת��
//�����ַ���������3,��Ҫת����������2,ת����Ľ��
//����002
//WY_nSource	Դ��
//WY_szDest		ת��Ϊ�ַ�������ַ���ָ��
//WY_nDestLen	Ŀ���ִ��ĳ���
void	itoaf(uint WY_nSource,char *WY_szDest,int WY_nDestLen);

//��һ������ת��Ϊ�ַ���, ��ȫ��������ת��
//�����ַ���������3,��Ҫת����������2,ת����Ľ��
//����2
//WY_nSource	Դ��
//WY_szDest		ת��Ϊ�ַ�������ַ���ָ��
//WY_nDestLen	Ŀ���ִ��ĳ���
//����ֵ:  ת������ַ�������,  ���Ϊ0  ���ʾ����
//�ַ���������
int itoap(uint WY_nSource,char *WY_szDest,int WY_nDestLen);

//��һ��������0xXXXXXXXX �ķ�ʽ���뵽һ���ַ�����
int htoa(uint WY_nSource,char *WY_szDest,int WY_nDestLen);

//��һ����ʾ���ֵ��ַ���ת��Ϊһ��int ����
//WY_szSource	Դ��
//����ֵ��:	ת���������
uint atoi(char *WY_szSource);

//��ʽ��һ���ַ���
int vsprintf(char * WY_pcbuf, char *WY_pcfmt, ...);

char * strcpy(char * WY_pcdest,const char *WY_pcsrc);


char * strncpy(char * WY_pcdest,const char *WY_pcsrc,int WY_nconunt);


char * strcat(char * WY_pcdest,const char * WY_pcsrc);


char * strncat(char * WY_pcdest,const char * WY_pcsrc,int WY_nconunt);


int strcmp(const char * WY_pccs,const char * WY_pcct);


int strncmp(const char * WY_pccs,const char * WY_pcct,int WY_nconunt);


void memset(char * WY_pcdest,int WY_n,int WY_nsize);

void memcpy(char * WY_pcdest,const char * WY_pcsource,int WY_ncount);

void Upper(char *WY_strsrc);
#endif

