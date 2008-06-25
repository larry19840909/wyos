;/***************************************************************************
;			WYOS WYOSBOOT.asm
;			引导文件
;		编码:WY.lslrt			editor	 :WY.lslrt
;		日期:2005/12/2			date	 :2005/12/2
;		版权:WY 和 WY.lslrt所有		copyright:WY and WY.lslrt
;		URL:http://www.wyos.net http://wylslrt.go.3322.org
;***************************************************************************/
;代码开始

bits		16
org		0x7C00
	JMP SHORT	WYOSSTART	;采用FAT12文件系统
	NOP
;=============================================================================
;FAT12文件系统头
OEM		DB	'WYOS0.01'	;文件系统的描述信息
SECTSIZE   	DW	512		;扇区大小（字节），应为 512
CLUSTSIZE	DB	1		;簇的扇区数，应为 2 的幂，FAT12 为 1
RESSECT		DW	1		;保留扇区，FAT12/16 应为 1
FATCNT		DB	2		;FAT 结构数目，一般为 2
ROOTSIZE	DW	224		;根目录项目数，FAT12 为 224
TOTALSECT	DW	2880		;扇区总数，1.44M 软盘为 2880
MEDIA		DB	0xF0		;设备类型，1.44M 软盘为 F0h
FATSIZE		DW	9		;FAT 占用扇区数，9
TRACKSECT	DW	18		;磁道扇区数，18
HEADCNT		DW	2		;磁头数，2
HIDENSECT	DD	0		;隐藏扇区，默认为 0
HUGESECT	DD	0		;隐藏扇区，默认为 0
;－－下面的内容为 FAT12/16 所有，和 FAT32 不同——
BSBOOTDRV	DB 	0		;MS-DOS 使用，0
BSRESERV	DB	0		;Windows NT 使用，0
BSBOOTSIGN	DB	29H		;附加的可启动标志，29h
BSVOLID		DD	0		;卷标序列号，00000000h
BSVOLABEL	DB	"WYOS       "   ;卷标，11 字节，必须用空格( 20h )补齐
BSFSTYPE	DB	"FAT12   "	;文件系统标志
;文件系统头结束
;=============================================================================
;=============================================================================
;WYOS BOOT START
;=============================================================================
WYOSSTART:
	cli
	mov	ax,cs
	mov	ds,ax

	mov	ax,50h
	mov	ss,ax
	mov	sp,7bffh
	mov	bp,7c00h
	sti
	mov	si,WYHELLO
	call	SHOWMSG
	mov	si,WYOSMSG1
	call	SHOWMSG
LoadingRootTable:			;加载根目录表
	mov	ax,50h
	mov	es,ax
	mov	ax,19			;根目录的起始逻辑扇区号
	mov	cx,14			;根目录所占用扇区数目
	mov	si,0
	call	ReadSectors
SearchKNL:				;查找内核文件
	cmp	BYTE [es:si],0		;比较第一个字节是否为0，如是则到有效根目录表的尾部
	jnz	cmpstr			;不为0，则比较文件名
	push	si
	mov	si,WYOSMSG2		;为0，输出错误信息，结束
	call	SHOWMSG
	pop	si
	jmp	end
cmpstr:
	mov	cx,11
	push	si			;保存内核文件根目录的首地址
	mov	di,WYOSKNL		;将内核文件名首地址送入di
loopnext:
	mov	al,[ds:di]		
	mov	ah,[es:si]
	cmp	al,ah			
	jnz	next			;如果不等则比较下一个
	inc	di			
	inc	si
	loop	loopnext		;比较下一个字符
	jmp	LoadKernel		;如果相等则开始加载内核文件
next:	pop	si
	add	si,32
	cmp	si,1c00h
	jb	SearchKNL
end:	jmp	$
LoadKernel:
	mov	ax,1			;读取fat表
	mov	cx,9
	mov	si,1c00h		;因为从50h:0000h开始已经读取了14个扇区
					;共1c00h字节，因此从50h:1c00h开始放入fat表
					;现在从内存500开始1c00,50h:0000-1bffh字节为根目录表
					;50h:1c00-2dff为第一个fat表
	call	ReadSectors
	pop	si			;读取内核文件根目录的地址
	mov	ax,[es:si+1ah]		;获得低簇号
	push	ax			;保存簇号
	mov	si,3300h
readkernel:
	call	ReadCluster		;将内核文件加载到0050:3300处
	pop	ax			;获得簇号
	add	si,200h			;增加缓存空间
	push	si			;保存内核文件缓冲区地址
	mov	si,ax
	shr	si,01h			;除2，判断奇偶,奇取簇的低12位，偶取高12位
	pushf				;保存标志
	add	si,ax
	mov	ax,[es:si+1c00h]
	popf
	jc	gethigh
	and	ax,0fffh		;取低十二位
	jmp	iseof			;判断是否到了文件尾
gethigh:
	mov	cl,4
	shr	ax,cl
iseof:
	cmp	ax,0ff8h		;如果小于则继续读取
	jae	runknl
	pop	si
	push	ax
	jmp	readkernel
runknl:	
	mov	si,WYOSMSG
	call	SHOWMSG
	xor	si,si
	mov	ax,330h
	mov	ds,ax
	mov	es,ax
	jmp	380h:0000
	jmp	$
;-------------END OF BOOT-------------
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
				;因为 扇区号 = 磁道数*磁头数*扇区数+磁头号*扇区数+扇区数-1
				;那么ax/18的商就是磁道数*磁头数+磁头号
				;其商再除以磁头数商就是磁道数，而余数就是磁头号
	div	bx
	inc	dx		;得到真实的扇区数
	mov	cx,dx		;保存到cx
	xor	dx,dx		;将得到商，再次除以磁头数目得到
	div	WORD [BYTE HEADCNT]
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
;SHOWMSG
;输出一些字符串
;入口: DS:SI指向目标字符
;返回: 空
;=============================================================================
SHOWMSG:
	PUSH	SI
	PUSH	AX
	PUSH	BX
	CLD
	MOV	AH,0EH			
	MOV	BX,7			;选用0页面和字符前景色7
NEXTCHAR:
	MOV	AL,[SI]
	INC	SI
	OR	AL,AL
	JZ	OK
	INT	10H
	JMP	NEXTCHAR
OK:
	POP	BX
	POP	AX
	POP	SI
	RET
;-------------END OF SHOWMSG---------------------
WYHELLO		DB	'WY: Hello!',0DH,0AH,0
WYOSMSG		DB	'WYOS Load Setup SUCCESS.',0DH,0AH,0
WYOSMSG1	DB	'LOADING SETUP CODE.',0DH,0AH,0
WYOSMSG2	db	'Setup not found.',0dh,0ah,0
WYOSKNL		db	'SETUP   BIN',0

;填充剩余的扇区内容
times 510-($-$$) DB 0
BOOT_SIGN	DW	0xAA55	;引导扇区标志
;--------------END OF BOOT PROGRAM------------------