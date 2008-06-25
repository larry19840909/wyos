/***************************************************************************
			WYOS video.h
			��ʾ����ͷ�ļ�
						����:WY.lslrt				editor	 :WY.lslrt
						����:2005/12/2			date	 :2005/12/2
						��Ȩ:WY �� WY.lslrt���� 	copyright:WY and WY.lslrt
						URL:http://www.wyos.net http://wylslrt.go.3322.org
***************************************************************************/
#ifndef	_WYOS_VIDEO_H
#define	_WYOS_VIDEO_H

int	WY_nVideoPos;	//��ǰ��ʾ��ַ
int	WY_nCharChara;	//�ַ�����

#define	VIDEO_TEXT_BASE_ADDR	0xB8000	//�ı�ģʽ����ʾ�ڴ�����Ļ���ַ
#define	VIDEO_TEXT_MAX_ADDR	0xF9F	//�ı�ģʽ��������ʾ��Ե�ַ

//print.asm��ShowChar����
extern void ShowChar(char *WY_pDisplay,char WY_chDisp);

//video��ʾ������ʼ������
void wyos_video_int();

//����
void	ClearVideo();

//����
void ScrollVideo();

//�����ַ���ʾ����
//WY_usFRGB		�ַ�ǰ��ɫ
//WY_bHighLight	�Ƿ������ʾTRUE����
//WY_usBRGB	�ַ�����ɫ
//WY_bBlink		�Ƿ���˸��ʾTRUE��˸
//����ֵ���ù�����ַ�����
char SetCharCharacterEx(ushort	WY_usFRGB,
					             BOOL	WY_bHighLight,
						      ushort 	WY_usBRGB,
						      BOOL 	WY_bBlink);

//�����ַ���ʾ����
//WY_cCharChara		ָ�����ַ�����
void SetCharaCHaracter(char WY_cCharChara);

#define	VIDEO_CHAR_BLACKBLUE		0x1
#define	VIDEO_CHAR_BLACKRED		0x4
#define	VIDEO_CHAR_BLACKWHITE	0x7
#define	VIDEO_CHAR_BLACKYELLO	0xE
#define	VIDEO_CHAR_WHITEBLACK	0x70
#define	VIDEO_CHAR_WHITERED		0x74

#define	VIDEO_RGB_BLACK			0x0
#define	VIDEO_RGB_BLUE			0x1
#define	VIDEO_RGB_GREEN			VIDEO_RGB_BLUE + 1
#define	VIDEO_RGB_DARKBLUE		VIDEO_RGB_GREEN + 1
#define	VIDEO_RGB_RED				VIDEO_RGB_DARKBLUE + 1
#define	VIDEO_RGB_FUCHSIN			VIDEO_RGB_RED + 1
#define	VIDEO_RGB_BROWN			VIDEO_RGB_FUCHSIN + 1
#define	VIDEO_RGB_WHITE			VIDEO_RGB_BROWN + 1
#define	VIDEO_RGB_LIGHTGRAY		VIDEO_RGB_WHITE + 1
#define	VIDEO_RGB_LIGHTBLUE		VIDEO_RGB_LIGHTGRAY + 1
#define	VIDEO_RGB_LIGHTGREEN		VIDEO_RGB_LIGHTBLUE + 1
#define	VIDEO_RGB_LIGHTCYAN		VIDEO_RGB_LIGHTGREEN + 1
#define	VIDEO_RGB_LIGHTRED		VIDEO_RGB_LIGHTCYAN + 1
#define	VIDEO_RGB_LIGHTFUCHSIN	VIDEO_RGB_LIGHTRED + 1
#define	VIDEO_RGB_YELLO			VIDEO_RGB_LIGHTFUCHSIN + 1
#define	VIDEO_RGB_LIGHTWHITE		VIDEO_RGB_YELLO + 1

//���һ���ַ�
//WY_cDisp	ϣ��������ַ�
void putc(char WY_cDisp);

//���һ���ַ���
//WY_szDisp	ϣ��������ַ���
void puts(char *WY_szDisp);

//���һ������
//WY_nDisp ϣ�����������
//WY_bFullPrn�Ƿ����ȫ������TRUE: 123 putn : 00000000123
//FALSE: 123 putn : 123
void putn(int WY_nDisp,BOOL WY_bFullPrn);

//����16�����������
void putx(int WY_nDisp);

//��ʽ������ַ���
void printk(char* WY_pcfmt,...);

//ϵͳ���ýӿڴ�ӡ�ַ�
ulong sysputc(pSyscallParam WY_pInputParam);

//ϵͳ���ýӿ�����
ulong syscls(pSyscallParam WY_pInputParam);

#endif
