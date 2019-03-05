#include "fixlen_que.h"
#include "string.h"

//----------------------------------------------------------------------
//�汾�Ź���
#define FIXLEN_QUE_VER_C 100

	#if FIXLEN_QUE_VER_C != FIXLEN_QUE_VER_H
		#ERROR
	#endif
//----------------------------------------------------------------------

//-----------------------------------------------------------------------------
//�򻺳�����дһ�鶨������
//�ɹ�������0
//ʧ�ܣ�����-1
int fixlen_que_Wr(FIXLEN_QUE_T *pr,unsigned char *buf,unsigned short n)
{
	unsigned int i;
	i=Ring_buf_leftbyte(pr);
	if((n+2)>i)
	{
		return(-1);
	}
	
	Ring_Buf_Wr(pr,(unsigned char *)&n,2);
	Ring_Buf_Wr(pr,buf,n);
	return(0);
}
//---------------------------------------------------
//�ӻ��������һ�鶨������
//�ɹ��������ֽ���
//ʧ�ܣ�����0
int fixlen_que_Rd(FIXLEN_QUE_T *pr,unsigned char *buf,unsigned short buf_size)
{
	unsigned short i;
	unsigned short n;
	
	i=Ring_buf_byte(pr);
	if(i<3)
	{
		return(0);
	}
	Ring_Buf_Copy(pr,(unsigned char *)&n,2);
	if(i < (n+2))
	{
		return(0);
	}
	if(n > buf_size)
	{
		return(-1);
	}
	Ring_Buf_Erase(pr,2);
	Ring_Buf_Rd(pr,buf,n);
	return(n);
}
//-----------------------------------------------------------------------------
