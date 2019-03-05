#ifndef __FLASH_STORE_H__
#define __FLASH_STORE_H__
//--------------------------------------------------------
//�汾�Ź���
#define FLASH_STORE_VER_H 200
//--------------------------------------------------------
/*
�洢ģʽ��Flash��nҳ�洢��ÿҳ�ڴ洢���ɸ����飬��һҳ�洢����ʱ�򣬴洢����һҳ����ɾ��ǰһҳ������
ÿ��������BLOCK_START_ID(short)��ͷ��������У����(short),������������
ÿ���ϵ��ȳ�ʼ�����ҳ����һ����Ч���ݵ�index����������
��û������������Ĭ�����ݲ�ɾ��ȫ��ҳ
���ݿ�ṹ
��ʼ��ID��CHKSUM��������

ʹ�÷�����
	//------------------------------------------------------------------------------------------------
	enum{
		CHZ_FDA_STA_ID,			//�������ID
		CHZ_FDA_STA_ID_NB
	};
	
	typedef struct{					//���洢�Ľṹ�嶨�壬ע�����word����
		unsigned char s_lock;
		unsigned char sw;
	}CHZ_FDA_STA_T;
	
	
	PR_CHZ_EXT CHZ_FDA_STA_T g_chz_fda_sta;		//�������ʵ��
	#define  0					
	
	PR_CHZ_EXT FLASH_STORE_DATA_INDEX_T g_chz_flash_data_index[CHZ_FDA_STA_ID_NB];		//��������
	PR_CHZ_EXT FLASH_STORE_T g_chz_flash_store;							//����洢���ṹ��
	
	
	#define CHZ_FLASH_STORE_ADR 0x801C000								//������flash�������ʼ��ַ
	#define CHZ_FLASH_STORE_SIZE 0x3000									//����洢flash�Ĵ�С

	//------------------------------------------------------------------------------------------------
	
	m_flash_data_index_reg(g_chz_flash_data_index[CHZ_FDA_STA_ID],g_chz_fda_sta);	//�ѱ���ע�ᵽ����

	//flash�洢��ʼ��
	if(m_flash_store_init(&g_chz_flash_store,CHZ_FLASH_STORE_ADR,CHZ_FLASH_STORE_SIZE,g_chz_flash_data_index) != FLASH_STORE_INIT_SUCCESS)
	{																		//��ȡ���ɹ���д��Ĭ������
		flash_store_wr(&g_chz_flash_store,CHZ_FDA_STA_ID);
	}
	else
	{
		g_chz_state.s_lock=g_chz_fda_sta.s_lock;							//��ȡ�ɹ�����Ϣ����
		g_chz_state.sw=g_chz_fda_sta.sw;
	}

*/
#define FLASH_STOER_START_ID 0x43		//���ݿ���ʼ��
#define FLASH_STOER_CHKSUM_ORG 0x435A		//У������ʼ����


typedef struct {				//��Ӧid��������Ϣ
	unsigned short ofs;		//��Ӧ��ǰ�ĵ�ַ�ڱ�ҳ��ƫ����
	unsigned short data_size;	//���������С
	void *buf;					//��Ӧ���ݽṹ��
}FLASH_STORE_DATA_INDEX_T;

typedef struct {			//�洢ͷ�ṹ
	unsigned short id;
	unsigned short sum;
}FLASH_STORE_HEAD_T;

typedef unsigned int FLASH_STORE_PAGE_SERIAL_T;
#define FLASH_STORE_DATAORG_OFS sizeof(FLASH_STORE_PAGE_SERIAL_T)

//����������ʼ����
#define m_flash_data_index_reg(A,B) A.buf=&B;A.data_size=sizeof(B)

//-----------------------------------------------------------------
typedef struct {
	unsigned int page_str_adr;		//��ʼ��ַ
	unsigned int store_size;		//�洢��С
	unsigned int serial;			//���к�
	unsigned short page_size;		//ÿҳ����
	unsigned short cur_page;		//��ǰҳ
	unsigned short next_ofs;		//��һ����д��ƫ����
	unsigned char page_n;			//һ���м�ҳ
	unsigned char id_n;				//һ���м���ģ��
	FLASH_STORE_DATA_INDEX_T *pdi;	//ָ��������������
}FLASH_STORE_T;


#define FLASH_STORE_INIT_SUCCESS 0
#define FLASH_STORE_INIT_NODATA -1
extern int flash_store_init(FLASH_STORE_T *p,unsigned int page_str_adr,unsigned int store_size,FLASH_STORE_DATA_INDEX_T *pbi,unsigned char block_index_n);
#define m_flash_store_init(A,B,C,D) flash_store_init(A,B,C,D,countof(D))
//-----------------------------------------------------------------
extern int flash_store_search_index(FLASH_STORE_T *p);

extern int flash_store_wr(FLASH_STORE_T *p,unsigned short id);


#endif
