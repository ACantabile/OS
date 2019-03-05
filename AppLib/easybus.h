#ifndef __EASYBUS_H__
#define __EASYBUS_H__

#define EASYBUS_VER_H 102
		
#define HR_GET_ADR(A) (&A - &HR[0])
#define HR_GET_N(A,B)	(&B-&A+1)
		
#define RD_SEG_N 4
#define WR_SEG_N 4
		
	typedef struct{
			unsigned short adr;	//�����Ĵ�����ַ
			unsigned short n;		//�����Ĵ�������
		} EASYBUF_ADR_T;
		
	typedef struct {
		unsigned char des_adr;		//Ŀ���ַ
		unsigned short func;		//����
		unsigned char wr_seg_n;		//д�����
		unsigned char rd_seg_n;		//��������
		EASYBUF_ADR_T wr[WR_SEG_N];
		EASYBUF_ADR_T rd[RD_SEG_N];
		unsigned char *buf;			//ͨѶ������
		unsigned short buf_size;	//ͨѶ��������С
		unsigned short buf_n;		//ͨѶ�ֽ���
	}EASTBUS_PACK_T;
	
	typedef struct {
		unsigned char src_adr;		//�Լ��ĵ�ַ
		unsigned char b_master;		//=0:slv,=1:master
		unsigned short hr_size;		//HR��С
		unsigned short *hr;			//HRָ��
		EASTBUS_PACK_T pk;
	}EASYBUS_T;
	
	//ͨѶ�����ڻ��������ƫ����
	#define EASYBUS_OFS_DESADR		0
	#define EASYBUS_OFS_SRCADR		1
	#define EASYBUS_OFS_FUNC		2
	#define EASYBUS_OFS_RD_INDEX	4
	
	
	//�Լ���Ӧ��־
	#define EASYBUS_FUNC_SLV		0x8000

	#define EASYBUS_FUNC_WR_SEG_BIT 4		//д���λ��
	#define EASYBUS_FUNC_RD_SEG_BIT 4		//��ȡ��λ��
	
		#define EASYBUS_FUNC_WR_SEG_MASK ((1 << EASYBUS_FUNC_WR_SEG_BIT )-1)
		#define EASYBUS_FUNC_RD_SEG_SHIFT EASYBUS_FUNC_WR_SEG_BIT
		#define EASYBUS_FUNC_RD_SEG_MASK ((1 << EASYBUS_FUNC_RD_SEG_BIT )-1)
	
	
	#define EASYBUS_SUCCESS					0
	#define EASYBUS_ERR_NOTMYDATA			-1
	#define EASYBUS_ERR_ADR					-2
	#define EASYBUS_ERR_FUNC				-3
	#define EASYBUS_ERR_HRADR				-4
	#define EASYBUS_ERR_BUFSIZE 			-5
	#define EASYBUS_ERR_REC_N				-6
	#define EASYBUS_ERR_REC_CHKSUM 			-7
	#define EASYBUS_ERR_MASTER_REC_HRADR 	-8
	#define EASYBUS_ERR_MASTER_REC_HRN 		-9
	#define EASYBUS_ERR_WR_SEG 				-10
	#define EASYBUS_ERR_RD_SEG 				-11
	#define EASYBUS_ERR_MASTER_SEG_N 		-12
	
#define EASYBUS_BASE_LEN	6	//des_adr,src_adr,func,chksum	
		
	extern int easybus_set_buf(EASYBUS_T *p);
	extern int easybus_receive(EASYBUS_T *p);
#endif
