/****************************Copyright (c)**********************************************                                     

**------------- �ļ���Ϣ ---------------------------------------------------------------
** �� �� ��: iap.c
** �� �� ��: �ܴ��� 	
** ��������: 2008��11��21��
** ��    ��: STM32��IAP����
**------------- �汾��Ϣ ---------------------------------------------------------------
** ��    ��: V1.0	 
** ��    ��: 2008��11��12��	
** ��    ��: ����汾��Ϣ
**--------------------------------------------------------------------------------------
***************************************************************************************/	   
#include "iap.h"

//----------------------------------------------------------------------
//�汾�Ź���
#define IAP_VER_C 101

	#if IAP_VER_C != IAP_VER_H
		#ERROR
	#endif
//----------------------------------------------------------------------



//#define Flash_Page_Size 0x400

#define Flash_Base_Adr 0x08000000

/*
SR : ����״̬�Ĵ���
	0: BUSY  ����״̬(1Ϊ��æ,0Ϊ����)
	2: PGERR ��̴���(1Ϊ����,0Ϊ�޴�),д1���״̬
	4: W_ERR д����  (1Ϊ����,0Ϊ�޴�),д1���״̬
	5: EOP   ��������(1Ϊ���)д1���״̬

CR : ������ƼĴ���
	0: PG    ���  (1=ѡ���̲���)
	1: PER   ҳ����(1=ѡ��ҳ��������)
	2: MER   ȫ����(ѡ����������û�ҳ)
	4: OPTPG ��дѡ���ֽ�(��ѡ���ֽڱ��)
	5: OPTER ����ѡ���ֽ�(����ѡ���ֽ�/С��Ϣ��)
	6: STRT  ��ʼ����(����λΪ��1��ʱ������һ�β�����������λֻ���������Ϊ��1������BSY��Ϊ��1��ʱ��Ϊ��0��)
	7: LOCK  ����(ֻ��д��1���� ����λΪ��1��ʱ��ʾFPEC��FLASH_CR����ס���ڼ�⵽��ȷ�Ľ�
             �����к�Ӳ�������λΪ��0������һ�β��ɹ��Ľ����������´θ�λǰ����
             λ�����ٱ��ı�)
	9: OPTWRE  ����дѡ���ֽ�(����λΪ��1��ʱ�������ѡ���ֽ�/С��Ϣ����б�̲���������
             FLASH_OPTKEYR�Ĵ���д����ȷ�ļ����к󣬸�λ����Ϊ��1��������������λ��)
	10:ERRIE �������״̬�ж�(��λ�����ڷ���FPEC����ʱ�����ж�(��FLASH_SR�Ĵ����е�
             PGERR/WRPRTERR��Ϊ��1��ʱ)��(0����ֹ�����ж� / 1����������ж�)
	12:EOPIE �����������ж�(��λ������FLASH_SR�Ĵ����е�EOPλ��Ϊ��1��ʱ�����жϡ�
             (0����ֹ�����ж� / 1����������ж�)
*/

/***************************************************************************************
** ��������: IAP_FlashWrite
** ��������: IAP������д����
** ��    ��: w_data		/ д������ָ��
** ��    ��: w_length	/ д�����ݳ���
** ��    ��: w_addr		/ д�����ݵ�ַ
** �� �� ֵ: д����(TRUE / FALSE)
** ��    ע: ������ʽд��,Ҫ���ַ�ͳ��ȶ���1024�ֽڶ���       
****************************************************************************************/
/*
uint32 IAP_FlashWrite(uint8* w_data, uint32 w_length, uint32 w_addr)
{
	uint32 sta_addr, end_addr;
	uint32 FlashDes;
	uint32 now_addr;  		 

	sta_addr = (w_addr & 0x0801FC00);			// 1K�ֽڶ���
	end_addr = (sta_addr + w_length) & 0x0801FC00;			
				  
	if((w_data == NULL) || (sta_addr < 0x08000000) || (sta_addr == end_addr))
	{
		return FALSE;
	}
	
	NVIC_SETPRIMASK();			// ���ж� 

	FLASH->KEYR = IAP_FLASH_KEY1;				// ����FLASH��д����������
	FLASH->KEYR = IAP_FLASH_KEY2;

	while(sta_addr < end_addr)
	{										   
		while(FLASH->SR & IAP_SR_BSY);			// ���æλ
		
		FLASH->CR |= CR_PER_Set;						  		   
		FLASH->AR  = sta_addr;					// ѡ��Ҫ������ҳ
	    FLASH->CR |= CR_STRT_Set;
		while(FLASH->SR & IAP_SR_BSY);			// ���æλ	
	    FLASH->CR &= CR_PER_Reset;		  
		while(FLASH->SR & IAP_SR_BSY);			// ���æλ	

		// �����в�� 

		now_addr = sta_addr + 0x400;			// �������ַ������1K 	 
		
	    FLASH->CR |= CR_PG_Set;
		for(FlashDes=sta_addr;FlashDes<now_addr;FlashDes+=2,w_data+=2)	// ������дѭ��
		{  
			*(vu16*)FlashDes = *(u16*)w_data;		// ���ַд������			   
			while(FLASH->SR & IAP_SR_BSY);			// ���æλ
			if(*(u16*)FlashDes != *(u16*)w_data)	// ����д�����
			{	
				FLASH->CR &= CR_PG_Reset;		
				FLASH->CR |= CR_LOCK_Set;			// ����FLASH��д����������	  
				NVIC_RESETPRIMASK();	 		
				return FALSE;			
			}
		}	

		sta_addr += 0x400;						// ��д��ַ��1024
	}

	FLASH->CR |= CR_LOCK_Set;					// ����FLASH��д����������
	
    NVIC_RESETPRIMASK();	 	//���ж� 

	return TRUE; 	
}			
*/
unsigned int IAP_Flash_Write(unsigned int adr, unsigned int n ,u8* p,u8 b_page_erase)
{
	unsigned int sta_addr, end_addr;
	unsigned int page_mask;

	u16 i;
	unsigned int end_adr_max;

	i=*((u16*)FLASH_SIZE_ADR);
	end_adr_max=i*1024 + Flash_Base_Adr;
	if(i>128)
	{
		page_mask=0x7ff;
	}
	else
	{
		page_mask=0x3ff;
	}


	sta_addr = adr;			// 1K�ֽڶ���
	end_addr = sta_addr + n;			
				  
	if((sta_addr < Flash_Base_Adr) || (end_addr >= end_adr_max) || (sta_addr & 0x01) || (n & 0x01))
	{
		return IAP_FLASH_FAIL;
	}
	
	__disable_irq();	//���ж�
	
	FLASH->KEYR = IAP_FLASH_KEY1;				// ����FLASH��д����������
	FLASH->KEYR = IAP_FLASH_KEY2;

	__enable_irq();		//���ж�
	
	while(sta_addr < end_addr)
	{										   
		while(FLASH->SR & IAP_SR_BSY)
		{
			__nop();			// ���æλ
		}
		
		//if(b_first || (sta_addr & (Flash_Page_Size-1)) ==0)
		if(b_page_erase && (sta_addr & page_mask) ==0)
		{
			FLASH->CR &= CR_PG_Reset;		//�¼���
			FLASH->CR |= CR_PER_Set;						  		   
			
			//FLASH->AR  = sta_addr & (0x0801FC00);					// ѡ��Ҫ������ҳ
			FLASH->AR  = sta_addr;					// ѡ��Ҫ������ҳ
		    FLASH->CR |= CR_STRT_Set;
			while(FLASH->CR & CR_STRT_Set){;}			//�ȴ�STRTλ���0���¼��룩
			while(FLASH->SR & IAP_SR_BSY){;}			// ���æλ	
		    FLASH->CR &= CR_PER_Reset;		  
			while(FLASH->SR & IAP_SR_BSY){;}			// ���æλ	
	
			//�����в��
		}
	 	
		if(*(u16*)sta_addr != *(u16*)p)
		{
			FLASH->CR |= CR_PG_Set;
			*(vu16*)sta_addr = *(u16*)p;		// ���ַд������			   
			
			while(FLASH->SR & IAP_SR_BSY)
			{
				;			// ���æλ
			}

			if(*(u16*)sta_addr != *(u16*)p)	// ����д�����
			{	
				FLASH->CR &= CR_PG_Reset;		
				FLASH->CR |= CR_LOCK_Set;			// ����FLASH��д����������	  
					 		
				return IAP_FLASH_FAIL;			
			}
		}
		sta_addr +=2;
		p+=2;
	}

	FLASH->CR &= CR_PG_Reset;
	FLASH->CR |= CR_LOCK_Set;					// ����FLASH��д����������
	
    //NVIC_RESETPRIMASK();	 	// ���ж� 

	return (IAP_FLASH_SUCCESS); 	
}	

/***************************************************************************************/
//V101����ҳ����
void IAP_Flash_Erase_Page(unsigned int adr)
{
	__asm("cpsid i");
	FLASH->KEYR = IAP_FLASH_KEY1;				// ����FLASH��д����������
	FLASH->KEYR = IAP_FLASH_KEY2;
	__asm("cpsie i");
	
	FLASH->CR &= CR_PG_Reset;					//�¼���
	FLASH->CR |= CR_PER_Set;						  		   
	
	FLASH->AR  = adr;							// ѡ��Ҫ������ҳ
	FLASH->CR |= CR_STRT_Set;
	while(FLASH->CR & CR_STRT_Set){;}			//�ȴ�STRTλ���0���¼��룩
	while(FLASH->SR & IAP_SR_BSY){;}			// ���æλ	
	FLASH->CR &= CR_PER_Reset;		  
	while(FLASH->SR & IAP_SR_BSY){;}			// ���æλ	

	//�����в��
		
	FLASH->CR &= CR_PG_Reset;					// ����FLASH��д����������
	FLASH->CR |= CR_LOCK_Set;
}
/***************************************************************************************
** ��������: IAP_ReadPro
** ��������: ���óɶ�����(��ֹ���ԺͷǷ�����)
** ��    ��: RDP��
** �� �� ֵ: ��  	  
** ��    ע: ��ɺ�λоƬ  
****************************************************************************************/
/*
void IAP_ReadPro(uint16 m_rdp)
{
	NVIC_SETPRIMASK(); 			// ���ж�	

	FLASH->KEYR     = IAP_FLASH_KEY1;		// ����FLASH��д����������
	FLASH->KEYR     = IAP_FLASH_KEY2;	 		   
	while(FLASH->SR & IAP_SR_BSY);			// ���æλ
									
    FLASH->OPTKEYR  = IAP_FLASH_KEY1;		// ������Ϣ��
    FLASH->OPTKEYR  = IAP_FLASH_KEY2;   
	while(FLASH->SR & IAP_SR_BSY);			// ���æλ
	
    FLASH->CR      |= CR_OPTWRE_Set;
    FLASH->CR      |= CR_OPTPG_Set; 	  
	OB->RDP         = m_rdp; 		  		// ��������Ĵ���д�����
	while(FLASH->SR & IAP_SR_BSY);			// ���æλ	  
	OB->WRP0        = 0x0;					// ֵΪ0:д����;
	while(FLASH->SR & IAP_SR_BSY);			// ���æλ
	OB->WRP1        = 0x0;					// ֵΪ1:�ޱ���;
	while(FLASH->SR & IAP_SR_BSY);			// ���æλ
	OB->WRP2        = 0x0;					// Ĭ��Ϊ1
	while(FLASH->SR & IAP_SR_BSY);			// ���æλ
//	OB->WRP3        = 0x0;		
//	while(FLASH->SR & IAP_SR_BSY);			// ���æλ
	 
    FLASH->CR      &= CR_OPTWRE_Reset;
	FLASH->CR      |= CR_LOCK_Set;			// ����FLASH��д����������
	
    NVIC_RESETPRIMASK();	 	// ���ж�
}
*/		  
/****************************************************************************************
**  End Of File                                                     
****************************************************************************************/
