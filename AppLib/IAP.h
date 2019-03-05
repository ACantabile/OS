/****************************Copyright (c)**********************************************                                     
**
**                     ƽ�������Զ����豸���޹�˾
**                        http://www.rlplc.com                         
**
**------------- �ļ���Ϣ ---------------------------------------------------------------
** �� �� ��: iap.h
** �� �� ��: �ܴ��� 	
** ��������: 2008��11��21��
** ��    ��: STM32��IAP����
**------------- �汾��Ϣ ---------------------------------------------------------------
** ��    ��: V1.0	 
** ��    ��: 2008��11��12��	
** ��    ��: ����汾��Ϣ
**--------------------------------------------------------------------------------------
***************************************************************************************/
#ifndef __IAP_H
#define __IAP_H
/**********************************************************/
//�汾�Ź���
#define IAP_VER_H 101
/*
V101 ����ҳ��������
***********************************************************/

#include "stm32f10x.h"

#define FLASH_SIZE_ADR 0x1ffff7e0
					  
#define IAP_RDP_Key			((unsigned short)0x00A5)	 	
#define IAP_FLASH_KEY1		((unsigned int)0x45670123)
#define IAP_FLASH_KEY2		((unsigned int)0xCDEF89AB)
#define IAP_PAGE_ZISE		0x800				// ������С

#define IAP_SR_BSY			0x0001				// SR �Ŀ���λ
#define IAP_SR_EOP			0x0020				// EOP �������

#define IAP_CR_PG			0x0001				// ���  (1=ѡ���̲���)
#define IAP_CR_PER			0x0002				// ҳ����(1=ѡ��ҳ��������)	
#define IAP_CR_MER			0x0008				// ȫ����(ѡ����������û�ҳ)
#define IAP_CR_OPTPG		0x0010				// ��дѡ���ֽ�(��ѡ���ֽڱ��)
#define IAP_CR_OPTER		0x0020				// ����ѡ���ֽ�(����ѡ���ֽ�/С��Ϣ��)
#define IAP_CR_STRT			0x0040				// ��ʼ����
#define IAP_CR_LOCK			0x0080				// ����(ֻ��д��1��)
#define IAP_CR_OPTWRE		0x0200				// ����дѡ���ֽ�
#define IAP_CR_ERRIE		0x0400				// �������״̬�ж�
#define IAP_CR_EOPIE		0x1000				// �����������ж�	
																	
#define CR_PG_Set           ((unsigned int)0x00000001)
#define CR_PG_Reset         ((unsigned int)0x00001FFE)
#define CR_PER_Set          ((unsigned int)0x00000002)
#define CR_PER_Reset        ((unsigned int)0x00001FFD)
#define CR_MER_Set          ((unsigned int)0x00000004)
#define CR_MER_Reset        ((unsigned int)0x00001FFB)
#define CR_OPTPG_Set        ((unsigned int)0x00000010)
#define CR_OPTPG_Reset      ((unsigned int)0x00001FEF)
#define CR_OPTER_Set        ((unsigned int)0x00000020)
#define CR_OPTER_Reset      ((unsigned int)0x00001FDF)
#define CR_STRT_Set         ((unsigned int)0x00000040)							 
#define CR_LOCK_Set         ((unsigned int)0x00000080) 
#define CR_OPTWRE_Set		((unsigned int)0x00000200)	// ����дѡ���ֽ�
#define CR_OPTWRE_Reset		((unsigned int)0x00001DFF)	// ����дѡ���ֽ�

#define IAP_RDP				0x1FFFF800			// ��д������RDP

/***************************************************************************************
** ��������: IAP_FlashWrite
** ��������: IAP������д����
** ��    ��: w_data		/ д������ָ��
** ��    ��: w_length	/ д�����ݳ���
** ��    ��: w_addr		/ д�����ݵ�ַ
** �� �� ֵ: д����(TRUE / FALSE)
** ��    ע: ����д�뷽ʽ,Ҫ���ַ�ͳ��ȶ���1024�ֽڶ���       
****************************************************************************************/
//extern uint32 IAP_FlashWrite(uint8* w_data, uint32 w_length, uint32 w_addr);
#define IAP_FLASH_ERASEPAGE 1
#define IAP_FLASH_NOTERASEPAGE 0
#define IAP_FLASH_SUCCESS 1
#define IAP_FLASH_FAIL 0
extern unsigned int IAP_Flash_Write(unsigned int adr, unsigned int n ,u8* p,u8 b_page_erase);
extern void IAP_Flash_Erase_Page(unsigned int adr);		//V101

/***************************************************************************************
** ��������: IAP_ReadPro
** ��������: ���óɶ�����(��ֹ���ԺͷǷ�����)
** ��    ��: RDP��
** �� �� ֵ: ��  	  
** ��    ע: ��ɺ�λоƬ  
****************************************************************************************/
//extern void IAP_ReadPro(uint16 m_rdp);	
	
#endif /* __IAP_H */					  
/****************************************************************************************
**  End Of File                                                     
****************************************************************************************/
