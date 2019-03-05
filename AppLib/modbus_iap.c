#include "modbus.h"
#include "stm32f10x_hw_def.h"
#include "f:\arm\arm_ccsec\security\security.h"
#include "iap.h"
#include "string.h"

#define MODBUS_ID_ADR 60000
#define MODBUS_ID_N	6		//6��short
#define STM32F_ID_ADR 0x1ffff7e8 

#include "CONFIG.H"


//#define STM32F_PSD_ADRBASE 0x08000000
//#define STM32F_PSD_ADR (STM32F_PSD_ADRBASE-0x400 + (MODBUS_CONST.flash_size_k * 0x400))

//�������0x3f0�����1K
const unsigned long STM32F_PSD[MODBUS_ID_N/2] __attribute__((section(".ARM.__at_0x80003f0")))=
	{0xffffffff,0xffffffff,0xffffffff};
#define STM32F_PSD_ADR ((unsigned long)STM32F_PSD)


#ifdef ENCRYPT_EN
	u8 b_sec_ok,b_sec_done;
#endif



void modbus_init(u16 hr_n,u8 slv_adr)
{
	MODBUS_CONST.hr_nb=hr_n;
	MODBUS_CONST.slv=slv_adr;
}

//�����ֽںϳ�һ������
unsigned short char_hl_short(unsigned char hi,unsigned lo)
{
	union{
	unsigned char ch[2];
	unsigned short s;
	}u;
	u.ch[0]=lo;
	u.ch[1]=hi;
	return(u.s);
}

//����һ���ֵĸߡ����ֽ�
unsigned short short_xch_hl(unsigned short i)
{
	union{
	unsigned char ch[2];
	unsigned short s;
	}u;
	u.ch[0]=(i>>8) & 0xff;
	u.ch[1]=i & 0xff;
	return(u.s);
}
//----------------------------------------------------------
void short_copy_xch(void *t,void *s,int n,u8 b_xch)
{
	int i;
	unsigned short *pt,*ps;
	
	pt=(unsigned short *)t;
	ps=(unsigned short *)s;
	if(n>0)
	{
	    if(!b_xch)
	    {
	        for(i=0;i<n;i++)
	        {
	            *pt=*ps;
	            pt++;
	            ps++;
			}
		}
	    else
	    {
	        for(i=0;i<n;i++)
	        {
	            *pt=short_xch_hl(*ps);
	            pt++;
	            ps++;
			}	         
		}
	}
}
//---------------------------------------------------------------------------


unsigned short modbus_crc(void *puchMsg, unsigned short usDataLen)
{
	return(modbus_crc_org(0xffff,puchMsg,usDataLen));	
}

unsigned short modbus_crc_org(unsigned short CRC_ORG,unsigned char *p,unsigned short length)
{
	unsigned char j;
	unsigned short reg_crc;

	reg_crc=short_xch_hl(CRC_ORG);
	
	do	{
		reg_crc^=*p++;
		for(j=0;j<8;j++)
		{
			if(reg_crc&0x01)
			{
				reg_crc=(reg_crc>>1)^0xa001;
			}
			else
			{
				reg_crc=reg_crc>>1;
			}
		}
	}while(--length!=0);
		
	return(reg_crc);
}

//==========================================================================

//���һ�����ݵĺϷ���,�����SLV������
//rb:ԭʼ���ݻ�����,nͨѶ��ȡ����������
int modbus_chk_format(unsigned char *rb,int n)
{
	//unsigned char slv;
	union{
	unsigned short ui;
	unsigned char ch[2];
	}crc;

	unsigned short crc1;

	
	//slv=rb[0];		//�ӻ���ַ
	
	if(n<5)
	{
		return(MODBUS_ERR_BYTELESS);		//�ֽ���̫��
	}
	//if(slv!=MODBUS_CONST.slv)
	//{
	//	return(MODBUS_ERR_SLV);		//�ӻ���ַ��
	//}
	crc.ch[0]=rb[n-2];
	crc.ch[1]=rb[n-1];
	crc1=modbus_crc(rb,n-2);
	if(crc.ui!=crc1)
	{
		return(MODBUS_ERR_CRC);		//crc��
	}
	
	return(MODBUS_FORMAT_OK);
}
//==============================================================================

//==============================================================================
//Modbus �ӻ�����
//p:ָ���ж˿ڽṹ���ָ�룬phr��ָ���Ӧ��HR��ָ��
//����1����ʾ����һ�����ݳɹ�
unsigned short task_modbus_receive(struct ST_USART *p,unsigned short *phr)
{
	u8 func,byte_count,b_broadcast,slv,b_f0;
	u16 da_adr,da_n,wr_adr,wr_n;
#ifdef ENCRYPT_EN
	u8 b_rw_sec =0;
	u16 psd_buf[MODBUS_ID_N];
	u32 l,i,*psd;
#endif
	
	u16 n,rsp_n,hr_nb,adr_end1;
	union{
		unsigned char ch[2];
		unsigned short ui;
		}crc;
	
	u8 b_hr_ok,b_hr_bc_ok;	


	unsigned char *rb;

	
	rsp_n=0;
	//if(TST_BIT(p->sta,B_EV_USART_RX))	//�ж��Ƿ��н��ձ�־
	if(tst_bit(&p->sta,B_EV_USART_RX))	//�ж��Ƿ��н��ձ�־
	{
		//CLR_BIT(p->sta,B_EV_USART_RX);	//����ձ�־
		clr_bit(&p->sta,B_EV_USART_RX);	//����ձ�־
		n=p->r_e;
		p->r_e=0;
		rb=&p->tr_buf[0];
		
		b_f0=0;

		if(modbus_chk_format(rb,n)==MODBUS_FORMAT_OK)
		{	
			slv=rb[0];
			if(slv==0)
			{
				b_broadcast=1;
				b_f0=1;
			}
			else
			{
				b_broadcast=0;
				if(slv==MODBUS_CONST.slv)
				{
					b_f0=1;
				}
			}
		}

		if(b_f0)
		{	
			func=rb[1];		//����
			da_adr=char_hl_short(rb[2],rb[3]);
			da_n=char_hl_short(rb[4],rb[5]);
			byte_count=rb[6];	//�ֽڼ���
			
			adr_end1=da_adr+da_n;
			hr_nb=MODBUS_CONST.hr_nb;
			
			
			//b_hr_ok=(da_n<128 && adr_end1<=hr_nb);
			//b_hr_bc_ok=(byte_count==da_n*2);
			
			if(da_n>127)
			{
				b_hr_bc_ok=0;
			}
			
			b_hr_ok=(adr_end1<=hr_nb);
			b_hr_bc_ok=1;
			
			
			switch(func)
			{
			
				case MD_RD_COIL:	//��ȡ��Ȧ
					
				    break;
				case MD_FR_MCOIL:	//ǿ�ƶ����Ȧ
					
					break;
			    case MD_FR_MHR_RDHR:	//ǿ�Ʋ���ȡ
					if(b_hr_ok)
					{
						wr_adr=char_hl_short(rb[6],rb[7]);
						wr_n=char_hl_short(rb[8],rb[9]);
						byte_count=rb[10];	//�ֽڼ���;
						//if(wr_n<128 && (wr_adr+wr_n)<=hr_nb && byte_count==wr_n*2)	//V200
						if((wr_adr+wr_n)<=hr_nb)
						{
							short_copy_xch(phr+wr_adr,&rb[11],wr_n,TRUE);
							rb[2]=da_n*2;
							short_copy_xch(&rb[3],phr+da_adr,da_n,TRUE);
							rsp_n=rb[2]+3;
						}
					}
					break;
			    case MD_RD_HR:		//��ȡHR
					if(da_adr==MODBUS_ID_ADR && da_n==MODBUS_ID_N)
					{											  //��ȡID
#ifdef ENCRYPT_EN
						b_rw_sec=1;
#endif
						rb[2]=da_n*2;
						//short_copy_xch(void *t,void *s,int n,bool b_xch)
						short_copy_xch(&rb[3],(u8 *)STM32F_ID_ADR,da_n,TRUE);
						rsp_n=rb[2]+3;
					}
					else if(da_adr==9688 && da_n==2)
					{
						rb[2]=da_n*2;
						
						crc.ui=VER;
						rb[3]=crc.ch[1];
						rb[4]=crc.ch[0];
						
						rb[5]=0;
						rb[6]=0;
						
						rsp_n=rb[2]+3;
					}
					else if(b_hr_ok)
					{											  //������ȡ
						rb[2]=da_n*2;
						//short_copy_xch(void *t,void *s,int n,bool b_xch)
						short_copy_xch(&rb[3],phr+da_adr,da_n,TRUE);
						rsp_n=rb[2]+3;
					}
			        break;
			    case MD_FR_SCOIL:	//ǿ�Ƶ�����Ȧ
					
			        break;
			    case MD_FR_SHR:		//ǿ�Ƶ���HR
					if(n==8 && da_adr<hr_nb)
					{
						*(phr+da_adr)=da_n;
						rsp_n=6;
					}
			        break;
			    case MD_FR_MHR:		//ǿ�ƶ��HR
#ifdef ENCRYPT_EN
					if(da_adr==MODBUS_ID_ADR && da_n==MODBUS_ID_N)
					{											  //����д��ID
						b_rw_sec=1;
						short_copy_xch(psd_buf,&rb[7],da_n,TRUE);
						IAP_Flash_Write(STM32F_PSD_ADR,MODBUS_ID_N*2,(u8*)psd_buf,IAP_Flash_NotErasePage);
						
						psd=(u32 *)STM32F_PSD_ADR;

						for(i=0;i<2;i++)
						{
							if(psd[0]==0xffffffff && psd[1]==0xffffffff && psd[2]==0xffffffff)
							{
								//IAP_Flash_Write(STM32F_PSD_ADR,MODBUS_ID_N*2,(u8*)psd_buf,IAP_Flash_NotErasePage);
								IAP_Flash_Write((u32)psd,MODBUS_ID_N*2,(u8*)psd_buf,IAP_Flash_NotErasePage);
								b_modbus_entry_iap=1;
								break;
							}
							l=*((u16*)Flash_Size_Adr);
							l--;
							l*=0x400;
							psd=(u32 *)(l+Flash_Base_Adr);
						}
						b_sec_done=0;
						rsp_n=6;	

					}

					else if(da_adr==9688)
#else
					if(da_adr==9688)	//IAP����
#endif
					{
						rsp_n=6;
					}
					else if(b_hr_ok && b_hr_bc_ok)
					{
						short_copy_xch(phr+da_adr,&rb[7],da_n,TRUE);
						rsp_n=6;
					}
			        break;
			    default:
			    			//�����ʽ��
				    break;
			}


			if(rsp_n!=0)
			{
				if(!b_broadcast)
				{
#ifdef ENCRYPT_EN
					if(b_sec_ok || !b_sec_done || b_rw_sec)					
					{
						crc.ui=modbus_crc(rb,rsp_n);
						rb[rsp_n++]=crc.ch[0];
						rb[rsp_n++]=crc.ch[1];
						
						//usart_send_start(p,rsp_n);	//�ŵ�task_modbus_send����
					}
					else
					{
						rsp_n=0;
					}

					if(!b_sec_done)
					{
						b_sec_done=1;
						memcpy(psd_buf,(u8 *)STM32F_PSD_ADR,sizeof(psd_buf));
						decrypt(REMAINDER2,MODBUS_ID_N*2,(u8 *)psd_buf);
						if(memcmp((u8 *)psd_buf,(u8 *)STM32F_ID_ADR,MODBUS_ID_N*2)==0)
						{
							b_sec_ok=1;
						}
					}
#else
					crc.ui=modbus_crc(rb,rsp_n);
					rb[rsp_n++]=crc.ch[0];
					rb[rsp_n++]=crc.ch[1];
#endif
				}			   
				else
				{
					rsp_n=0;
				}
			}
		}//if(modbus_chk_format(rb,n)==MODBUS_FORMAT_OK)
		if(rsp_n==0)
		{
			set_bit((void *)(p->base+USART_CR1),B_USART_CR1_RE);	//û�з��ͣ��������
		}
	}//if(TST_BIT(p->sta,B_EV_USART_RX))	//�ж��Ƿ��н��ձ�־
	return(rsp_n);
}
void task_modbus_send(struct ST_USART *p,unsigned short *phr,unsigned short rsp_n)
{
	unsigned char *rb;
	union{
		unsigned char ch[2];
		unsigned short ui;
		}crc;

	if(rsp_n > 2)
	{
		rb=&p->tr_buf[0];
		rsp_n-=2;
		crc.ui=modbus_crc(rb,rsp_n);
		rb[rsp_n++]=crc.ch[0];
		rb[rsp_n++]=crc.ch[1];
		usart_send_start(p,rsp_n);
	}
}
//==============================================================================



