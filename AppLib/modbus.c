#include "modbus.h"
#include "utility.h"
#include "chksums.h"
#include "string.h"


//#define MODBUS_COIL_EN
#define MODBUS_HR_EN
#define MODBUS_RW_EN


//==============================================================================
void modbus_init(MODBUS_T *p,unsigned short *phr,unsigned short hr_n,MODBUS_HR_MODE_E  b_little_endian)
{
	memset(p,0,sizeof(MODBUS_T));
	p->phr=phr;
	p->hr_n=hr_n;
	p->b_xch=!b_little_endian;
}
//==============================================================================


//==============================================================================
//���ݸ���������ȡCoil����������������Ҫ���ݵ��ֽ���
unsigned short read_coil_buf(unsigned short *pt16,unsigned short *phr,	//��������ַ��HR�׵�ַ
							 unsigned short da_adr,unsigned short da_n)	//COIL��ַ��COIL����
{
	union{
		u32 *p32;
		u16 *p16;
		}ps;
	unsigned short i,j,k;

	//pt16=(u16 *)&rb[3];
	i=da_adr % 16;	//ȷ����λֵ
	j=(da_n+15)/16;	//HR����
	
	ps.p16=phr+(da_adr/16);
	for(k=0;k<j;k++)
	{
		*pt16=(*ps.p32 >> i);
		ps.p16++;
		pt16++;
	}
	i=da_n%16;
	if(i!=0)
	{
		pt16--;
		*pt16 &=((1<<i) -1);
	}

	return((da_n+7)/8);
}
//==============================================================================
//���ݲ����趨COIL
void set_hr_coil(unsigned short *phr,unsigned short *buf,	//HR�׵�ַ,��������ַ
				 unsigned short da_adr,unsigned short da_n)//COIL��ַ��COIL����
{
	union{
		u32 *p32;
		u16 *p16;
		}ps;
	unsigned short i,j,k,s_mask,e_mask;
	unsigned short *pt16;

	//ps.p16=(u16 *)&rb[7];
	ps.p16=buf;
	//pt16=&HR[da_adr/16];
	pt16=phr+da_adr/16;

	i=da_adr % 16;	//ȷ����λֵ
	j=(i+da_n+15)/16;	//HR����
	s_mask=0-(1<<i);	//��Чλ=1
	e_mask=(da_adr+da_n)%16;
	if(e_mask !=0)
	{
		e_mask=(1<<(e_mask))-1;  //��Чλ=1
	}
	else
	{
		e_mask=0xffff;
	}

	
	if(j==1)
	{	//HR��βһ��
		j=s_mask & e_mask;
		i=*ps.p16 << i;
		i &= j;
		*pt16 &= ~j;
		*pt16 |= i;
	}
	else
	{
		k=*ps.p16 << i;		//�趨��һ��HR
		//k &= s_mask;
		*pt16 &= ~s_mask;
		*pt16 |= k;

		pt16++;
		//ps.p16++;
		i=16-i;

		j--;
		for(k=1;k<j;k++)
		{
			*pt16=*ps.p32 >>i;
			pt16++;
			ps.p16++;
		}
		k= *ps.p32 >>i;
		k &= e_mask;
		*pt16 &= ~e_mask;
		*pt16 |= k;
	}	
}
//==============================================================================
//Modbus �ӻ�����
//p:ָ���ж˿ڽṹ���ָ�룬phr��ָ���Ӧ��HR��ָ��
//����1����ʾ����һ�����ݳɹ�
int modbus_slv_rec(MODBUS_T *p,unsigned char *rb,int n)
{
	unsigned char func;
	unsigned short da_adr,da_n,rww_adr,rww_n,crc;
	unsigned short hr_nb,adr_end1;
	unsigned char r;

#ifdef MODBUS_COIL_EN
	unsigned char b_coil_ok,,b_coil_bc_ok;
#endif
	unsigned char b_hr_ok,b_hr_bc_ok;	
	unsigned short *phr;
	
	phr=p->phr;

	if(n < MODBUS_SLV_BASE_NB)
	{
		return(MODBUS_ERR_BYTELESS);		//�ֽ���̫��
	}
	
	
	if((rb[0] != MODBUS_ADR_BOADCAST) && (rb[0] != p->slv) && (rb[0] != MODBUS_SLV_IAP))
	{
		return(MODBUS_ERR_SLV);
	}
	
	crc = ModBus_FastCRC(rb,n-2);
	
	if(crc != short_rd_buf(&rb[n-2]))
	{
		return(MODBUS_ERR_CRC);
	}
	
	func=rb[1];		//����
	da_adr=char_hl_short(rb[2],rb[3]);
	da_n=char_hl_short(rb[4],rb[5]);
	//byte_count=rb[6];	//�ֽڼ���
	
	adr_end1=da_adr+da_n;
	hr_nb=p->hr_n;
	
	
#ifdef MODBUS_COIL_EN
	b_coil_ok=((da_n < (256*8)) && (adr_end1<=hr_nb*16));
	b_coil_bc_ok=(byte_count == (da_n+7)/8);
#endif
	
	//b_hr_ok=(da_n<128 && adr_end1<=hr_nb);
	//b_hr_bc_ok=(byte_count==da_n*2);
	
	b_hr_ok=(adr_end1<=hr_nb);
	p->b_ext=!b_hr_ok;
	
	b_hr_bc_ok=1;
	
	p->func=func;
	p->da_adr=da_adr;
	p->da_n=da_n;
	p->rww_adr=char_hl_short(rb[6],rb[7]);
	p->rww_n=char_hl_short(rb[8],rb[9]);
	r=0;	
	switch(func)
	{
	//==========================================================================
	#ifdef MODBUS_COIL_EN
		case MD_RD_COIL:	//��ȡ��Ȧ
			if(b_coil_ok)
			{
				r=1;
			}
			break;
		case MD_FR_MCOIL:	//ǿ�ƶ����Ȧ
			if(b_coil_ok && b_coil_bc_ok)
			{
				set_hr_coil(phr,(u16 *)&rb[7],da_adr,da_n);
				r=1;
			}
			break;
		case MD_FR_SCOIL:	//ǿ�Ƶ�����Ȧ
			if(da_adr<hr_nb)
			{
				pt16=phr+(da_adr/16);
				i=1<<(da_adr %16);
				
				if(da_n==0xff00)
				{
					*pt16 |= i;
				}
				else if(da_n==0x0000)
				{
					*pt16 &= ~i;
				}
				else
				{
					break;
				}
				r=1;
			}
			break;
	#endif
	//==========================================================================		
	#ifdef MODBUS_RW_EN	
		case MD_FR_MHR_RDHR:	//ǿ�Ʋ���ȡ
			p->rww_adr=rww_adr=char_hl_short(rb[6],rb[7]);		//RWָ��޸�p->wr_adr��p->wr_n
			p->rww_n=rww_n=char_hl_short(rb[8],rb[9]);
			if(b_hr_ok)
			{
				//byte_count=rb[10];	//�ֽڼ���;
				//if(wr_n<128 && (wr_adr+wr_n)<=hr_nb && byte_count==wr_n*2)
				if((rww_adr+rww_n)<=hr_nb)
				{
					short_copy_xch(phr+rww_adr,&rb[11],rww_n,p->b_xch);
				}
			}
			r=1;
			break;
	#endif
	//==========================================================================		
	#ifdef MODBUS_HR_EN	
		case MD_RD_HR:		//��ȡHR
			r=1;			 //������ȡ
			break;
		case MD_FR_SHR:		//ǿ�Ƶ���HR
			if(n==8 && da_adr<hr_nb)
			{
				*(phr+da_adr)=da_n;
				r=1;
			}
			break;
		case MD_FR_MHR:		//ǿ�ƶ��HR
			if(b_hr_ok && b_hr_bc_ok)
			{	
				short_copy_xch(phr+da_adr,&rb[7],da_n,p->b_xch);
			}
			r=1;
			break;
	#endif
	//==========================================================================		
		default:
					//�����ʽ��
			break;
	}
	//==========================================================================
	if(!r)
	{
		p->func=0;
	}
	//==========================================================================
	r+=p->b_ext;
	return(r);
}
//==================================================================================================
int modbus_slv_send(MODBUS_T *p,unsigned char *rb)
{
	unsigned short rsp_n=0;
	unsigned short *phr,da_n,da_adr,crc;
	unsigned char b_ext;
	
	b_ext=p->b_ext;
	
	phr=p->phr;
	da_adr=p->da_adr;
	da_n=p->da_n;
	
	switch(p->func)
	{
	//==========================================================================
	#ifdef MODBUS_COIL_EN
		case MD_RD_COIL:	//��ȡ��Ȧ
			rb[2]=read_coil_buf((u16 *)&rb[3],phr,da_adr,da_n);
			rsp_n=rb[2]+3;
			break;
		case MD_FR_MCOIL:	//ǿ�ƶ����Ȧ
			rsp_n=6;
			break;
		case MD_FR_SCOIL:	//ǿ�Ƶ�����Ȧ
			rsp_n=6;
			break;
	#endif
	//==========================================================================		
	#ifdef MODBUS_RW_EN	
		case MD_FR_MHR_RDHR:	//ǿ�Ʋ���ȡ
	#endif
	#ifdef MODBUS_HR_EN	
		case MD_RD_HR:		//��ȡHR
	#endif
			if(!b_ext)
			{
				short_copy_xch(&rb[3],phr+da_adr,da_n,p->b_xch);
			}
			rb[2]=da_n*2;
			rsp_n=da_n*2+3;
			break;
	//==========================================================================		
		case MD_FR_SHR:		//ǿ�Ƶ���HR
		case MD_FR_MHR:		//ǿ�ƶ��HR
			rsp_n=6;
			break;
	//==========================================================================		
		default:
					//�����ʽ��
			break;
	//==========================================================================
	}

	if(rsp_n!=0)
	{
		if(rb[0]==MODBUS_ADR_BOADCAST)
		{			
			rsp_n=0;		//�㲥��������
		}			   
		else
		{
//			if(rb[0] !=MODBUS_SLV_IAP)
//			{
//				rb[0]=p->slv;
//			}
			rb[1]=p->func;
			crc=ModBus_FastCRC(rb,rsp_n);			//����CRC
			short_wr_buf(rb+rsp_n,crc);		//����CRC
			rsp_n+=2;		  
		}
	}
	return(rsp_n);
}

//**************************************************************************************************************//
int modbus_master_send(MODBUS_T *p,unsigned char *rb)
{
	unsigned short send_n=0;
	unsigned short *phr,da_n,da_adr,rww_adr,rww_n,crc;
	unsigned char b_ext;

	
	b_ext=p->b_ext;
		
	phr=p->phr;
	da_adr=p->da_adr;
	da_n=p->da_n;
	rww_adr=p->rww_adr;
	rww_n=p->rww_n;

	
	rb[0]=p->slv;
	rb[1]=p->func;
	rb[2]=da_adr >> 8;
	rb[3]=da_adr & 0xff;
	rb[4]=da_n >> 8;
	rb[5]=da_n & 0xff;
	
	
	switch(p->func)
	{
	//==========================================================================
	#ifdef MODBUS_COIL_EN
		case MD_RD_COIL:	//��ȡ��Ȧ
			send_n=6;
			break;
		case MD_FR_SCOIL:	//ǿ�Ƶ�����Ȧ
			rb[5]=0x00;
			i=*(phr+da_adr/16);
			if(i & (1<<da_adr))
			{
				rb[4]=0xff;
				p->da_n=0xff00;
			}
			else
			{
				rb[4]=0x00;
				p->da_n=0x0000;
			}
			send_n=6;
			break;
		case MD_FR_MCOIL:	//ǿ�ƶ����Ȧ
			if(b_coil_ok)
			{
				rb[6]=read_coil_buf((u16 *)&rb[7],phr,da_adr,da_n);	//COIL��ַ��COIL����
				send_n=rb[6]+7;
			}	
			break;
	#endif
	//==========================================================================		
	#ifdef MODBUS_RW_EN	
		case MD_FR_MHR_RDHR:	//ǿ�Ʋ���ȡ
			rb[6]=rww_adr>>8;
			rb[7]=rww_adr & 0xff;
			rb[8]=rww_n >> 8;
			rb[9]=rww_n;
			rb[10]=rww_n * 2;	//�ֽڼ���;
			send_n=11 + rww_n * 2;
			if(!b_ext)
			{
				short_copy_xch(&rb[11],phr+p->rww_adr,p->rww_n,p->b_xch);
			}
			break;
	#endif
	#ifdef MODBUS_HR_EN	
		case MD_RD_HR:		//��ȡHR
			send_n=6;
			break;
	#endif
		//==========================================================================		
		case MD_FR_SHR:		//ǿ�Ƶ���HR
			if(da_adr<p->hr_n)
			{
				da_n=*(phr+da_adr);
				rb[4]=da_n >> 8;
				rb[5]=da_n & 0xff;
				p->da_n=da_n;
				send_n=6;
			}
			break;
		case MD_FR_MHR:		//ǿ�ƶ��HR
			rb[6]=da_n*2;
			send_n=da_n*2+7;
			if(!b_ext)
			{
				short_copy_xch(&rb[7],(phr+da_adr),da_n,p->b_xch);
			}
			break;
	//==========================================================================		
		default:
					//�����ʽ��
			break;
	//==========================================================================
	}

	if(send_n!=0)
	{
		rb[0]=p->slv;
		rb[1]=p->func;
		crc=ModBus_FastCRC(rb,send_n);			//����CRC
		short_wr_buf(rb+send_n,crc);		//����CRC
		send_n+=2;		  
	}
	return(send_n);
}
//==============================================================================
//Modbus �ӻ�����
//p:ָ���ж˿ڽṹ���ָ�룬phr��ָ���Ӧ��HR��ָ��
//����1����ʾ����һ�����ݳɹ�
int modbus_master_rec(MODBUS_T *p,unsigned char *rb,int n)
{
	unsigned char func;
	unsigned short crc;

	unsigned char r;

#ifdef MODBUS_COIL_EN
	unsigned char b_coil_ok,,b_coil_bc_ok;
#endif
	unsigned char b_hr_match;	
	unsigned short *phr;
	
	phr=p->phr;

	if(n < MODBUS_SLV_BASE_NB)
	{
		return(MODBUS_ERR_BYTELESS);		//�ֽ���̫��
	}
	
	
	if(rb[0] != p->slv)
	{
		return(MODBUS_ERR_SLV);
	}
	
	crc = ModBus_FastCRC(rb,n-2);
	
	if(crc != short_rd_buf(&rb[n-2]))
	{
		return(MODBUS_ERR_CRC);
	}
	
	func=rb[1];		//����
	
	if(p->func != func)
	{
		return(MODBUS_ERR_FUNC);
	}
	
	
	b_hr_match=(rb[2] == ((p->da_n*2) & 0xff));
	
	
	
	r=MODBUS_FAIL;	
	
	
#ifdef MODBUS_COIL_EN
	b_coil_ok=((da_n < (256*8)) && (adr_end1<=hr_nb*16));
	b_coil_bc_ok=(byte_count == (da_n+7)/8);
#endif
	
	
	switch(func)
	{
	//==========================================================================
	#ifdef MODBUS_COIL_EN
		case MD_RD_COIL:	//��ȡ��Ȧ
//			if(b_coil_ok && b_coil_bc_ok)
//			{
//				set_hr_coil(phr,(u16 *)&rb[3],da_adr,da_n);
//				r=MODBUS_SUCCESS;
//			}
			break;
		case MD_FR_MCOIL:	//ǿ�ƶ����Ȧ
//			if(b_coil_ok)
//			{
//				r=MODBUS_SUCCESS;
//			}
			break;
		case MD_FR_SCOIL:	//ǿ�Ƶ�����Ȧ
			/*
			if(da_adr<hr_nb)
			{
				pt16=phr+(da_adr/16);
				i=1<<(da_adr %16);
				
				if(da_n==0xff00)
				{
					*pt16 |= i;
				}
				else if(da_n==0x0000)
				{
					*pt16 &= ~i;
				}
				else
				{
					break;
				}
				r=1;
			}
			*/
			break;
	#endif
	//==========================================================================		
	#ifdef MODBUS_RW_EN	
		case MD_FR_MHR_RDHR:	//ǿ�Ʋ���ȡ
	#endif	
	#ifdef MODBUS_HR_EN	
		case MD_RD_HR:		//��ȡHR
	#endif
			if(b_hr_match)
			{
				if(!p->b_ext)
				{
					short_copy_xch(phr+p->da_adr,&rb[3],p->da_n,p->b_xch);
				}
				r=MODBUS_SUCCESS;
			}
			break;
	//==========================================================================		
	#ifdef MODBUS_HR_EN	
		case MD_FR_SHR:		//ǿ�Ƶ���HR
			r=MODBUS_SUCCESS;
			break;
		case MD_FR_MHR:		//ǿ�ƶ��HR
			r=MODBUS_SUCCESS;
			break;
	#endif
	//==========================================================================		
		default:
					//�����ʽ��
			break;
	}
	//==========================================================================
	return(r);
}

	

//****************************************************************************************************//
