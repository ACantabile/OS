#include "ring_buf.h"
#include "string.h"
#include "utility.h"

//----------------------------------------------------------------------
//版本号管理
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
	
	
	
	//left=(head-tail)%size;	//求剩余的空间
	tail_end = size - tail;						//求尾到最后的空间
	left=(head + tail_end)%size;	//求剩余的空间
	
	if(left==0)
	{
		left=size;		//缓冲区空
	}
	
	left--;			//区别缓冲区空与缓冲区满，实际空间减少一个
	
	if(n > left)
	{
		n=left;
	}
		
	if(n>0)
	{
		if(head > tail)
		{
			memcpy(pr->buffer+tail,buf,n);			//头大于尾，剩余空间在连续区域
		}
		else
		{
			//tail_end = size - tail;						//求尾到最后的空间
			if(tail_end>=n)
			{
				memcpy(pr->buffer+tail,buf,n);		//比n大，这段连续空间直接拷贝
			}
			else
			{
				memcpy(pr->buffer+tail,buf,tail_end);		//先拷贝尾到最后的字节数
				memcpy(pr->buffer,buf+tail_end,n-tail_end);	//再拷贝剩下的字节数
			}
		}
		
		pr->tail=(tail + n) % size;		//调整尾指针
	}

	//返回拷贝的字节数
	return(n);
}
//--------------------------------------------------------------------------------
//写循环器，如果写入字节数多余剩下的空间则覆盖最早的内容
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
	
	//left=(head-tail)%size;	//求剩余的空间
	tail_end= size - tail;						//求尾到最后的空间
	left=(head + tail_end)%size;	//求剩余的空间
	
	if(left==0)
	{
		left=size;		//缓冲区空
	}
	
	left--;			//区别缓冲区空与缓冲区满，实际空间减少一个
	
	if(n >= left)
	{
		b_over=1;	//输入字节数大于剩下的字节数，需要调整head
	}
	
	if(n >= size)	//输入的字节数大于整个空间，取最后的size-1部分
	{
		buf=buf+(n-(size-1));	//指向最后(size-1)大小的位置
		n=size-1;				//数量调整为size-1
	}
	
	//tail_end=size-tail;						//求尾到最后的空间
	
	if(tail_end >= n)
	{
		memcpy(pr->buffer+tail,buf,n);		//比n大或相等，这段连续空间直接拷贝
	}
	else
	{
		memcpy(pr->buffer+tail,buf,tail_end);		//先拷贝尾到最后的字节数
		memcpy(pr->buffer,buf+tail_end,n-tail_end);	//再拷贝剩下的字节数
	}
	
	pr->tail=(tail+n) % size;
	if(b_over)
	{
		pr->head=(pr->tail+1) % size;			//超出空间，设最大
	}
}
*/
//-----------------------------------------------------------------------------
//写循环器，如果写入字节数多余剩下的空间则移出最早的内容，把新内容放在最后，最后一个字节填0
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
	{					//如果要写入的内容大于缓冲区，取要写入内容的后面一部分
		buf+=(n-size);
		n=size;
	}
	
	left=size-tail;	//求剩余的空间
	
	if(left>=n)
	{
		memcpy(pr->buffer+tail,buf,n);		//比n大或相等，这段连续空间直接拷贝
		tail+=n;
	}
	else
	{					//剩余空间比要写入的少
		i=n-left;		//求出少几个
		pd=pr->buffer;	//把缓冲区里面最早的几个去掉
		ps=pd+i;
		i=tail-i;		//求出要移位的数量
		for(;i>0;i--)	//数据搬家
		{
			*pd++=*ps++;
		}
		memcpy(pd,buf,n);//拷贝新数据
		tail=size;
	}
	
	pr->buffer[tail]=0;
	pr->tail=tail;
	pr->head=0;			//超出空间，设最大
}
//--------------------------------------------------------------------------------
//检查Ring_buf的字节数
unsigned int Ring_buf_byte(RING_BUF_T *pr)
{
	unsigned int head,tail,size;
	
	head=pr->head;
	tail=pr->tail;
	size=pr->size;
	return((tail+size-head) % size);
}
//--------------------------------------------------------------------------------
//检查Ring_buf的剩余字节数
unsigned int Ring_buf_leftbyte(RING_BUF_T *pr)
{
	unsigned int i;
	
	i=Ring_buf_byte(pr);
	return(pr->size-i-1);
}
//--------------------------------------------------------------------------------
//从循环缓冲区读取数据
//pr：循环结构体指针
//buf：读取数据指针
//n：要读取的字节数
//返回：实际读取字节数
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
	
		
	//rn=(tail-head)%size;		//求出目前接收到的字节数
	head_end=size-head;		//求头到最后的字节数
	rn=(tail+head_end)%size;		//求出目前接收到的字节数
		
	if(rn>0)
	{						//有数据
		if(rn > n)
		{					//数据量大于接收的数据n，只接收n个
			rn=n;
			tail=(head+rn)%size;		//重新计算尾部！防止后面出错
		}
		
		if(tail > head)
		{							//尾比头大，数据在连续区，直接拷贝
			memcpy(buf,pr->buffer+head,rn);
		}
		else
		{							//尾比头小，没有头尾相等状态
			//head_end=size-head;		//求头到最后的字节数
			memcpy(buf,pr->buffer+head,head_end);	//拷贝头到最后的字节
			memcpy(buf+head_end,pr->buffer,rn-head_end);	//拷贝剩下的字节
			
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
	
	rn=(tail+size-head)%size;		//求出目前接收到的字节数
	
	if(n > rn)
	{
		n=rn;
	}
	
	pr->head=(head + n) % size;
	return(n);
}
//----------------------------------------------------------------------------------
//清空缓冲器
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
