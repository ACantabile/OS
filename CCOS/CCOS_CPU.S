;******************************** Copyright (c) ***************************************
;COS_VER_ASM_204
;**************************************************************************************
		IMPORT  OSSys				;系统控制区
		IMPORT	OS_Tcb				;任务控制块
;--------------------------------------------------
		EXPORT	OS_ENTER_CRITICAL
		EXPORT	OS_EXIT_CRITICAL
			
		EXPORT	OSIntEnter
		EXPORT	OSIntExit
		
		EXPORT  OSStartTask
        
		EXPORT	OS_Sched
		            
        EXPORT  PendSV_Handler		;中断入口
;***************************************************************************************        	

REG_PendSV_PRI    	EQU     0xE000ED22  ; PEND优先级寄存器地址，字节形式
NVIC_PendSV_PRI 	EQU     0xFF        ; 软件中断最低优先级

REG_ICSR		   	EQU     0xE000ED04  ; 中断控制寄存器
PENDSVSET_BIT	  	EQU     0x10000000  ; BIT28是触发软件中断的值.

;REG_SysTick_PRI    EQU     0xE000ED23  ; SysTick优先级寄存器地址，字节形式
;NVIC_SysTick_PRI   EQU		0xFF        ; 最低，0xff

;***************************************************************************************
;		系统控制变量索引
Rdytbl_OFS			EQU		0			;就绪表
PrioCur_OFS			EQU		4
PrioHighRdy_OFS		EQU		5
CriNesting_OFS		EQU		6
IntNesting_OFS		EQU		7
LockNesting_OFS		EQU		8
Running_OFS			EQU		9
TCB_Len_OFS			EQU		10
;					EQU		11
;---------------------------------------------------------------------------------------
;		控制块变量索引
TCB_Stk_OFS			EQU		0
;***************************************************************************************
		PRESERVE8 
			
        THUMB 

		AREA ||i.OS_ENTER_CRITICAL||, CODE, READONLY, ALIGN=3
;/***************************************************************************************
;* 函数名称: OS_ENTER_CRITICAL
;*
;* 功能描述: 进入临界区 
;*            
;* 参    数: None
;*
;* 返 回 值: None
;*         
;* 作　  者: 陈冲
;* 日  　期: 2016年10月15日
;*----------------------------------------------------------------------------------------
;*****************************************************************************************/ 

OS_ENTER_CRITICAL PROC
		      
		LDR 	R1, =OSSys	    

		CPSID   I                       	;关中断
		
		LDRB 	R0, [R1,#CriNesting_OFS]	;OSCriNesting++
		ADD  	R0, R0, #1
		STRB 	R0, [R1,#CriNesting_OFS]	; 
		
  		BX LR
		
		ENDP
;/***************************************************************************************
;* 函数名称: OS_EXIT_CRITICAL
;*
;* 功能描述: 退出临界区 
;*            
;* 参    数: None
;*
;* 返 回 值: None
;*         
;* 作　  者: 陈冲
;* 日  　期: 2016年10月15日
;*----------------------------------------------------------------------------------------
;*****************************************************************************************/

OS_EXIT_CRITICAL	PROC
		LDR     R1, =OSSys     
		LDRB    R0, [R1,#CriNesting_OFS]	;OSCriNesting--
		SUBS    R0, R0, #1					;已经改变了标志位
        STRB    R0, [R1,#CriNesting_OFS]	;不影响标志位
		;CMP     R0,  #0			        	; if OSCriNesting=0,enable                                 
		MSREQ  PRIMASK, R0					;如果CriNesting=0，执行PRIMASK=0相当于"CPSIE	I"
	    
		BX LR
		NOP
		ENDP

		AREA    |.text|, CODE, READONLY
;/**************************************************************************************
;//进入中断，嵌套加1


OSIntEnter	PROC
		;PUSH	{R0-R2}						;来自中断程序，R0-R3已经自动压栈
		LDR     R1, =OSSys
		LDRB	R0,[R1,#Running_OFS]
		CBZ		R0,OSIntEnter_EXIT
		
		;MRS     R2,PRIMASK					;保存中断屏蔽
		CPSID	I							;能运行到这里，中断肯定是开的，关中断
		
		LDRB	R0,[R1,#IntNesting_OFS]
		ADD		R0,R0,#1
		STRB	R0,[R1,#IntNesting_OFS]
		
		CPSIE	I
		;MSR     PRIMASK, R2				;恢复中断优先屏蔽
OSIntEnter_EXIT
		;POP		{R0-R2}
		BX		LR
		
		ENDP
;/**************************************************************************************
;* 函数名称: OSIntExit,OS_Sched,OS_Task_Sw,OSStartTask
;*
;* 功能描述: 任务切换         
;*
;* 参    数: None
;*
;* 返 回 值: None
;*         
;* 作　  者: 陈冲
;* 日  　期: 2016年10月15日
;/**************************************************************************************
;任务开始运行
OSStartTask		PROC
		CPSID	I							;关中断
		LDR		R0, =REG_PendSV_PRI     	;设 PendSV中断 优先级
        LDR		R1, =NVIC_PendSV_PRI
        STRB	R1, [R0]
		
		LDR     R1, =OSSys
		MOV		R0,#0

        MSR     PSP, R0                 	;设 PSP = 0 用于切换任务识别首次切换
        STRB	R0,[R1,#IntNesting_OFS]		;OSIntNesting=0
		STRB	R0,[R1,#LockNesting_OFS]	;OSLockNesting=0;
		SUB		R0,R0,#1
		STRB	R0,[R1,#PrioCur_OFS]		;OSPrioCur=0xFF,首次切换产生优先级不同效果
        MOV     R0, #1
        STRB    R0, [R1,#Running_OFS]		;OSRunning=1

		B		OS_Sched					;切换至就绪表上最高优先级
;-------------------------------------------------------------------------------
OSIntExit
		LDR     R1, =OSSys
		LDRB	R0,[R1,#Running_OFS]
		CBZ		R0,OSIntExit_EXIT
		
		CPSID	I	;能运行到这里，中断肯定是开的，关中断
		
		LDRB	R0,[R1,#IntNesting_OFS]		;OSIntNesting--
		SUB		R0,R0,#1
		STRB	R0,[R1,#IntNesting_OFS]
		
		B		OS_Sched_Chk_IntNesting
;--------------------------------------------------------------------------------
OS_Sched
		LDR		R1,=OSSys					;系统变量结构体指针，其余变量都用偏移量读取
		CPSID	I
		LDRB	R0,[R1,#IntNesting_OFS]
OS_Sched_Chk_IntNesting
		CBNZ	R0,OS_Sched_Exit
		
		LDRB	R0,[R1,#LockNesting_OFS]
		CBNZ	R0,OS_Sched_Exit
		;------------------------------------------------------
		LDR		R0,[R1,#Rdytbl_OFS]			;就绪表,计算最新的最高就绪优先级
		RBIT	R0,R0						;按位翻转
		CLZ		R0,R0						;求前导零个数
		STRB	R0,[R1,#PrioHighRdy_OFS]	;存入高优先级就绪变量
		;------------------------------------------------------
		LDRB	R2,[R1,#PrioCur_OFS]		;取当前运行任务优先级
		CMP		R0,R2
		BEQ		OS_Sched_Exit				;如果相等，不切换

OS_Task_Sw
        LDR     R0, =REG_ICSR		      	;触发PendSV异常
        LDR     R1, =PENDSVSET_BIT
        STR     R1, [R0]
OS_Sched_Exit
OSIntExit_EXIT
		CPSIE	I
        BX      LR
        
		ENDP
;/**************************************************************************************
;* 函数名称: PendSV_Handler
;*
;* 功能描述: PendSV_Handler 任务切换中断
;*
;* 参    数: None
;*
;* 返 回 值: None
;*         
;* 作　  者: 陈冲
;* 日  　期: 2016年10月15日
;*--------------------------------------------------------------------------------------
;***************************************************************************************/

;异常发生时，CPU自动依次把xPSR, PC, LR, R12以及R3‐R0由硬件自动压入适当的堆栈中

PendSV_Handler	PROC
		LDR		R1,=OSSys					;系统变量结构体指针，其余变量都用偏移量读取
		LDRB	R2,[R1,#TCB_Len_OFS]		;控制块长度
		LDR		R3,=OS_Tcb					;控制块首指针

		CPSID   I							;关中断
		
		MRS     R0, PSP                 	;当前中断使用MSP,寄存器压入任务堆栈必须依靠R0,PSP是当前线程的堆栈指针
        CBZ		R0, OSPendSV_Chg_SP			;第一次调用，不需要压栈R4-R11
;----------------------------------------
		STMDB	R0!, {R4-R11}				;把R4-R11送入R0内容为地址的单元,相当于PUSH{R4-R11}
		LDRB	R4,	[R1,#PrioCur_OFS]
		MLA		R5,R2,R4,R3					;R5 = OSPrioCur * TCB_LENGTH + OS_Tcb
		STR     R0, [R5,#TCB_Stk_OFS]   	;保存切换出去的线程堆栈指针，OSTCBHighRdy->OSTCBStkPtr = PSP
;-------------------------------------------------------------------------------------------
OSPendSV_Chg_SP								;上一个任务结束，将下一个任务的内容装载入寄存器
        LDRB    R4, [R1,#PrioHighRdy_OFS] 	;R4=新的任务号
		STRB	R4,	[R1,#PrioCur_OFS] 		;把新任务号赋给当前的优先级号 OSPrioCur = OSPrioHighRdy
		MLA		R5,R2,R4,R3					;R5 = OSPrioCur * TCB_LENGTH + OS_Tcb
											;R5指向新线程的区块
        LDR     R0, [R5,#TCB_Stk_OFS]   	;读取新任务的堆栈指针; PSP = OSTCBHighRdy->OSTCBStkPtr;
		
		LDMIA   R0!, {R4-R11}          		;相当于POP{R4-R11}

        MSR     PSP, R0                 	;装载新的PSP  load PSP with new process SP
        ORR     LR, LR, #0x04           	;指示中断返回使用PSP堆栈 ensure exception return uses process stack

		CPSIE	I
        BX      LR                      	;退出中断,切换至新任务

        ENDP

		ALIGN

;****************************************************************************************
;*  End Of File                                                     
;****************************************************************************************				
        END
        
        
