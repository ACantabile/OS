#include "event_oper.h"

void event_oper(EVENT_OPER_T *p,EVENT_BITBUF_T input,unsigned char cnt_1s_in)
{
	unsigned char sec_pass,thre;
	int i,j,mulp,val;
	unsigned int bit_shift;
	
	if(cnt_1s_in != p->cnt_1s_last)
	{
		sec_pass = cnt_1s_in - p->cnt_1s_last;		//过去的秒数
		p->cnt_1s_last = cnt_1s_in;
		//----------------------------------------------------------
		//通过积分器滤波，产生实际值val
		bit_shift=1;
		
		thre=p->integral_thre;
		mulp=p->integral_false_mulp;
		if(mulp==0)
		{
			mulp=1;
		}
		val=p->val;
		for(i=0;i<EVENT_OPER_BITS;i++)
		{
			j=p->integral[i];		//得到当前通道的积分量
			
			if(input & bit_shift)
			{						//有效积分
				j += sec_pass;
				if(j >= thre)
				{
					j=thre;			//达到有效门限
					val |= bit_shift;
				}
			}
			else
			{						//反积分
				j -= (int)sec_pass * mulp;
				if(j<=0)
				{
					j=0;			//达到失效门限
					val &= (~bit_shift);
				}
			}
			
			p->integral[i]=j;
			bit_shift <<= 1;
	
		}
		p->val = val;
		
		p->mask &= val;		//撤销无效的掩码
		//----------------------------------------------------------
				
		i=val ^ p->val_last;
		
		p->val_last = val;
		
		p->ev_occ = i & val;		//刚产生的事件
		
		p->ev_dis = i & (~val);		//刚消失的事件
	}//if(p->cnt_1s_in != p->cnt_1s_last)
	p->out = p->val & p->mask;
}

void event_oper_mask(EVENT_OPER_T *p,EVENT_BITBUF_T mask)
{
	p->mask=mask;
}

void event_oper_init(EVENT_OPER_T *p,unsigned char integral_thre,unsigned char integral_false_mulp)
{
	p->integral_thre= integral_thre;
	p->integral_false_mulp=integral_false_mulp;
}
