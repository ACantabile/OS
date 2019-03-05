#ifndef __PR_PWM_H__
#define __PR_PWM_H__

	#ifdef PR_PWM_MAIN
	 	#define PR_PWM_EXT
	#else
		#define PR_PWM_EXT extern
	#endif
	
		
	#define LED_PWM GPIO_PIN_10	
		
	#define FREQ_MIN 10		//最小频率
	#define FREQ_DFT_MIN 20		//最小频率		
	#define FREQ_DFT 50		//最小频率
	#define FREQ_DFT_MAX 100
	#define FREQ_MAX 200	//最大频率
	#define DEAD_TIME 36	//死区时间
	#define TIM1_IT_PI 0	//最高优先级
	#define TAB_SINE_BIT 12	//4096	正弦表点数
	#define PWM_FREQ 40000
	#define PWM_PERIOD (168000000/PWM_FREQ)	//载波周期
	#define DEG_INC_ORG	(0x100000000/360)
	
		
	typedef struct{
		unsigned int deg_cnt;
		unsigned int deg_inc;
		unsigned int freq;
		unsigned int amp;
	}PWM_T;
	
	PR_PWM_EXT PWM_T g_pwm;
	
	PR_PWM_EXT unsigned char PR_PWM_Init(void);
	PR_PWM_EXT void PR_PWM_Task(void);
	PR_PWM_EXT void PWM_Update(PWM_T *p);
	

#endif
