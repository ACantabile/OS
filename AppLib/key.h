#ifndef __KEY_H__
#define __KEY_H__
	
	#define KEY8_CODE_POINT_MASK 0x07

	typedef  struct{
		unsigned int input_last;		//按键原始状态，每位代表每个按键
		unsigned int sta;			//经过防抖处理的按键信息，每位代表每个按键当前的有效状态
		unsigned char code_head;	//按键代码的头指针，指向最早进入缓存的按键代码
		unsigned char code_tail;	//按键代码的尾指针，指向最后进入缓存的按键代码的下一个位置
		unsigned char code[KEY8_CODE_POINT_MASK+1];		//按键代码缓存区，每产生一个新的按键信息，代码加入缓存
	}KEY8_T;
//----------------------------------------------------------------
//每10-20mS执行一次，新的键码送入缓冲区
extern unsigned char key8_oper(KEY8_T *p,unsigned char key_input);
//----------------------------------------------------------------
//从按键缓冲区里面获取一个键码，成功返回非零，无键码返回0
extern unsigned char get_key8(KEY8_T *p);
//----------------------------------------------------------------
#endif
