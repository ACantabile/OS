#include "flash_store.h"
#include "iap.h"
#include "string.h"
//----------------------------------------------------------------------
//�汾�Ź���
#define FLASH_STORE_VER_C 200

	#if FLASH_STORE_VER_C != FLASH_STORE_VER_H
		#ERROR
	#endif
//----------------------------------------------------------------------
//��flash�洢�Ľṹ����г�ʼ��

int flash_store_init(FLASH_STORE_T *p,unsigned int page_str_adr,unsigned int store_size,FLASH_STORE_DATA_INDEX_T *pdi,unsigned char id_n)
{
	unsigned short page_size;
	unsigned int i;
	
	p->pdi=pdi;
	//-------------------------------------------------------
	for(i=0;i<id_n;i++)
	{
		while((pdi->data_size & 0x01) || (pdi->buf==NULL))
		{
			;					//���ݿ�ĳ��ȱ�����short����
		}
	}
	//--------------------------------------------------------
	//��оƬIDʶ��ҳ��С
	page_size=*((unsigned short*)FLASH_SIZE_ADR);
	
	if(page_size>128)
	{
		page_size=0x800;
	}
	else
	{
		page_size=0x400;
	}
	//--------------------------------------------------------
	p->page_str_adr=page_str_adr;		//�洢��ʼ��ַ
	p->store_size=store_size;			//�洢�����С
	p->page_size=page_size;				//ÿҳ�ֽ���
	p->page_n=store_size/page_size;		//һ������ҳ
	p->id_n=id_n;						//һ�����������ݿ�
	p->cur_page=0;						//��ǰ��д��ҳ
	p->next_ofs=0;						//��ǰ��д��ƫ����
	p->serial=0;						//��ǰ���к�
	
	if(flash_store_search_index(p)==-1)
	{	
		return(FLASH_STORE_INIT_NODATA);
	}
	else
	{
		return(FLASH_STORE_INIT_SUCCESS);
	}
}

//---------------------------------------------------------------------------------
//����chksum
unsigned short calc_short_sum(unsigned char *p,unsigned short n,unsigned short org)
{
	unsigned short i;
	unsigned char j;
	unsigned char add_sum,xor_sum;

	add_sum=org;
	xor_sum=org >> 8;

	for(i=0;i<n;i++)
	{
		j=*p++;
		add_sum += j;
		xor_sum ^= j;
	}
	return(((unsigned short)add_sum << 8 ) | xor_sum);
}

//---------------------------------------------------------------------------------
//���洢�����У��ͺϷ���
int flash_store_chksum(FLASH_STORE_T *p,unsigned int cur_page,unsigned int cur_ofs)
{
	unsigned char *pch;
	unsigned short sum;
	unsigned short data_size;
	unsigned short id;

	FLASH_STORE_HEAD_T *phd;
	FLASH_STORE_DATA_INDEX_T *pdi;
	
	unsigned int next_ofs;
	
	
	if(cur_ofs >= p->page_size)return(-1);
	phd=(FLASH_STORE_HEAD_T *)(p->page_str_adr+cur_page*p->page_size+cur_ofs);
	
	id=phd->id;
	if(id>=p->id_n)return(-1);	//id�Ƿ�
	pdi=p->pdi+id;
	data_size=pdi->data_size;	//���ݿ��ֽ���
	
	next_ofs=cur_ofs+sizeof(FLASH_STORE_HEAD_T)+data_size;
	if(next_ofs > p->page_size)return(-1);	//�������
	
	pch=(unsigned char *)phd+sizeof(FLASH_STORE_HEAD_T);	//ָ�����ݿ�
	sum=calc_short_sum(pch,data_size,FLASH_STOER_CHKSUM_ORG);	//�������ݿ��SUM
	
	if(sum==phd->sum)
	{
		pdi->ofs =cur_ofs;
		p->next_ofs = next_ofs;
		return(0);
	}
	else
	{
		return(-1);		//У��ʹ���
	}
}
//---------------------------------------------------------------------------------
//��flash��ȡ���ݿ�
void flash_store_rd_id(FLASH_STORE_T *p,unsigned short id)
{
	unsigned int adr,data_size;
	unsigned char *buf;
	FLASH_STORE_DATA_INDEX_T *pdi;
	
	pdi=p->pdi+id;
	
	data_size=pdi->data_size;
	buf=pdi->buf;
	
	adr=p->page_str_adr+p->cur_page*p->page_size+pdi->ofs + sizeof(FLASH_STORE_HEAD_T);
	memcpy(buf,(void *)adr,data_size);
}
	



//-----------------------------------------------------------------------------------
//��ʼ����ʱ�����������һ����Ч������
int flash_store_search_index(FLASH_STORE_T *p)
{
	unsigned int i;
	unsigned int cur_page;
	unsigned int cur_ofs;
	unsigned int serial_max=0;
	
	FLASH_STORE_PAGE_SERIAL_T *pserial;
	FLASH_STORE_DATA_INDEX_T *pdi;
	while(1)
	{
		for(i=0;i<p->page_n;i++)
		{								//�ҵ���1�����к�����ҳ				
			pserial=(FLASH_STORE_PAGE_SERIAL_T *)(p->page_str_adr + i*p->page_size);	//�õ�ҳͷ���кŵ�ָ��
			if(*pserial == 0xffffffff)
			{
				continue;	//��ҳ���кŲ��Ϸ�������
			}
			
			if(flash_store_chksum(p,i,FLASH_STORE_DATAORG_OFS)==0)
			{				//��ҳ��1�����кźϷ�
				if(*pserial >= serial_max)
				{
					serial_max=*pserial;		//ˢ�����к����ֵ
					cur_page=i;
				}
			}
		}
		//---------------------------------------------------------------------
		if(serial_max==0)return(-1);	//û���ҵ��Ϸ���ҳ���˳�
		//---------------------------------------------------------------------
		//Ѱ����һҳ���Ƿ������кϷ�������ģ��
		cur_ofs=FLASH_STORE_DATAORG_OFS;
		p->serial=serial_max;
		p->cur_page=cur_page;
		
		pdi=p->pdi;
		for(i=0;i<p->id_n;i++)
		{
			pdi->ofs=0;	//���������������
			pdi++;
		}
		
		while(1)
		{
			if(flash_store_chksum(p,cur_page,cur_ofs)==0)
			{												//��⵽�Ϸ����ݿ�
				cur_ofs=p->next_ofs;
			}
			else
			{
				break;				//��⵽�Ƿ����ݿ�����ҳ
			}
		}
		
		pdi=p->pdi;
		for(i=0;i<p->id_n;i++)
		{
			if(pdi->ofs==0)
			{
				break;				//���е�IDû����ȫ
			}
			else
			{
				flash_store_rd_id(p,i);	//��ȡ����
			}
			pdi++;
		}
		if(i != p->id_n)
		{
			IAP_Flash_Erase_Page(p->page_str_adr+p->cur_page*p->page_size);	//������ҳ,��������
			continue;
		}
		
		return(0);						//��ȷ�˳�
	}
	//---------------------------------------------------------------------
}
//==============================================================================================================================================
void flash_store_wr_nochk(FLASH_STORE_T *p,unsigned short id)
{
	FLASH_STORE_HEAD_T hd;
	FLASH_STORE_DATA_INDEX_T *pdi;
	unsigned char *buf;
	unsigned int data_size;
	unsigned int adr;
	
	pdi=p->pdi+id;
	data_size=pdi->data_size;
	buf=pdi->buf;
	pdi->ofs=p->next_ofs;	//��¼���δ洢��ƫ����
	adr=p->page_str_adr + p->cur_page*p->page_size + p->next_ofs;
		
	p->next_ofs+=data_size + sizeof(FLASH_STORE_HEAD_T);	//ˢ����һ��ƫ����
	
	hd.id=id;
	hd.sum=calc_short_sum(buf,data_size,FLASH_STOER_CHKSUM_ORG);	//����chksum
	IAP_Flash_Write(adr, sizeof(FLASH_STORE_HEAD_T),(unsigned char *)&hd,IAP_FLASH_ERASEPAGE);		//д��ͷ��
	IAP_Flash_Write(adr + sizeof(FLASH_STORE_HEAD_T),data_size,buf,IAP_FLASH_ERASEPAGE);			//д�����ݽṹ��
}

//���ݽṹд��flash
int flash_store_wr(FLASH_STORE_T *p,unsigned short id)
{
	unsigned int adr;
	unsigned short *ps;
	unsigned int i,hd_data_size;
	unsigned char b_newpage;
	FLASH_STORE_DATA_INDEX_T *pdi;
	
	pdi=p->pdi;
	
	
	if(id >= p->id_n)return(-1);
	
	hd_data_size=pdi->data_size + sizeof(FLASH_STORE_HEAD_T);
	
	b_newpage=0;
	adr=p->page_str_adr+p->cur_page*p->page_size+p->next_ofs;
	
	if((p->next_ofs + hd_data_size) > p->page_size)
	{	//��Ҫд����ҳ
		b_newpage=1;
	}
	else
	{
		ps=(unsigned short *)adr;
		for(i=hd_data_size/2;i>0;i--)
		{
			if(*ps != 0xffff)
			{
				break;
			}
		}
		if(i!=0)
		{
			b_newpage=1;
		}
	}
	
	if(b_newpage)
	{	//д��ҳ����Ҫд���µ����к���ȫ������
		if(++p->cur_page >= p->page_n)
		{
			p->cur_page=0;
		}
		adr=p->page_str_adr+p->cur_page*p->page_size;
		p->serial++;
		IAP_Flash_Write(adr, sizeof(FLASH_STORE_PAGE_SERIAL_T),(unsigned char *)&p->serial,IAP_FLASH_ERASEPAGE);	//д��serial
		p->next_ofs=FLASH_STORE_DATAORG_OFS;
		
		for(i=0;i<p->id_n;i++)
		{
			flash_store_wr_nochk(p,i);	//д��ȫ��
		}
	}
	else
	{
		flash_store_wr_nochk(p,id);	//д��һ��
	}
	//--------------------------------------------------------------------------------------------	
	return(0);
}
//==============================================================================================================================================

//==============================================================================================================================================
