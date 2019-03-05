#define PR_KEY_MAIN
#include "includes.h"
void PR_KEY_Task(void);
void Pr_key8(void);
void act_key(void);
OSStk_T PR_KEY_Stk[64];



unsigned char PR_KEY_Init(void)
{
	return(OSTaskCreate(PR_KEY_Task, (OSStk_T *)&LastMember(PR_KEY_Stk), OS_PI_PR_KEY,OS_TaskNoPar));
}

void PR_KEY_Task(void)
{
	while(1)
	{
		Pr_key8();
		act_key();
		OSTimeDly (10);
	}
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
	//--------------------------------------------------
	key8_oper(&g_key,input);	//对按键进行处理
	//--------------------------------------------------
}
void act_key(void)
{
	unsigned char key_code;

	key_code=get_key8(&g_key);

	switch(key_code)
	{
		case KEY_CODE0:
			HAL_GPIO_TogglePin(GPIOC, LED_KEY);
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
		default:
			break;
	}
}
