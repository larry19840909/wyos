/***************************************************************************
			WYOS metux.h
			�������ͷ�ļ�
						����:WY.lslrt				editor	 :WY.lslrt
						����:2006/5/2				date	 :2006/5/2
						��Ȩ:WY �� WY.lslrt���� 	copyright:WY and WY.lslrt
						URL:http://www.wyos.net http://wylslrt.go.3322.org
***************************************************************************/
#ifndef _WYOS_METUX_H
#define _WYOS_METUX_H

//
//����������
//WY_ulKey:Կ�ױ���ָ��WY_ulLockHole:���ױ���
//WY_ulLockHoleΪ1��ʾû��ʹ��
//���ױ���һ��Ҫ��һ��ȫ�ֱ���
//WY_ulKey��ֵһ��ҪΪ0����WY_ulKey��ֵ
//Ϊ1  ʱ���ִ��Ȩ������������
void SpinLock(ulong * WY_ulKey,ulong *WY_ulLockHole);

//
//�ͷ�������һ��Ҫ��SpinLock  ���ʹ�ã������쳣
//WY_ulKey:Կ��WY_ulLockHole:���ױ���
void ReleaseSpinLock(ulong *WY_ulKey,ulong *WY_ulLockHole);


#endif
