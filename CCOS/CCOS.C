#define CCOS_GLOBALS
#include "CCOS.H"

//----------------------------------------------------------------------
//版本号管理
#define CCOS_VER_C 204

	#if CCOS_VER_C != CCOS_VER_H
		#ERROR
	#endif
//----------------------------------------------------------------------

//=============================================================================
//初始化任务系统
void OSInit(void)
{
	int i;
	OS_I_DisAll();
	
	
	
#ifdef OSTimeSlices_EN
	
	i=OS_TASK_TimeSlices;
	OSSys.OSTimeSlices_Pri=i;		//参与时间片调度的起始优先级
	OSSys.OSTimeSlices_PriCur=i;	//参与时间片调度的当前优先级
	OSSys.OSTimeSlices_Tbl= ((OSRdy_T)(-1)) << i;	//设时间片调度任务就绪登记表，以便后续程序判断任务是否属于时间片调度任务
	
#endif
	
	OSSys.OSRunning=0;							//运行标志清零
	OSSys.OSRdytbl=mPri_Tbl(OS_TASK_MAX);		//就绪表初始化，Idle任务永远就绪状态
	OSSys.OSPrioCur=OS_IDLE_PRI;				//当前任务设为系统任务
	OSSys.TCB_Len=sizeof(OS_Tcb[0]);			//任务控制块长度，用于汇编程序识别长度
	
	for(i=0;i<OS_TASK_MAX;i++)
	{
		OS_Tcb[i].Stk=0;						//堆栈设0表示没有该任务
	}
	
	//创建系统后台Idle任务	
	OS_Tcb[OS_TASK_MAX].Stk=OSTaskStkInit(OS_TaskIdle, &OSTaskIdleStk[OS_TASK_IDLE_STK_SIZE-1]);	//初始Idle化任务堆栈
}
//=============================================================================
//创建任务
//输入参数：任务入口指针，优先级
//返回：操作结果：成功，超出最大优先级，优先级冲突
OSErr_T OSTaskCreate (void (*task)(void), OSStk_T *Stk_Top,OSPri_T Pri,OSTaskOpt_Par_T O_Par) 
{
	struct OS_TCB *p;
	
	if(Pri>=OS_TASK_MAX)
	{
		return(OS_ERR_CREATE_TASK_MAX);			//超过最大任务数
	}
	if(OS_Tcb[Pri]. Stk!=0)
	{
		return(OS_ERR_CREATE_TASK_PRI);			//优先级冲突
	}
	
	p=&OS_Tcb[Pri];								//初始化对应的控制块
#ifdef OSTimeSlices_EN								
	if(O_Par & OS_TaskOpt_TimeSlices)			//最高8位选项类型
	{											//时间片调度任务
		p->TimeSlices=O_Par & OS_TaskPar_Mask;	//时间片设置
	}
#endif
	
	p->Stk=OSTaskStkInit(task, Stk_Top);		//初始化任务堆栈
	p->EventCnt=0;								//事件清零
	p->Delay=0;									//延时清零
	OS_SetRdy(Pri);								//设就绪表
	
	return(OS_ERR_CREATE_TASK_OK);				//返回成功
}
//=============================================================================
//根据优先级设就绪表
void OS_SetRdy(OSPri_T pri)
{
	OSRdy_T i;
	i=mPri_Tbl(pri);	//搜寻最高优先级使用计算前导零的指令，所以用最高位标志最高优先级
	OSSys.OSRdytbl |= i;
}
//=============================================================================
//清相应优先级就绪表
void OS_ClrRdy(OSPri_T pri)
{
	OSRdy_T i;
	i=~mPri_Tbl(pri);	//搜寻最高优先级使用计算前导零的指令，所以用最高位标志最高优先级
	OSSys.OSRdytbl &= i;
#ifdef OSCHK_RunTime_EN
	OS_Tcb[pri].RunTime=0;	//从就绪表清除，同时清零有效就绪运行时间
#endif
}
//=============================================================================
#ifdef OS_ChkLowRdy_EN
	//检查是否存在低优先级的任务就绪情况
	//返回比当前任务优先级低的就绪表，Idle任务位除外
	OSRdy_T OS_ChkLowRdy(void)
	{
		OSRdy_T i;
		
		i = ((OSRdy_T)(-1) - OSRDY_H) << (OSSys.OSPrioCur);			//0xfffffffe << (OSSys.OSPrioCur);
		i &= ~(mPri_Tbl(OS_IDLE_PRI));								//去掉Idle任务
		i &= OSSys.OSRdytbl;
		return(i);
	}
#endif
//=============================================================================
//初始化堆栈
//异常发生时，CPU自动依次把xPSR, PC, LR, R12以及R3‐R0由硬件自动压入适当的堆栈中
//模拟这个过程已经发生过了,压入16个寄存器
OSStk_T *OSTaskStkInit (void(*task)(void), OSStk_T  *Stk_Top)
{
    OSStk_T *stk;
                                                                        
    stk = Stk_Top;                          //  装载栈顶
                                                      
	                                     //  模拟成异常，自动把寄存器压栈
    *(stk)   = (OSStk_T)0x01000000L;     //  xPSR                        */ 
    *(--stk) = (OSStk_T)task;            //恢复的PC值，首次就是任务入口
    
	/*
    *(--stk) = (OSStk_T)0xFFFFFFFEL;     //  R14 (LR) 如果使用就会产生fault异常
                                                                       
    *(--stk) = (OSStk_T)0x12121212L;     //R12
    *(--stk) = (OSStk_T)0x03030303L;     //R3
    *(--stk) = (OSStk_T)0x02020202L;     //R2
    *(--stk) = (OSStk_T)0x01010101L;     //R1
    *(--stk) = (OSStk_T)0;               //R0 : argument  输入参数
                                         //Remaining registers saved on
                                         //process stack
	                                     //剩下的寄存器保存到堆栈

    *(--stk) = (OSStk_T)0x11111111L;     //R11
    *(--stk) = (OSStk_T)0x10101010L;     //R10
    *(--stk) = (OSStk_T)0x09090909L;     //R9
    *(--stk) = (OSStk_T)0x08080808L;     //R8
    *(--stk) = (OSStk_T)0x07070707L;     //R7
    *(--stk) = (OSStk_T)0x06060606L;     //R6
    *(--stk) = (OSStk_T)0x05050505L;     //R5
    *(--stk) = (OSStk_T)0x04040404L;     //R4
	*/
	stk -= 14;		//R0-R12，R14（LR），第一次不必关心压栈内容
	return(stk);
}
//=============================================================================
//锁定任务切换
void  OSSchedLock (void)
{
	if(OSSys.OSLockNesting<OSLockNesting_MAX)                 //嵌套计数	
	{
		OSSys.OSLockNesting++;
	}
}
//=============================================================================
//解锁任务切换
void  OSSchedUnlock (void)
{
	if(OSSys.OSLockNesting !=0)
	{
		OSSys.OSLockNesting--;
	}
	if(OSSys.OSLockNesting==0)
	{
		OS_Sched();	//解锁任务切换，马上切至新的高优先级任务
	}
}
//=============================================================================
void  OSTimeDly (OSTick_T Ticks)
{
	struct OS_TCB *p;
#ifdef OSTimeSlices_EN
	OSDelay_VT *vp;
#endif
	
    if (Ticks > 0) 
	{
        p=&OS_Tcb[OSSys.OSPrioCur];
		OS_ENTER_CRI_S();
		p->Delay=Ticks;			//设延时数
		
	#ifdef OSTimeSlices_EN
		if(OSSys.OSPrioCur < OSSys.OSTimeSlices_Pri)
		{								//抢先式任务
			OS_ClrRdy(OSSys.OSPrioCur);	//从就绪表去掉
			OS_EXIT_CRI_S();
			OS_Sched();
		}
		else
		{								//时间片任务
			OS_EXIT_CRI_S();			//不加NOP，CPSIE I会被意外优化掉
			vp=&p->Delay;				//加这句后，CPSIE I也不会被意外优化掉
			while(*vp);					//死循环等待
		}		
	#else
		OS_ClrRdy(OSSys.OSPrioCur);	//从就绪表去掉
        OS_EXIT_CRI_S();
        OS_Sched();             	//切换任务
	#endif
		
		
    }
}
//=============================================================================
//1mS时钟滴答处理，对每个控制块进行时间计时并判断是否就绪
//在中断里面调用
void  OSTimeTick_Int (void)
{
	int i;
	struct OS_TCB *p;

    if (OSSys.OSRunning) 
	{
		p=OS_Tcb;
		//===========================================================================================
		//抢先式任务定时器中断开始
	#ifdef OSTimeSlices_EN
		for(i=0;i<OSSys.OSTimeSlices_Pri;i++)
	#else
		for(i=0;i<OS_TASK_MAX;i++)
	#endif
		{
			OS_I_DisAll();		//在中断程序里面，边界计数肯定=0，不采用边界进入函数以减少关中断时间
			if(p->Tout != 0)
			{
				p->Tout--;
			}
			if(p->Delay != 0)
			{
				if(--p->Delay==0)
				{						//延时时间到，置就绪表
					p->Event_trig=OS_TIMEOUT;
					OS_SetRdy(i);
				}
			}
		//-----------------------------------------
		#ifdef OSCHK_RunTime_EN
			else if(i==OSSys.OSPrioCur)
			{							//当前任务正在运行，运行时间+1
				if(p->RunTime < RunTime_MAX)
				{
					p->RunTime++;
				//---------------------------------
				#ifdef OSCHK_RunTimeMax_EN
					if(p->RunTime > p->RunTime_Max)
					{
						p->RunTime_Max=p->RunTime;
					}
				#endif
				//---------------------------------
				}
			}
		#endif
		//-----------------------------------------
			OS_I_EnAll();		//在中断程序里面，边界计数肯定=0，不采用边界进入函数以减少关中断时间
			p++;
		}
	//抢先式任务定时器中断结束
	//===========================================================================================
	
	//===========================================================================================
	//时间片调度任务定时器中断开始
	#ifdef OSTimeSlices_EN
		for(;i<OS_TASK_MAX;i++)
		{
			OS_I_DisAll();		//在中断程序里面，边界计数肯定=0，不采用边界进入函数以减少关中断时间
			if(p->Delay != 0)
			{
				p->Delay--;
			}
			
			if(i==OSSys.OSPrioCur)
			{							//当前任务正在运行，运行时间+1
				if(p->RunTime < RunTime_MAX)
				{
					p->RunTime++;
				//---------------------------------
				#ifdef OSCHK_RunTimeMax_EN
					if(p->RunTime > p->RunTime_Max)
					{
						p->RunTime_Max=p->RunTime;
					}
				#endif
				//---------------------------------	
					if(p->RunTime >= p->TimeSlices)	//当前有效运行时间超过预定时间片事件，切换到下一个时间片
					{
						OS_ClrRdy(i);							//清除当前的就绪状态
						if(++OSSys.OSTimeSlices_PriCur >= OS_TASK_MAX)
						{										//时间片就绪任务号循环+1
							OSSys.OSTimeSlices_PriCur = OSSys.OSTimeSlices_Pri;
						}
						OS_SetRdy(OSSys.OSTimeSlices_PriCur);	//下一个时间片任务就绪
					}
				}
			}
	
	
			OS_I_EnAll();		//在中断程序里面，边界计数肯定=0，不采用边界进入函数以减少关中断时间
			p++;
		}
	//时间片调度任务定时器中断结束
	//===========================================================================================
	#endif
    }
}
//=============================================================================
//激活对应优先级的事件
void OSEventPost (OSPri_T pri)
{
	struct OS_TCB *p;
	int i=0;						//V2.03
	
	
	OS_ENTER_CRI_S();
	
	p=&OS_Tcb[pri];
	
	if(p->Opt & OSTcbOpt_WEvent)	//2.01 增加等待状态以避免任务在延时中被事件打断
	{
		p->Opt &= ~OSTcbOpt_WEvent;
	#ifdef OSTimeSlices_EN
		if(pri < OSSys.OSTimeSlices_Pri)
		{
			OS_SetRdy(pri);			//不是时间片任务，设相应任务就绪
		}
	#else
		OS_SetRdy(pri);				//设相应任务就绪
	#endif
		
		
		p->Delay=0;					//延时器清零
		p->Event_trig=OS_NML;		//事件方式激活
		i=1;						//V2.03
	}
#ifdef OSTimeSlices_EN
	if(i==0 || pri > OSSys.OSPrioHighRdy || pri==OSSys.OSPrioCur || pri >= OSSys.OSTimeSlices_Pri)	//当前优先级比事件激活的优先级高，
		//或者正在当前的优先级运行，或者属于时间片调度任务，事件+1
#else
	if(i==0 || pri>OSSys.OSPrioHighRdy || pri==OSSys.OSPrioCur)	//当前优先级比事件激活的优先级高，或者正在当前的优先级运行，事件+1
#endif
	{									
		if(p->EventCnt < OSEvent_MAX)
		{
			p->EventCnt++;
		#ifdef OS_CHK_Event_Max_EN
			if(p->EventCnt>p->EventCnt_Max)
			{
				p->EventCnt_Max=p->EventCnt;
			}
		#endif
		}	               
	}
	OS_EXIT_CRI_S();
	
	OS_Sched();						//切换任务
}
//=============================================================================
//等待事件并挂起自己,可以设定超时时间
OSErr_T  OSEventPend (OSTick_T timeout)
{
	struct OS_TCB *p;
#ifdef OSTimeSlices_EN
	OSDelay_VT *vp_delay;
	OSEventCnt_VT *vp_event;
#endif	
	p=&OS_Tcb[OSSys.OSPrioCur];
	
	OS_ENTER_CRI_S();
	if(p->EventCnt != 0)
	{										//还有事件任务没有处理完，直接响应一次事件，事件-1
		p->EventCnt--;
		p->Delay=0;
		p->Event_trig=OS_NML;			//V2.03
		OS_EXIT_CRI_S();
		return(OS_NML);					//事件方式激活
	}
	else
	{												//没有待处理的事件
		p->Delay=timeout;							//设超时时间
		
		p->Opt |= OSTcbOpt_WEvent;					//2.01 增加等待状态以避免任务在延时中被时间打断
	#ifdef OSTimeSlices_EN
		if(OSSys.OSPrioCur < OSSys.OSTimeSlices_Pri)
		{												//不是时间片切换任务
			OS_ClrRdy(OSSys.OSPrioCur);					//清除就绪表
			OS_EXIT_CRI_S();
			OS_Sched();									//切换任务
		}
		else
		{												//时间片调度任务，做死循环等待
			OS_EXIT_CRI_S();
			vp_event=&p->EventCnt;   
			vp_delay=&p->Delay;
			while((*vp_event==0) || (*vp_delay != 0))
			{
				;
			}
			if(p->Event_trig==OS_NML)					//属于事件激活，事件数-1
			{
				if(p->EventCnt!=0)
				{
					OS_ENTER_CRI_S();
					p->EventCnt--;
					OS_EXIT_CRI_S();
				}
			}
		}
	#else
		OS_ClrRdy(OSSys.OSPrioCur);					//清除就绪表
		OS_EXIT_CRI_S();
		OS_Sched();									//切换任务
	#endif
		
		return(OS_Tcb[OSSys.OSPrioCur].Event_trig);	//获得事件相应状态
	}
}
//=============================================================================
//清除事件计数器
void OSClear_Event(OSPri_T Pri)
{
	OS_ENTER_CRI_S();
	OS_Tcb[OSSys.OSPrioCur].EventCnt=0;
	OS_EXIT_CRI_S();
}
//=============================================================================
//空闲任务
void  OS_TaskIdle (void)
{
    while(1) 
	{
        OSTaskIdleHook();	//用户自定义函数
    }
}
//=============================================================================
