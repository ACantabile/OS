/****************************Copyright (c)**********************************************                                     

**------------- 文件信息 ---------------------------------------------------------------
** 文 件 名: iap.c
** 创 建 人: 周从文 	
** 创建日期: 2008年11月21日
** 描    述: STM32的IAP程序
**------------- 版本信息 ---------------------------------------------------------------
** 版    本: V1.0	 
** 日    期: 2008年11月12日	
** 描    述: 加入版本信息
**--------------------------------------------------------------------------------------
***************************************************************************************/	   
#include "iap.h"

//----------------------------------------------------------------------
//版本号管理
#define IAP_VER_C 101

	#if IAP_VER_C != IAP_VER_H
		#ERROR
	#endif
//----------------------------------------------------------------------



//#define Flash_Page_Size 0x400

#define Flash_Base_Adr 0x08000000

/*
SR : 闪存状态寄存器
	0: BUSY  空闲状态(1为繁忙,0为空闲)
	2: PGERR 编程错误(1为错误,0为无错),写1清除状态
	4: W_ERR 写保护  (1为错误,0为无错),写1清除状态
	5: EOP   操作结束(1为完成)写1清除状态

CR : 闪存控制寄存器
	0: PG    编程  (1=选择编程操作)
	1: PER   页擦除(1=选择页擦除操作)
	2: MER   全擦除(选择擦除所有用户页)
	4: OPTPG 烧写选择字节(对选择字节编程)
	5: OPTER 擦除选择字节(擦除选择字节/小信息块)
	6: STRT  开始擦除(当该位为’1’时将触发一次擦除操作。该位只可由软件置为’1’并在BSY变为’1’时清为’0’)
	7: LOCK  锁定(只能写’1’。 当该位为’1’时表示FPEC和FLASH_CR被锁住，在检测到正确的解
             锁序列后，硬件清除此位为’0’。在一次不成功的解锁操作后，下次复位前，该
             位不能再被改变)
	9: OPTWRE  允许写选择字节(当该位为’1’时，允许对选择字节/小信息块进行编程操作。当在
             FLASH_OPTKEYR寄存器写入正确的键序列后，该位被置为’1’。软件可清除此位。)
	10:ERRIE 允许错误状态中断(该位允许在发生FPEC错误时产生中断(当FLASH_SR寄存器中的
             PGERR/WRPRTERR置为’1’时)。(0：禁止产生中断 / 1：允许产生中断)
	12:EOPIE 允许操作完成中断(该位允许在FLASH_SR寄存器中的EOP位变为’1’时产生中断。
             (0：禁止产生中断 / 1：允许产生中断)
*/

/***************************************************************************************
** 函数名称: IAP_FlashWrite
** 功能描述: IAP扇区烧写函数
** 参    数: w_data		/ 写入数据指针
** 参    数: w_length	/ 写入数据长度
** 参    数: w_addr		/ 写入数据地址
** 返 回 值: 写入结果(TRUE / FALSE)
** 备    注: 扇区方式写入,要求地址和长度都以1024字节对齐       
****************************************************************************************/
/*
uint32 IAP_FlashWrite(uint8* w_data, uint32 w_length, uint32 w_addr)
{
	uint32 sta_addr, end_addr;
	uint32 FlashDes;
	uint32 now_addr;  		 

	sta_addr = (w_addr & 0x0801FC00);			// 1K字节对齐
	end_addr = (sta_addr + w_length) & 0x0801FC00;			
				  
	if((w_data == NULL) || (sta_addr < 0x08000000) || (sta_addr == end_addr))
	{
		return FALSE;
	}
	
	NVIC_SETPRIMASK();			// 关中断 

	FLASH->KEYR = IAP_FLASH_KEY1;				// 解锁FLASH编写擦除控制器
	FLASH->KEYR = IAP_FLASH_KEY2;

	while(sta_addr < end_addr)
	{										   
		while(FLASH->SR & IAP_SR_BSY);			// 检测忙位
		
		FLASH->CR |= CR_PER_Set;						  		   
		FLASH->AR  = sta_addr;					// 选择要擦除的页
	    FLASH->CR |= CR_STRT_Set;
		while(FLASH->SR & IAP_SR_BSY);			// 检测忙位	
	    FLASH->CR &= CR_PER_Reset;		  
		while(FLASH->SR & IAP_SR_BSY);			// 检测忙位	

		// 不进行查空 

		now_addr = sta_addr + 0x400;			// 加入基地址和增量1K 	 
		
	    FLASH->CR |= CR_PG_Set;
		for(FlashDes=sta_addr;FlashDes<now_addr;FlashDes+=2,w_data+=2)	// 进入烧写循环
		{  
			*(vu16*)FlashDes = *(u16*)w_data;		// 向地址写入数据			   
			while(FLASH->SR & IAP_SR_BSY);			// 检测忙位
			if(*(u16*)FlashDes != *(u16*)w_data)	// 核验写入的字
			{	
				FLASH->CR &= CR_PG_Reset;		
				FLASH->CR |= CR_LOCK_Set;			// 锁定FLASH编写擦除控制器	  
				NVIC_RESETPRIMASK();	 		
				return FALSE;			
			}
		}	

		sta_addr += 0x400;						// 烧写地址加1024
	}

	FLASH->CR |= CR_LOCK_Set;					// 锁定FLASH编写擦除控制器
	
    NVIC_RESETPRIMASK();	 	//开中断 

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


	sta_addr = adr;			// 1K字节对齐
	end_addr = sta_addr + n;			
				  
	if((sta_addr < Flash_Base_Adr) || (end_addr >= end_adr_max) || (sta_addr & 0x01) || (n & 0x01))
	{
		return IAP_FLASH_FAIL;
	}
	
	__disable_irq();	//关中断
	
	FLASH->KEYR = IAP_FLASH_KEY1;				// 解锁FLASH编写擦除控制器
	FLASH->KEYR = IAP_FLASH_KEY2;

	__enable_irq();		//开中断
	
	while(sta_addr < end_addr)
	{										   
		while(FLASH->SR & IAP_SR_BSY)
		{
			__nop();			// 检测忙位
		}
		
		//if(b_first || (sta_addr & (Flash_Page_Size-1)) ==0)
		if(b_page_erase && (sta_addr & page_mask) ==0)
		{
			FLASH->CR &= CR_PG_Reset;		//新加入
			FLASH->CR |= CR_PER_Set;						  		   
			
			//FLASH->AR  = sta_addr & (0x0801FC00);					// 选择要擦除的页
			FLASH->AR  = sta_addr;					// 选择要擦除的页
		    FLASH->CR |= CR_STRT_Set;
			while(FLASH->CR & CR_STRT_Set){;}			//等待STRT位变成0（新加入）
			while(FLASH->SR & IAP_SR_BSY){;}			// 检测忙位	
		    FLASH->CR &= CR_PER_Reset;		  
			while(FLASH->SR & IAP_SR_BSY){;}			// 检测忙位	
	
			//不进行查空
		}
	 	
		if(*(u16*)sta_addr != *(u16*)p)
		{
			FLASH->CR |= CR_PG_Set;
			*(vu16*)sta_addr = *(u16*)p;		// 向地址写入数据			   
			
			while(FLASH->SR & IAP_SR_BSY)
			{
				;			// 检测忙位
			}

			if(*(u16*)sta_addr != *(u16*)p)	// 核验写入的字
			{	
				FLASH->CR &= CR_PG_Reset;		
				FLASH->CR |= CR_LOCK_Set;			// 锁定FLASH编写擦除控制器	  
					 		
				return IAP_FLASH_FAIL;			
			}
		}
		sta_addr +=2;
		p+=2;
	}

	FLASH->CR &= CR_PG_Reset;
	FLASH->CR |= CR_LOCK_Set;					// 锁定FLASH编写擦除控制器
	
    //NVIC_RESETPRIMASK();	 	// 开中断 

	return (IAP_FLASH_SUCCESS); 	
}	

/***************************************************************************************/
//V101加入页擦除
void IAP_Flash_Erase_Page(unsigned int adr)
{
	__asm("cpsid i");
	FLASH->KEYR = IAP_FLASH_KEY1;				// 解锁FLASH编写擦除控制器
	FLASH->KEYR = IAP_FLASH_KEY2;
	__asm("cpsie i");
	
	FLASH->CR &= CR_PG_Reset;					//新加入
	FLASH->CR |= CR_PER_Set;						  		   
	
	FLASH->AR  = adr;							// 选择要擦除的页
	FLASH->CR |= CR_STRT_Set;
	while(FLASH->CR & CR_STRT_Set){;}			//等待STRT位变成0（新加入）
	while(FLASH->SR & IAP_SR_BSY){;}			// 检测忙位	
	FLASH->CR &= CR_PER_Reset;		  
	while(FLASH->SR & IAP_SR_BSY){;}			// 检测忙位	

	//不进行查空
		
	FLASH->CR &= CR_PG_Reset;					// 锁定FLASH编写擦除控制器
	FLASH->CR |= CR_LOCK_Set;
}
/***************************************************************************************
** 函数名称: IAP_ReadPro
** 功能描述: 设置成读保护(禁止调试和非法读出)
** 参    数: RDP码
** 返 回 值: 无  	  
** 备    注: 完成后复位芯片  
****************************************************************************************/
/*
void IAP_ReadPro(uint16 m_rdp)
{
	NVIC_SETPRIMASK(); 			// 关中断	

	FLASH->KEYR     = IAP_FLASH_KEY1;		// 解锁FLASH编写擦除控制器
	FLASH->KEYR     = IAP_FLASH_KEY2;	 		   
	while(FLASH->SR & IAP_SR_BSY);			// 检测忙位
									
    FLASH->OPTKEYR  = IAP_FLASH_KEY1;		// 解锁信息块
    FLASH->OPTKEYR  = IAP_FLASH_KEY2;   
	while(FLASH->SR & IAP_SR_BSY);			// 检测忙位
	
    FLASH->CR      |= CR_OPTWRE_Set;
    FLASH->CR      |= CR_OPTPG_Set; 	  
	OB->RDP         = m_rdp; 		  		// 向读保护寄存器写入参数
	while(FLASH->SR & IAP_SR_BSY);			// 检测忙位	  
	OB->WRP0        = 0x0;					// 值为0:写保护;
	while(FLASH->SR & IAP_SR_BSY);			// 检测忙位
	OB->WRP1        = 0x0;					// 值为1:无保护;
	while(FLASH->SR & IAP_SR_BSY);			// 检测忙位
	OB->WRP2        = 0x0;					// 默认为1
	while(FLASH->SR & IAP_SR_BSY);			// 检测忙位
//	OB->WRP3        = 0x0;		
//	while(FLASH->SR & IAP_SR_BSY);			// 检测忙位
	 
    FLASH->CR      &= CR_OPTWRE_Reset;
	FLASH->CR      |= CR_LOCK_Set;			// 锁定FLASH编写擦除控制器
	
    NVIC_RESETPRIMASK();	 	// 开中断
}
*/		  
/****************************************************************************************
**  End Of File                                                     
****************************************************************************************/
