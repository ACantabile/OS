#define PR_PWM_MAIN
#include "PR_PWM.h"
#include "includes.h"
void PR_PWM_Task(void);
OSStk_T PR_PWM_Stk[64];
extern const unsigned short tb_sine[];
extern TIM_HandleTypeDef htim1;
unsigned char PR_PWM_Init(void)
{
	return(OSTaskCreate(PR_PWM_Task, (OSStk_T *)&LastMember(PR_PWM_Stk), OS_PI_PR_PWM,OS_TaskNoPar));
}

void PR_PWM_Task(void)
{
	g_pwm.amp = 32768;
	g_pwm.deg_inc = 5368709;
	while(1)
	{
		HAL_GPIO_TogglePin(GPIOC, LED_PWM);
		OSTimeDly (200);
	}
}

void PWM_Update(PWM_T *p)
{
	unsigned int i;
	unsigned int p1,amp;
	
	
	p->deg_cnt += p->deg_inc;
	
		
	p1=p->deg_cnt ;
		
	amp=p->amp;
	
	i=( p1>> (32-TAB_SINE_BIT) );
	
	i=(unsigned int)tb_sine[i];	//??0-32767

	i=(i*(PWM_PERIOD)+0x4000)>>15;	//+0.5
	
	i*=amp;
	
	i+=0x8000;	//+0.5

	i>>=17;
	if(p1>0x80000000)
	{
		i = (PWM_PERIOD/2)-i;
	}
	else
	{
		i = (PWM_PERIOD/2)+i;
	}
	//------------------------------------
	//死区补偿
	//i+=DEAD_TIME*3/4;	
	//------------------------------------
	//if(i>PWM_PERIOD_MAX)i=PWM_PERIOD_MAX;		//限幅,防止上管不导通
	//------------------------------------
	

		//TIM_SetCompare1(TIM1, 0);
		__HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, i);
//		TIM_SetCompare2(TIM1, 0);
//		TIM_SetCompare3(TIM1, 0);
	
	//DAC_SetChannel1Data(DAC_Align_12b_R,i+2048);
	//DAC_SetChannel2Data(DAC_Align_12b_R,i+2048);
}

