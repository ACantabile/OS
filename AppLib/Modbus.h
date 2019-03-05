#ifndef __MODBUS_H__
#define __MODBUS_H__


#include "STM32F10X_USART_FD.h"
/*------------------------------------------------------------------------------
ModBus ���ļ�ʹ�÷���
1.��main.c�ļ���ͷ���ֶ���"#define MODBUS_HR_NB xxxx",ȷ��HR����
2.����֮�����ͷ�ļ�"arm_cc_lib.h",���ļ�Ҳ���԰��������ļ���ͷ�ļ�����
3.�ڴ��ڳ�ʼ�������м����´��루������Ҫɾ����
void USART_Configuration(void)
{
	modbus_init(10,MODBUS_HR_NB);
	Open_USART(1,38400,USART_WL9 | USART_PI_EVEN | USART_ST_1 | USART_RS485_N,RS485_1);
	Open_USART(2,38400,USART_WL9 | USART_PI_EVEN | USART_ST_1 | USART_RS485_N,RS485_2);
	Open_USART(3,9600,USART_WL9 | USART_PI_EVEN | USART_ST_1 | USART_RS485_N,0,0);
}
4.��250uS��ϵͳ��ʱ���ж�����������´���
	usart_tm_250us(&ST_USART1);
	usart_tm_250us(&ST_USART2);
	usart_tm_250us(&ST_USART3);
5.��main_loopѭ���������������
	task_usart1();
	task_usart2();
	task_usart3();
6.��Ϊ�ӻ��Ĵ��ж˿�������ʾ��
void task_usart1(void)
{
	
	if(task_modbus_slv(&ST_USART1,HR))
	{
		//һ�γɹ���ͨѶ��������Ҫ�Ĵ���
	}
}
7.��Ϊ�����Ĵ��ж˿�������ʾ��
void task_usart2(void)
{
	if(modbus_master2.sta==MD_MST_STA_READY)	//�ж��������������Ƿ����
	{
		//�Ե�Դģ��ͨѶ						//�Է��͵����ݽ�������
		modbus_master2.phr=(u16*)&pow_cmd.I_STATUS;
		modbus_master2.pwr=(u16*)&pow_cmd;
		modbus_master2.slv=10;
		modbus_master2.func=MD_FR_MHR_RDHR;
		//modbus_master2.func=MD_RD_HR;
		modbus_master2.da_adr=3;
		modbus_master2.da_n=3;
		modbus_master2.wr_adr=0;
		modbus_master2.wr_n=3;
		if(comm_pow_cnt<3)						//���ͼ���
		{
			comm_pow_cnt++;
		}
		else
		{
			b_comm_pow_ok=0;					//���ͼ�����������,˵�����ͳ�ʱ,�ñ�ֵ
		}
	}
	task_modbus_master(&ST_USART2,&modbus_master2,1);	//��ģʽ����
	
	if(modbus_master2.sta==MD_MST_STA_OK)
	{											//�յ���ȷ�Ļ�Ӧ
		modbus_master2.sta=MD_MST_STA_NULL;		//��״̬Ϊ��
		tm_send2=20;							//��ʱ20mS(���Ը�����Ҫ�޸�)
		b_comm_pow_ok=1;
		comm_pow_cnt=0;
	}
}
8.���жϷ�������м���
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
#define MODBUS_ERR_TIMEOUT      -1      //��ʱ
#define MODBUS_ERR_BYTELESS     -2      //�ֽ�����
#define MODBUS_ERR_CRC          -3      //CRC��
#define MODBUS_ERR_SLV          -4      //�ӻ���ַ��
#define MODBUS_ERR_FUNC         -5      //�ӻ������
#define MODBUS_ERR_BYTE         -6      //�����ֽ�����
#define MODBUS_ERR_BYTECOUNT    -7      //ByteCount��
#define MODBUS_ERR_ADR          -8      //��ַ��
#define MODBUS_ERR_DAT          -9      //���ݴ�
#define MODBUS_ERR_N            -10     //�ֽ�����
#define MODBUS_SLV_RETURN_NML	1
#define MODBUS_SLV_RETURN_EXT	2

//MODBUS �����ṹ��
typedef struct {
	unsigned char slv;		//�ӻ���ַ
	unsigned char func;		//����
	unsigned char b_xch;	//ѡ��
	unsigned char b_ext;	//״̬
	unsigned short hr_n;	//HR������
	unsigned short *phr;	//HR��ַ
	unsigned short da_adr;	//���ݵ�ַ
	unsigned short da_n;	//���ݸ���/��������
	unsigned short rww_adr;	//RW�����д���ַ
	unsigned short rww_n;	//RW�����д������
	}MODBUS_T;
	
	#define	MODBUS_ADR_BOADCAST 0x00
	#define MODBUS_SLV_IAP 0xff
	



typedef enum {
	MD_MST_STA_NULL,
	MD_MST_STA_OK,			//���Ͳ��������
	MD_MST_STA_READY,		//��һ֡��ɣ�������������
	MD_MST_STA_INPROCESS,	//���ڽ�����
	MD_MST_STA_SEND_ERR,	//���ʹ���
	MD_MST_STA_REC_ERR		//���մ���
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

#define MODBUS_SLV_NULL	0		//�޲���
#define MODBUS_SLV_OK	1		//������ȷ
#define MODBUS_SLV_FORMAT_OK	2	//��ʽ��ȷ,�������Լ������ݰ�
#define MODBUS_SLV_FORMAT_ERR	3	//��ʽ����
#define MODBUS_SLV_FUNC_ERR		4	//�������
//Modbus��ģʽ����;p:���ڽṹ��;phr:HR��ָ��,option:bit0=1ֻ��,=0��д
extern unsigned char task_modbus_slv(USART_T *p,unsigned short *phr,u16 option);

//Modbus��ģʽ����;p:���ڽṹ��;pm:��ģʽ����ṹ��ָ��,
//b_dir:�Ƿ�ֱ�ӿ���,=0��ȡ��д�����ݸ������ݵ�ַƫ�ƶ�.phr.pwr����,=1�������ݵ�ַֻ��д��.phr.pwr
#define MODBUS_MASTER_RW_DIR 1
#define MODBUS_MASTER_RW_HR 0
//extern void task_modbus_master(USART_T *p,struct ST_MODBUS_MASTER *pm,u8 b_dir);



//---------------------------------------------------------------------------
#define MODBUS_COIL(ADR)		((HR[(ADR)/16] & (0x01 << (ADR&0x0f)))!=0) 	//����COIL��ַ���ڵ�����
#define SET_MODBUS_COIL(ADR)	(HR[(ADR)/16] |= (0x01 << (ADR&0x0f)))		//��λCOIL
#define CLR_MODBUS_COIL(ADR)	(HR[(ADR)/16] &= ~(0x01 << (ADR&0x0f)))	//����COIL
#define CPL_MODBUS_COIL(ADR)	(HR[(ADR)/16] ^= (0x01 << (ADR&0x0f)))		//��תCOIL
//---------------------------------------------------------------------------
#define MODBUS_ADR(HRN) (unsigned short)(&HRN-HR)



#endif
