#ifndef __MODBUS_H__
#define __MODBUS_H__


#include "STM32F10X_USART_FD.h"
/*------------------------------------------------------------------------------
ModBus 库文件使用方法
1.在main.c文件开头部分定义"#define MODBUS_HR_NB xxxx",确定HR数量
2.在其之后包含头文件"arm_cc_lib.h",该文件也可以包含在主文件的头文件里面
3.在串口初始化函数中加以下代码（根据需要删减）
void USART_Configuration(void)
{
	modbus_init(10,MODBUS_HR_NB);
	Open_USART(1,38400,USART_WL9 | USART_PI_EVEN | USART_ST_1 | USART_RS485_N,RS485_1);
	Open_USART(2,38400,USART_WL9 | USART_PI_EVEN | USART_ST_1 | USART_RS485_N,RS485_2);
	Open_USART(3,9600,USART_WL9 | USART_PI_EVEN | USART_ST_1 | USART_RS485_N,0,0);
}
4.在250uS的系统定时器中断里面加入以下代码
	usart_tm_250us(&ST_USART1);
	usart_tm_250us(&ST_USART2);
	usart_tm_250us(&ST_USART3);
5.在main_loop循环里面加入任务函数
	task_usart1();
	task_usart2();
	task_usart3();
6.作为从机的串行端口任务函数示例
void task_usart1(void)
{
	
	if(task_modbus_slv(&ST_USART1,HR))
	{
		//一次成功的通讯，加入需要的代码
	}
}
7.作为主机的串行端口任务函数示例
void task_usart2(void)
{
	if(modbus_master2.sta==MD_MST_STA_READY)	//判断主机发送任务是否就绪
	{
		//对电源模块通讯						//对发送的内容进行设置
		modbus_master2.phr=(u16*)&pow_cmd.I_STATUS;
		modbus_master2.pwr=(u16*)&pow_cmd;
		modbus_master2.slv=10;
		modbus_master2.func=MD_FR_MHR_RDHR;
		//modbus_master2.func=MD_RD_HR;
		modbus_master2.da_adr=3;
		modbus_master2.da_n=3;
		modbus_master2.wr_adr=0;
		modbus_master2.wr_n=3;
		if(comm_pow_cnt<3)						//发送计数
		{
			comm_pow_cnt++;
		}
		else
		{
			b_comm_pow_ok=0;					//发送计数超过限制,说明发送超时,置标值
		}
	}
	task_modbus_master(&ST_USART2,&modbus_master2,1);	//主模式函数
	
	if(modbus_master2.sta==MD_MST_STA_OK)
	{											//收到正确的回应
		modbus_master2.sta=MD_MST_STA_NULL;		//置状态为空
		tm_send2=20;							//延时20mS(可以根据需要修改)
		b_comm_pow_ok=1;
		comm_pow_cnt=0;
	}
}
8.在中断服务程序中加入
void USART1_IRQHandler(void)
{
	usart_isr(&ST_USART1);
}

void USART2_IRQHandler(void)
{
	usart_isr(&ST_USART2);
}

void USART3_IRQHandler(void)
{
	usart_isr(&ST_USART3);
}
------------------------------------------------------------------------------*/


#define MODBUS_SLV_BASE_NB	5

typedef enum{
MD_FUNC_NULL,
MD_RD_COIL=1,
MD_RD_HR=3,
MD_FR_SCOIL=5,
MD_FR_SHR=6,
MD_FR_MCOIL=15,
MD_FR_MHR=16,
MD_FR_MHR_RDHR=23
} MD_FUNC;


#define MODBUS_FAIL				0
#define MODBUS_SUCCESS			1
#define MODBUS_SUCCESS_EXT		2
#define MODBUS_FORMAT_OK		0
#define MODBUS_ERR_TIMEOUT      -1      //超时
#define MODBUS_ERR_BYTELESS     -2      //字节数少
#define MODBUS_ERR_CRC          -3      //CRC错
#define MODBUS_ERR_SLV          -4      //从机地址错
#define MODBUS_ERR_FUNC         -5      //从机命令错
#define MODBUS_ERR_BYTE         -6      //接收字节数错
#define MODBUS_ERR_BYTECOUNT    -7      //ByteCount错
#define MODBUS_ERR_ADR          -8      //地址错
#define MODBUS_ERR_DAT          -9      //数据错
#define MODBUS_ERR_N            -10     //字节数错
#define MODBUS_SLV_RETURN_NML	1
#define MODBUS_SLV_RETURN_EXT	2

//MODBUS 常量结构体
typedef struct {
	unsigned char slv;		//从机地址
	unsigned char func;		//命令
	unsigned char b_xch;	//选项
	unsigned char b_ext;	//状态
	unsigned short hr_n;	//HR的数量
	unsigned short *phr;	//HR地址
	unsigned short da_adr;	//数据地址
	unsigned short da_n;	//数据个数/数据内容
	unsigned short rww_adr;	//RW命令的写入地址
	unsigned short rww_n;	//RW命令的写入数量
	}MODBUS_T;
	
	#define	MODBUS_ADR_BOADCAST 0x00
	#define MODBUS_SLV_IAP 0xff
	



typedef enum {
	MD_MST_STA_NULL,
	MD_MST_STA_OK,			//发送并接收完成
	MD_MST_STA_READY,		//上一帧完成，可以启动发送
	MD_MST_STA_INPROCESS,	//正在进程中
	MD_MST_STA_SEND_ERR,	//发送错误
	MD_MST_STA_REC_ERR		//接收错误
} MD_MST_STA;


typedef enum{
	MODBUS_BIG_ENDIAN,
	MODBUS_LITTLE_ENDIAN
} MODBUS_HR_MODE_E;

extern void modbus_init(MODBUS_T *p,unsigned short *phr,unsigned short hr_n,MODBUS_HR_MODE_E  b_little_endian);
extern int modbus_slv_rec(MODBUS_T *p,unsigned char *rb,int n);
extern int modbus_slv_send(MODBUS_T *p,unsigned char *rb);
extern int modbus_master_send(MODBUS_T *p,unsigned char *rb);
extern int modbus_master_rec(MODBUS_T *p,unsigned char *rb,int n);

#define MODBUS_OT_RO	1
#define MODBUS_OT_RW	0

#define MODBUS_SLV_NULL	0		//无操作
#define MODBUS_SLV_OK	1		//操作正确
#define MODBUS_SLV_FORMAT_OK	2	//格式正确,但不是自己的数据包
#define MODBUS_SLV_FORMAT_ERR	3	//格式错误
#define MODBUS_SLV_FUNC_ERR		4	//命令错误
//Modbus从模式函数;p:串口结构体;phr:HR的指针,option:bit0=1只读,=0读写
extern unsigned char task_modbus_slv(USART_T *p,unsigned short *phr,u16 option);

//Modbus主模式函数;p:串口结构体;pm:主模式命令结构体指针,
//b_dir:是否直接控制,=0读取、写入内容根据数据地址偏移对.phr.pwr操作,=1忽略数据地址只接写入.phr.pwr
#define MODBUS_MASTER_RW_DIR 1
#define MODBUS_MASTER_RW_HR 0
//extern void task_modbus_master(USART_T *p,struct ST_MODBUS_MASTER *pm,u8 b_dir);



//---------------------------------------------------------------------------
#define MODBUS_COIL(ADR)		((HR[(ADR)/16] & (0x01 << (ADR&0x0f)))!=0) 	//返回COIL地址所在的内容
#define SET_MODBUS_COIL(ADR)	(HR[(ADR)/16] |= (0x01 << (ADR&0x0f)))		//置位COIL
#define CLR_MODBUS_COIL(ADR)	(HR[(ADR)/16] &= ~(0x01 << (ADR&0x0f)))	//清零COIL
#define CPL_MODBUS_COIL(ADR)	(HR[(ADR)/16] ^= (0x01 << (ADR&0x0f)))		//翻转COIL
//---------------------------------------------------------------------------
#define MODBUS_ADR(HRN) (unsigned short)(&HRN-HR)



#endif
