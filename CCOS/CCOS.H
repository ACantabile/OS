/******************************************************************************
CCOS是一个最精简的嵌入式操作系统，属于优先级抢先式操作系统
CCOS 2.00版本 
使用时用户文件只需包含CCOS.H，在CCOS_CFG.H里面做出相应的配置，在工程里面添加CCOS.C,CCOS_CPU.S即可


文件:CCOS.C,CCOS.H,CCOS_CPU.S,CCOS_CFG.H


基本功能：
1.可以创建1-32个优先级任务
2.具有带超时的事件触发功能
3.挂起延时功能
4.具有任务切换锁定、解锁功能
5.具有查询是否有地优先级任务就绪功能（可选）
6.具有计算有效就绪运行时间，及最大值的功能（可选）
7.具有计算事件被挂起的最大次数功能（可选）
V2.00
8.增加时间片调度任务功能（可选）
V2.01
增加控制字的选项字节，并加入等待时间状态，避免任务在延时中被事件打断
V2.02
增加OS_ETE,OS_ETT定义
V2.03
控制块增加超时计时器
V2.04
增加调度锁计数
******************************************************************************/
#ifndef __CCOS_H__
#define __CCOS_H__

//----------------------------------------------------------------------
//版本号管理
#define CCOS_VER_H 204
//----------------------------------------------------------------------

#ifdef   CCOS_GLOBALS
	#define  OS_EXT
#else
	#define  OS_EXT  extern
#endif
#include "CCOS_CFG.H"
/*****************************************************************************/
//进行预定义方便不同的CPU系统的移植
typedef unsigned int   	OSStk_T;		//堆栈类型
typedef unsigned int	OSRdy_T;		//就绪表类型

typedef unsigned short	OSTick_T;		//滴答定时器类型
typedef unsigned short	OSDelay_T;		//滴答定时器类型,设敏感性为了在时间片任务判断事件时不会被编译器优化掉
typedef volatile unsigned short	OSDelay_VT;		//滴答定时器类型,设敏感性为了在时间片任务判断事件时不会被编译器优化掉


//---------------------
typedef unsigned char	OSRunTime_T;	//有效就绪运行时间类型
#define RunTime_MAX	255
//---------------------
typedef unsigned char	OSRun_T;		//运行标志
typedef unsigned char	OSPri_T;		//优先级号类型
	
typedef unsigned short	OSSysOpt_Par_T;	//系统选项类型 选项+参数
typedef unsigned short	OSTaskOpt_Par_T;	//任务选项类型 选项+参数	

typedef unsigned char	OSIntNest_T;	//中断嵌套数

typedef unsigned char	OSCriticalNest_T;	//进入边界嵌套类型

typedef unsigned char	OSLockNest_T;	//任务锁定嵌套类型

typedef unsigned char	OSErr_T;		//错误类型

typedef unsigned char	OSTcbOpt_T;		//控制块选项

//---------------------
typedef unsigned char	OSEventCnt_T;	//控制块事件计数类型,设敏感性为了在时间片任务判断事件时不会被编译器优化掉
typedef volatile unsigned char	OSEventCnt_VT;	//控制块事件计数类型,设敏感性为了在时间片任务判断事件时不会被编译器优化掉
#define OSEvent_MAX 255
//---------------------
typedef unsigned char	OSEventTrig_T;	//控制块事件触发类型
typedef unsigned char	OSTcb_Len_T;	//控制块事件触发类型
typedef unsigned char	OSTask_Max_T;	//控制块事件触发类型
#define OSRDY_H		((OSRdy_T)0x01)		//就绪表的最高优先级
#define mPri_Tbl(A)	(OSRDY_H << A)
/*****************************************************************************/

#ifdef OSTimeSlices_EN
	#ifndef OSCHK_RunTime_EN
		#define OSCHK_RunTime_EN		//如果时间片功能使能，检测任务有效就绪运行时间必须使能
	#endif
#endif

#ifdef OSCHK_RunTimeMax_EN
	#ifndef OSCHK_RunTime_EN
		#define OSCHK_RunTime_EN		//如果检测任务有效就绪运行时间最大值使能，检测任务有效就绪运行时间必须使能
	#endif
#endif

/*****************************************************************************/
#define OS_IDLE_PRI OS_TASK_MAX	//最大任务号就是Idle任务
//-----------------------------------------------------------------------------
#define OS_TRUE 1							//运行
#define OS_FALSE 0							//停止

#define OS_NML 0							//OSEventPend函数事件触发
#define OS_TIMEOUT 1						//OSEventPend函数超时触发

#define OS_ERR_CREATE_TASK_OK  	0			//创建任务成功
#define OS_ERR_CREATE_TASK_MAX 	1			//超过最大任务数
#define OS_ERR_CREATE_TASK_PRI 	2			//优先级冲突
//=============================================================================

//创建任务选项定义
#define OS_TaskOpt_LSL	8
#define OS_TaskPar_Mask	0xff
#define OS_TaskNoPar 0
#define OS_TaskOpt_TimeSlices		(0x01 << OS_TaskOpt_LSL)		//时间片调度任务选项，表示这是个时间片调度的任务

//系统控制全局变量
//-----------------------------------------------------------------------------
OS_EXT  OSStk_T	OSTaskIdleStk[OS_TASK_IDLE_STK_SIZE];	//后台任务堆栈定义
//-----------------------------------------------------------------------------

#define OSTcbOpt_WEvent 0x01		//正在等待事件

//控制块结构体
struct OS_TCB {
	OSStk_T *Stk;					//4	#0	堆栈指针
	
	OSDelay_T Delay;				//2	延迟时间
	OSRunTime_T RunTime;			//1	有效就绪运行时间，从就绪表删除时清零
	OSRunTime_T RunTime_Max;		//1	有效就绪运行时间最大值
	
	OSTcbOpt_T Opt;					//1 选项
	OSEventCnt_T EventCnt;			//1	事件计数器
	OSEventTrig_T Event_trig;		//1	事件任务触发原因	
	OSEventCnt_T EventCnt_Max;		//1	事件计数器最大值
	
	OSDelay_T Tout;					//2	用于任务超时计算	V2.03
	
#ifdef OSTimeSlices_EN
	OSRunTime_T TimeSlices;			//1 时间片调度时，占用的时间片
	unsigned char rsv0;
	unsigned char rsv1;
	unsigned char rsv2;
#endif
};
OS_EXT struct OS_TCB OS_Tcb[OS_TASK_MAX+1];	//任务控制块,最后一个为后台IDLE任务控制块

#define OSLockNesting_MAX 255

//系统变量
struct OS_Sys{
	OSRdy_T OSRdytbl;					//4	#0	就绪表
	
	OSPri_T OSPrioCur;					//1	#4	当前优先级
	OSPri_T OSPrioHighRdy;				//1	#5	根据就绪表计算出来的最高优先级
	OSCriticalNest_T OSCriNesting;		//1	#6	进入边界次数
	OSIntNest_T OSIntNesting;			//1	#7	中断嵌套数
	
	OSLockNest_T OSLockNesting;			//1	#8	任务锁定嵌套数，暂时取0与1
	OSRun_T OSRunning;					//1	#9	运行指示
	OSTcb_Len_T TCB_Len;				//1	#10	控制块结构体的大小
	unsigned char rsv0;					//1 #11 暂时预留
	
	//以下变量不在汇编文件里使用，不需要在汇编文件里面索引
#ifdef OSTimeSlices_EN
	OSRdy_T OSTimeSlices_Tbl;			//时间片任务表
	OSPri_T OSTimeSlices_Pri;			//时间片任务当前正在执行的任务
	OSPri_T OSTimeSlices_PriCur;		//时间片任务中当前激活的任务
#endif
};

OS_EXT struct OS_Sys OSSys;

//=============================================================================
//外部调用函数
extern void OSInit(void);		//任务系统初始化
//创建任务,输入任务函数指针，堆栈顶部指针，优先级，选项+参数
extern OSErr_T OSTaskCreate (void (*task)(void), OSStk_T *Stk_Top,OSPri_T pri,OSTaskOpt_Par_T O_Par); 
extern void OSStartTask(void);			//启动操作系统
//-----------------------------------------------------------------------------
extern OSErr_T OSEventPend (OSTick_T timeout);	//等待事件并挂起
extern void OSEventPost (OSPri_T ev);				//触发事件，并判断任务切换
extern void OSClear_Event(OSPri_T Pri);		//清除事件计数器
#define OSClear_MyEvent() OSClear_Event(OSSys.OSPrioCur)
extern void OSTimeDly (OSTick_T ticks);			//延时函数

#define OS_ETE (OS_Tcb[OSSys.OSPrioCur].Event_trig==OS_NML)		//正常事件触发
#define OS_ETT (OS_Tcb[OSSys.OSPrioCur].Event_trig==OS_TIMEOUT)	//超时触发
//-----------------------------------------------------------------------------
#define OS_TOUT (OS_Tcb[OSSys.OSPrioCur].Tout==0)		//任务超时计数器回零	V2.03
#define OS_SetTout(A) OS_Tcb[OSSys.OSPrioCur].Tout=A	//设任务超时计数器	V2.03
#define OS_ClrTout() OS_Tcb[OSSys.OSPrioCur].Tout=0	//设任务超时计数器	V2.03
//-----------------------------------------------------------------------------
//用户使用与中断共用的跨界变量时，必须使用以下函数保护变量不被切断，函数必须成对使用
//尽量减少进入边界保护以后的代码，以减少中断被延迟的时间
//如果只是使用任务间的跨界变量，建议使用OSSchedLock()，OSSchedUnlock()函数，
//这样只是任务切换可能被延迟，但不影响中断
extern void OS_ENTER_CRITICAL(void);	//进入边界，具有嵌套功能
extern void OS_EXIT_CRITICAL(void);		//退出边界，具有嵌套功能
extern void OSSchedLock (void);			//锁定任务
extern void OSSchedUnlock (void);		//解锁任务并且换
//-----------------------------------------------------------------------------
#ifdef OS_ChkLowRdy_EN
	OSRdy_T OS_ChkLowRdy(void);				//检查是否存在低优先级的任务就绪情况
#endif
//-----------------------------------------------------------------------------
//中断程序相关
extern void  OSTimeTick_Int (void);		//1mS滴答函数
extern void  OSIntEnter (void);			//进入中断处理
extern void  OSIntExit (void);			//退出中断处理，并判断任务切换
//-----------------------------------------------------------------------------
//内部函数
//由于程序中没有嵌套使用OS_ENTER_CRITICAL与OS_EXIT_CRITICAL，因此可以简单定义这两个函数
#define OS_ENTER_CRI_S() 		__asm("cpsid i")	//__asm("nop;cpsid i")	//进入边界
#define OS_EXIT_CRI_S()			__asm("cpsie i")	//__asm("cpsie i;nop")	//退出边界,不加NOP这句在有些时候会被优化没了
#define OS_I_DisAll()			__asm("cpsid i")	//__asm("nop;cpsid i")
#define OS_I_EnAll()			__asm("cpsie i")	//__asm("cpsie i;nop")

extern OSStk_T *OSTaskStkInit (void(*task)(void), OSStk_T  *ptos);	//堆栈初始化

extern void OS_SetRdy(OSPri_T pri);		//设就绪表，激活任务
extern void OS_ClrRdy(OSPri_T pri);		//清就绪表，挂起任务
extern void OS_Sched (void);			//任务切换

extern void OS_TaskIdle (void);			//系统后台任务
/*************************************************************************************/
#endif


