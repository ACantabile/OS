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
//����ַ�Ƿ��ޣ����������Ƿ񳬹���������С
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
		return(EASYBUS_ERR_WR_SEG);		//д���Խ��
	}
	
	if(rd_seg_n > RD_SEG_N)
	{
		return(EASYBUS_ERR_RD_SEG);		//��ȡ��Խ��
	}
	
	for(i=0;i<wr_seg_n;i++)
	{
		wr_adr=pk->wr[i].adr;
		wr_n=pk->wr[i].n;
		
		if((wr_adr+wr_n) > p->hr_size)
		{
			return(EASYBUS_ERR_HRADR);		//д��������г���HR��ַ��Χ
		}
		bytes+=sizeof(EASYBUF_ADR_T);		//�������ֽ���
		if(p->b_master)
		{
			bytes += wr_n*2;				//��ģʽ���ֽ�������Ҫд��ĳ���
		}
	}
	
	for(i=0;i<rd_seg_n;i++)
	{
		rd_adr=pk->rd[i].adr;
		rd_n=pk->rd[i].n;
		
		if((rd_adr+rd_n) > p->hr_size)
		{
			return(EASYBUS_ERR_HRADR);		//��ȡ�����г���HR��ַ��Χ
		}
		
		bytes+=sizeof(EASYBUF_ADR_T);
		if(!p->b_master)
		{
			bytes += rd_n*2;				//��ģʽ���ֽ�������Ҫ��ȡ�ĳ���
		}
	}
	
	if( bytes > p->pk.buf_size)
	{
		return(EASYBUS_ERR_BUFSIZE);	//�������ͻ�������Χ
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
	
	
	i=easybus_send_chk_adr_buf(p);		//���HR��ַ��д�뻺������û��Խ��
	
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
	
	buf=short_wr_buf(buf,func);			//��������
	
	for(i=0;i<rd_seg_n;i++)				//���ض�ȡ������
	{
		buf=short_wr_buf(buf,pk->rd[i].adr);
		buf=short_wr_buf(buf,pk->rd[i].n);
	}
	for(i=0;i<wr_seg_n;i++)				//����д�������
	{
		buf=short_wr_buf(buf,pk->wr[i].adr);
		buf=short_wr_buf(buf,pk->wr[i].n);
	}
	//----------------------------------------------------------------------------------
	//��ģʽ
	if(p->b_master)
	{	
		for(i=0;i<wr_seg_n;i++)				//����д�������
		{
			bytes=pk->wr[i].n*2;
			memcpy(buf,&p->hr[pk->wr[i].adr],bytes);
			buf+=bytes;
		}
	}
	else
	//----------------------------------------------------------------------------------
	//��ģʽ
	{	
		for(i=0;i<rd_seg_n;i++)		//�ӻ�����������ȡ��д
		{
			bytes=pk->rd[i].n*2;
			memcpy(buf,&p->hr[pk->rd[i].adr],bytes);
			buf+=bytes;
		}
	}
	//-----------------------------------------------------
	i=buf-pk->buf;	//�������ֽ���
	chksum=easybus_chksum(pk->buf,i);
	*buf++=chksum;
	*buf++=chksum >> 8;
	i+=2;
	pk->buf_n=i;
	return(EASYBUS_SUCCESS);
}
//===============================================================================
//���պ���
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
		return(EASYBUS_ERR_REC_N);		//�ܳ���̫��
	}
	
	if(*buf != p->src_adr)
	{
		return(EASYBUS_ERR_NOTMYDATA);	//�Ǳ�����ַ
	}
	
	i=pk->buf_n-2;							//��λ��chksumλ��
	chksum=short_rd_buf(buf+pk->buf_n-2);	//����һ�����ݵ�У��
	
	if(chksum != easybus_chksum(pk->buf,i))
	{
		return(EASYBUS_ERR_REC_CHKSUM);		//chksum����
	}
	//---------------------------------------------------
	pk->des_adr=*(buf+EASYBUS_OFS_SRCADR);		//�ѶԷ���Դ��ַ���ص�Ŀ���ַ
	//func=*(buf+EASYBUS_OFS_FUNC);				//�õ�������
	func=short_rd_buf(buf+EASYBUS_OFS_FUNC);	//�õ�������
	wr_seg_n=func & EASYBUS_FUNC_WR_SEG_MASK;	//�õ�д�����
	rd_seg_n=(func >> EASYBUS_FUNC_RD_SEG_SHIFT) & EASYBUS_FUNC_RD_SEG_MASK;	//�õ���ȡ����
	
	buf+=EASYBUS_OFS_RD_INDEX;
	j=sizeof(EASYBUF_ADR_T);
	for(i=0;i<rd_seg_n;i++)		//��ȡ��ȡ������
	{
		memcpy(&rd[i],buf,j);
		buf+=j;					//bufָ�����
	}
	for(i=0;i<wr_seg_n;i++)		//��ȡд�������
	{
		memcpy(&wr[i],buf,j);
		buf+=j;					//bufָ�����
	}
	//-------------------------------------------------------------------------
	if(p->b_master)
	{										//��ģʽ
		if(!(func & EASYBUS_FUNC_SLV))
		{
			return(EASYBUS_ERR_FUNC);
		}
		if((wr_seg_n != pk->wr_seg_n) || (rd_seg_n != pk->rd_seg_n))
		{
			return(EASYBUS_ERR_MASTER_SEG_N);		//��ģʽ��д���ȡ����������
		}
		bytes=EASYBUS_BASE_LEN;
		for(i=0;i<wr_seg_n;i++)
		{
			if((wr[i].adr != pk->wr[i].adr) || (wr[i].n != pk->wr[i].n))
			{
				return(EASYBUS_ERR_WR_SEG);			//��ģʽ��д������ƥ��
			}
			bytes+=sizeof(EASYBUF_ADR_T);
		}
		for(i=0;i<rd_seg_n;i++)
		{
			if((rd[i].adr != pk->rd[i].adr) || (rd[i].n != pk->rd[i].n))
			{
				return(EASYBUS_ERR_RD_SEG);			//��ģʽ�¶�������ƥ��
			}
			bytes+=sizeof(EASYBUF_ADR_T);
			bytes+=rd[i].n*2;
		}
		if(bytes != pk->buf_n)
		{
			return(EASYBUS_ERR_REC_N);				//�ֽ�����ƥ��
		}
		for(i=0;i<rd_seg_n;i++)						//��ȷ����ȡ��������
		{
			j=rd[i].n*2;
			memcpy(&p->hr[rd[i].adr],buf,j);
			buf+=j;
		}
	}
	else
	//-----------------------------------------------------------
	//��ģʽ����
	{
		if(func & EASYBUS_FUNC_SLV)
		{
			return(EASYBUS_ERR_FUNC);		//�������
		}
		
		if((wr_seg_n > WR_SEG_N) || (rd_seg_n > RD_SEG_N))
		{
			return(EASYBUS_ERR_MASTER_SEG_N);		//��ģʽ��д���ȡ����������
		}
		bytes=EASYBUS_BASE_LEN;
		for(i=0;i<rd_seg_n;i++)
		{
			if((rd[i].adr + rd[i].n) > p->hr_size)
			{
				return(EASYBUS_ERR_RD_SEG);			//��ģʽ��ȡ��������
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
				return(EASYBUS_ERR_WR_SEG);		//��ģʽд����������
			}
			pk->wr[i]=wr[i];
			bytes+=sizeof(EASYBUF_ADR_T);
			bytes+=wr[i].n*2;
		}
		pk->wr_seg_n=wr_seg_n;
		
		if(bytes != pk->buf_n)
		{
			return(EASYBUS_ERR_REC_N);			//��ģʽ���ֽ�������
		}
		//-----------------------------------------
		//��ʽ��ȷ��д���������
		for(i=0;i<wr_seg_n;i++)
		{
			j=wr[i].n*2;
			memcpy(&p->hr[wr[i].adr],buf,j);
			buf+=j;
		}
		//-----------------------------------------
		pk->func=func;							//��������
	}//��ģʽ
	//---------------------------------------------------------
	//__enable_irq();
	return(EASYBUS_SUCCESS);
}
