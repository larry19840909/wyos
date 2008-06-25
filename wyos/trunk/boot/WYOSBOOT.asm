;/***************************************************************************
;			WYOS WYOSBOOT.asm
;			�����ļ�
;		����:WY.lslrt			editor	 :WY.lslrt
;		����:2005/12/2			date	 :2005/12/2
;		��Ȩ:WY �� WY.lslrt����		copyright:WY and WY.lslrt
;		URL:http://www.wyos.net http://wylslrt.go.3322.org
;***************************************************************************/
;���뿪ʼ

bits		16
org		0x7C00
	JMP SHORT	WYOSSTART	;����FAT12�ļ�ϵͳ
	NOP
;=============================================================================
;FAT12�ļ�ϵͳͷ
OEM		DB	'WYOS0.01'	;�ļ�ϵͳ��������Ϣ
SECTSIZE   	DW	512		;������С���ֽڣ���ӦΪ 512
CLUSTSIZE	DB	1		;�ص���������ӦΪ 2 ���ݣ�FAT12 Ϊ 1
RESSECT		DW	1		;����������FAT12/16 ӦΪ 1
FATCNT		DB	2		;FAT �ṹ��Ŀ��һ��Ϊ 2
ROOTSIZE	DW	224		;��Ŀ¼��Ŀ����FAT12 Ϊ 224
TOTALSECT	DW	2880		;����������1.44M ����Ϊ 2880
MEDIA		DB	0xF0		;�豸���ͣ�1.44M ����Ϊ F0h
FATSIZE		DW	9		;FAT ռ����������9
TRACKSECT	DW	18		;�ŵ���������18
HEADCNT		DW	2		;��ͷ����2
HIDENSECT	DD	0		;����������Ĭ��Ϊ 0
HUGESECT	DD	0		;����������Ĭ��Ϊ 0
;�������������Ϊ FAT12/16 ���У��� FAT32 ��ͬ����
BSBOOTDRV	DB 	0		;MS-DOS ʹ�ã�0
BSRESERV	DB	0		;Windows NT ʹ�ã�0
BSBOOTSIGN	DB	29H		;���ӵĿ�������־��29h
BSVOLID		DD	0		;������кţ�00000000h
BSVOLABEL	DB	"WYOS       "   ;��꣬11 �ֽڣ������ÿո�( 20h )����
BSFSTYPE	DB	"FAT12   "	;�ļ�ϵͳ��־
;�ļ�ϵͳͷ����
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
LoadingRootTable:			;���ظ�Ŀ¼��
	mov	ax,50h
	mov	es,ax
	mov	ax,19			;��Ŀ¼����ʼ�߼�������
	mov	cx,14			;��Ŀ¼��ռ��������Ŀ
	mov	si,0
	call	ReadSectors
SearchKNL:				;�����ں��ļ�
	cmp	BYTE [es:si],0		;�Ƚϵ�һ���ֽ��Ƿ�Ϊ0����������Ч��Ŀ¼���β��
	jnz	cmpstr			;��Ϊ0����Ƚ��ļ���
	push	si
	mov	si,WYOSMSG2		;Ϊ0�����������Ϣ������
	call	SHOWMSG
	pop	si
	jmp	end
cmpstr:
	mov	cx,11
	push	si			;�����ں��ļ���Ŀ¼���׵�ַ
	mov	di,WYOSKNL		;���ں��ļ����׵�ַ����di
loopnext:
	mov	al,[ds:di]		
	mov	ah,[es:si]
	cmp	al,ah			
	jnz	next			;���������Ƚ���һ��
	inc	di			
	inc	si
	loop	loopnext		;�Ƚ���һ���ַ�
	jmp	LoadKernel		;��������ʼ�����ں��ļ�
next:	pop	si
	add	si,32
	cmp	si,1c00h
	jb	SearchKNL
end:	jmp	$
LoadKernel:
	mov	ax,1			;��ȡfat��
	mov	cx,9
	mov	si,1c00h		;��Ϊ��50h:0000h��ʼ�Ѿ���ȡ��14������
					;��1c00h�ֽڣ���˴�50h:1c00h��ʼ����fat��
					;���ڴ��ڴ�500��ʼ1c00,50h:0000-1bffh�ֽ�Ϊ��Ŀ¼��
					;50h:1c00-2dffΪ��һ��fat��
	call	ReadSectors
	pop	si			;��ȡ�ں��ļ���Ŀ¼�ĵ�ַ
	mov	ax,[es:si+1ah]		;��õʹغ�
	push	ax			;����غ�
	mov	si,3300h
readkernel:
	call	ReadCluster		;���ں��ļ����ص�0050:3300��
	pop	ax			;��ôغ�
	add	si,200h			;���ӻ���ռ�
	push	si			;�����ں��ļ���������ַ
	mov	si,ax
	shr	si,01h			;��2���ж���ż,��ȡ�صĵ�12λ��żȡ��12λ
	pushf				;�����־
	add	si,ax
	mov	ax,[es:si+1c00h]
	popf
	jc	gethigh
	and	ax,0fffh		;ȡ��ʮ��λ
	jmp	iseof			;�ж��Ƿ����ļ�β
gethigh:
	mov	cl,4
	shr	ax,cl
iseof:
	cmp	ax,0ff8h		;���С���������ȡ
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
				;��Ϊ ������ = �ŵ���*��ͷ��*������+��ͷ��*������+������-1
				;��ôax/18���̾��Ǵŵ���*��ͷ��+��ͷ��
				;�����ٳ��Դ�ͷ���̾��Ǵŵ��������������Ǵ�ͷ��
	div	bx
	inc	dx		;�õ���ʵ��������
	mov	cx,dx		;���浽cx
	xor	dx,dx		;���õ��̣��ٴγ��Դ�ͷ��Ŀ�õ�
	div	WORD [BYTE HEADCNT]
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
;SHOWMSG
;���һЩ�ַ���
;���: DS:SIָ��Ŀ���ַ�
;����: ��
;=============================================================================
SHOWMSG:
	PUSH	SI
	PUSH	AX
	PUSH	BX
	CLD
	MOV	AH,0EH			
	MOV	BX,7			;ѡ��0ҳ����ַ�ǰ��ɫ7
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

;���ʣ�����������
times 510-($-$$) DB 0
BOOT_SIGN	DW	0xAA55	;����������־
;--------------END OF BOOT PROGRAM------------------