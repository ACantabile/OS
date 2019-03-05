#include "key.h"

//每10-20mS执行一次，新的键码送入缓冲区
unsigned char key8_oper(KEY8_T *p,unsigned char key_input)
{
	unsigned char key_input_last,sta,sta_last;
	unsigned char r,i,j,k,tail;
	
	r=0;
	sta=p->sta;				//频繁使用的变量，采用动态变量过度效率更高
	sta_last=sta;			//保存上次实际值
	
	key_input_last=p->input_last;	//取出上次状态
	
	i = key_input & key_input_last;		//新状态与上次状态进行与操作，剩下的'1'表示两次都是'1'
	
	sta |= i;						//确认1的位加载到实际状态
	
	i = key_input | key_input_last;		//新状态与上次状态进行与操作，剩下的'0'表示两次都是'0'
	
	sta &= i;						//确认0的位加载到实际状态
	
	p->input_last = key_input;		//更新最后一次输入值
	
	p->sta = sta;					//存回防抖后的状态
	
	j = (sta ^ sta_last) & sta;		//j里面的内容表示从0跳变为1的位
	
	tail=p->code_tail;
	k=0x01;				//用于测试的位
	for(i=0;i<8;i++)	//循环8次，测试每一位
	{
		if(j & k)
		{
			p->code[tail]=(i+1);	//有新的键从0跳变到1，加入一个键码到缓冲区，键值>0
			tail = (++tail) & KEY8_CODE_POINT_MASK;	//尾指针循环增量
			if(tail == p->code_head)
			{
				break;	//循环遇到头指针，说明缓冲区已满，抛弃新来的按键码，结束判断
			}
			p->code_tail=tail;	//刷新尾指针
			r=1;	//有新按键
		}
		k <<= 1;	//测试位左移一位
	}
	return(r);
}

//从按键缓冲区里面获取一个键码，成功返回非零，无键码返回0
unsigned char get_key8(KEY8_T *p)
{
	unsigned char key_code=0;
	if(p->code_tail != p->code_head)
	{
		key_code=p->code[p->code_head];	//缓冲区有键码
		p->code_head = ++p->code_head & KEY8_CODE_POINT_MASK;	//调整头指针
	}
	return(key_code);		//返回键码
}
