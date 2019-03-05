/** @file pid.c
 *  @version 1.0
 *  @date March 2019
 *
 *  @brief pid parameter initialization, position and delta pid calculate
 *         slightly modified, including addition of half integration
 *  @copyright 2017 DJI RoboMaster. All rights reserved.
 *
 */

#include "pid.h"
#include "math.h"


/**
  * @brief     Limit the output of PID value
  * @param[in] a: control pid struct
  * @param[in] ABS_MAX: measure feedback value
  * @retval    none
  */
void abs_limit(float *a, float ABS_MAX)
{	
  if (*a > ABS_MAX)
    *a = ABS_MAX;
  if (*a < -ABS_MAX)
    *a = -ABS_MAX;  
}

/**
  * @brief     Initiate a pid parameter
  * @param[in] pid_t empty pit para
  * @param[in] mode pid mode
  * @param[in] maxout max output value
  * @param[in] intergral_limit pid intergral limit
  * @param[in] kp p const
  * @param[in] ki i const
  * @param[in] kd d const
  * @param[in] nip: kp for Incomplete Integral
  * @param[in] nii: ki for Incomplete Integral
  * @retval    none
  */
static void pid_param_init(
    pid_t*   pid,
    uint32_t mode,
    uint32_t maxout,
    uint32_t intergral_limit,
    float    kp,
    float    ki,
    float    kd,
	float    nip,
	float    nii)
{

  pid->integral_limit = intergral_limit;
  pid->max_out        = maxout;
  pid->pid_mode       = mode;

  pid->p = kp;
  pid->i = ki;
  pid->d = kd;
  pid->IntePar = nii;
  pid->NoInteKp = nip;

}
/**
  * @brief     modify pid parameter when code running
  * @param[in] pid: control pid struct
  * @param[in] p/i/d: pid parameter
  * @param[in] nip: kp for Incomplete Integral
  * @param[in] nii: ki for Incomplete Integral
  * @retval    none
  */
static void pid_reset(pid_t *pid, float kp, float ki, float kd, float nip, float nii)
{
  pid->p = kp;
  pid->i = ki;
  pid->d = kd;
  pid->IntePar = nii;
  pid->NoInteKp = nip;
  
  pid->pout = 0;
  pid->iout = 0;
  pid->dout = 0;
  pid->out  = 0;
  
}

/**
  * @brief     calculate delta PID and position PID
  * @param[in] pid: control pid struct
  * @param[in] get: measure feedback value
  * @param[in] set: target value
  * @retval    pid calculate output 
  */
float pid_calc(pid_t *pid, float get, float set)
{
  static float Real_Kp = 0;
  pid->get = get;
  pid->set = set;
  pid->err[NOW] = set - get;

  if ((pid->input_max_err != 0) && (fabs(pid->err[NOW]) > pid->input_max_err))
      return 0;

  if (pid->pid_mode == POSITION_PID) //position PID
  {
      pid->dout = pid->d * (pid->err[NOW] - pid->err[LAST]);
      
	  if(pid->err[NOW] > pid->input_max_err || pid->err[NOW] < -pid->input_max_err)
	  {
		  Real_Kp = pid->NoInteKp;
		  pid->iout += pid->IntePar * pid->err[NOW];
	  }
	  else
	  {
		  Real_Kp = pid->p;
		  pid->iout += pid->i * pid->err[NOW];
	  }
	  pid->pout = Real_Kp * pid->err[NOW];
	  
	  
      abs_limit(&(pid->iout), pid->integral_limit);
      pid->out = pid->pout + pid->iout + pid->dout;
      abs_limit(&(pid->out), pid->max_out);
  }
  else if (pid->pid_mode == DELTA_PID) //delta PID
  {
      pid->pout = pid->p * (pid->err[NOW] - pid->err[LAST]);
      pid->iout = pid->i * pid->err[NOW];
      pid->dout = pid->d * (pid->err[NOW] - 2 * pid->err[LAST] + pid->err[LLAST]);

      pid->out += pid->pout + pid->iout + pid->dout;
      abs_limit(&(pid->out), pid->max_out);
  }
	
	 

  pid->err[LLAST] = pid->err[LAST];
  pid->err[LAST]  = pid->err[NOW];
  
  if ((pid->output_deadband != 0) && (fabs(pid->out) < pid->output_deadband))
    return 0;
  else
    return pid->out;

}
/**
  * @brief     initialize pid parameter
  * @retval    none
  */
void PID_struct_init(
    pid_t*   pid,
    uint32_t mode,
    uint32_t maxout,
    uint32_t intergral_limit,

    float kp,
    float ki,
    float kd,
	float nip,
    float nii
    )
{
  pid->f_param_init = pid_param_init;
  pid->f_pid_reset  = pid_reset;
	
  pid->f_param_init(pid, mode, maxout, intergral_limit, kp, ki, kd, nip, nii);
  pid->f_pid_reset(pid, kp, ki, kd, nip, nii);
	
	pid->min_out=0;
}
