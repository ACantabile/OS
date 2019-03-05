#ifndef __PR_KEY_H__
#define __PR_KEY_H__

	#ifdef PR_KEY_MAIN
	 	#define PR_KEY_EXT
	#else
		#define PR_KEY_EXT extern
	#endif
	
#include "key.h"
	
	#define B_KEY0_IN XXXX		//定义输入端口
	#define B_KEY1_IN XXXX		//定义输入端口
	#define B_KEY2_IN XXXX		//定义输入端口
	#define B_KEY3_IN XXXX		//定义输入端口
	#define B_KEY4_IN XXXX		//定义输入端口
	#define B_KEY5_IN XXXX		//定义输入端口
	#define B_KEY6_IN XXXX		//定义输入端口
	#define B_KEY7_IN XXXX		//定义输入端口
		
	#define KEY_CODE0	1
	#define KEY_CODE1	2
	#define KEY_CODE2	3
	#define KEY_CODE3	4
	#define KEY_CODE4	5
	#define KEY_CODE5	6
	#define KEY_CODE6	7
	#define KEY_CODE7	8
		
	
	PR_KEY_EXT KEY8_T g_key;	//定义按键全局变量

#endif
