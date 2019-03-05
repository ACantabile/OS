#include "key.h"

//ÿ10-20mSִ��һ�Σ��µļ������뻺����
unsigned char key8_oper(KEY8_T *p,unsigned char key_input)
{
	unsigned char key_input_last,sta,sta_last;
	unsigned char r,i,j,k,tail;
	
	r=0;
	sta=p->sta;				//Ƶ��ʹ�õı��������ö�̬��������Ч�ʸ���
	sta_last=sta;			//�����ϴ�ʵ��ֵ
	
	key_input_last=p->input_last;	//ȡ���ϴ�״̬
	
	i = key_input & key_input_last;		//��״̬���ϴ�״̬�����������ʣ�µ�'1'��ʾ���ζ���'1'
	
	sta |= i;						//ȷ��1��λ���ص�ʵ��״̬
	
	i = key_input | key_input_last;		//��״̬���ϴ�״̬�����������ʣ�µ�'0'��ʾ���ζ���'0'
	
	sta &= i;						//ȷ��0��λ���ص�ʵ��״̬
	
	p->input_last = key_input;		//�������һ������ֵ
	
	p->sta = sta;					//��ط������״̬
	
	j = (sta ^ sta_last) & sta;		//j��������ݱ�ʾ��0����Ϊ1��λ
	
	tail=p->code_tail;
	k=0x01;				//���ڲ��Ե�λ
	for(i=0;i<8;i++)	//ѭ��8�Σ�����ÿһλ
	{
		if(j & k)
		{
			p->code[tail]=(i+1);	//���µļ���0���䵽1������һ�����뵽����������ֵ>0
			tail = (++tail) & KEY8_CODE_POINT_MASK;	//βָ��ѭ������
			if(tail == p->code_head)
			{
				break;	//ѭ������ͷָ�룬˵�����������������������İ����룬�����ж�
			}
			p->code_tail=tail;	//ˢ��βָ��
			r=1;	//���°���
		}
		k <<= 1;	//����λ����һλ
	}
	return(r);
}

//�Ӱ��������������ȡһ�����룬�ɹ����ط��㣬�޼��뷵��0
unsigned char get_key8(KEY8_T *p)
{
	unsigned char key_code=0;
	if(p->code_tail != p->code_head)
	{
		key_code=p->code[p->code_head];	//�������м���
		p->code_head = ++p->code_head & KEY8_CODE_POINT_MASK;	//����ͷָ��
	}
	return(key_code);		//���ؼ���
}
