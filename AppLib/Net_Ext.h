#ifndef __NET_EXT_H__
#define __NET_EXT_H__

typedef struct{
		char *pHost;				//�Ļ����ڵ㣬HOST�䳤
		char *pPort;				//���Ӷ˿ں�
		//RING_BUF_T *pr;				//���ݷ���ring_buf��ָ��
		unsigned char net_sta;		//����״̬
		unsigned char func;			//����
		char func_rsp;				//����ִ�н��
		void *send_buf;				//����ָ��
		unsigned short send_n;		//�����ֽ���	
	}GPRS_LINK_T;
	
#endif
