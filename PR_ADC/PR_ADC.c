#define PR_ADC_MAIN
#include "PR_ADC.h"
#include "includes.h"
void PR_ADC_Task(void);
OSStk_T PR_ADC_Stk[64];

unsigned char PR_ADC_Init(void)
{
	g_adc_1.ac=0;
	g_adc_2.ac=1;
	g_adc_1.vol_sum=0;
	g_adc_2.vol_sum=0;
	g_adc_1.adc_cnt_thre=ADC1_PERIOD_CNT;
	return(OSTaskCreate(PR_ADC_Task, (OSStk_T *)&LastMember(PR_ADC_Stk), OS_PI_PR_ADC,OS_TaskNoPar));
}

void PR_ADC_Task(void)
{
	while(1)
	{
		OSEventPend(0);
		HAL_GPIO_TogglePin(GPIOC, LED_ADC);	
	}
}
