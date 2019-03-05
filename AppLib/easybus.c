#define EASYBUS_MAIN
#include "utility.h"
#include "easybus.h"
#include "string.h"

#define EASYBUS_VER_C 101
//*****************************************************************************


unsigned short easybus_chksum(unsigned char *buf,unsigned short n)
{
	unsigned char xor_sum=0xff;
	unsigned char add_sum=0xff;
	int i;
	unsigned char ch;
	
	for(i=0;i<n;i++)
	{
		ch = *buf++;
		xor_sum ^= ch;
		add_sum += ch;
	}
	return(((unsigned short)add_sum<<8)+xor_sum);
}
//===============================================================================
//检测地址是否超限，发送内容是否超过缓冲区大小
int easybus_send_chk_adr_buf(EASYBUS_T *p)
{
	unsigned short rd_adr;
	unsigned short rd_n;
	unsigned short wr_adr;
	unsigned short wr_n;
	
	unsigned char rd_seg_n;
	unsigned char wr_seg_n;
	unsigned short func;
	int i,bytes=EASYBUS_BASE_LEN;
	EASTBUS_PACK_T *pk;
	
	pk=&p->pk;
	
	func=pk->func;
	wr_seg_n= func & EASYBUS_FUNC_WR_SEG_MASK;
	rd_seg_n= (func >> EASYBUS_FUNC_RD_SEG_SHIFT) & EASYBUS_FUNC_RD_SEG_MASK;
	
	if(wr_seg_n > WR_SEG_N)
	{
		return(EASYBUS_ERR_WR_SEG);		//写入段越限
	}
	
	if(rd_seg_n > RD_SEG_N)
	{
		return(EASYBUS_ERR_RD_SEG);		//读取段越限
	}
	
	for(i=0;i<wr_seg_n;i++)
	{
		wr_adr=pk->wr[i].adr;
		wr_n=pk->wr[i].n;
		
		if((wr_adr+wr_n) > p->hr_size)
		{
			return(EASYBUS_ERR_HRADR);		//写入段里面有超出HR地址范围
		}
		bytes+=sizeof(EASYBUF_ADR_T);		//计算总字节数
		if(p->b_master)
		{
			bytes += wr_n*2;				//主模式，字节数加上要写入的长度
		}
	}
	
	for(i=0;i<rd_seg_n;i++)
	{
		rd_adr=pk->rd[i].adr;
		rd_n=pk->rd[i].n;
		
		if((rd_adr+rd_n) > p->hr_size)
		{
			return(EASYBUS_ERR_HRADR);		//读取段里有超出HR地址范围
		}
		
		bytes+=sizeof(EASYBUF_ADR_T);
		if(!p->b_master)
		{
			bytes += rd_n*2;				//从模式，字节数加上要读取的长度
		}
	}
	
	if( bytes > p->pk.buf_size)
	{
		return(EASYBUS_ERR_BUFSIZE);	//超出发送缓冲区范围
	}
	return(EASYBUS_SUCCESS);
}
//-------------------------------
int easybus_set_buf(EASYBUS_T *p)
{
	unsigned short func;
	unsigned char wr_seg_n;
	unsigned char rd_seg_n;
	unsigned short chksum;
	unsigned short bytes;
	
	int i;
	unsigned char *buf;
	EASTBUS_PACK_T *pk;
	
	
	i=easybus_send_chk_adr_buf(p);		//检查HR地址与写入缓冲区有没有越限
	
	if(i!=EASYBUS_SUCCESS)
	{
		return(i);
	}
	
	pk=&p->pk;
	
	wr_seg_n=pk->wr_seg_n;
	rd_seg_n=pk->rd_seg_n;
	
	func=(rd_seg_n << EASYBUS_FUNC_RD_SEG_SHIFT)+wr_seg_n;
	if(!p->b_master)
	{
		func |= EASYBUS_FUNC_SLV;
	}
	buf=pk->buf;
	
	*buf++=pk->des_adr;
	*buf++=p->src_adr;
	
	buf=short_wr_buf(buf,func);			//加载命令
	
	for(i=0;i<rd_seg_n;i++)				//加载读取段索引
	{
		buf=short_wr_buf(buf,pk->rd[i].adr);
		buf=short_wr_buf(buf,pk->rd[i].n);
	}
	for(i=0;i<wr_seg_n;i++)				//加载写入段索引
	{
		buf=short_wr_buf(buf,pk->wr[i].adr);
		buf=short_wr_buf(buf,pk->wr[i].n);
	}
	//----------------------------------------------------------------------------------
	//主模式
	if(p->b_master)
	{	
		for(i=0;i<wr_seg_n;i++)				//加载写入段内容
		{
			bytes=pk->wr[i].n*2;
			memcpy(buf,&p->hr[pk->wr[i].adr],bytes);
			buf+=bytes;
		}
	}
	else
	//----------------------------------------------------------------------------------
	//从模式
	{	
		for(i=0;i<rd_seg_n;i++)		//从机，按主机读取的写
		{
			bytes=pk->rd[i].n*2;
			memcpy(buf,&p->hr[pk->rd[i].adr],bytes);
			buf+=bytes;
		}
	}
	//-----------------------------------------------------
	i=buf-pk->buf;	//求命令字节数
	chksum=easybus_chksum(pk->buf,i);
	*buf++=chksum;
	*buf++=chksum >> 8;
	i+=2;
	pk->buf_n=i;
	return(EASYBUS_SUCCESS);
}
//===============================================================================
//接收函数
//unsigned char test;
int easybus_receive(EASYBUS_T *p)
{
	unsigned short func;
	unsigned short chksum;
	unsigned short bytes;
	
	unsigned char wr_seg_n;
	unsigned char rd_seg_n;
	EASYBUF_ADR_T wr[WR_SEG_N];
	EASYBUF_ADR_T rd[RD_SEG_N];
	
	int i,j;
	unsigned char *buf;
	EASTBUS_PACK_T *pk;
	
	pk=&p->pk;
	buf=pk->buf;
	
	if(pk->buf_n < EASYBUS_BASE_LEN)
	{
		return(EASYBUS_ERR_REC_N);		//总长度太短
	}
	
	if(*buf != p->src_adr)
	{
		return(EASYBUS_ERR_NOTMYDATA);	//非本机地址
	}
	
	i=pk->buf_n-2;							//定位到chksum位置
	chksum=short_rd_buf(buf+pk->buf_n-2);	//计算一包数据的校验
	
	if(chksum != easybus_chksum(pk->buf,i))
	{
		return(EASYBUS_ERR_REC_CHKSUM);		//chksum错误
	}
	//---------------------------------------------------
	pk->des_adr=*(buf+EASYBUS_OFS_SRCADR);		//把对方的源地址加载到目标地址
	//func=*(buf+EASYBUS_OFS_FUNC);				//得到命令码
	func=short_rd_buf(buf+EASYBUS_OFS_FUNC);	//得到命令码
	wr_seg_n=func & EASYBUS_FUNC_WR_SEG_MASK;	//得到写入段数
	rd_seg_n=(func >> EASYBUS_FUNC_RD_SEG_SHIFT) & EASYBUS_FUNC_RD_SEG_MASK;	//得到读取段数
	
	buf+=EASYBUS_OFS_RD_INDEX;
	j=sizeof(EASYBUF_ADR_T);
	for(i=0;i<rd_seg_n;i++)		//获取读取段索引
	{
		memcpy(&rd[i],buf,j);
		buf+=j;					//buf指针调节
	}
	for(i=0;i<wr_seg_n;i++)		//获取写入段索引
	{
		memcpy(&wr[i],buf,j);
		buf+=j;					//buf指针调节
	}
	//-------------------------------------------------------------------------
	if(p->b_master)
	{										//主模式
		if(!(func & EASYBUS_FUNC_SLV))
		{
			return(EASYBUS_ERR_FUNC);
		}
		if((wr_seg_n != pk->wr_seg_n) || (rd_seg_n != pk->rd_seg_n))
		{
			return(EASYBUS_ERR_MASTER_SEG_N);		//主模式下写入读取段数量错误
		}
		bytes=EASYBUS_BASE_LEN;
		for(i=0;i<wr_seg_n;i++)
		{
			if((wr[i].adr != pk->wr[i].adr) || (wr[i].n != pk->wr[i].n))
			{
				return(EASYBUS_ERR_WR_SEG);			//主模式下写索引不匹配
			}
			bytes+=sizeof(EASYBUF_ADR_T);
		}
		for(i=0;i<rd_seg_n;i++)
		{
			if((rd[i].adr != pk->rd[i].adr) || (rd[i].n != pk->rd[i].n))
			{
				return(EASYBUS_ERR_RD_SEG);			//主模式下读索引不匹配
			}
			bytes+=sizeof(EASYBUF_ADR_T);
			bytes+=rd[i].n*2;
		}
		if(bytes != pk->buf_n)
		{
			return(EASYBUS_ERR_REC_N);				//字节数不匹配
		}
		for(i=0;i<rd_seg_n;i++)						//正确，读取各段内容
		{
			j=rd[i].n*2;
			memcpy(&p->hr[rd[i].adr],buf,j);
			buf+=j;
		}
	}
	else
	//-----------------------------------------------------------
	//从模式接收
	{
		if(func & EASYBUS_FUNC_SLV)
		{
			return(EASYBUS_ERR_FUNC);		//命令错误
		}
		
		if((wr_seg_n > WR_SEG_N) || (rd_seg_n > RD_SEG_N))
		{
			return(EASYBUS_ERR_MASTER_SEG_N);		//从模式下写入读取段数量错误
		}
		bytes=EASYBUS_BASE_LEN;
		for(i=0;i<rd_seg_n;i++)
		{
			if((rd[i].adr + rd[i].n) > p->hr_size)
			{
				return(EASYBUS_ERR_RD_SEG);			//从模式读取索引错误
			}
			pk->rd[i]=rd[i];
			bytes+=sizeof(EASYBUF_ADR_T);
		}
		pk->rd_seg_n=rd_seg_n;
		
		/*
		__disable_irq();
		
		if(wr_seg_n>2)
		{
			test++;
		}
		*/
		
		for(i=0;i<wr_seg_n;i++)
		{
			if(( wr[i].adr + wr[i].n ) > p->hr_size)
			{
				return(EASYBUS_ERR_WR_SEG);		//从模式写入索引错误
			}
			pk->wr[i]=wr[i];
			bytes+=sizeof(EASYBUF_ADR_T);
			bytes+=wr[i].n*2;
		}
		pk->wr_seg_n=wr_seg_n;
		
		if(bytes != pk->buf_n)
		{
			return(EASYBUS_ERR_REC_N);			//从模式下字节数错误
		}
		//-----------------------------------------
		//格式正确，写入各段内容
		for(i=0;i<wr_seg_n;i++)
		{
			j=wr[i].n*2;
			memcpy(&p->hr[wr[i].adr],buf,j);
			buf+=j;
		}
		//-----------------------------------------
		pk->func=func;							//保存命令
	}//从模式
	//---------------------------------------------------------
	//__enable_irq();
	return(EASYBUS_SUCCESS);
}
