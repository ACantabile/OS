#ifndef __RING_BUF_H__
#define __RING_BUF_H__
//--------------------------------------------------------
//版本号管理
#define RING_BUF_VER_H 101	//改进Ring_Buf_Wr_Cover函数，不再循环写入，head一直为0，新来的数据一直写在最后，并在最后一个字节填0
//--------------------------------------------------------
#include "utility.h"

typedef struct
{
	unsigned int size;	//大小
	unsigned int head;	//头
	unsigned int tail;	//尾
	unsigned char *buffer;	//缓冲区指针
} RING_BUF_T;


extern unsigned int Ring_Buf_Wr(RING_BUF_T *pr,unsigned char *buf,unsigned int n);
extern void Ring_Buf_Wr_Cover(RING_BUF_T *pr,unsigned char *buf,unsigned int n);
extern unsigned int Ring_buf_byte(RING_BUF_T *pr);		//缓冲区的字节数
extern unsigned int Ring_buf_leftbyte(RING_BUF_T *pr);	//剩余字节数
extern unsigned int Ring_Buf_Rd(RING_BUF_T *pr,unsigned char *buf,unsigned int n);
extern unsigned int Ring_Buf_Copy(RING_BUF_T *pr,unsigned char *buf,unsigned int n);
extern unsigned int Ring_Buf_Erase(RING_BUF_T *pr,unsigned short n);
extern void Ring_Buf_Clr(RING_BUF_T *pr);
extern void Ring_Buf_Init(RING_BUF_T *pr,unsigned int size);

#endif
