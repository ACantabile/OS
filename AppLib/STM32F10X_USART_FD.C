/****************************Copyright (c)**********************************************                                     
**
**                     温州大学智能电子电器重点实验室
**
**
**------------- 文件信息 ---------------------------------------------------------------
** 文 件 名: Usart.c
** 创 建 人: 陈冲
** 创建日期: 2009年03月19日
** 描    述: 串口驱动程序
**------------- 版本信息 ---------------------------------------------------------------
** 版    本: V1.0	 
** 日    期: 2009年03月19日	
** 描    述: 串口初始化函数，中断函数，通过串口数据结构指针操作不同的串口
**           
**           
**------------- 修改信息 ---------------------------------------------------------------

**--------------------------------------------------------------------------------------
** 备    注: 从模式时需在1MS定时中断是加入如下代码:
****************************************************************************************/

#define STM32F10X_USART_FD_MAIN
#include "STM32F10X_USART_FD.H"
//--------------------------------------------------------------------------------------
//版本号管理
#define STM32F10X_USART_VER_C 100

	#if STM32F10X_USART_VER_C != STM32F10X_USART_VER_H
		#ERROR "STM32F10X_USART_C_VER ERROR!"
	#endif
//--------------------------------------------------------------------------------------

#include "string.h"

#define USART_SR	0x00	//状态寄存器
	enum{
		B_USART_SR_PE,
		B_USART_SR_FE,
		B_USART_SR_NE,
		B_USART_SR_ORE,
		B_USART_SR_IDLE,
		B_USART_SR_RXNE,
		B_USART_SR_TC,
		B_USART_SR_TXE,
		B_USART_SR_LBD,
		B_USART_SR_CTS
	};

#define USART_DR	0x04	//数据寄存器
#define USART_BRR	0x08	//波特率寄存器
#define USART_CR1	0x0c	//控制寄存器1
	enum{
		B_USART_CR1_SBK,
		B_USART_CR1_RWU,
		B_USART_CR1_RE,
		B_USART_CR1_TE,
		B_USART_CR1_IDLEIE,
		B_USART_CR1_RXNEIE,
		B_USART_CR1_TCIE,
		B_USART_CR1_TXEIE,
		B_USART_CR1_PEIE,
		B_USART_CR1_PS,
		B_USART_CR1_PCE,
		B_USART_CR1_WAKE,
		B_USART_CR1_M,
		B_USART_CR1_UE
	};
#define USART_CR2	0x10	//控制寄存器2
#define USART_CR3	0x14	//控制寄存器3
#define USART_GTPR	0x18	//保护时间及与分频器



//通讯插入到定时器的程序
u8 usart_tm_1mS(USART_T *p)
{
	u8 r=0;
	if(p->tm_1mS!=0 && --p->tm_1mS==0)
	{
		if(p->adr==0 || TstB(p->sta,B_USART_ADR_OK))
		{
			//!禁止继续接收
			//U32_ClrB((unsigned long*)(p->base+USART_CR1),B_USART_CR1_RE);	//禁止接收
			U8_SetB(&p->sta,B_EV_USART_RX);			//置接收标志
			//U8_ClrB(&p->sta,B_USART_ADR_OK);
			r=1;
		}
		else
		{
			p->r_e=0;		//地址不对,马上启动新的接收
		}
	}
	return(r);
	/*
	if(p->tm_send!=0)
	{
		if(--(p->tm_send)==0)
		{
			U32_SetB((unsigned long*)(p->base+USART_CR1),B_USART_CR1_TXEIE);//开启TXE中断
		}
	}
	
	if(p->tm_485!=0)
	{
		if(--(p->tm_485)==0)
		{
			HWREG(p->RS485)=~p->sta;	//切换RS485到接受状态
		}
	}
	*/
}

//通讯中断服务函数
//
#define COMM_STA_ERR_BIT ((1<<B_USART_SR_FE) | (1<<B_USART_SR_PE))
//#define COMM_STA_ERR_BIT ((1<<B_USART_SR_FE))
void usart_isr(USART_T *p)
{
	unsigned long base;
	unsigned long i;
	unsigned long j;
	unsigned char ch;
	unsigned short n;
	
	base=p->base;

	i=HWREG(base+USART_SR); 
	j= HWREG(base+USART_CR1);		//读取状态
	
	//if(p->baud_sta & (1<<B_USART_BAUD_LOCK))
	{
		j |= COMM_STA_ERR_BIT;
	}
	i&=j;

	if(TstB(i,B_USART_SR_RXNE))
	{	//接收数据
		ch=HWREG(base+USART_DR);	//先读,有帧错误的时候可以清除标志
		
//		r_e=p->r_e;
//		if(r_e==0 || (i & COMM_STA_ERR_BIT)==0)
//		{
//			if(r_e<p->rx_buf_len)
//			{	
//				p->rx_buf[r_e]=ch;
//				if(r_e==0)
//				{
//					if(ch==p->adr || ch==0)
//					{
//						U8_SetB(&(p->sta),B_USART_ADR_OK);
//					}
//				}
//				p->r_e++;
//			}
//		p->tm_1mS=p->tm_1mS_init;
//		}
		//n=(p->r_e-p->r_s)%(p->rx_buf_len);	//求缓冲区已经存在的字节数
		//n %= (p->rx_buf_len);	//求缓冲区已经存在的字节数
		
		n=((unsigned int)(p->r_e) + (unsigned int)(p->rx_buf_len) - (unsigned int)(p->r_s)) % (unsigned int)(p->rx_buf_len);
		
		
		if(n==0 || (i & COMM_STA_ERR_BIT)==0)
		{									//收到第一个字节或者有帧错误，帧错误一般用于波特率自适应
			if((n+1) < p->rx_buf_len)
			{								//小于缓冲区
				p->rx_buf[p->r_e]=ch;		//接收一个字节
				if(TstB(p->sta,B_USART_CHKADR) && n==0)
				{							//需要判断第一个地址，第一个字节判断一下地址
					if(ch==p->adr || ch==0)
					{
						U8_SetB(&(p->sta),B_USART_ADR_OK);	//设地址匹配标志
					}
				}
				//p->r_e=(++p->r_e)%(p->rx_buf_len);			//对尾指针增量
				j=p->r_e+1;
				j %= (p->rx_buf_len);			//对尾指针增量
				p->r_e=j;						//一次性赋值
			}
		p->tm_1mS=p->tm_1mS_init;							//设超时寄存器
		}
	}
	//--------------------------------------------------------------------------------------------
	if(TstB(i,B_USART_SR_TXE))
	{								//发送寄存器空中断
		if(p->t_s!=p->t_e) 
		{
			HWREG(base+USART_DR)=p->tx_buf[p->t_s];	//发送字节
			//p->t_s++;
			//p->t_s=(++p->t_s)%p->tx_buf_len;		//发送首指针增量
			j=p->t_s+1;
			j %= (p->tx_buf_len);
			p->t_s=j;
		}
		else
		{	//最后一个字节移入发送寄存器
			U32_ClrB((unsigned long*)(base+USART_CR1),B_USART_CR1_TXEIE);//禁止TXE中断
			U32_SetB((unsigned long*)(base+USART_CR1),B_USART_CR1_TCIE);//允许TC中断
		}
	}
	else if(TstB(i,B_USART_SR_TC))
	{								//发送移位寄存器空，完全发送完毕
		//HWREG(p->RS485)=~p->sta;	//切换RS485到接收状态
		HWREGH(p->RS485_GPIO_RECV)=p->RS485_PIN;		//切换485至接收状态
		
		U8_ClrB(&(p->sta),B_USART_TX_BUSY);			//清除发送忙标志
		U32_ClrB((unsigned long*)(base+USART_CR1),B_USART_CR1_TCIE);//禁止TC中断
		U32_SetB((unsigned long*)(base+USART_CR1),B_USART_CR1_RE);	//与半双工兼容，每次发送完成允许接收
	}
}
//-------------------------------------------------------------------------------
//清零接收指针，切换为RSP85接收状态，允许接收
void usart_rx_rst(USART_T *p)
{
	p->r_s=0;
	p->r_e=0;
	HWREGH(p->RS485_GPIO_RECV)=p->RS485_PIN;
	U32_ClrB((unsigned long*)(p->base+USART_CR1),B_USART_CR1_TCIE);//禁止TC中断
	U32_SetB((unsigned long*)(p->base+USART_CR1),B_USART_CR1_RE);	//允许接收
}

void usart_rx_en(USART_T *p)
{
	HWREGH(p->RS485_GPIO_RECV)=p->RS485_PIN;
	U32_ClrB((unsigned long*)(p->base+USART_CR1),B_USART_CR1_TCIE);//禁止TC中断
	U32_SetB((unsigned long*)(p->base+USART_CR1),B_USART_CR1_RE);	//允许接收
}

void usart_rx_clr(USART_T *p)
{
	p->r_s=0;
	p->r_e=0;
	U8_ClrB(&(p->sta),B_EV_USART_RX);
}

//-------------------------------------------------------------------------------


//兼容半双工发送函数
//启动发送函数，调用函数前必须填充好发送缓冲区
void usart_hd_send_start(USART_T *pu,unsigned short n)
{
	if(n!=0)
	{
		U8_SetB(&(pu->sta),B_USART_TX_BUSY);
		pu->t_s=0;		//半双工，每次清零
		pu->t_e=n;
		
		HWREGH(pu->RS485_GPIO_SEND)=pu->RS485_PIN;		//RS485端口指向发送状态	
		U32_ClrB((unsigned long*)(pu->base+USART_CR1),B_USART_CR1_RE);		//半双工发送时禁止接收
		pu->r_s=0;		//半双工对接收清零
		pu->r_e=0;		//半双工对接收清零
		U32_SetB((unsigned long*)(pu->base+USART_CR1),B_USART_CR1_TXEIE);//允许TXE中断
	}
}
//-------------------------------------------------------------------------------


//从接收缓冲区读取数据,最多n个
int usart_buf(USART_T *p,unsigned char *buf,unsigned short n,unsigned char b_move)
{
	unsigned int head,tail,head_end,rn,len;
	
	if(n==0)
	{
		return(0);
	}
	
	head=p->r_s;
	tail=p->r_e;
	len=p->rx_buf_len;
	
	//rn=(tail-head)%(p->rx_buf_len);	//short编译会出错,换成int
	//rn=(tail-head)%len;		//求出目前接收到的字节数
	head_end = len - head;		//求头到最后的字节数
	rn=( tail + head_end ) % len;		//求出目前接收到的字节数
		
	if(rn>0)
	{						//有数据
		if(rn > n)
		{					//数据量大于接收的数据n，只接收n个
			rn=n;
			tail=(head+rn)%len;		//重新计算尾部！防止后面出错
		}
		
		if(tail > head)
		{							//尾比头大，数据在连续区，直接拷贝
			memcpy(buf,p->rx_buf+head,rn);
		}
		else
		{							//尾比头小，没有头尾相等状态
			//head_end=len-head;		//求头到最后的字节数
			memcpy(buf,p->rx_buf+head,head_end);	//拷贝头到最后的字节
			memcpy(buf+head_end,p->rx_buf,rn-head_end);	//拷贝剩下的字节
			
		}
		if(b_move)
		{							//如果删除接收缓冲区的内容，调整头指针
			p->r_s=(head + rn) % len;
		}
	}
	return(rn);
}
	
//从接收缓冲区读取数据,最多n个	
int usart_recv(USART_T *p,unsigned char *buf,unsigned short n)
{
	return(usart_buf(p,buf,n,1));
}

//从接收缓冲区拷贝数据,最多n个
int usart_copy(USART_T *p,unsigned char *buf,unsigned short n)
{
	return(usart_buf(p,buf,n,0));
}

//求接收缓冲区已经接收的字节数
unsigned int usart_recv_bytes(USART_T *p)
{
	unsigned int i;
	i=(unsigned int)(p->r_e) + (unsigned int)(p->rx_buf_len) - (unsigned int)(p->r_s);
	return(i % p->rx_buf_len);
}

//从接收缓冲区删除n个数据
unsigned int usart_rbuf_erase(USART_T *p,unsigned short n)
{
	unsigned int head,tail,rn,len;
	head=p->r_s;
	tail=p->r_e;
	len=p->rx_buf_len;
	
	rn=(tail + len - head) % len;		//求出目前接收到的字节数
	
	if(n > rn)
	{
		n=rn;
	}
	
	p->r_s=(head + n) % len;
	return(n);
}

//向通讯缓冲区填入字节，并启动发送
//p:通讯结构体指针
//buf:要发送字节的指针
//n：要发送的字节数
//返回：发送成功的字节数，由于通讯口剩余空间有可能比要发送的字节数小，返回的字节数小于等于n
int usart_send(USART_T *p,unsigned char *buf,unsigned short n)
{
	unsigned int tn,head,tail,tail_end,len;
	
	if(n>0)
	{
		if(buf == p->tx_buf)		//通讯发送缓冲区直接作为工作缓冲区
		{
			p->t_s=0;
			p->t_e=0;
		}
		
		head=p->t_s;
		tail=p->t_e;
		len=p->tx_buf_len;
		
		
		tail_end = len - tail;						//求尾到最后的空间
		tn=(head + tail_end) % len;		//求出缓冲区剩下的空间
		
		if(tn==0)
		{
			tn=len;		//缓冲区空
		}
		
		tn--;			//区别缓冲区空与缓冲区满，实际空间减少一个
		
		if(n > tn)
		{
			n=tn;		//如果要求发送的内容超过缓冲区剩余的空间，则发送剩余空间大小的字节数
		}
		
		if(n!=0)
		{
			U8_SetB(&(p->sta),B_USART_TX_BUSY);			//切换成忙状态
			HWREGH(p->RS485_GPIO_SEND)=p->RS485_PIN;		//RS485端口指向发送状态	
			
			if(buf != p->tx_buf)
			{
				if(head > tail)
				{
					memcpy(p->tx_buf+tail,buf,n);			//头大于尾，剩余空间在连续区域
				}
				else
				{
					//tail_end=len-tail;						//求尾到最后的空间
					if(tail_end>=n)
					{
						memcpy(p->tx_buf+tail,buf,n);		//比n大，这段连续空间直接拷贝
					}
					else
					{
						memcpy(p->tx_buf+tail,buf,tail_end);		//先拷贝尾到最后的字节数
						memcpy(p->tx_buf,buf+tail_end,n-tail_end);	//再拷贝剩下的字节数
					}
				}
			}
			p->t_e=(tail + n) % len;		//调整尾指针

			U32_SetB((unsigned long*)(p->base+USART_CR1),B_USART_CR1_TXEIE);//允许TXE中断
		}
		return(n);		//返回发送字节数
	}
	else
	{		
		return(0);
	}
}



//==============================================================================
/*
const u16 tb_usart_wl[]={USART_WordLength_8b,USART_WordLength_9b};
const u16 tb_usart_pi[]={USART_Parity_No,USART_Parity_Even,USART_Parity_Odd};
const u16 tb_usart_st[]={USART_StopBits_1,USART_StopBits_0_5,USART_StopBits_2,USART_StopBits_1_5};


//打开串口函数，使用FIFO，接收6/8，发送4/8，使能中断
//int Open_USART(unsigned long ulComm,unsigned long ulBaud,unsigned long ulConfig,
//			  GPIO_TypeDef * RS485_GPIO_BASE,unsigned long RS485_BIT,unsigned char INT_PI)
USART_T* Open_USART(USART_INIT_T *pinit)
{
	USART_InitTypeDef USART_InitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;	//定义GOIP配置结构体
	NVIC_InitTypeDef NVIC_InitStructure;	//定义中断配置结构体
	USART_TypeDef * USART;
	u8 USART_IRQChannel;
	u16 GPIO_RX_PIN,GPIO_TX_PIN;
	GPIO_TypeDef * USART_GPIO;
	u16 i;
	u32 map_r;
	u32 RCC_APB2Periph;
	unsigned char ulComm;
	unsigned long ulBaud;
	unsigned long ulConfig;
	GPIO_TypeDef * RS485_GPIO_BASE;
	


	USART_T *p=NULL;
	
	//map_r=AFIO->MAPR;
	ulComm=pinit->ulComm;
	ulBaud=pinit->ulBaud;
	ulConfig=pinit->ulConfig;

	if(ulComm==1)
	{
		USART=USART1;
		
		
		if((map_r & GPIO_Remap_USART1)==GPIO_Remap_USART1)
		{
			USART_GPIO=GPIOB;			//重映射至PB6,7
			GPIO_RX_PIN=GPIO_Pin_7;
			GPIO_TX_PIN=GPIO_Pin_6;
			RCC_APB2Periph=	RCC_APB2Periph_USART1 | RCC_APB2Periph_GPIOB | RCC_APB2Periph_AFIO;

		}
		else
		{
			USART_GPIO=GPIOA;
			GPIO_RX_PIN=GPIO_Pin_10;
			GPIO_TX_PIN=GPIO_Pin_9;
			RCC_APB2Periph=	RCC_APB2Periph_USART1 | RCC_APB2Periph_GPIOA | RCC_APB2Periph_AFIO;
		}
		USART_IRQChannel=USART1_IRQn;	//USART1_IRQChannel;

		g_USART[0].base=USART1_BASE;
		p=&g_USART[0];	
		//--------------------------------------------------	
	}
	else if(ulComm==2)
	{
		// 使能UART1外设
		RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);	
		//-------------------------------------------------
		USART=USART2;
		
		if((map_r & GPIO_Remap_USART2) == GPIO_Remap_USART2)
		{
			USART_GPIO=GPIOD;			//重映射至PD5,6
			GPIO_RX_PIN=GPIO_Pin_6;
			GPIO_TX_PIN=GPIO_Pin_5;
			RCC_APB2Periph=RCC_APB2Periph_GPIOD | RCC_APB2Periph_AFIO;
		}
		else
		{
			USART_GPIO=GPIOA;
			GPIO_RX_PIN=GPIO_Pin_3;
			GPIO_TX_PIN=GPIO_Pin_2;
			RCC_APB2Periph=RCC_APB2Periph_GPIOA | RCC_APB2Periph_AFIO;
		}
		
		USART_IRQChannel=USART2_IRQn;	//USART2_IRQChannel;

		g_USART[1].base=USART2_BASE;
		p=&g_USART[1];
		//--------------------------------------------------	
	}
	else if(ulComm==3)
	{
		// 使能UART1外设
		RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);	
		//-------------------------------------------------
		USART=USART3;
		
		map_r&=GPIO_FullRemap_USART3;
		if(map_r == GPIO_FullRemap_USART3)
		{
			USART_GPIO=GPIOD;			//全映射,至PD8,9
			GPIO_RX_PIN=GPIO_Pin_9;
			GPIO_TX_PIN=GPIO_Pin_8;
			RCC_APB2Periph=RCC_APB2Periph_GPIOD | RCC_APB2Periph_AFIO;
		}
		else if(map_r==GPIO_PartialRemap_USART3)
		{
			USART_GPIO=GPIOC;			//部分映射,至PC10,11
			GPIO_RX_PIN=GPIO_Pin_11;
			GPIO_TX_PIN=GPIO_Pin_10;
			RCC_APB2Periph=RCC_APB2Periph_GPIOC | RCC_APB2Periph_AFIO;
		}
		else
		{
			USART_GPIO=GPIOB;
			GPIO_RX_PIN=GPIO_Pin_11;
			GPIO_TX_PIN=GPIO_Pin_10;
			RCC_APB2Periph=RCC_APB2Periph_GPIOB | RCC_APB2Periph_AFIO;
		}
		
		USART_IRQChannel=USART3_IRQn;	//USART3_IRQChannel;

		g_USART[2].base=USART3_BASE;
		p=&g_USART[2];
		//--------------------------------------------------
	}
#if USART_N >3
	else if(ulComm==4)
	{
		// 使能UART4外设
		RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART4, ENABLE);	
		//-------------------------------------------------
		USART=UART4;
		
		
		USART_GPIO=GPIOC;
		GPIO_RX_PIN=GPIO_Pin_11;
		GPIO_TX_PIN=GPIO_Pin_10;
		RCC_APB2Periph=RCC_APB2Periph_GPIOC | RCC_APB2Periph_AFIO;
		
		
		USART_IRQChannel=UART4_IRQn;	//USART3_IRQChannel;

		g_USART[3].base=UART4_BASE;
		p=&g_USART[3];
		//--------------------------------------------------
	}
	else if(ulComm==5)		//串口5 TX-PC12，RX-PD2
	{
		// 使能UART5外设
		RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART5, ENABLE);	
		//-------------------------------------------------
		USART=UART5;
				
		USART_GPIO=GPIOD;
		GPIO_RX_PIN=GPIO_Pin_2;		//PD2
		
		GPIO_TX_PIN=GPIO_Pin_12;	//PC12
		RCC_APB2Periph=RCC_APB2Periph_GPIOD | RCC_APB2Periph_AFIO;
		
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC | RCC_APB2Periph_AFIO , ENABLE);	//com5 开PC
		
		USART_IRQChannel=UART5_IRQn;	//USART3_IRQChannel;

		g_USART[4].base=UART5_BASE;
		p=&g_USART[4];
		//--------------------------------------------------
	}
#endif
	else
	{
		return(p);
	}
	//--------------------------------------------------------------------------
	RCC_APB2PeriphClockCmd(RCC_APB2Periph, ENABLE);
	p->rx_buf=pinit->rx_buf;
	p->rx_buf_len=pinit->rx_buf_len;
	p->tx_buf=pinit->tx_buf;
	p->tx_buf_len=pinit->tx_buf_len;
	//--------------------------------------------------------------------------

	i= ulConfig;

	//配置输入GPIO
	// Configure USART1 Rx (PA.10) as input floating 
	GPIO_InitStructure.GPIO_Pin = GPIO_RX_PIN;
	if(i & USART_RX_INF)
	{
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	}
	else
	{
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;  	//默认
	}
	GPIO_Init(USART_GPIO, &GPIO_InitStructure);
	
	//配置输出端口
	GPIO_InitStructure.GPIO_Pin = GPIO_TX_PIN ;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	if(i & USART_TX_OD)
	{
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_OD;	
	}
	else
	{
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	//默认
	}
	if(ulComm==5)
	{
		GPIO_Init(GPIOC, &GPIO_InitStructure);	//串口5 TX-PC12，RX-PD2
	}
	else
	{
		GPIO_Init(USART_GPIO, &GPIO_InitStructure);
	}

	i >>= USART_CFG_RRN;
	//p->sta=i & 0x01;	//配置RS485极性
	//HWREG(p->RS485)=~p->sta;

	//配置RS485端口
	RS485_GPIO_BASE=pinit->RS485_GPIO_BASE;
	if(RS485_GPIO_BASE==0)
	{
		//p->RS485=REG_BIT(&(p->sta),B_USART_SRS485);//指向端口或指向结构sta的位
		p->RS485_GPIO_SEND=&p->RS485_SOFT;
		p->RS485_GPIO_RECV=&p->RS485_SOFT;
	}
	else
	{
		p->RS485_PIN=1 << pinit->RS485_GPIO_PIN;
		GPIO_InitStructure.GPIO_Pin = p->RS485_PIN;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;	//推挽输出
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	//快速模式输出
		GPIO_Init(pinit->RS485_GPIO_BASE, &GPIO_InitStructure);
		
		//p->RS485=REG_BIT(((u32)RS485_GPIO_BASE+GPIO_ODR),RS485_BIT);
		if(i & 0x01)
		{	//RS485高电平发送
			p->RS485_GPIO_SEND=(unsigned short *)&(((GPIO_TypeDef*)RS485_GPIO_BASE)->BSRR);
			p->RS485_GPIO_RECV=(unsigned short *)&(((GPIO_TypeDef*)RS485_GPIO_BASE)->BRR);
		}
		else
		{	//RS485低电平发送
			p->RS485_GPIO_SEND=(unsigned short *)&(((GPIO_TypeDef*)RS485_GPIO_BASE)->BRR);
			p->RS485_GPIO_RECV=(unsigned short *)&(((GPIO_TypeDef*)RS485_GPIO_BASE)->BSRR);
		}
		*(p->RS485_GPIO_RECV)=p->RS485_PIN;		//初始化后切为接收状态
	}
	//-------------------------------------------------------------------------
	if(pinit->ulConfig & USART_CFG_FD)
	{
		p->sta |= 1<<B_USART_FD;				//全双工标志
	}
	//-------------------------------------------------------------------------
	//初始化串行端口
	if(ulBaud==BAUD_USE_NB)
	{
		if(p->baud_nb>=BAUD_NB_MAX)p->baud_nb=0;
		ulBaud=Tb_BaudRate[p->baud_nb];
	}
	USART_InitStructure.USART_BaudRate = ulBaud;

	Set_Usart_Tout_1mS(p,ulBaud);
	
	

	//---------------------------------------------------------------------

	i >>= USART_CFG_RRN;
	USART_InitStructure.USART_WordLength = tb_usart_wl[i & USART_CFG_MASK];

	i >>= USART_CFG_RRN;
	USART_InitStructure.USART_Parity = tb_usart_pi[i & USART_CFG_MASK];

	i >>= USART_CFG_RRN;
	USART_InitStructure.USART_StopBits = tb_usart_st[i & USART_CFG_MASK];

	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	
	USART_Init(USART, &USART_InitStructure);	//配置端口

	USART_ITConfig(USART, USART_IT_RXNE, ENABLE);	//使能中断
	
	USART_Cmd(USART, ENABLE);	//使能端口
	//-----------------------------------------------------------
	//使能中断
	NVIC_InitStructure.NVIC_IRQChannel = USART_IRQChannel;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = pinit->INT_PI & 0x0f;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	//------------------------------------------------------------
	return(p);	
}
*/
//配置字符串"数据位，校验位，停止位，半双工/全双工，输出PP或OD，输入上拉或悬空，中断优先级，RS485DPIO，RS485PIN，RS485极性"
//CfgStr:"7/8,n/e/o,1/2,p/o,u/f,0-15,A-F,0-15,P/N"

typedef struct{
	char comm_port;
	char data_bits;
	char parity;
	char stop_bits;
	char hf_mode;
	char tx_port_mode;
	char rx_port_mode;
	char interrupt;
	char RS485_GPIO_BASE;
	char RS485_GPIO_PIN;
	char RS485_PORT_MODE;
	char RS485_GPIO_polarity;
} USART_CFGSTR_T;



USART_T* Open_USART(const char *cfg_str,unsigned int ulBaud,unsigned short tx_buf_size,unsigned int rx_buf_size)
{
	USART_InitTypeDef USART_InitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;	//定义GOIP配置结构体
	NVIC_InitTypeDef NVIC_InitStructure;	//定义中断配置结构体
	USART_TypeDef * USART;
	u8 USART_IRQChannel;
	u16 GPIO_RX_PIN,GPIO_TX_PIN;
	GPIO_TypeDef * USART_GPIO;
	int i;
	u32 map_r;
	u32 RCC_APB2Periph;

	GPIO_TypeDef * RS485_GPIO_BASE;
	USART_CFGSTR_T CFG;
	
	//------------------------------------------------------------
	
	USART_T *p=NULL;
	
	if(strlen(cfg_str)!=sizeof(USART_CFGSTR_T))
	{
		return(NULL);
	}
	
	memcpy(&CFG,cfg_str,sizeof(CFG));
	str2lwr((char *)&CFG);
		
	i=CFG.comm_port;

	map_r=AFIO->MAPR;
	if(i=='1')
	{
		USART=USART1;
		
		if((map_r & GPIO_Remap_USART1)==GPIO_Remap_USART1)
		{
			USART_GPIO=GPIOB;			//重映射至PB6,7
			GPIO_RX_PIN=GPIO_Pin_7;
			GPIO_TX_PIN=GPIO_Pin_6;
			RCC_APB2Periph=	RCC_APB2Periph_USART1 | RCC_APB2Periph_GPIOB | RCC_APB2Periph_AFIO;

		}
		else
		{
			USART_GPIO=GPIOA;
			GPIO_RX_PIN=GPIO_Pin_10;
			GPIO_TX_PIN=GPIO_Pin_9;
			RCC_APB2Periph=	RCC_APB2Periph_USART1 | RCC_APB2Periph_GPIOA | RCC_APB2Periph_AFIO;
		}
		USART_IRQChannel=USART1_IRQn;	//USART1_IRQChannel;

		g_USART[0].base=USART1_BASE;
		p=&g_USART[0];	
		//--------------------------------------------------	
	}
	else if(i=='2')
	{
		// 使能UART1外设
		RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);	
		//-------------------------------------------------
		USART=USART2;
		
		if((map_r & GPIO_Remap_USART2) == GPIO_Remap_USART2)
		{
			USART_GPIO=GPIOD;			//重映射至PD5,6
			GPIO_RX_PIN=GPIO_Pin_6;
			GPIO_TX_PIN=GPIO_Pin_5;
			RCC_APB2Periph=RCC_APB2Periph_GPIOD | RCC_APB2Periph_AFIO;
		}
		else
		{
			USART_GPIO=GPIOA;
			GPIO_RX_PIN=GPIO_Pin_3;
			GPIO_TX_PIN=GPIO_Pin_2;
			RCC_APB2Periph=RCC_APB2Periph_GPIOA | RCC_APB2Periph_AFIO;
		}
		
		USART_IRQChannel=USART2_IRQn;	//USART2_IRQChannel;

		g_USART[1].base=USART2_BASE;
		p=&g_USART[1];
		//--------------------------------------------------	
	}
#if USART_N >2
	else if(i=='3')
	{
		// 使能UART1外设
		RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);	
		//-------------------------------------------------
		USART=USART3;
		
		map_r&=GPIO_FullRemap_USART3;
		if(map_r == GPIO_FullRemap_USART3)
		{
			USART_GPIO=GPIOD;			//全映射,至PD8,9
			GPIO_RX_PIN=GPIO_Pin_9;
			GPIO_TX_PIN=GPIO_Pin_8;
			RCC_APB2Periph=RCC_APB2Periph_GPIOD | RCC_APB2Periph_AFIO;
		}
		else if(map_r==GPIO_PartialRemap_USART3)
		{
			USART_GPIO=GPIOC;			//部分映射,至PC10,11
			GPIO_RX_PIN=GPIO_Pin_11;
			GPIO_TX_PIN=GPIO_Pin_10;
			RCC_APB2Periph=RCC_APB2Periph_GPIOC | RCC_APB2Periph_AFIO;
		}
		else
		{
			USART_GPIO=GPIOB;
			GPIO_RX_PIN=GPIO_Pin_11;
			GPIO_TX_PIN=GPIO_Pin_10;
			RCC_APB2Periph=RCC_APB2Periph_GPIOB | RCC_APB2Periph_AFIO;
		}
		
		USART_IRQChannel=USART3_IRQn;	//USART3_IRQChannel;

		g_USART[2].base=USART3_BASE;
		p=&g_USART[2];
		//--------------------------------------------------
	}
#endif
#if USART_N >3
	else if(i=='4')
	{
		// 使能UART4外设
		RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART4, ENABLE);	
		//-------------------------------------------------
		USART=UART4;
		
		
		USART_GPIO=GPIOC;
		GPIO_RX_PIN=GPIO_Pin_11;
		GPIO_TX_PIN=GPIO_Pin_10;
		RCC_APB2Periph=RCC_APB2Periph_GPIOC | RCC_APB2Periph_AFIO;
		
		
		USART_IRQChannel=UART4_IRQn;	//USART3_IRQChannel;

		g_USART[3].base=UART4_BASE;
		p=&g_USART[3];
		//--------------------------------------------------
	}
	else if(i=='5')		//串口5 TX-PC12，RX-PD2
	{
		// 使能UART5外设
		RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART5, ENABLE);	
		//-------------------------------------------------
		USART=UART5;
				
		USART_GPIO=GPIOD;
		GPIO_RX_PIN=GPIO_Pin_2;		//PD2
		
		GPIO_TX_PIN=GPIO_Pin_12;	//PC12
		RCC_APB2Periph=RCC_APB2Periph_GPIOD | RCC_APB2Periph_AFIO;
		
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC | RCC_APB2Periph_AFIO , ENABLE);	//com5 开PC
		
		USART_IRQChannel=UART5_IRQn;	//USART3_IRQChannel;

		g_USART[4].base=UART5_BASE;
		p=&g_USART[4];
		//--------------------------------------------------
	}
#endif
	else
	{
		return(NULL);
	}
	//--------------------------------------------------------------------------
	RCC_APB2PeriphClockCmd(RCC_APB2Periph, ENABLE);
	//--------------------------------------------------------------------------
	p->rx_buf_len=rx_buf_size;
	p->rx_buf=DataPool_Get(rx_buf_size);
	if(p->rx_buf == NULL)
	{
		return(NULL);
	}
	
	if(CFG.hf_mode=='h')
	{
		p->tx_buf=p->rx_buf;
		p->tx_buf_len=rx_buf_size;
	}
	else if(CFG.hf_mode=='f')
	{
		p->tx_buf_len=tx_buf_size;
		p->tx_buf=DataPool_Get(tx_buf_size);
		p->sta |= 1<<B_USART_FD;				//全双工标志
		if(p->tx_buf==NULL)
		{
			return(NULL);
		}
	}
	else
	{
		return(NULL);
	}
	//--------------------------------------------------------------------------
	//配置输入TX，RX 的GPIO
	GPIO_InitStructure.GPIO_Pin = GPIO_RX_PIN;
	if(CFG.rx_port_mode=='f')
	{
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	}
	else if(CFG.rx_port_mode=='u')
	{
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;  	//默认
	}
	else
	{
		return(NULL);
	}
	GPIO_Init(USART_GPIO, &GPIO_InitStructure);
	
	//配置输出端口
	GPIO_InitStructure.GPIO_Pin = GPIO_TX_PIN ;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	if(CFG.tx_port_mode=='o')
	{
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_OD;	
	}
	else if(CFG.tx_port_mode=='p')
	{
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	//默认
	}
	else
	{
		return(NULL);
	}
	
	//-----------------------------------------------------------------
	if(i=='5')
	{
		//通讯口5：TX,RX不在一个端口
		GPIO_Init(GPIOC, &GPIO_InitStructure);	//串口5 TX-PC12，RX-PD2
	}
	else
	{
		GPIO_Init(USART_GPIO, &GPIO_InitStructure);
	}
	//-----------------------------------------------------------------
	
	if(CFG.RS485_GPIO_BASE=='0')
	{
		p->RS485_GPIO_SEND=&p->RS485_SOFT;
		p->RS485_GPIO_RECV=&p->RS485_SOFT;
	}
	else if(CFG.RS485_GPIO_BASE >='a' && CFG.RS485_GPIO_BASE <='g')
	{
		RS485_GPIO_BASE=(GPIO_TypeDef *)(GPIOA_BASE + (GPIOB_BASE-GPIOA_BASE)*(CFG.RS485_GPIO_BASE-'a'));
		
		i=hexchar_btye(CFG.RS485_GPIO_PIN);
		if(i>=0)
		{	
			p->RS485_PIN=1 << i;
			GPIO_InitStructure.GPIO_Pin = p->RS485_PIN;
			if(CFG.RS485_PORT_MODE=='p')
			{
				GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;	//推挽输出
			}
			else if(CFG.RS485_PORT_MODE=='o')
			{
				GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD;	//推挽输出
			}
			else
			{
				return(NULL);
			}
			GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	//快速模式输出
			GPIO_Init(RS485_GPIO_BASE, &GPIO_InitStructure);
			
			if(CFG.RS485_GPIO_polarity=='+')
			{	//RS485高电平发送
				p->RS485_GPIO_SEND=(unsigned short *)&(((GPIO_TypeDef*)RS485_GPIO_BASE)->BSRR);
				p->RS485_GPIO_RECV=(unsigned short *)&(((GPIO_TypeDef*)RS485_GPIO_BASE)->BRR);
			}
			else if(CFG.RS485_GPIO_polarity=='-')
			{	//RS485低电平发送
				p->RS485_GPIO_SEND=(unsigned short *)&(((GPIO_TypeDef*)RS485_GPIO_BASE)->BRR);
				p->RS485_GPIO_RECV=(unsigned short *)&(((GPIO_TypeDef*)RS485_GPIO_BASE)->BSRR);
			}
			else
			{
				return(NULL);
			}
			*(p->RS485_GPIO_RECV)=p->RS485_PIN;		//初始化后切为接收状态
		}
		else
		{
			return(NULL);
		}
	}
	else
	{
		return(NULL);
	}
	
	
	//-------------------------------------------------------------------------
	if(CFG.hf_mode=='f')
	{
		p->sta |= 1<<B_USART_FD;				//全双工标志
	}
	else if(CFG.hf_mode != 'h')
	{
		return(NULL);
	}
	//-------------------------------------------------------------------------
	//初始化串行端口
	if(ulBaud==BAUD_USE_NB)
	{
		if(p->baud_nb>=BAUD_NB_MAX)p->baud_nb=0;
		ulBaud=Tb_BaudRate[p->baud_nb];
	}
	USART_InitStructure.USART_BaudRate = ulBaud;

	Set_Usart_Tout_1mS(p,ulBaud);
	//---------------------------------------------------------------------

	if(CFG.parity=='n')
	{
		i=USART_Parity_No;
	}
	else if(CFG.parity=='e')
	{
		i=USART_Parity_Even;
	}
	else if(CFG.parity=='o')
	{
		i=USART_Parity_Odd;
	}
	else
	{
		return(NULL);
	}
	
	USART_InitStructure.USART_Parity = i;
	
	if((CFG.data_bits == '8') &&  (CFG.parity=='n'))
	{
		i = USART_WordLength_8b;
	}
	else if(CFG.data_bits == '8')
	{
		i = USART_WordLength_9b;
	}
	else
	{
		return(NULL);
	}

	USART_InitStructure.USART_WordLength = i;
	
	if(CFG.stop_bits=='1')
	{
		i=USART_StopBits_1;
	}
	else if(CFG.stop_bits=='2')
	{
		i=USART_StopBits_2;
	}
	
	USART_InitStructure.USART_StopBits = i;
	

	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	
	USART_Init(USART, &USART_InitStructure);	//配置端口
	USART_ITConfig(USART, USART_IT_RXNE, ENABLE);	//使能中断
	//-----------------------------------------------------------
	i=hexchar_btye(CFG.interrupt);
	if(i<0)
	{
		return(NULL);
	}
	
	//使能中断
	NVIC_InitStructure.NVIC_IRQChannel = USART_IRQChannel;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = i;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	
	USART_Cmd(USART, ENABLE);	//使能端口
	//------------------------------------------------------------
	return(p);	
}

//-----------------------------------------------------------------------------------
void Set_Usart_Adr(USART_T *p,unsigned char adr)
{
	p->adr=adr;
}

void Set_Usart_BaudRate(USART_T *p,u32 BaudRate)
{
	u32 tmpreg = 0x00, apbclock = 0x00;
	u32 integerdivider = 0x00;
	u32 fractionaldivider = 0x00;
	//u32 usartxbase = 0;

	RCC_ClocksTypeDef RCC_ClocksStatus;
	USART_TypeDef* USARTx;

	//-------------------------------------------------------------------------------
	USARTx=(USART_TypeDef*)p->base;

	RCC_GetClocksFreq(&RCC_ClocksStatus);
	//if (usartxbase == USART1_BASE)
	if ((u32)USARTx == USART1_BASE)
	{
		apbclock = RCC_ClocksStatus.PCLK2_Frequency;
	}
	else
	{
		apbclock = RCC_ClocksStatus.PCLK1_Frequency;
	}
	
	// Determine the integer part
	//integerdivider = ((0x19 * apbclock) / (0x04 * (USART_InitStruct->USART_BaudRate)));
	integerdivider = ((0x19 * apbclock) / (0x04 * BaudRate));
	tmpreg = (integerdivider / 0x64) << 0x04;
	
	// Determine the fractional part
	fractionaldivider = integerdivider - (0x64 * (tmpreg >> 0x04));
	tmpreg |= ((((fractionaldivider * 0x10) + 0x32) / 0x64)) & ((u8)0x0F);
	
	// Write to USART BRR
	USARTx->BRR = (u16)tmpreg;
	//-------------------------------------------------------------------------------
	Set_Usart_Tout_1mS(p,BaudRate);
}
//-----------------------------------------------------------------------------------
#define AutoBaud_Retry_NB	4
u8 Usart_Auto_BaudRate(u8 comm_ret,USART_T *p)
{
	u8 r=0;

	if(!TstB(p->baud_sta,B_USART_BAUD_LOCK) && (comm_ret!=0))
	{//波特率没有锁定
		if(comm_ret==USART_REC_OK || comm_ret==USART_REC_FORMATOK)
		{//收到正确的格式包或不采用波特率自适应,锁定波特率
			U8_SetB(&(p->baud_sta),B_USART_BAUD_LOCK);
			Set_Usart_Tout_1mS(p,BAUD_USE_NB);
			r=1;
		}
		else
		{
			if(++p->baud_sta>AutoBaud_Retry_NB)
			{
				p->baud_sta=0;//cnt_1ms+p->baud_tout;
				if(++(p->baud_nb)>=BAUD_NB_MAX)
				{
					p->baud_nb=0;
				}
				Set_Usart_BaudRate(p,Tb_BaudRate[p->baud_nb]);
			}
		}	
	}
	return(r);	
}
//-----------------------------------------------------------------------------------
void Set_Usart_Tout_1mS(USART_T *p,u32 ulBaud)
{
	u32 j,k;

	if(ulBaud==BAUD_USE_NB)
	{
		ulBaud=Tb_BaudRate[p->baud_nb];
	}

	if(TstB(p->baud_sta,B_USART_BAUD_LOCK))
	{
		k=2;
		j=(u32)1000*4*11;	//(1/1mS)*4T*11B/BaudRate
	}	
	else
	{
		k=1;
		j=(u32)1000*3*11;	//(1/1mS)*3T*11B/BaudRate
	}

	j/=ulBaud;		

	if(j==0)
	{
		j++;
	}
	j+=k;	

	p->tm_1mS_init=j;
}
//----------------------------------------------------------------------------


unsigned int usart_copy_usart(USART_T *pd,USART_T *ps,unsigned char b_erase)
{
	unsigned int d_e,d_len,s_s,s_e,s_len;
	unsigned char *pdb,*psb; 
	unsigned int cnt=0;
	
	d_e=pd->t_e;
	d_len=pd->tx_buf_len;
	pdb=pd->tx_buf;
	s_s=ps->r_s;
	s_e=ps->r_e;
	s_len=ps->rx_buf_len;
	psb=ps->rx_buf;
	
	if(s_s!=s_e)
	{
	
		U8_SetB(&(pd->sta),B_USART_TX_BUSY);			//切换成忙状态
		HWREGH(pd->RS485_GPIO_SEND)=pd->RS485_PIN;		//RS485端口指向发送状态	
		
		while(s_s!=s_e)
		{
			pdb[d_e++]=psb[s_s++];
			s_s %= s_len;
			d_e %= d_len;
			cnt++;
		}
		
		if(b_erase)
		{
			ps->r_s=s_s;
		}
		pd->t_e=d_e;
		
		U32_SetB((unsigned long*)(pd->base+USART_CR1),B_USART_CR1_TXEIE);//允许TXE中断
	}
	return(cnt);
}


void USART1_IRQHandler(void)
{
	usart_isr(&g_USART[0]);	
}

void USART2_IRQHandler(void)
{
	usart_isr(&g_USART[1]);	
}

void USART3_IRQHandler(void)
{
	usart_isr(&g_USART[2]);	
}

#if USART_N > 3
void UART4_IRQHandler(void)
{
	usart_isr(&g_USART[3]);	
}

void UART5_IRQHandler(void)
{
	usart_isr(&g_USART[4]);	
}
#endif
