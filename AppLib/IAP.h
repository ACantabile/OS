/****************************Copyright (c)**********************************************                                     
**
**                     平阳瑞来自动化设备有限公司
**                        http://www.rlplc.com                         
**
**------------- 文件信息 ---------------------------------------------------------------
** 文 件 名: iap.h
** 创 建 人: 周从文 	
** 创建日期: 2008年11月21日
** 描    述: STM32的IAP程序
**------------- 版本信息 ---------------------------------------------------------------
** 版    本: V1.0	 
** 日    期: 2008年11月12日	
** 描    述: 加入版本信息
**--------------------------------------------------------------------------------------
***************************************************************************************/
#ifndef __IAP_H
#define __IAP_H
/**********************************************************/
//版本号管理
#define IAP_VER_H 101
/*
V101 加入页擦除函数
***********************************************************/

#include "stm32f10x.h"

#define FLASH_SIZE_ADR 0x1ffff7e0
					  
#define IAP_RDP_Key			((unsigned short)0x00A5)	 	
#define IAP_FLASH_KEY1		((unsigned int)0x45670123)
#define IAP_FLASH_KEY2		((unsigned int)0xCDEF89AB)
#define IAP_PAGE_ZISE		0x800				// 扇区大小

#define IAP_SR_BSY			0x0001				// SR 的空闲位
#define IAP_SR_EOP			0x0020				// EOP 操作完成

#define IAP_CR_PG			0x0001				// 编程  (1=选择编程操作)
#define IAP_CR_PER			0x0002				// 页擦除(1=选择页擦除操作)	
#define IAP_CR_MER			0x0008				// 全擦除(选择擦除所有用户页)
#define IAP_CR_OPTPG		0x0010				// 烧写选择字节(对选择字节编程)
#define IAP_CR_OPTER		0x0020				// 擦除选择字节(擦除选择字节/小信息块)
#define IAP_CR_STRT			0x0040				// 开始擦除
#define IAP_CR_LOCK			0x0080				// 锁定(只能写’1’)
#define IAP_CR_OPTWRE		0x0200				// 允许写选择字节
#define IAP_CR_ERRIE		0x0400				// 允许错误状态中断
#define IAP_CR_EOPIE		0x1000				// 允许操作完成中断	
																	
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
#define CR_OPTWRE_Set		((unsigned int)0x00000200)	// 允许写选择字节
#define CR_OPTWRE_Reset		((unsigned int)0x00001DFF)	// 允许写选择字节

#define IAP_RDP				0x1FFFF800			// 读写保护字RDP

/***************************************************************************************
** 函数名称: IAP_FlashWrite
** 功能描述: IAP扇区烧写函数
** 参    数: w_data		/ 写入数据指针
** 参    数: w_length	/ 写入数据长度
** 参    数: w_addr		/ 写入数据地址
** 返 回 值: 写入结果(TRUE / FALSE)
** 备    注: 扇区写入方式,要求地址和长度都以1024字节对齐       
****************************************************************************************/
//extern uint32 IAP_FlashWrite(uint8* w_data, uint32 w_length, uint32 w_addr);
#define IAP_FLASH_ERASEPAGE 1
#define IAP_FLASH_NOTERASEPAGE 0
#define IAP_FLASH_SUCCESS 1
#define IAP_FLASH_FAIL 0
extern unsigned int IAP_Flash_Write(unsigned int adr, unsigned int n ,u8* p,u8 b_page_erase);
extern void IAP_Flash_Erase_Page(unsigned int adr);		//V101

/***************************************************************************************
** 函数名称: IAP_ReadPro
** 功能描述: 设置成读保护(禁止调试和非法读出)
** 参    数: RDP码
** 返 回 值: 无  	  
** 备    注: 完成后复位芯片  
****************************************************************************************/
//extern void IAP_ReadPro(uint16 m_rdp);	
	
#endif /* __IAP_H */					  
/****************************************************************************************
**  End Of File                                                     
****************************************************************************************/
