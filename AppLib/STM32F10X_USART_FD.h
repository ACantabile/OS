#ifndef __STM32F10X_USART_FD_H__
#define __STM32F10X_USART_FD_H__

#include "utility.h"
#include "ring_buf.h"
//--------------------------------------------------------
//�汾�Ź���
#define STM32F10X_USART_VER_H 100
//--------------------------------------------------------


	#ifdef STM32F10X_USART_FD_MAIN
	 	#define STM32F10X_USART_FD_EXT
	#else
		#define STM32F10X_USART_FD_EXT extern
	#endif


//#define TR_BUF_LENGTH	1024	//(256+16)	//�շ���������С




//���ڳ�ʼ���ṹ��
typedef struct{
	unsigned char ulComm;			//�˿ں�
	unsigned char INT_PI;			//�ж����ȼ�
	unsigned short tx_buf_len;
	unsigned short rx_buf_len;
	unsigned long ulBaud;			//������
	unsigned long ulConfig;			//������
	GPIO_TypeDef * RS485_GPIO_BASE;	//RS485PIN�˿ڻ�ַ
	unsigned long RS485_GPIO_PIN;		//RS485PIN��
	
	unsigned char *tx_buf;			//���ͻ�����
	unsigned char *rx_buf;			//���ջ�����
}USART_INIT_T;
	
	

//ͨѶ�ṹ
typedef struct {
	unsigned long 	base;			//ָ��ͨѶ�˿ڵļĴ�����ַ
	unsigned short *RS485_GPIO_SEND;		//RS485���Ϳ���GPIO��ַ
	unsigned short *RS485_GPIO_RECV;		//RS485���տ���GPIO��ַ
	unsigned short 	RS485_PIN;				//RS485 PIN��
	unsigned short 	RS485_SOFT;				//RS485���͡����տ�������GPIO

	unsigned char sta;			//ͨѶ״̬
	unsigned char tm_1mS;		//�������ʱ��
	unsigned char tm_1mS_init;	//�������ʼ��ֵ
	unsigned char adr;			//�ӻ���ַ,������߶��ͨѶЧ��
	
	unsigned char baud_sta;	//���ڲ����ʵ�״̬λ
	unsigned char baud_nb;	//�����ʺ�

	unsigned short r_s;		//������ָ��
	unsigned short r_e;		//����βָ��
	unsigned short t_s;		//������ָ��
	unsigned short t_e;		//����βָ��
	
	unsigned short tx_buf_len;
	unsigned short rx_buf_len;
	unsigned char *tx_buf;
	unsigned char *rx_buf;
}USART_T;


#ifdef STM32F10X_USART_FD_MAIN
	const unsigned long Tb_BaudRate[]={2400,4800,9600,19200,38400,57600,115200};	//�����ʺŶ�Ӧ�Ĳ�����
	USART_T g_USART[USART_N];
#else
	extern unsigned long Tb_BaudRate[];
	extern USART_T g_USART[USART_N];
#endif

//sta,ͨѶ״̬�ĸ�λ�Ķ���
enum{
	B_USART_P485,		//BIT0��ʾRS485�������ŵĵ�ƽ���ԣ�=1��ʾ�ߵ�ƽ����
	B_USART_CHKADR,		//��Ҫ����ַ
	B_EV_USART_RX,		//�յ�һ�������¼�
	B_EV_USART_TX, 		//�����¼�
	B_USART_REC_1BYTE,	//�յ�һ���ֽڵ�����
	B_USART_ADR_OK,		//�Ѿ��յ�һ���Ϸ��ĵ�ַ
	B_USART_FD,			//ȫ˫����־
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
//adr:��ַ,=0�����κ����ݰ�,��0,ֻ���յ�ַ=0���ߵ�ַƥ������ݰ�
//---------------------------------------------
//baud_sta,ͨѶ����������Ӧ״̬λ����
//0-6�������ݰ�����
//��7λ���������Ƿ�����
#define B_USART_BAUD_LOCK 7	//baud_sta �� BIT7
//---------------------------------------------
#define BAUD_USE_NB 0
//baud_nb,ͨѶ�����ʺŶ���
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


extern unsigned char usart_tm_1mS(USART_T *p);	//����1mS��ʱ���ĺ���
extern void usart_isr(USART_T *p);		//ͨѶ�жϷ������
extern void usart_hd_send_start(USART_T *p,unsigned short n);	//���ݰ�˫�����ͺ���
	
#define usart_send_start usart_hd_send_start
	
extern int usart_send(USART_T *p,unsigned char *buf,unsigned short n);	//����n�ֽ�
extern int usart_recv(USART_T *p,unsigned char *buf,unsigned short n);	//�ӻ����������ֽڣ����n��
extern int usart_copy(USART_T *p,unsigned char *buf,unsigned short n);	//�ӻ����������ֽڣ����n��

extern void usart_rx_rst(USART_T *p);	//������ܻ���
extern unsigned int usart_copy_usart(USART_T *pd,USART_T *ps,unsigned char b_erase);	//��1�����ڿ������ݵ���һ������,b_erase=1ɾ��Դ��������
extern unsigned int usart_recv_bytes(USART_T *p);	//����ջ�����������ֽ���
extern unsigned int usart_rbuf_erase(USART_T *p,unsigned short n);//�ӽ��ջ�����ɾ��n������,����ɾ�����ֽ���
extern void usart_rx_clr(USART_T *p);	//������ܻ�����������
//------------------------------------------------------------------------------
//�򿪴��ں���
//�򿪴��ں�����ʹ��FIFO������6/8������4/8��ʹ���ж�
//�˿ںţ������ʣ��ֳ���ֹͣλ��У��λ��RS485���ƶ˿ڵ�ַ | ���Ƽ���
//���Ƽ���=1��ʾ�˿�=1�Ƿ���

//ulComm :�˿ں� 1-5��ulBaud:�����ʣ�ulConfig:�����֣�ulRS485:RS485�˿�λ��ַ��=0��ʾ��Ӳ��RS485�˿�
//INT_PI:����ʽ�ж����ȼ�
//ulConfig:ͨѶ����λ����
#define USART_CFG_RRN	2	//ÿ2λΪһ��������
#define USART_CFG_MASK	((1<<USART_CFG_RRN)-1)

#define USART_RX_UP		0		//Bit0:RXD��Ϊ�������Ǹ���
#define USART_RX_INF	1

#define USART_TX_PP		0		//Bit1:TXD��ΪPP����OD
#define USART_TX_OD		2

#define USART_RS485_N	(0 << USART_CFG_RRN)	//Bit2:RS485���Ƽ���
#define USART_RS485_P	(1 << USART_CFG_RRN)

#define USART_WL8		(0 << (USART_CFG_RRN*2))	//Bit4:����λ��
#define USART_WL9		(1 << (USART_CFG_RRN*2))

#define USART_PI_NONE	(0 << (USART_CFG_RRN*3))	//Bit6-7:У�鷽ʽ
#define USART_PI_EVEN	(1 << (USART_CFG_RRN*3))
#define	USAAT_PI_ODD	(2 << (USART_CFG_RRN*3))


#define USART_ST_1		(0 << (USART_CFG_RRN*4))	//Bit8-9:ֹͣλ
#define USART_ST_0_5	(1 << (USART_CFG_RRN*4))
#define USART_ST_2_0	(2 << (USART_CFG_RRN*4))
#define USART_ST_1_5	(3 << (USART_CFG_RRN*4))

#define USART_CFG_FD	(1 << (USART_CFG_RRN*5))	//Bit10:ȫ˫��
//----------------------------------------------
#define USART_8_N_1 (USART_WL8 | USART_ST_1)
#define USART_8_E_1 (USART_WL9 | USART_PI_EVEN | USART_ST_1)
#define USART_8_O_1 (USART_WL9 | USART_PI_ODD | USART_ST_1)

#define USART_7_E_1 (USART_WL8 | USART_PI_EVEN | USART_ST_1)
#define USART_7_O_1 (USART_WL8 | USART_PI_ODD | USART_ST_1)
//------------------------------------------------------------------------------
#define USART_RS485_GPIO_NULL	((GPIO_TypeDef *) 0)	//û�з��ͽ��շ�����ƿ�
#define USART_RS485_PIN_NULL	0	//û�з��ͽ��շ�����ƿ�
//----------------------------------------------
//�����ַ���"���ںţ�����λ��У��λ��ֹͣλ����˫��/ȫ˫�������PP��OD���������������գ��ж����ȼ���RS485DPIO��RS485PIN��RS485����"
//CfgStr:"1-5��7/8,n/e/o,1/2,h/d,p/o,u/f,0-15,a-g,0-f,p/n"
extern USART_T* Open_USART(const char *cfg_str,unsigned int ulBaud,unsigned short rx_buf_size,unsigned int tx_buf_size);
//extern USART_T* Open_USART(USART_INIT_T *pinit);
//------------------------------------------------------------------------------
//����USART��ַ,���ڶ༶ͨѶ�ĵ�ַʶ��,���ݰ��ĵ�һ���ֽ����׵�ַ,�����0,�����ȫ�������ݰ�
extern void Set_Usart_Adr(USART_T *p,unsigned char adr);
//------------------------------------------------------------------------------
//ֱ������USART�Ĳ�����,ǰ������������Ѿ���
extern void Set_Usart_BaudRate(USART_T *p,unsigned longBaudRate);
//------------------------------------------------------------------------------
#define USART_REC_OK	1
#define USART_REC_FORMATOK 2
//����������Ӧ����,����1��ʾ�ո�����������
//comm_ret�ǽ������񷵻ص�ֵ,=1���յ�һ����ȷ�����ݰ�,=2���յ�һ����ȷ�����ݸ�ʽ,����ַ�����Լ���
//�յ����ϵĲ���,������������
extern u8 Usart_Auto_BaudRate(u8 comm_ret,USART_T *p);
//------------------------------------------------------------------------------
//�趨֡��ʱֵ
//���ݲ������趨֡��ʱֵ
extern void Set_Usart_Tout_1mS(USART_T *p,unsigned longulBaud);
//------------------------------------------------------------------------------
//ʹ��ͨѶ���л�������״̬
extern void usart_rx_en(USART_T *p);
//ʹ��ͨѶ���л�������״̬,�����������
extern void usart_rx_rst(USART_T *p);
//------------------------------------------------------------------------------
//extern void usart_copy2tx_buf(USART_T *pd,unsigned char *ps,unsigned short n);
//------------------------------------------------------------------------------

#define MONI_MODE_NULL 0		//���ӿ��޲���
#define MINI_MODE_MANUAL 1		//�ֹ�����
#define MONI_MODE_NET 2			//������
#define MONI_MODE_RETRAN 3		//ת��ģʽ
#define MONI_MODE_ADJUST 4		//����ģʽ

#define MONI_RETRAN_ADR 0xfe

	typedef struct {
		unsigned char mode;		//ģʽ
		unsigned char b_retran_up;		//����ת��
		unsigned char b_retran_down;	//����ת��
		unsigned char rev[1];	//����
		USART_T *p_usart_moni;	//����ָ��
		RING_BUF_T rb;
	}USART_MONI_T;

#endif
