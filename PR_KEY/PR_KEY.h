#ifndef __PR_KEY_H__
#define __PR_KEY_H__

	#ifdef PR_KEY_MAIN
	 	#define PR_KEY_EXT
	#else
		#define PR_KEY_EXT extern
	#endif
	
		
	#define LED_KEY GPIO_PIN_11	
	
	#define KEY_UP GPIO_PIN_15
	#define KEY_DOWN GPIO_PIN_4
	#define KEY_LEFT GPIO_PIN_2
	#define KEY_RIGHT GPIO_PIN_3
	#define KEY_OK GPIO_PIN_5
	#define KEY_CANCEL GPIO_PIN_2
		
	#define B_KEY0_IN HAL_GPIO_ReadPin(GPIOA, KEY_UP)		//定义输入端口
	#define B_KEY1_IN HAL_GPIO_ReadPin(GPIOC, KEY_DOWN)		//定义输入端口
	#define B_KEY2_IN HAL_GPIO_ReadPin(GPIOA, KEY_LEFT)		//定义输入端口
	#define B_KEY3_IN HAL_GPIO_ReadPin(GPIOA, KEY_RIGHT)		//定义输入端口
	#define B_KEY4_IN HAL_GPIO_ReadPin(GPIOC, KEY_OK)		//定义输入端口
	#define B_KEY5_IN HAL_GPIO_ReadPin(GPIOB, KEY_CANCEL)		//定义输入端口

		
	#define KEY_CODE0	1
	#define KEY_CODE1	2
	#define KEY_CODE2	3
	#define KEY_CODE3	4
	#define KEY_CODE4	5
	#define KEY_CODE5	6

		
	
	PR_KEY_EXT KEY8_T g_key;	//定义按键全局变量

	PR_KEY_EXT unsigned char PR_KEY_Init(void);
	PR_KEY_EXT void PR_KEY_Task(void);
	

#endif
