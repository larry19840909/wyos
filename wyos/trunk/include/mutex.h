/***************************************************************************
			WYOS metux.h
			互斥操作头文件
						编码:WY.lslrt				editor	 :WY.lslrt
						日期:2006/5/2				date	 :2006/5/2
						版权:WY 和 WY.lslrt所有 	copyright:WY and WY.lslrt
						URL:http://www.wyos.net http://wylslrt.go.3322.org
***************************************************************************/
#ifndef _WYOS_METUX_H
#define _WYOS_METUX_H

//
//设置自旋锁
//WY_ulKey:钥匙变量指针WY_ulLockHole:锁孔变量
//WY_ulLockHole为1表示没有使用
//锁孔变量一定要是一个全局变量
//WY_ulKey初值一定要为0。当WY_ulKey的值
//为1  时获得执行权力，否则自旋
void SpinLock(ulong * WY_ulKey,ulong *WY_ulLockHole);

//
//释放自旋锁一定要和SpinLock  配对使用，否则异常
//WY_ulKey:钥匙WY_ulLockHole:锁孔变量
void ReleaseSpinLock(ulong *WY_ulKey,ulong *WY_ulLockHole);


#endif
