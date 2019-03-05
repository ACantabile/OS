#ifndef __RING_BUF_H__
#define __RING_BUF_H__
//--------------------------------------------------------
//�汾�Ź���
#define RING_BUF_VER_H 101	//�Ľ�Ring_Buf_Wr_Cover����������ѭ��д�룬headһֱΪ0������������һֱд����󣬲������һ���ֽ���0
//--------------------------------------------------------
#include "utility.h"

typedef struct
{
	unsigned int size;	//��С
	unsigned int head;	//ͷ
	unsigned int tail;	//β
	unsigned char *buffer;	//������ָ��
} RING_BUF_T;


extern unsigned int Ring_Buf_Wr(RING_BUF_T *pr,unsigned char *buf,unsigned int n);
extern void Ring_Buf_Wr_Cover(RING_BUF_T *pr,unsigned char *buf,unsigned int n);
extern unsigned int Ring_buf_byte(RING_BUF_T *pr);		//���������ֽ���
extern unsigned int Ring_buf_leftbyte(RING_BUF_T *pr);	//ʣ���ֽ���
extern unsigned int Ring_Buf_Rd(RING_BUF_T *pr,unsigned char *buf,unsigned int n);
extern unsigned int Ring_Buf_Copy(RING_BUF_T *pr,unsigned char *buf,unsigned int n);
extern unsigned int Ring_Buf_Erase(RING_BUF_T *pr,unsigned short n);
extern void Ring_Buf_Clr(RING_BUF_T *pr);
extern void Ring_Buf_Init(RING_BUF_T *pr,unsigned int size);

#endif
