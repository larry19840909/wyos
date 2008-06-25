;/***************************************************************************
;			WYOS Setup.asm
;			操作系统内核加载代码,仅在WYOSBOOT.asm后执行			
;		编码:WY.lslrt			editor	 :WY.lslrt
;		日期:2005/12/2			date	 :2005/12/2
;		更新:2006/2/19
;		版权:WY 和 WY.lslrt所有		copyright:WY and WY.lslrt
;		URL:http://www.wyos.net http://wylslrt.go.3322.org
;***************************************************************************/
;本代码进行初始化工作和加载些中断
;中断设为22h
;输出字符串		输入：ah = 0 DS:SI字符串，0为结束
;读取文件到指定内存区域 输入：ah = 1 DS:SI文件名，ES:DI目录表内存区域,
;			输出：DX:BX读入到内存区域
;			      al: 0eh 文件未找到 0fh找到文件											
;中断设为21h
;输出各寄存器的值	输出：AX,CX,DX,BX,SP,BP,SI,DI,FLAG
;50h:0000h-------------------------------
;	           根目录表
;50h:1c00h-------------------------------
;		   FAT表
;50h:3300h-------------------------------
;		   Kernel代码区域
bits	16
org	0
jmp	main
GDT_DESC
	dw	0x18
gdtaddr	dd	0x0
GDT:
NULL:
	dw	0x0,0x0,0x0,0x0
TempCode:
	dw	0xFFFF
	dw	0x0
	dw	0x9A00
	dw	0x00CF
TempDate:
	dw	0xFFFF
	dw	0x0
	dw	0x9200
	dw	0x00CF

main:
	mov	ax,0x9000
	mov	ss,ax
	mov	sp,0x4000
	xor	ax,ax
	mov	ds,ax
	mov	es,ax

;-------加载实模式中断

	;开始加载中断，为了避免段间错误，以后所有代码都有段超越前缀
	;写入22h中断
	cli
	mov	di,22h*4
	mov	ax,cs
	mov	[ds:di+2],ax	;向中断向量表写入段值
	mov	ax,WYOS_int22h_proc
	mov	[ds:di],ax	;向中断向量表写入偏移
	;写入21h中断
	mov	di,21h*4
	mov	ax,cs
	mov	[ds:di+2],ax
	mov	ax,WYOS_int21h_proc
	mov	[ds:di],ax
	sti
	mov	ax,cs
	mov	ds,ax
	mov	si,WYOSMSG
	mov	ah,0
	int	22h

	
;-------读取test.exe的代码	4KB

	;读取text.exe代码到1000h:0000
	mov	si,WYOSTST
	mov	ax,50h
	mov	es,ax
	mov	di,0
	mov	ah,1
	mov	dx,1000h
	mov	bx,0
	int	22h

;-------移动test代码到4000h处

	cli	
	cld
	mov	ax,0x1000
	mov	ds,ax
	mov	si,0
	mov     ax,0x400
	mov	es,ax
	mov	di,0
	mov	cx,0x1000
	rep	movsd

;-------读取tmp.bin
;
;代码读到1000:0000H处
;
	mov	ax,cs
	mov	ds,ax
	mov	si,WYOSTMP
	mov	ax,50h
	mov	es,ax
	mov	di,0
	mov	ah,1
	mov	dx,1000h
	mov	bx,0
	int	22h
;移动代码到2000h
	cli	
	cld
	mov	ax,0x1000
	mov	ds,ax
	mov	si,0
	mov     ax,0x200
	mov	es,ax
	mov	di,0
	mov	cx,0x40
	rep	movsd
	
;-------读取kernel.bin的代码,512KB

	mov	ax,cs
	mov	ds,ax
	;读取32位kernel代码到1000h:0000
	mov	si,WYOSKNL
	mov	ax,50h
	mov	es,ax
	mov	di,0
	mov	ah,1
	mov	dx,1000h
	mov	bx,0
	int	22h

;-------移动kernel代码到5000h处

	cli	
	cld
	push	bp
	mov	bp,8
	mov	ax,0x0
	setseg:
		dec	bp
		add	ax,0x500
		mov	es,ax
		add	ax,0xB00
		mov	ds,ax
	movecode:
		mov	si,0
		mov	di,0
		mov	cx,0x4000
		rep	movsd
	cmp	bp,0
	jnz	setseg
	pop	bp
	
;-------获取物理内存大小
	mov	ax,0x9400
	mov	es,ax
	mov	ds,ax
	mov	si,0
	mov	di,4
	call	PhysicalMemoryCheck
	mov	ax,cs
	mov	ds,ax
	mov	es,ax
;-------设置GDT伪描述符

	mov	ax,cs
	mov	ds,ax
	mov	bx,16
	mul	bx
	add	ax,GDT
	adc	dx,0
	mov	word [gdtaddr],ax
	mov	word [gdtaddr+2],dx

;-------加载GDTR

	mov	ax,cs
	mov	ds,ax
	lgdt	[GDT_DESC]

;-------打开A20地址线

	call	Empty_8042
	mov	al,0xd1
	out	0x64,al
	call	Empty_8042
	mov	al,0xdf
	out	0x64,al
	call	Empty_8042
;-------设置cr0，进入保护模式

	mov	eax,cr0
	or	eax,1
	mov	cr0,eax

;-------设置段选择子，跳转到tmp代码中
	mov	ax,0x10
	mov	ds,ax
	mov	es,ax
	mov	fs,ax
	mov	gs,ax
	mov	ss,ax
	mov	sp,0xffff
	
	jmp	0x8:0x2000
	;加载完成，剩下的就是完成中断函数
bits	16
;----------------------------------21h中断代码区------------------------------	
;21h中断代码
WYOS_int21h_proc:
	pusha
	push	bx
	push	ax
	mov	bx,7
	mov	ah,0eh
	mov	al,0dh
	int	10h
	mov	al,0ah
	int	10h		;重新换一行
	;输出ax
	mov	al,'A'
	int	10h
	mov	al,'X'
	int	10h
	mov	al,'='
	int	10h
	pop	ax		;取得ax,先输出ah,再输出al
	push	ax		;保存ax
	mov	al,ah
	call	SHOWALASCII
	pop	ax
	call	SHOWALASCII
	mov	ah,0eh
	mov	al,'h'
	int	10h
	mov	al,20h
	int	10h
	;输出cx
	mov	al,'C'
	int	10h
	mov	al,'X'
	int	10h
	mov	al,'='
	int	10h
	mov	al,ch
	call	SHOWALASCII
	mov	al,cl
	call	SHOWALASCII
	mov	ah,0eh
	mov	al,'h'
	int	10h
	mov	al,20h
	int	10h
	;输出DX
	mov	al,'D'
	int	10h
	mov	al,'X'
	int	10h
	mov	al,'='
	int	10h
	mov	al,dh
	call	SHOWALASCII
	mov	al,dl
	call	SHOWALASCII
	mov	ah,0eh
	mov	al,'h'
	int	10h
	mov	al,20h
	int	10h
	;输出BX
	mov	al,'B'
	int	10h
	mov	al,'X'
	int	10h
	mov	al,'='
	int	10h
	pop	bx	;取得bx
	mov	cx,bx
	mov	bx,7
	mov	al,ch
	call	SHOWALASCII
	mov	al,cl
	call	SHOWALASCII
	mov	ah,0eh
	mov	al,'h'
	int	10h
	mov	al,20h
	int	10h
	;输出SP
	mov	al,'S'
	int	10h
	mov	al,'P'
	int	10h
	mov	al,'='
	int	10h
	mov	cx,sp
	mov	al,ch
	call	SHOWALASCII
	mov	al,cl
	call	SHOWALASCII
	mov	ah,0eh
	mov	al,'h'
	int	10h
	mov	al,20h
	int	10h
	;输出BP
	mov	al,'B'
	int	10h
	mov	al,'P'
	int	10h
	mov	al,'='
	int	10h
	mov	cx,bp
	mov	al,ch
	call	SHOWALASCII
	mov	al,cl
	call	SHOWALASCII
	mov	ah,0eh
	mov	al,'h'
	int	10h
	mov	al,20h
	int	10h
	;输出SI
	mov	al,'S'
	int	10h
	mov	al,'I'
	int	10h
	mov	al,'='
	int	10h
	mov	cx,si
	mov	al,ch
	call	SHOWALASCII
	mov	al,cl
	call	SHOWALASCII
	mov	ah,0eh
	mov	al,'h'
	int	10h
	mov	al,20h
	int	10h
	;输出DI
	mov	al,'D'
	int	10h
	mov	al,'I'
	int	10h
	mov	al,'='
	int	10h
	mov	cx,di
	mov	al,ch
	call	SHOWALASCII
	mov	al,cl
	call	SHOWALASCII
	mov	ah,0eh
	mov	al,'h'
	int	10h
	mov	al,20h
	int	10h
	;输出FLAG寄存器
	mov	al,0dh
	int	10h
	mov	al,0ah
	int	10h		;重新换一行
	mov	al,'F'
	int	10h
	mov	al,'L'
	int	10h
	mov	al,'A'
	int	10h
	mov	al,'G'
	int	10h
	mov	al,':'
	int	10h
	call	SHOWFLAG
	mov	al,0dh
	int	10h
	mov	al,0ah
	int	10h
	;输出gdtr寄存器
	mov	al,'G'
	int	10h
	mov	al,'D'
	int	10h
	mov	al,'T'
	int	10h
	mov	al,'R'
	int	10h
	mov	al,':'
	int	10h
	sgdt	[GDT_DESC]
	mov	al,byte [gdtaddr+3]
	call	SHOWALASCII
	mov	al,byte [gdtaddr+2]
	call	SHOWALASCII
	mov	al,byte [gdtaddr+1]
	call	SHOWALASCII
	mov	al,byte [gdtaddr]
	call	SHOWALASCII
	mov	al,byte [GDT_DESC+1]
	call	SHOWALASCII
	mov	al,byte [GDT_DESC]
	call	SHOWALASCII
	
	mov	ah,0eh
	mov	al,'h'
	int	10h
	mov	al,20h
	int	10h
	popa
	iret
	
;----------------------------------22h中断代码区------------------------------		
WYOS_int22h_proc:
	push	ds
printstr:
	cmp	ah,0
	jnz	readfile
	call	showmsg
	pop	ds
	iret
readfile:
	cmp	ah,1
	jz	start
	jmp	other
	start:	
	push	cx
	push	bx			
	push	di			;保存di的基址
	
	pop	bx	;取得di的基址
	push	bx	;保存di的基址
	add	bx,1c00h;计算根目录表的结尾地址

	searchfile:
		cmp	BYTE [es:di],0	;比较目录项第一个字节,如果为0则是到达目录表项尾
		jnz	cmpstr		;不为0,比较文件名
		mov	al,0eh		;未找到文件
		pop	di
		pop	bx
		pop	cx
;		pop	ds
		iret
	cmpstr:
		push	ax			;保护环境
		push	si
		push	di
		
		mov	cx,11		
		loopnext:
			mov	al,[ds:si]	;比较文件名
			mov	ah,[es:di]
			cmp	al,ah
			jnz	next
			inc	di
			inc	si
			loop	loopnext
			pop	di
			pop	si
			pop	ax
			jmp	loadfile
		next:	
			pop	di
			pop	si	;恢复环境
			pop	ax
			
			add	di,32
			cmp	di,bx	;如果大于等于则没发现文件
			jb	searchfile
			mov	al,0eh	;未发现文件
			pop	di
			pop	bx
			pop	cx
			pop	ds
			iret
	loadfile:
		mov	ax,[es:di+1ah]	;获得文件第一个簇号
		pop	di
		pop	bx
		pop	cx
		
		push	ds		;保存ds
		push	es
		pop	ds		;将簇表内存段传给DS
		push	si		
		push	es
		mov	es,dx		;获得目标段基址
		mov	si,bx		;或的目标段偏移
		push	ax		;保存簇号
		loadfiletomem:
			call	ReadCluster
			pop	ax		;获得簇号
			add	si,200h		;增加缓存空间
			push	si		;保存si
			mov	si,ax		;获得簇号在fat表中的位置
			shr	si,01h		;判断是奇簇还是偶簇
			pushf
			add	si,ax
			mov	ax,[ds:si+1c00h]	
			popf
			jc	gethigh
			and	ax,0fffh
			jmp	iseof
		gethigh:
			push	cx
			mov	cl,4
			shr	ax,cl
			pop	cx
		iseof:
			cmp	ax,0ff8h
			pop	si
			jae	loadOK
			push	ax
			jmp	loadfiletomem
	loadOK:
		pop	es
		pop	si
		pop	ds
		pop	ds
		iret
other:	
	pop	ds
	iret
	
;==========================================================================	
;SHOWAL
;
;
;
;==========================================================================	
SHOWALASCII:
	push	ax
	push	ax
al_showhigh:
	shr	al,4
	add	al,30h
	cmp	al,39h
	jbe	al_showlow
	add	al,7
	mov	ah,0eh
	int	10h
	jmp	al_low
al_showlow:
	mov	ah,0eh
	int	10h
al_low:	pop	ax
	and	al,0fh
	add	al,30h
	cmp	al,39h
	jbe	al_endshow
	add	al,7
	mov	ah,0eh
	int	10h
	jmp	al_back
al_endshow:
	mov	ah,0eh
	int	10h
al_back:	pop	ax
	ret
;==========================================================================	
;SHOWFLAG
;
;
;
;==========================================================================	
SHOWFLAG:
	PUSH	BX
	PUSHF
	POP	BX
	PUSHF	
	PUSH	AX
	push	cx
	MOV	CX,16
LAST1:
	MOV	AL,'0'
	RCL	BX,1
	JNC	NEXT1
	MOV	AL,'1'
NEXT1:	MOV	AH,0EH
	INT	10H
	LOOP	LAST1
	MOV	AL,'B'
	INT	10H
	POP	CX
	POP	AX
	POPF
	POP	BX
	RET
;=============================================================================
;SHOWMSG
;输出一些字符串
;入口: DS:SI指向目标字符
;返回: 空
;=============================================================================
showmsg:
	push	si
	push	ax
	push    bx
	cld
	mov	ah,0eh			
	mov	bx,7			;选用0页面和字符前景色7
nextchar:
	mov	al,[si]
	inc	si
	or	al,al
	jz	ok
	int	10h
	jmp	nextchar
ok:
	pop	bx
	pop	ax
	pop	si
	ret
;=============================================================================
;ReadCluster
;读取指定簇号
;入口: AX为簇号,ES:SI为缓冲区起始地址
;返回: 无
;=============================================================================
ReadCluster:
	dec	ax		;修正簇号，前两个簇号表示的不是数据区域
	dec	ax
	add	ax,33
	xor	dx,dx
	mov	cx,1
;=============================================================================
;ReadSectors
;读取指定扇区号，指定数目的连续扇区
;入口: AX为逻辑扇区号，从零开始。CX为读取的扇区数。ES:SI为缓冲区起始地址
;返回: 无
;=============================================================================
ReadSectors:
	pusha
	push	cx
	mov	bx,18		;每磁道的扇区数目,ax % 18 + 1就是物理扇区号，而其商就是含磁头的
				;磁道号，
				;因为 扇区号 = 磁道数1*磁头数*扇区数+磁头数1*扇区数+扇区数1-1
				;那么ax/18的商就是磁道数1*磁头数+磁头数1
				;其商再除以磁头数商就是磁道数，而余数就是磁头数
	div	bx
	inc	dx		;得到真实的扇区数
	mov	cx,dx		;保存到cx
	xor	dx,dx		;将得到商，再次除以磁头数目得到
	push	cx
	mov	cx,2		
	div	cx
	pop	cx
				;AX就是磁道数，dx就是磁头数
	mov	ch,al		;磁道的低8位
	ror	ah,2		;将ah的低二位放到高二位
	or	cl,ah		;高二位放到cl的高二位
	mov	dh,dl		;将磁头号送入dh
	mov	dl,0		;驱动器号
	pop	ax		;获得读取扇区数目
	mov	ah,2
	mov	bx,si
	int	13h
	popa
	ret
;=============================================================================
;Empty_8042
;等待键盘控制器闲
;入口: 无
;返回: 无
;=============================================================================
Empty_8042:
	in	al,0x64
	test	al,0x2
	jnz	Empty_8042
	ret
;=============================================================================
;PhyiscMemoryCheck
;获得物理内存大小
;入口: es:di指向地址结构描述符结构起始地址,ds:si地址描述符结构个数
;返回: 无
;=============================================================================
PhysicalMemoryCheck:
	push	ax
	push	bx
	push	cx
	push	dx
	push	di
	push	si
	
	mov	ebx,0
	mov	eax,0x534d4150		;SMAP标志
	GetMemorySize:
		mov	edx,eax
		mov	ecx,20
		mov	eax,0xe820
		int	15h
		jc	MemoryCheckFaild
		add	di,20		;移至下一结构块	
		inc	dword [si]	;增加结构个数
		cmp	ebx,0		;判断是否为最后一个结构块
		jne	GetMemorySize
		jmp	CheckOK
	MemoryCheckFaild:
		mov	dword [si],0
	CheckOK:
		pop	si
		pop	di
		pop	dx
		pop	cx
		pop	bx
		pop	ax
	ret
;------------------------数据区域---------------------------------------------
WYOSMSG		db	'Kernel Run',0dh,0ah,0
WYOSERR		db	'File not found',0dh,0ah,0
WYOSKNL		db	'KERNEL  BIN',0
WYOSTMP		db	'TMP     BIN',0
WYOSTST		db	'TEST    EXE',0
