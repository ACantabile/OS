#ifndef __NET_EXT_H__
#define __NET_EXT_H__

typedef struct{
		char *pHost;				//改华东节点，HOST变长
		char *pPort;				//连接端口号
		//RING_BUF_T *pr;				//数据返回ring_buf的指针
		unsigned char net_sta;		//网络状态
		unsigned char func;			//命令
		char func_rsp;				//命令执行结果
		void *send_buf;				//发送指针
		unsigned short send_n;		//发送字节数	
	}GPRS_LINK_T;
	
#endif
