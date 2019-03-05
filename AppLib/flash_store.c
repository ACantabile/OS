#include "flash_store.h"
#include "iap.h"
#include "string.h"
//----------------------------------------------------------------------
//版本号管理
#define FLASH_STORE_VER_C 200

	#if FLASH_STORE_VER_C != FLASH_STORE_VER_H
		#ERROR
	#endif
//----------------------------------------------------------------------
//对flash存储的结构体进行初始化

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
			;					//数据块的长度必须是short对齐
		}
	}
	//--------------------------------------------------------
	//读芯片ID识别页大小
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
	p->page_str_adr=page_str_adr;		//存储初始地址
	p->store_size=store_size;			//存储区块大小
	p->page_size=page_size;				//每页字节数
	p->page_n=store_size/page_size;		//一共多少页
	p->id_n=id_n;						//一共多少种数据块
	p->cur_page=0;						//当前可写的页
	p->next_ofs=0;						//当前可写的偏移量
	p->serial=0;						//当前序列号
	
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
//计算chksum
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
//检测存储区块的校验和合法性
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
	if(id>=p->id_n)return(-1);	//id非法
	pdi=p->pdi+id;
	data_size=pdi->data_size;	//数据块字节数
	
	next_ofs=cur_ofs+sizeof(FLASH_STORE_HEAD_T)+data_size;
	if(next_ofs > p->page_size)return(-1);	//溢出错误
	
	pch=(unsigned char *)phd+sizeof(FLASH_STORE_HEAD_T);	//指向数据块
	sum=calc_short_sum(pch,data_size,FLASH_STOER_CHKSUM_ORG);	//计算数据块的SUM
	
	if(sum==phd->sum)
	{
		pdi->ofs =cur_ofs;
		p->next_ofs = next_ofs;
		return(0);
	}
	else
	{
		return(-1);		//校验和错误
	}
}
//---------------------------------------------------------------------------------
//从flash读取数据块
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
//初始化的时候搜索到最后一组有效的区块
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
		{								//找到第1个序列号最大的页				
			pserial=(FLASH_STORE_PAGE_SERIAL_T *)(p->page_str_adr + i*p->page_size);	//得到页头序列号的指针
			if(*pserial == 0xffffffff)
			{
				continue;	//该页序列号不合法继续找
			}
			
			if(flash_store_chksum(p,i,FLASH_STORE_DATAORG_OFS)==0)
			{				//该页第1号序列号合法
				if(*pserial >= serial_max)
				{
					serial_max=*pserial;		//刷新序列号最大值
					cur_page=i;
				}
			}
		}
		//---------------------------------------------------------------------
		if(serial_max==0)return(-1);	//没有找到合法的页，退出
		//---------------------------------------------------------------------
		//寻找这一页中是否含有所有合法的数据模块
		cur_ofs=FLASH_STORE_DATAORG_OFS;
		p->serial=serial_max;
		p->cur_page=cur_page;
		
		pdi=p->pdi;
		for(i=0;i<p->id_n;i++)
		{
			pdi->ofs=0;	//清除所有数据索引
			pdi++;
		}
		
		while(1)
		{
			if(flash_store_chksum(p,cur_page,cur_ofs)==0)
			{												//检测到合法数据块
				cur_ofs=p->next_ofs;
			}
			else
			{
				break;				//检测到非法数据块或溢出页
			}
		}
		
		pdi=p->pdi;
		for(i=0;i<p->id_n;i++)
		{
			if(pdi->ofs==0)
			{
				break;				//所有的ID没有搜全
			}
			else
			{
				flash_store_rd_id(p,i);	//读取内容
			}
			pdi++;
		}
		if(i != p->id_n)
		{
			IAP_Flash_Erase_Page(p->page_str_adr+p->cur_page*p->page_size);	//擦除该页,继续搜索
			continue;
		}
		
		return(0);						//正确退出
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
	pdi->ofs=p->next_ofs;	//记录本次存储的偏移量
	adr=p->page_str_adr + p->cur_page*p->page_size + p->next_ofs;
		
	p->next_ofs+=data_size + sizeof(FLASH_STORE_HEAD_T);	//刷新下一个偏移量
	
	hd.id=id;
	hd.sum=calc_short_sum(buf,data_size,FLASH_STOER_CHKSUM_ORG);	//放置chksum
	IAP_Flash_Write(adr, sizeof(FLASH_STORE_HEAD_T),(unsigned char *)&hd,IAP_FLASH_ERASEPAGE);		//写入头部
	IAP_Flash_Write(adr + sizeof(FLASH_STORE_HEAD_T),data_size,buf,IAP_FLASH_ERASEPAGE);			//写入数据结构体
}

//数据结构写入flash
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
	{	//需要写入新页
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
	{	//写新页，需要写入新的序列号与全部变量
		if(++p->cur_page >= p->page_n)
		{
			p->cur_page=0;
		}
		adr=p->page_str_adr+p->cur_page*p->page_size;
		p->serial++;
		IAP_Flash_Write(adr, sizeof(FLASH_STORE_PAGE_SERIAL_T),(unsigned char *)&p->serial,IAP_FLASH_ERASEPAGE);	//写入serial
		p->next_ofs=FLASH_STORE_DATAORG_OFS;
		
		for(i=0;i<p->id_n;i++)
		{
			flash_store_wr_nochk(p,i);	//写入全部
		}
	}
	else
	{
		flash_store_wr_nochk(p,id);	//写入一个
	}
	//--------------------------------------------------------------------------------------------	
	return(0);
}
//==============================================================================================================================================

//==============================================================================================================================================
