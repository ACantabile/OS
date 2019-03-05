#define PR_KEY_MAIN
#include "Pr_key.h"


void Pr_Key_Init(void)
{
	//对GPIO进行初始化等
}


void Pr_key8(void)		//这个程序每10-20mS调用一次
{
	unsigned char input;
	//--------------------------------------------------
	//获取按键原始端口信息
	input=0;
	if(!B_KEY0_IN)		//低电平有效
	{
		input |= 0x01;
	}
	
	if(!B_KEY1_IN)
	{
		input |= 0x02;
	}
	
	if(!B_KEY2_IN)
	{
		input |= 0x04;
	}
	
	if(!B_KEY3_IN)
	{
		input |= 0x08;
	}
	
	if(!B_KEY4_IN)
	{
		input |= 0x10;
	}
	
	if(!B_KEY5_IN)
	{
		input |= 0x20;
	}
	
	if(!B_KEY6_IN)
	{
		input |= 0x40;
	}
	
	if(!B_KEY7_IN)
	{
		input |= 0x80;
	}
	//--------------------------------------------------
	key8_oper(&g_key,input);	//对按键进行处理
	//--------------------------------------------------
}


//键码的使用
void xxxxx(void)
{
	unsigned char key_code;

	key_code=get_key8(&g_key);

	switch(key_code)
	{
		case KEY_CODE0:
			//键码0的操作
			break;
		case KEY_CODE1:
			break;
		case KEY_CODE2:
			break;
		case KEY_CODE3:
			break;
		case KEY_CODE4:
			break;
		case KEY_CODE5:
			break;
		case KEY_CODE6:
			break;
		case KEY_CODE7:
			break;
		default:
			break;
	}
}
