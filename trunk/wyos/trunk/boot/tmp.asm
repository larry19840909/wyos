;
;�ô���ֻ�����ں˴�����˵�0x100000��
;
;
bits	32
org	2000h

start:
	;�ƶ����Գ������
	mov	esi,0x4000
	mov	edi,0x1000000
	mov	ecx,0x1000
	rep	movsd

	;�ƶ��ں˴���
	mov	esi,0x5000
	mov	edi,0x100000
	mov	ecx,0x20000
	rep	movsd
	jmp	0x100000
