#ifndef __FLASH_STORE_H__
#define __FLASH_STORE_H__
//--------------------------------------------------------
//版本号管理
#define FLASH_STORE_VER_H 200
//--------------------------------------------------------
/*
存储模式：Flash分n页存储，每页内存储若干个区块，当一页存储满的时候，存储到下一页，并删除前一页的内容
每个区块以BLOCK_START_ID(short)开头，接着是校验码(short),再下面是数据
每次上电先初始化并找出最后一组有效数据的index，载入数据
若没有数据则载入默认数据并删除全部页
数据块结构
起始码ID，CHKSUM，数据体

使用范例：
	//------------------------------------------------------------------------------------------------
	enum{
		CHZ_FDA_STA_ID,			//定义变量ID
		CHZ_FDA_STA_ID_NB
	};
	
	typedef struct{					//待存储的结构体定义，注意必须word对齐
		unsigned char s_lock;
		unsigned char sw;
	}CHZ_FDA_STA_T;
	
	
	PR_CHZ_EXT CHZ_FDA_STA_T g_chz_fda_sta;		//定义变量实体
	#define  0					
	
	PR_CHZ_EXT FLASH_STORE_DATA_INDEX_T g_chz_flash_data_index[CHZ_FDA_STA_ID_NB];		//定义索引
	PR_CHZ_EXT FLASH_STORE_T g_chz_flash_store;							//定义存储主结构体
	
	
	#define CHZ_FLASH_STORE_ADR 0x801C000								//定义在flash里面的起始地址
	#define CHZ_FLASH_STORE_SIZE 0x3000									//定义存储flash的大小

	//------------------------------------------------------------------------------------------------
	
	m_flash_data_index_reg(g_chz_flash_data_index[CHZ_FDA_STA_ID],g_chz_fda_sta);	//把变量注册到索引

	//flash存储初始化
	if(m_flash_store_init(&g_chz_flash_store,CHZ_FLASH_STORE_ADR,CHZ_FLASH_STORE_SIZE,g_chz_flash_data_index) != FLASH_STORE_INIT_SUCCESS)
	{																		//读取不成功，写入默认数据
		flash_store_wr(&g_chz_flash_store,CHZ_FDA_STA_ID);
	}
	else
	{
		g_chz_state.s_lock=g_chz_fda_sta.s_lock;							//读取成功的信息处理
		g_chz_state.sw=g_chz_fda_sta.sw;
	}

*/
#define FLASH_STOER_START_ID 0x43		//数据块起始码
#define FLASH_STOER_CHKSUM_ORG 0x435A		//校验码起始数据


typedef struct {				//对应id的数据信息
	unsigned short ofs;		//对应当前的地址在本页的偏移量
	unsigned short data_size;	//数据区块大小
	void *buf;					//对应数据结构体
}FLASH_STORE_DATA_INDEX_T;

typedef struct {			//存储头结构
	unsigned short id;
	unsigned short sum;
}FLASH_STORE_HEAD_T;

typedef unsigned int FLASH_STORE_PAGE_SERIAL_T;
#define FLASH_STORE_DATAORG_OFS sizeof(FLASH_STORE_PAGE_SERIAL_T)

//变量索引初始化宏
#define m_flash_data_index_reg(A,B) A.buf=&B;A.data_size=sizeof(B)

//-----------------------------------------------------------------
typedef struct {
	unsigned int page_str_adr;		//起始地址
	unsigned int store_size;		//存储大小
	unsigned int serial;			//序列号
	unsigned short page_size;		//每页长度
	unsigned short cur_page;		//当前页
	unsigned short next_ofs;		//下一个可写的偏移量
	unsigned char page_n;			//一共有几页
	unsigned char id_n;				//一共有几种模块
	FLASH_STORE_DATA_INDEX_T *pdi;	//指向数据区块索引
}FLASH_STORE_T;


#define FLASH_STORE_INIT_SUCCESS 0
#define FLASH_STORE_INIT_NODATA -1
extern int flash_store_init(FLASH_STORE_T *p,unsigned int page_str_adr,unsigned int store_size,FLASH_STORE_DATA_INDEX_T *pbi,unsigned char block_index_n);
#define m_flash_store_init(A,B,C,D) flash_store_init(A,B,C,D,countof(D))
//-----------------------------------------------------------------
extern int flash_store_search_index(FLASH_STORE_T *p);

extern int flash_store_wr(FLASH_STORE_T *p,unsigned short id);


#endif
