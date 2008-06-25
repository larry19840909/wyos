;/***************************************************************************
;			WYOS Setup.asm
;			����ϵͳ�ں˼��ش���,����WYOSBOOT.asm��ִ��			
;		����:WY.lslrt			editor	 :WY.lslrt
;		����:2005/12/2			date	 :2005/12/2
;		����:2006/2/19
;		��Ȩ:WY �� WY.lslrt����		copyright:WY and WY.lslrt
;		URL:http://www.wyos.net http://wylslrt.go.3322.org
;***************************************************************************/
;��������г�ʼ�������ͼ���Щ�ж�
;�ж���Ϊ22h
;����ַ���		���룺ah = 0 DS:SI�ַ�����0Ϊ����
;��ȡ�ļ���ָ���ڴ����� ���룺ah = 1 DS:SI�ļ�����ES:DIĿ¼���ڴ�����,
;			�����DX:BX���뵽�ڴ�����
;			      al: 0eh �ļ�δ�ҵ� 0fh�ҵ��ļ�											
;�ж���Ϊ21h
;������Ĵ�����ֵ	�����AX,CX,DX,BX,SP,BP,SI,DI,FLAG
;50h:0000h-------------------------------
;	           ��Ŀ¼��
;50h:1c00h-------------------------------
;		   FAT��
;50h:3300h-------------------------------
;		   Kernel��������
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

;-------����ʵģʽ�ж�

	;��ʼ�����жϣ�Ϊ�˱���μ�����Ժ����д��붼�жγ�Խǰ׺
	;д��22h�ж�
	cli
	mov	di,22h*4
	mov	ax,cs
	mov	[ds:di+2],ax	;���ж�������д���ֵ
	mov	ax,WYOS_int22h_proc
	mov	[ds:di],ax	;���ж�������д��ƫ��
	;д��21h�ж�
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

	
;-------��ȡtest.exe�Ĵ���	4KB

	;��ȡtext.exe���뵽1000h:0000
	mov	si,WYOSTST
	mov	ax,50h
	mov	es,ax
	mov	di,0
	mov	ah,1
	mov	dx,1000h
	mov	bx,0
	int	22h

;-------�ƶ�test���뵽4000h��

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

;-------��ȡtmp.bin
;
;�������1000:0000H��
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
;�ƶ����뵽2000h
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
	
;-------��ȡkernel.bin�Ĵ���,512KB

	mov	ax,cs
	mov	ds,ax
	;��ȡ32λkernel���뵽1000h:0000
	mov	si,WYOSKNL
	mov	ax,50h
	mov	es,ax
	mov	di,0
	mov	ah,1
	mov	dx,1000h
	mov	bx,0
	int	22h

;-------�ƶ�kernel���뵽5000h��

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
	
;-------��ȡ�����ڴ��С
	mov	ax,0x9400
	mov	es,ax
	mov	ds,ax
	mov	si,0
	mov	di,4
	call	PhysicalMemoryCheck
	mov	ax,cs
	mov	ds,ax
	mov	es,ax
;-------����GDTα������

	mov	ax,cs
	mov	ds,ax
	mov	bx,16
	mul	bx
	add	ax,GDT
	adc	dx,0
	mov	word [gdtaddr],ax
	mov	word [gdtaddr+2],dx

;-------����GDTR

	mov	ax,cs
	mov	ds,ax
	lgdt	[GDT_DESC]

;-------��A20��ַ��

	call	Empty_8042
	mov	al,0xd1
	out	0x64,al
	call	Empty_8042
	mov	al,0xdf
	out	0x64,al
	call	Empty_8042
;-------����cr0�����뱣��ģʽ

	mov	eax,cr0
	or	eax,1
	mov	cr0,eax

;-------���ö�ѡ���ӣ���ת��tmp������
	mov	ax,0x10
	mov	ds,ax
	mov	es,ax
	mov	fs,ax
	mov	gs,ax
	mov	ss,ax
	mov	sp,0xffff
	
	jmp	0x8:0x2000
	;������ɣ�ʣ�µľ�������жϺ���
bits	16
;----------------------------------21h�жϴ�����------------------------------	
;21h�жϴ���
WYOS_int21h_proc:
	pusha
	push	bx
	push	ax
	mov	bx,7
	mov	ah,0eh
	mov	al,0dh
	int	10h
	mov	al,0ah
	int	10h		;���»�һ��
	;���ax
	mov	al,'A'
	int	10h
	mov	al,'X'
	int	10h
	mov	al,'='
	int	10h
	pop	ax		;ȡ��ax,�����ah,�����al
	push	ax		;����ax
	mov	al,ah
	call	SHOWALASCII
	pop	ax
	call	SHOWALASCII
	mov	ah,0eh
	mov	al,'h'
	int	10h
	mov	al,20h
	int	10h
	;���cx
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
	;���DX
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
	;���BX
	mov	al,'B'
	int	10h
	mov	al,'X'
	int	10h
	mov	al,'='
	int	10h
	pop	bx	;ȡ��bx
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
	;���SP
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
	;���BP
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
	;���SI
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
	;���DI
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
	;���FLAG�Ĵ���
	mov	al,0dh
	int	10h
	mov	al,0ah
	int	10h		;���»�һ��
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
	;���gdtr�Ĵ���
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
	
;----------------------------------22h�жϴ�����------------------------------		
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
	push	di			;����di�Ļ�ַ
	
	pop	bx	;ȡ��di�Ļ�ַ
	push	bx	;����di�Ļ�ַ
	add	bx,1c00h;�����Ŀ¼��Ľ�β��ַ

	searchfile:
		cmp	BYTE [es:di],0	;�Ƚ�Ŀ¼���һ���ֽ�,���Ϊ0���ǵ���Ŀ¼����β
		jnz	cmpstr		;��Ϊ0,�Ƚ��ļ���
		mov	al,0eh		;δ�ҵ��ļ�
		pop	di
		pop	bx
		pop	cx
;		pop	ds
		iret
	cmpstr:
		push	ax			;��������
		push	si
		push	di
		
		mov	cx,11		
		loopnext:
			mov	al,[ds:si]	;�Ƚ��ļ���
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
			pop	si	;�ָ�����
			pop	ax
			
			add	di,32
			cmp	di,bx	;������ڵ�����û�����ļ�
			jb	searchfile
			mov	al,0eh	;δ�����ļ�
			pop	di
			pop	bx
			pop	cx
			pop	ds
			iret
	loadfile:
		mov	ax,[es:di+1ah]	;����ļ���һ���غ�
		pop	di
		pop	bx
		pop	cx
		
		push	ds		;����ds
		push	es
		pop	ds		;���ر��ڴ�δ���DS
		push	si		
		push	es
		mov	es,dx		;���Ŀ��λ�ַ
		mov	si,bx		;���Ŀ���ƫ��
		push	ax		;����غ�
		loadfiletomem:
			call	ReadCluster
			pop	ax		;��ôغ�
			add	si,200h		;���ӻ���ռ�
			push	si		;����si
			mov	si,ax		;��ôغ���fat���е�λ��
			shr	si,01h		;�ж�����ػ���ż��
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
;���һЩ�ַ���
;���: DS:SIָ��Ŀ���ַ�
;����: ��
;=============================================================================
showmsg:
	push	si
	push	ax
	push    bx
	cld
	mov	ah,0eh			
	mov	bx,7			;ѡ��0ҳ����ַ�ǰ��ɫ7
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
;��ȡָ���غ�
;���: AXΪ�غ�,ES:SIΪ��������ʼ��ַ
;����: ��
;=============================================================================
ReadCluster:
	dec	ax		;�����غţ�ǰ�����غű�ʾ�Ĳ�����������
	dec	ax
	add	ax,33
	xor	dx,dx
	mov	cx,1
;=============================================================================
;ReadSectors
;��ȡָ�������ţ�ָ����Ŀ����������
;���: AXΪ�߼������ţ����㿪ʼ��CXΪ��ȡ����������ES:SIΪ��������ʼ��ַ
;����: ��
;=============================================================================
ReadSectors:
	pusha
	push	cx
	mov	bx,18		;ÿ�ŵ���������Ŀ,ax % 18 + 1�������������ţ������̾��Ǻ���ͷ��
				;�ŵ��ţ�
				;��Ϊ ������ = �ŵ���1*��ͷ��*������+��ͷ��1*������+������1-1
				;��ôax/18���̾��Ǵŵ���1*��ͷ��+��ͷ��1
				;�����ٳ��Դ�ͷ���̾��Ǵŵ��������������Ǵ�ͷ��
	div	bx
	inc	dx		;�õ���ʵ��������
	mov	cx,dx		;���浽cx
	xor	dx,dx		;���õ��̣��ٴγ��Դ�ͷ��Ŀ�õ�
	push	cx
	mov	cx,2		
	div	cx
	pop	cx
				;AX���Ǵŵ�����dx���Ǵ�ͷ��
	mov	ch,al		;�ŵ��ĵ�8λ
	ror	ah,2		;��ah�ĵͶ�λ�ŵ��߶�λ
	or	cl,ah		;�߶�λ�ŵ�cl�ĸ߶�λ
	mov	dh,dl		;����ͷ������dh
	mov	dl,0		;��������
	pop	ax		;��ö�ȡ������Ŀ
	mov	ah,2
	mov	bx,si
	int	13h
	popa
	ret
;=============================================================================
;Empty_8042
;�ȴ����̿�������
;���: ��
;����: ��
;=============================================================================
Empty_8042:
	in	al,0x64
	test	al,0x2
	jnz	Empty_8042
	ret
;=============================================================================
;PhyiscMemoryCheck
;��������ڴ��С
;���: es:diָ���ַ�ṹ�������ṹ��ʼ��ַ,ds:si��ַ�������ṹ����
;����: ��
;=============================================================================
PhysicalMemoryCheck:
	push	ax
	push	bx
	push	cx
	push	dx
	push	di
	push	si
	
	mov	ebx,0
	mov	eax,0x534d4150		;SMAP��־
	GetMemorySize:
		mov	edx,eax
		mov	ecx,20
		mov	eax,0xe820
		int	15h
		jc	MemoryCheckFaild
		add	di,20		;������һ�ṹ��	
		inc	dword [si]	;���ӽṹ����
		cmp	ebx,0		;�ж��Ƿ�Ϊ���һ���ṹ��
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
;------------------------��������---------------------------------------------
WYOSMSG		db	'Kernel Run',0dh,0ah,0
WYOSERR		db	'File not found',0dh,0ah,0
WYOSKNL		db	'KERNEL  BIN',0
WYOSTMP		db	'TMP     BIN',0
WYOSTST		db	'TEST    EXE',0
