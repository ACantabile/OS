#ifndef __EVENT_OPER_H__
#define __EVENT_OPER_H__

typedef unsigned short EVENT_BITBUF_T;

#define EVENT_OPER_BITS (sizeof(EVENT_BITBUF_T)*8)

typedef struct {
	EVENT_BITBUF_T val;		//滤波后实际值
	EVENT_BITBUF_T val_last;	//上次实际值
	EVENT_BITBUF_T mask;		//屏蔽位
	EVENT_BITBUF_T out;		//输出值
	EVENT_BITBUF_T ev_occ;	//新事件发生
	EVENT_BITBUF_T ev_dis;	//事件消失
	unsigned char integral[EVENT_OPER_BITS];	//积分器
	unsigned char integral_thre;				//积分器有效门限
	unsigned char integral_false_mulp;			//积分器反积分倍率
	unsigned char cnt_1s_last;
	}EVENT_OPER_T;


//调用该函数之前，要先对结构体的input与cnt_1s_in赋值;
extern void event_oper(EVENT_OPER_T *p,EVENT_BITBUF_T input,unsigned char cnt_1s_in);
extern void event_oper_mask(EVENT_OPER_T *p,EVENT_BITBUF_T mask);
extern void event_oper_init(EVENT_OPER_T *p,unsigned char integral_thre,unsigned char integral_false_mulp);

#endif

