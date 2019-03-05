#include "ring_buf.h"
#include "string.h"
#include "utility.h"

//----------------------------------------------------------------------
//�汾�Ź���
#define RING_BUF_VER_C 101

	#if RING_BUF_VER_C != RING_BUF_VER_H
		#ERROR
	#endif
//----------------------------------------------------------------------

//-----------------------------------------------------------------------------
unsigned int Ring_Buf_Wr(RING_BUF_T *pr,unsigned char *buf,unsigned int n)
{
	unsigned int head,tail,size,left,tail_end;
	
	head=pr->head;
	tail=pr->tail;
	size=pr->size;
	
	
	
	//left=(head-tail)%size;	//��ʣ��Ŀռ�
	tail_end = size - tail;						//��β�����Ŀռ�
	left=(head + tail_end)%size;	//��ʣ��Ŀռ�
	
	if(left==0)
	{
		left=size;		//��������
	}
	
	left--;			//���𻺳������뻺��������ʵ�ʿռ����һ��
	
	if(n > left)
	{
		n=left;
	}
		
	if(n>0)
	{
		if(head > tail)
		{
			memcpy(pr->buffer+tail,buf,n);			//ͷ����β��ʣ��ռ�����������
		}
		else
		{
			//tail_end = size - tail;						//��β�����Ŀռ�
			if(tail_end>=n)
			{
				memcpy(pr->buffer+tail,buf,n);		//��n����������ռ�ֱ�ӿ���
			}
			else
			{
				memcpy(pr->buffer+tail,buf,tail_end);		//�ȿ���β�������ֽ���
				memcpy(pr->buffer,buf+tail_end,n-tail_end);	//�ٿ���ʣ�µ��ֽ���
			}
		}
		
		pr->tail=(tail + n) % size;		//����βָ��
	}

	//���ؿ������ֽ���
	return(n);
}
//--------------------------------------------------------------------------------
//дѭ���������д���ֽ�������ʣ�µĿռ��򸲸����������
/*
void Ring_Buf_Wr_Cover(RING_BUF_T *pr,unsigned char *buf,unsigned int n)
{
	unsigned int head,tail,size,tail_end,left;
	int b_over=0;
	
	if(n==0)
	{
		return;
	}
	
	head=pr->head;
	tail=pr->tail;
	size=pr->size;
	
	//left=(head-tail)%size;	//��ʣ��Ŀռ�
	tail_end= size - tail;						//��β�����Ŀռ�
	left=(head + tail_end)%size;	//��ʣ��Ŀռ�
	
	if(left==0)
	{
		left=size;		//��������
	}
	
	left--;			//���𻺳������뻺��������ʵ�ʿռ����һ��
	
	if(n >= left)
	{
		b_over=1;	//�����ֽ�������ʣ�µ��ֽ�������Ҫ����head
	}
	
	if(n >= size)	//������ֽ������������ռ䣬ȡ����size-1����
	{
		buf=buf+(n-(size-1));	//ָ�����(size-1)��С��λ��
		n=size-1;				//��������Ϊsize-1
	}
	
	//tail_end=size-tail;						//��β�����Ŀռ�
	
	if(tail_end >= n)
	{
		memcpy(pr->buffer+tail,buf,n);		//��n�����ȣ���������ռ�ֱ�ӿ���
	}
	else
	{
		memcpy(pr->buffer+tail,buf,tail_end);		//�ȿ���β�������ֽ���
		memcpy(pr->buffer,buf+tail_end,n-tail_end);	//�ٿ���ʣ�µ��ֽ���
	}
	
	pr->tail=(tail+n) % size;
	if(b_over)
	{
		pr->head=(pr->tail+1) % size;			//�����ռ䣬�����
	}
}
*/
//-----------------------------------------------------------------------------
//дѭ���������д���ֽ�������ʣ�µĿռ����Ƴ���������ݣ��������ݷ���������һ���ֽ���0
//void Ring_Buf_Wr_Tail(RING_BUF_T *pr,unsigned char *buf,unsigned int n)
void Ring_Buf_Wr_Cover(RING_BUF_T *pr,unsigned char *buf,unsigned int n)
{
	unsigned int tail,size,left,i;
	
	unsigned char *ps;
	unsigned char *pd;
	
	if(n==0)
	{
		return;
	}
	
	tail=pr->tail;
	size=pr->size-1;
	
	if(n>size)
	{					//���Ҫд������ݴ��ڻ�������ȡҪд�����ݵĺ���һ����
		buf+=(n-size);
		n=size;
	}
	
	left=size-tail;	//��ʣ��Ŀռ�
	
	if(left>=n)
	{
		memcpy(pr->buffer+tail,buf,n);		//��n�����ȣ���������ռ�ֱ�ӿ���
		tail+=n;
	}
	else
	{					//ʣ��ռ��Ҫд�����
		i=n-left;		//����ټ���
		pd=pr->buffer;	//�ѻ�������������ļ���ȥ��
		ps=pd+i;
		i=tail-i;		//���Ҫ��λ������
		for(;i>0;i--)	//���ݰ��
		{
			*pd++=*ps++;
		}
		memcpy(pd,buf,n);//����������
		tail=size;
	}
	
	pr->buffer[tail]=0;
	pr->tail=tail;
	pr->head=0;			//�����ռ䣬�����
}
//--------------------------------------------------------------------------------
//���Ring_buf���ֽ���
unsigned int Ring_buf_byte(RING_BUF_T *pr)
{
	unsigned int head,tail,size;
	
	head=pr->head;
	tail=pr->tail;
	size=pr->size;
	return((tail+size-head) % size);
}
//--------------------------------------------------------------------------------
//���Ring_buf��ʣ���ֽ���
unsigned int Ring_buf_leftbyte(RING_BUF_T *pr)
{
	unsigned int i;
	
	i=Ring_buf_byte(pr);
	return(pr->size-i-1);
}
//--------------------------------------------------------------------------------
//��ѭ����������ȡ����
//pr��ѭ���ṹ��ָ��
//buf����ȡ����ָ��
//n��Ҫ��ȡ���ֽ���
//���أ�ʵ�ʶ�ȡ�ֽ���
static unsigned int Ring_Buf_RC(RING_BUF_T *pr,unsigned char *buf,unsigned int n,unsigned char b_rd)
{
	unsigned int head,tail,head_end,rn,size;
	
	if(n==0)
	{
		return(0);
	}
	
	head=pr->head;
	tail=pr->tail;
	size=pr->size;
	
		
	//rn=(tail-head)%size;		//���Ŀǰ���յ����ֽ���
	head_end=size-head;		//��ͷ�������ֽ���
	rn=(tail+head_end)%size;		//���Ŀǰ���յ����ֽ���
		
	if(rn>0)
	{						//������
		if(rn > n)
		{					//���������ڽ��յ�����n��ֻ����n��
			rn=n;
			tail=(head+rn)%size;		//���¼���β������ֹ�������
		}
		
		if(tail > head)
		{							//β��ͷ����������������ֱ�ӿ���
			memcpy(buf,pr->buffer+head,rn);
		}
		else
		{							//β��ͷС��û��ͷβ���״̬
			//head_end=size-head;		//��ͷ�������ֽ���
			memcpy(buf,pr->buffer+head,head_end);	//����ͷ�������ֽ�
			memcpy(buf+head_end,pr->buffer,rn-head_end);	//����ʣ�µ��ֽ�
			
		}
		
		if(b_rd)
		{
			pr->head=(head + rn) % size;
		}
	}
	return(rn);
}

//-------------------------------------------------------------------------------
unsigned int Ring_Buf_Copy(RING_BUF_T *pr,unsigned char *buf,unsigned int n)
{
	return(Ring_Buf_RC(pr,buf,n,0));
}
//-------------------------------------------------------------------------------
unsigned int Ring_Buf_Rd(RING_BUF_T *pr,unsigned char *buf,unsigned int n)
{
	return(Ring_Buf_RC(pr,buf,n,1));
}
//-------------------------------------------------------------------------------
unsigned int Ring_Buf_Erase(RING_BUF_T *pr,unsigned short n)
{
	unsigned int head,tail,rn,size;
	head=pr->head;
	tail=pr->tail;
	size=pr->size;
	
	rn=(tail+size-head)%size;		//���Ŀǰ���յ����ֽ���
	
	if(n > rn)
	{
		n=rn;
	}
	
	pr->head=(head + n) % size;
	return(n);
}
//----------------------------------------------------------------------------------
//��ջ�����
void Ring_Buf_Clr(RING_BUF_T *pr)
{
	pr->head=0;
	pr->tail=0;
	pr->buffer[0]=0;
	//memset(pr->buffer,0,pr->size);
}
//----------------------------------------------------------------------------------
void Ring_Buf_Init(RING_BUF_T *pr,unsigned int size)
{
	pr->buffer=DataPool_Get(size);
	pr->size=size;
	pr->head=0;
	pr->tail=0;
}
