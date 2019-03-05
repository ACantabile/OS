#ifndef __STM32F10X_USART_FD_H__
#define __STM32F10X_USART_FD_H__

#include "utility.h"
#include "ring_buf.h"
//--------------------------------------------------------
//版本号管理
#define STM32F10X_USART_VER_H 100
//--------------------------------------------------------


	#ifdef STM32F10X_USART_FD_MAIN
	 	#define STM32F10X_USART_FD_EXT
	#else
		#define STM32F10X_USART_FD_EXT extern
	#endif


//#define TR_BUF_LENGTH	1024	//(256+16)	//收发缓冲区大小




//串口初始化结构体
typedef struct{
	unsigned char ulComm;			//端口号
	unsigned char INT_PI;			//中断优先级
	unsigned short tx_buf_len;
	unsigned short rx_buf_len;
	unsigned long ulBaud;			//波特率
	unsigned long ulConfig;			//配置字
	GPIO_TypeDef * RS485_GPIO_BASE;	//RS485PIN端口基址
	unsigned long RS485_GPIO_PIN;		//RS485PIN号
	
	unsigned char *tx_buf;			//发送缓冲区
	unsigned char *rx_buf;			//接收缓冲区
}USART_INIT_T;
	
	

//通讯结构
typedef struct {
	unsigned long 	base;			//指向通讯端口的寄存器基址
	unsigned short *RS485_GPIO_SEND;		//RS485发送控制GPIO地址
	unsigned short *RS485_GPIO_RECV;		//RS485接收控制GPIO地址
	unsigned short 	RS485_PIN;				//RS485 PIN脚
	unsigned short 	RS485_SOFT;				//RS485发送、接收控制虚拟GPIO

	unsigned char sta;			//通讯状态
	unsigned char tm_1mS;		//幀间隔计时器
	unsigned char tm_1mS_init;	//幀间隔初始化值
	unsigned char adr;			//从机地址,用于提高多机通讯效率
	
	unsigned char baud_sta;	//关于波特率的状态位
	unsigned char baud_nb;	//波特率号

	unsigned short r_s;		//接收首指针
	unsigned short r_e;		//接收尾指针
	unsigned short t_s;		//发送首指针
	unsigned short t_e;		//发送尾指针
	
	unsigned short tx_buf_len;
	unsigned short rx_buf_len;
	unsigned char *tx_buf;
	unsigned char *rx_buf;
}USART_T;


#ifdef STM32F10X_USART_FD_MAIN
	const unsigned long Tb_BaudRate[]={2400,4800,9600,19200,38400,57600,115200};	//波特率号对应的波特率
	USART_T g_USART[USART_N];
#else
	extern unsigned long Tb_BaudRate[];
	extern USART_T g_USART[USART_N];
#endif

//sta,通讯状态的各位的定义
enum{
	B_USART_P485,		//BIT0表示RS485控制引脚的电平极性，=1表示高电平发送
	B_USART_CHKADR,		//需要检测地址
	B_EV_USART_RX,		//收到一包数据事件
	B_EV_USART_TX, 		//发送事件
	B_USART_REC_1BYTE,	//收到一个字节的数据
	B_USART_ADR_OK,		//已经收到一个合法的地址
	B_USART_FD,			//全双工标志
	B_USART_TX_BUSY,
};


enum{
	USART1_NB=1,
	USART2_NB,
	USART3_NB,
	USART4_NB,
	USART5_NB
};

//-----------------------------------------------------------------------
//adr:地址,=0接收任何数据包,非0,只接收地址=0或者地址匹配的数据包
//---------------------------------------------
//baud_sta,通讯波特率自适应状态位定义
//0-6用于数据包计数
//第7位代表波特率是否被锁定
#define B_USART_BAUD_LOCK 7	//baud_sta 的 BIT7
//---------------------------------------------
#define BAUD_USE_NB 0
//baud_nb,通讯波特率号定义
enum{
	BAUD_2400,
	BAUD_4800,
	BAUD_9600,
	BAUD_19200,
	BAUD_38400,
	BAUD_57600,
	BAUD_115200,
	BAUD_NB_MAX
	};


extern unsigned char usart_tm_1mS(USART_T *p);	//插入1mS定时器的函数
extern void usart_isr(USART_T *p);		//通讯中断服务程序
extern void usart_hd_send_start(USART_T *p,unsigned short n);	//兼容半双工发送函数
	
#define usart_send_start usart_hd_send_start
	
extern int usart_send(USART_T *p,unsigned char *buf,unsigned short n);	//发送n字节
extern int usart_recv(USART_T *p,unsigned char *buf,unsigned short n);	//从缓冲区接受字节，最多n个
extern int usart_copy(USART_T *p,unsigned char *buf,unsigned short n);	//从缓冲区拷贝字节，最多n个

extern void usart_rx_rst(USART_T *p);	//清理接受缓存
extern unsigned int usart_copy_usart(USART_T *pd,USART_T *ps,unsigned char b_erase);	//从1个串口拷贝数据到另一个串口,b_erase=1删除源串口内容
extern unsigned int usart_recv_bytes(USART_T *p);	//求就收缓冲区里面的字节数
extern unsigned int usart_rbuf_erase(USART_T *p,unsigned short n);//从接收缓冲区删除n个数据,返回删除的字节数
extern void usart_rx_clr(USART_T *p);	//清除接受缓冲区的内容
//------------------------------------------------------------------------------
//打开串口函数
//打开串口函数，使用FIFO，接收6/8，发送4/8，使能中断
//端口号，波特率，字长、停止位、校验位，RS485控制端口地址 | 控制极性
//控制极性=1表示端口=1是发送

//ulComm :端口号 1-5，ulBaud:波特率，ulConfig:配置字，ulRS485:RS485端口位地址，=0表示无硬件RS485端口
//INT_PI:抢先式中断优先级
//ulConfig:通讯配置位定义
#define USART_CFG_RRN	2	//每2位为一个配置项
#define USART_CFG_MASK	((1<<USART_CFG_RRN)-1)

#define USART_RX_UP		0		//Bit0:RXD设为上拉还是浮空
#define USART_RX_INF	1

#define USART_TX_PP		0		//Bit1:TXD设为PP还是OD
#define USART_TX_OD		2

#define USART_RS485_N	(0 << USART_CFG_RRN)	//Bit2:RS485控制极性
#define USART_RS485_P	(1 << USART_CFG_RRN)

#define USART_WL8		(0 << (USART_CFG_RRN*2))	//Bit4:数据位数
#define USART_WL9		(1 << (USART_CFG_RRN*2))

#define USART_PI_NONE	(0 << (USART_CFG_RRN*3))	//Bit6-7:校验方式
#define USART_PI_EVEN	(1 << (USART_CFG_RRN*3))
#define	USAAT_PI_ODD	(2 << (USART_CFG_RRN*3))


#define USART_ST_1		(0 << (USART_CFG_RRN*4))	//Bit8-9:停止位
#define USART_ST_0_5	(1 << (USART_CFG_RRN*4))
#define USART_ST_2_0	(2 << (USART_CFG_RRN*4))
#define USART_ST_1_5	(3 << (USART_CFG_RRN*4))

#define USART_CFG_FD	(1 << (USART_CFG_RRN*5))	//Bit10:全双工
//----------------------------------------------
#define USART_8_N_1 (USART_WL8 | USART_ST_1)
#define USART_8_E_1 (USART_WL9 | USART_PI_EVEN | USART_ST_1)
#define USART_8_O_1 (USART_WL9 | USART_PI_ODD | USART_ST_1)

#define USART_7_E_1 (USART_WL8 | USART_PI_EVEN | USART_ST_1)
#define USART_7_O_1 (USART_WL8 | USART_PI_ODD | USART_ST_1)
//------------------------------------------------------------------------------
#define USART_RS485_GPIO_NULL	((GPIO_TypeDef *) 0)	//没有发送接收方向控制口
#define USART_RS485_PIN_NULL	0	//没有发送接收方向控制口
//----------------------------------------------
//配置字符串"串口号，数据位，校验位，停止位，半双工/全双工，输出PP或OD，输入上拉或悬空，中断优先级，RS485DPIO，RS485PIN，RS485极性"
//CfgStr:"1-5，7/8,n/e/o,1/2,h/d,p/o,u/f,0-15,a-g,0-f,p/n"
extern USART_T* Open_USART(const char *cfg_str,unsigned int ulBaud,unsigned short rx_buf_size,unsigned int tx_buf_size);
//extern USART_T* Open_USART(USART_INIT_T *pinit);
//------------------------------------------------------------------------------
//设置USART地址,用于多级通讯的地址识别,数据包的第一个字节是首地址,若设成0,则接收全部的数据包
extern void Set_Usart_Adr(USART_T *p,unsigned char adr);
//------------------------------------------------------------------------------
//直接设置USART的波特率,前提是这个串口已经打开
extern void Set_Usart_BaudRate(USART_T *p,unsigned longBaudRate);
//------------------------------------------------------------------------------
#define USART_REC_OK	1
#define USART_REC_FORMATOK 2
//波特率自适应函数,返回1表示刚刚锁定波特率
//comm_ret是接收任务返回的值,=1接收到一包正确的数据包,=2接收到一包正确的数据格式,但地址不是自己的
//收到以上的参数,则锁定波特率
extern u8 Usart_Auto_BaudRate(u8 comm_ret,USART_T *p);
//------------------------------------------------------------------------------
//设定帧超时值
//根据波特率设定帧超时值
extern void Set_Usart_Tout_1mS(USART_T *p,unsigned longulBaud);
//------------------------------------------------------------------------------
//使该通讯口切换到接收状态
extern void usart_rx_en(USART_T *p);
//使该通讯口切换到接收状态,并清零计数器
extern void usart_rx_rst(USART_T *p);
//------------------------------------------------------------------------------
//extern void usart_copy2tx_buf(USART_T *pd,unsigned char *ps,unsigned short n);
//------------------------------------------------------------------------------

#define MONI_MODE_NULL 0		//监视口无操作
#define MINI_MODE_MANUAL 1		//手工调试
#define MONI_MODE_NET 2			//网络监控
#define MONI_MODE_RETRAN 3		//转发模式
#define MONI_MODE_ADJUST 4		//调试模式

#define MONI_RETRAN_ADR 0xfe

	typedef struct {
		unsigned char mode;		//模式
		unsigned char b_retran_up;		//上行转发
		unsigned char b_retran_down;	//下行转发
		unsigned char rev[1];	//保留
		USART_T *p_usart_moni;	//串口指针
		RING_BUF_T rb;
	}USART_MONI_T;

#endif
