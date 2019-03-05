#ifndef __PR_ADC_H__
#define __PR_ADC_H__

	#ifdef PR_ADC_MAIN
	 	#define PR_ADC_EXT
	#else
		#define PR_ADC_EXT extern
	#endif
	
		
	#define LED_ADC GPIO_PIN_12
	#define BUFFER_SIZE 4
	#define ADC1_TRIG_PERIOD	100	//200uS PWM				//ADC采样周期
	#define ADC1_PERIOD_CNT		(20000/ADC1_TRIG_PERIOD)	//一个正弦周期采样点
	#define U_MID_CNT_THRE 50
	#define U_MID_CNT_THRE0 25		//对中点采取5000点平均
	#define U_MID_CNT_THRE1 (U_MID_CNT_THRE0+U_MID_CNT_THRE)
	
	typedef struct{
		unsigned char ac;                        //Determine if the assessed voltage is AC
		unsigned int adc_cnt;                   //Used for counting of the 
		unsigned int adc_cnt_buf;
		unsigned int adc_cnt_thre;
		
		unsigned int vol;
		unsigned int vol_sum;					 //Sum of the voltage, used for calculation
		unsigned int vol_sq_sum;                 //Sum of the squared voltage, used for calculation
		unsigned int vol_sq_sum_buf;
		unsigned int vol_avg;                    //Average of the voltage
				
		int vol_mid_sum;				         //
		unsigned int vol_mid_cnt;           	 //
		int vol_mid;				             //the value of the mid point voltage
		unsigned char flag_vol_mid;		         //Flag indicates if the mid point is determined
		unsigned char flag_vol_pull;
	}ADC_T;
	
	PR_ADC_EXT ADC_T g_adc_1;
	PR_ADC_EXT ADC_T g_adc_2;
	
	PR_ADC_EXT unsigned int g_adc_raw[BUFFER_SIZE]; //Array obtain from the DMA, since there could be lots of adc groups, it's better to have the array seperated.
	
	
	PR_ADC_EXT unsigned char PR_ADC_Init(void);
	PR_ADC_EXT void PR_ADC_Task(void);
#endif
