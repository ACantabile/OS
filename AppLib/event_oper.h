#ifndef __EVENT_OPER_H__
#define __EVENT_OPER_H__

typedef unsigned short EVENT_BITBUF_T;

#define EVENT_OPER_BITS (sizeof(EVENT_BITBUF_T)*8)

typedef struct {
	EVENT_BITBUF_T val;		//�˲���ʵ��ֵ
	EVENT_BITBUF_T val_last;	//�ϴ�ʵ��ֵ
	EVENT_BITBUF_T mask;		//����λ
	EVENT_BITBUF_T out;		//���ֵ
	EVENT_BITBUF_T ev_occ;	//���¼�����
	EVENT_BITBUF_T ev_dis;	//�¼���ʧ
	unsigned char integral[EVENT_OPER_BITS];	//������
	unsigned char integral_thre;				//��������Ч����
	unsigned char integral_false_mulp;			//�����������ֱ���
	unsigned char cnt_1s_last;
	}EVENT_OPER_T;


//���øú���֮ǰ��Ҫ�ȶԽṹ���input��cnt_1s_in��ֵ;
extern void event_oper(EVENT_OPER_T *p,EVENT_BITBUF_T input,unsigned char cnt_1s_in);
extern void event_oper_mask(EVENT_OPER_T *p,EVENT_BITBUF_T mask);
extern void event_oper_init(EVENT_OPER_T *p,unsigned char integral_thre,unsigned char integral_false_mulp);

#endif

