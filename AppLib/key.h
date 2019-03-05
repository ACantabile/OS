#ifndef __KEY_H__
#define __KEY_H__
	
	#define KEY8_CODE_POINT_MASK 0x07

	typedef  struct{
		unsigned int input_last;		//����ԭʼ״̬��ÿλ����ÿ������
		unsigned int sta;			//������������İ�����Ϣ��ÿλ����ÿ��������ǰ����Ч״̬
		unsigned char code_head;	//���������ͷָ�룬ָ��������뻺��İ�������
		unsigned char code_tail;	//���������βָ�룬ָ�������뻺��İ����������һ��λ��
		unsigned char code[KEY8_CODE_POINT_MASK+1];		//�������뻺������ÿ����һ���µİ�����Ϣ��������뻺��
	}KEY8_T;
//----------------------------------------------------------------
//ÿ10-20mSִ��һ�Σ��µļ������뻺����
extern unsigned char key8_oper(KEY8_T *p,unsigned char key_input);
//----------------------------------------------------------------
//�Ӱ��������������ȡһ�����룬�ɹ����ط��㣬�޼��뷵��0
extern unsigned char get_key8(KEY8_T *p);
//----------------------------------------------------------------
#endif
