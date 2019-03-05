/** @file pid.h
 *  @version 1.0
 *  @date March 2019
 *
 *  @brief pid parameter initialization, position and delta pid calculate
 *         slightly modified, including addition of half integration.
 *  @copyright 2017 DJI RoboMaster. All rights reserved.
 *
 */
	
#ifndef __pid_H__
#define __pid_H__

#include "stm32f4xx_hal.h"

enum
{
  LLAST = 0,
  LAST,
  NOW,
  POSITION_PID,
  DELTA_PID,
};
typedef struct pid_t       //definition of pid_t
{
  float p;                 //param p
  float i;                 //param i
  float d;                 //param d

  float set;               //user set value
  float get;               //current detected value
  float err[3];            //the error value
  
  float NoInteKp;          //Kp when the error is too large 
  float pout;              //calculated pout
  float iout;              //calculated iout
  float dout;              //calculated dout
  float out;               //calculated overall output

  float input_max_err;     //input max err;
  float IntePar;           //the parameter of nonitegration
  float output_deadband;   //output deadband; 
  
  uint32_t pid_mode;       //currently 2 modes
  uint32_t max_out;        //
  uint32_t integral_limit; //maximum value to enable the integral

	
  float min_out;           //minimum output value 
  void (*f_param_init)(struct pid_t *pid, 
                       uint32_t      pid_mode,
                       uint32_t      max_output,
                       uint32_t      inte_limit,
                       float         p,
                       float         i,
                       float         d,
					   float         nip,
					   float         nii);
  void (*f_pid_reset)(struct pid_t *pid, float p, float i, float d,float nip,float nii);
 
} pid_t;

void PID_struct_init(
    pid_t*   pid,
    uint32_t mode,
    uint32_t maxout,
    uint32_t intergral_limit,

    float kp,
    float ki,
    float kd,
    float nip,
	float nii);

float pid_calc(pid_t *pid, float fdb, float ref);

#endif
