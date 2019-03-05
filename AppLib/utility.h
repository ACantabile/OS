#ifndef __UTILITY_H__
#define __UTILITY_H__

#include "prj_config.h"

#ifdef USE_STM32F10X
	#include "stm32f10x.h"
#endif

#ifdef USE_STM32F0XX
	#include "stm32f0xx.h"
#endif

//------------------------------------------------------------------------------
	typedef struct{
		unsigned int size;
		unsigned int used_size;
		unsigned char buf[DATA_POOL_SIZE];
	}DATA_POOL_T;

#ifdef UTILITY_MAIN		
	DATA_POOL_T g_data_pool={DATA_POOL_SIZE};
	volatile unsigned long cnt_1ms __attribute__((section(".ARM.__at_0x20000000")));
	volatile unsigned long cnt_1ms_H __attribute__((section(".ARM.__at_0x20000004")));
	unsigned long delay_k __attribute__((section(".ARM.__at_0x20000008")));
	DATA_POOL_T *g_p_data_pool __attribute__((section(".ARM.__at_0x2000000c")))=&g_data_pool;
	unsigned char reserved[16] __attribute__((section(".ARM.__at_0x20000010")));
#else
	extern DATA_POOL_T g_data_pool;
	extern volatile unsigned long cnt_1ms;
	extern volatile unsigned long cnt_1ms_H;
	extern unsigned long delay_k;
	extern DATA_POOL_T *g_p_data_pool;
	extern unsigned char reserved[];
#endif

	

//------------------------------------------------------------------------------
#define HWREG(x)                                                              \
        (*((volatile unsigned long *)(x)))
#define HWREGH(x)                                                             \
        (*((volatile unsigned short *)(x)))
#define HWREGB(x)                                                             \
        (*((volatile unsigned char *)(x)))
#define HWREGBITW(x, b)                                                       \
        HWREG(((unsigned long)(x) & 0xF0000000) | 0x02000000 |                \
              (((unsigned long)(x) & 0x000FFFFF) << 5) | ((b) << 2))
#define HWREGBITH(x, b)                                                       \
        HWREGH(((unsigned long)(x) & 0xF0000000) | 0x02000000 |               \
               (((unsigned long)(x) & 0x000FFFFF) << 5) | ((b) << 2))
#define HWREGBITB(x, b)                                                       \
        HWREGB(((unsigned long)(x) & 0xF0000000) | 0x02000000 |               \
               (((unsigned long)(x) & 0x000FFFFF) << 5) | ((b) << 2))

#define REG_BIT(x,b)\
		(((unsigned long)(x) & 0xF0000000) | 0x02000000 |                \
        (((unsigned long)(x) & 0x000FFFFF) << 5) | ((b) << 2))

//------------------------------------------------------------------------------
#define TstB(A,B)	((A & (1<<B))!=0)

#define SetGB(A) GPIO_SetBits(A)
#define ClrGB(A) GPIO_ResetBits(A)
#define CplGB(A) GPIO_WriteBit(A, (BitAction)(1 - GPIO_ReadInputDataBit(A)))
#define RdGB(A) GPIO_ReadInputDataBit(A)
#define WrGB(A,B) GPIO_WriteBit(A, B)
//------------------------------------------------------------------------------
//两个字节合成一个整数
extern unsigned short char_hl_short(unsigned char hi,unsigned lo);
//------------------------------------------------------------------------------
//交换一个字的高、低字节
extern unsigned short short_xch_hl(unsigned short i);
//------------------------------------------------------------------------------

//从源缓冲区拷贝n个short到目的缓冲区,交换高低字节
void short_copy_xch(void *t,void *s,int n,unsigned char b_xch);
//------------------------------------------------------------------------------
//short写入缓冲区
extern unsigned char * short_wr_buf(unsigned char *buf,unsigned short s);
extern unsigned short short_rd_buf(unsigned char *buf);
//------------------------------------------------------------------------------
//short交换高低字节后写入缓冲区
extern unsigned char * short_wr_buf_xch(unsigned char *buf,unsigned short s);
//缓冲区交换高低字节读入short
extern unsigned short short_rd_buf_xch(unsigned char *buf);
//------------------------------------------------------------------------------
//IAP 跳转
typedef  void (*Function_T)(void);
extern void jump_to_app(unsigned int ApplicationAddress);
#define m_jump_to_app_disallint(A) {\
	__disable_irq();	\
	NVIC->ICER[0]=0xffffffff;	\
	NVIC->ICER[1]=0xffffffff;	\
	NVIC->ICER[2]=0xffffffff;	\
	jump_to_app(A);\
}
//------------------------------------------------------------------------------
extern unsigned long delay_k;									//延时系数
extern void delay_us_set_k(unsigned long us);								//计算延时系数delay_k，并且延时us数
extern void delay_us(unsigned long us);							//通过死循环办法延时us数
extern unsigned long * calc_bitadr(void *x,unsigned char b);	//计算位地址
//------------------------------------------------------------------------------
//取HR的高，低字节
#define HRL(x) (x & 0xff)
#define HRH(x) (x >> 8)
//------------------------------------------------------------------------------
#define countof(A) (sizeof(A)/(sizeof(*A)))
#define LastMember(A)	A[countof(A)-1]
#define BufEnd(A)	A[sizeof(A)-1]
//-------------------- STM32F103_HW_ASM.S ----------------------
extern void U32_SetB(unsigned long *x,unsigned char b);
extern void U16_SetB(unsigned short *x,unsigned char b);
extern void U8_SetB(unsigned char *x,unsigned char b);
extern void U32_ClrB(unsigned long *x,unsigned char b);
extern void U16_ClrB(unsigned short *x,unsigned char b);
extern void U8_ClrB(unsigned char *x,unsigned char b);
extern void U32_CplB(unsigned long *x,unsigned char b);
extern void U16_CplB(unsigned short *x,unsigned char b);
extern void U8_CplB(unsigned char *x,unsigned char b);
extern unsigned char U32_TscB(unsigned long *x,unsigned char b);
extern unsigned char U16_TscB(unsigned short *x,unsigned char b);
extern unsigned char U8_TscB(unsigned char *x,unsigned char b);
extern unsigned short short_xch_hl(unsigned short i);
extern unsigned char tst_bit(void *x,unsigned char b);

typedef unsigned long time_ms_T;
extern void left_ms_set(time_ms_T *p,time_ms_T val);	//设置延时比较点
extern unsigned long left_ms(time_ms_T *p);		//计算剩下的ms数，过延时点，返回0
//-----------------------------------------------------------------------------------
#define GET_JSON_1FS_FAIL	0
extern int get_json_1fs(char *buf,int buf_size,char *s,char *name);
#define STR_TO_HEX_FAIL 0
#define STR_TO_HEX_SUCCESS 1
extern int str_to_hex(unsigned char *buf,unsigned int buf_size,char *s);
extern int hexchar_btye(unsigned char ch);
extern void hex_to_str(char *str_buf,unsigned char *hex_buf,unsigned int hex_n);
extern void str2lwr(char *p);
extern void str2upr(char *p);
//-----------------------------------------------------------------------------------
	typedef struct {
		unsigned short year;	//1970-9999
		unsigned char month;	//1-12
		unsigned char day;		//1-31
		unsigned char hour;		//0-23
		unsigned char min;		//0-59
		unsigned char sec;		//0-59
		unsigned char ctrl;
		unsigned int sec1970;	//从1970年以来的秒数
	}TIME_DATE_T;
	
	#define TIME_DATE_CTRL_NEW 0x01
	#define TIME_DATE_CTRL_SET 0x02
	
	#define TIME_DATA_SUCCESS			0
	#define TIME_DATA_FAIL				-1
extern int data_time_int(TIME_DATE_T *p);
extern void int_date_time(TIME_DATE_T *p);
//-----------------------------------------------------------------------------------
extern int Dword2Str(char *s,unsigned long data,unsigned char i_nb,unsigned char dec_nb,unsigned char b_sign,unsigned char b_inv0);
extern void Short_BinStr(char *buf,unsigned short us);	
//-----------------------------------------------------------------------------------
extern unsigned char * DataPool_Get(unsigned int size);		
//-----------------------------------------------------------------------------------
//ASC码转换成整型，转换失败，输出dft
extern int asc2int_dft(char *p,int dft);
extern int digchar_btye(unsigned char ch);
	typedef unsigned __int64 U64_T;
	typedef __int64 S64_T;
//ASC码转换成64位整型，转换失败，输出dft
extern S64_T asc2s64_dft(char *p,S64_T dft);

#define ASC2INT_SUCCESS 1
#define ASC2INT_FAIL 0
//ASC码转换成整型，转换成功，输出数值，resault=1;转换失败，输出0，resault=0
extern int asc2int(char *p,int *resault);
	typedef unsigned __int64 U64_T;
	typedef __int64 S64_T;
//ASC码转换成64位整型，转换成功，输出数值，resault=1;转换失败，输出0，resault=0
extern S64_T asc2s64(char *p,int *resault);

//-----------------------------------------------------------------------------------
//函数传递表格式
	typedef struct{
		char *func_name;
		unsigned int func_adr;
	}FUNC_TB_T;
//从函数传递表里面搜索名字匹配的函数指针
extern unsigned int Get_Func(FUNC_TB_T *func_tb,const char *func_name);
//-----------------------------------------------------------------------------------
#define TST_BIT(A,bit) ((A & (1<<bit)) != 0)
#define U8_TstB(A,bit) ((A & (1<<bit)) != 0)
//-----------------------------------------------------------------------------------

	
		
	//typedef unsigned long Tout_CP_T;
	//typedef unsigned long Tout_ms_T;
	typedef unsigned long time_ms_T;

	
#endif
