/*****************************************************************
 * File: ENTimer.c
 * Date: 2015/08/04 14:06
 * Copyright@2015-2025: MedRena, All Right Reserved
 *
 * Note:
 *
*****************************************************************/
#include "Timer.h"
#include "stm32f10x.h"	
#include "stm32f10x_tim.h"
#include "stm32f10x_flash.h"
#include "core_cm3.h"
#include "FreeRTOS.h"
#include "task.h"

#define ENTIMER_PERIOD_MS                   (1)          

#define ENTIMER_GROUP_CLK_IN_APB1           (1)
#define ENTIMER_GROUP_CLK                   (RCC_APB1Periph_TIM5)
#define ENTIMER_GROUP                       (TIM5)

#define ENTIMER_NIVC_CHNL                   (TIM5_IRQn)
#define ENTIMER_NIVC_PRIO_GROUP             (NVIC_PriorityGroup_4)
#define ENTIMER_NIVC_PREEMPTION_PRIO        (15)
#define ENTIMER_NIVC_SUB_PRIO               (0)

#define ENTimer_IRQHandler                  (TIM5_IRQHandler)

#define ENTIMER_NUM_MAX                     (10)   

typedef struct
{
    UINT32 ENTimer_Count;
    UINT32 ENTimer_Period;
    UINT32 ENTimer_RepeatTimes;
    ENTIMER_ISR_CB_PROC   pENTimerIsrCbProc;    
    UINT8 ENTimerStatus;
} stENTimer;

typedef enum
{
    TIMER_STATUS_UNCREATED = 0,            
    TIMER_STATUS_CREATED,                  
    TIMER_STATUS_RUNNING,                 
} E_STIMER_RUN_STATUS;


extern void xPortSysTickHandler( void );

static stENTimer gstENTimer[ENTIMER_NUM_MAX] = {0};

volatile UINT64 gSysTick;


void RCC_Configuration(void)
{  
	ErrorStatus HSEStartUpStatus;
	RCC_DeInit();
	RCC_HSEConfig(RCC_HSE_ON);
	HSEStartUpStatus = RCC_WaitForHSEStartUp();
	
	if (HSEStartUpStatus == SUCCESS)
	{
		FLASH_PrefetchBufferCmd(FLASH_PrefetchBuffer_Enable);
		FLASH_SetLatency(FLASH_Latency_2);
		RCC_HCLKConfig(RCC_SYSCLK_Div1);
		RCC_PCLK2Config(RCC_HCLK_Div1);
		RCC_PCLK1Config(RCC_HCLK_Div1);
#if defined APM32E103
		RCC_PLLConfig(RCC_PLLSource_HSE_Div1, RCC_PLLMul_15);//120M
#else
		RCC_PLLConfig(RCC_PLLSource_HSE_Div1, RCC_PLLMul_9);//72M
#endif
		RCC_PLLCmd(ENABLE);
		while (RCC_GetFlagStatus(RCC_FLAG_PLLRDY) == RESET)
		{}
		RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK);
		while (RCC_GetSYSCLKSource() != 0x08)
		{}
	}
//	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);
    
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_CRC, ENABLE);
}

void ENSysTickInit(void)
{
    RCC_Configuration();
	if(SysTick_Config(SystemCoreClock / 1000))
	while(1);
}

UINT64 ENSysTickGet(void)
{
	return gSysTick;
}

void ENSleep(UINT32 ms)
{
	vTaskDelay(ms);
}

void ENDelayus(UINT32 Time)
{
#if defined APM32E103
	int i = 16;
#else
	int i = 9;
	
#endif
	Time *= i;
	while(Time--);
}

/*******************************************************************************
* Function Name  : 
* Description    :  
* Input           : 
* Output         :
* Return         :
*******************************************************************************/

void ENTimerInit(void)
{
	TIM_TimeBaseInitTypeDef TIM_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
#if (ENTIMER_GROUP_CLK_IN_APB1 == 1)   
	RCC_APB1PeriphClockCmd(ENTIMER_GROUP_CLK,ENABLE);
#else
    RCC_APB2PeriphClockCmd(ENTIMER_GROUP_CLK,ENABLE);
#endif
	
	TIM_DeInit(ENTIMER_GROUP);

	TIM_InitStructure.TIM_ClockDivision		= TIM_CKD_DIV1;
	TIM_InitStructure.TIM_CounterMode		= TIM_CounterMode_Up;
	TIM_InitStructure.TIM_Period			= ENTIMER_PERIOD_MS*10-1;			
	TIM_InitStructure.TIM_Prescaler			= SystemCoreClock/10000-1;				//7199 计数频率为10K
	TIM_TimeBaseInit (ENTIMER_GROUP,&TIM_InitStructure);	
	
	TIM_ITConfig(ENTIMER_GROUP,TIM_IT_Update,ENABLE);

	NVIC_InitStructure.NVIC_IRQChannel						= ENTIMER_NIVC_CHNL; 
	NVIC_InitStructure.NVIC_IRQChannelCmd					= ENABLE;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority	= 15;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority			= 0;
	NVIC_Init(&NVIC_InitStructure);

	TIM_Cmd(ENTIMER_GROUP,ENABLE);
}

/*******************************************************************************
* Function Name  : 
* Description    :  
* Input           : timer cycle period,    call-back function
* Output         : 
* Return         :timer ID (>0)
*******************************************************************************/

TIMER ENTimerCreate(UINT32 intervalByus,ENTIMER_ISR_CB_PROC pTimerCBProc)
{
    TIMER i;
    UINT32 nPeriodInMs = intervalByus/1000;
    if (nPeriodInMs == 0)
    {
        nPeriodInMs = 1;
    }
    
    for (i=0; i<ENTIMER_NUM_MAX; i++)
    {
        if (TIMER_STATUS_UNCREATED == gstENTimer[i].ENTimerStatus)
        {
            gstENTimer[i].ENTimerStatus = TIMER_STATUS_CREATED;
            gstENTimer[i].pENTimerIsrCbProc = pTimerCBProc;
            gstENTimer[i].ENTimer_Period = nPeriodInMs/ENTIMER_PERIOD_MS;
            if (gstENTimer[i].ENTimer_Period == 0)
            {
                gstENTimer[i].ENTimer_Period = 1;
            }
            return (i+1);
        }
    }
    return 0;       // invalid timer ID
}

/*******************************************************************************
* Function Name  : 
* Description    :  
* Input           : 
* Output         :
* Return         :
*******************************************************************************/

void ENTimerDestory(TIMER timer)
{
    if (timer > 0)
    {
        gstENTimer[timer-1].ENTimerStatus = TIMER_STATUS_UNCREATED;
        gstENTimer[timer-1].pENTimerIsrCbProc = NULL;
        gstENTimer[timer-1].ENTimer_Period = 0;
    }
}

/*******************************************************************************
* Function Name  : 
* Description    :  
* Input           : 
* Output         :
* Return         :
*******************************************************************************/

void ENTimerStart(TIMER timer, UINT32 nRepeatTimes)
{
    if (timer > 0)
    {
        gstENTimer[timer-1].ENTimerStatus = TIMER_STATUS_RUNNING;        
        gstENTimer[timer-1].ENTimer_Count = 0;
        gstENTimer[timer-1].ENTimer_RepeatTimes = nRepeatTimes;
    }
}

/*******************************************************************************
* Function Name  : 
* Description    :  
* Input           : 
* Output         :
* Return         :
*******************************************************************************/

void ENTimerStop(TIMER timer)
{
    if (timer > 0)
    {
        gstENTimer[timer-1].ENTimerStatus = TIMER_STATUS_CREATED;
    }    
}

/*******************************************************************************
* Function Name  : 
* Description    :  TIMx ISR.  
* Input           : 
* Output         :
* Return         :
*******************************************************************************/
void ENTimer_IRQHandler(void)
{
    TIMER i;
    
    if(TIM_GetITStatus(ENTIMER_GROUP, TIM_IT_Update) == SET)
    {
        TIM_ClearITPendingBit(ENTIMER_GROUP, TIM_IT_Update);//清除中断标志位	        

        for (i = 0; i < ENTIMER_NUM_MAX; i++)
        {
            if (TIMER_STATUS_RUNNING == gstENTimer[i].ENTimerStatus)
            {
                gstENTimer[i].ENTimer_Count++;
                if (gstENTimer[i].ENTimer_Count >= gstENTimer[i].ENTimer_Period)
                {
                    gstENTimer[i].ENTimer_Count = 0;
                    
                    if (REPEAT_UNLIMITED_TIMES == gstENTimer[i].ENTimer_RepeatTimes)
                    {
                        if (gstENTimer[i].pENTimerIsrCbProc)
                        {
                            gstENTimer[i].pENTimerIsrCbProc();
                        }    
                    }
                    else    // limited cycle
                    {
                        if (gstENTimer[i].ENTimer_RepeatTimes > 0)
                        {
                            gstENTimer[i].ENTimer_RepeatTimes--; 
                            if (gstENTimer[i].pENTimerIsrCbProc)
                            {
                                gstENTimer[i].pENTimerIsrCbProc();
                            }                                                       
                        }                        
                    }                 
                
                }
            }
        }
        
    }
}
#ifndef BARE_MACHINE
extern void SysTick_ISR(void);	/* 声明调用外部的函数 */
#endif
void SysTick_Handler(void)
{
	gSysTick++;
#ifndef BARE_MACHINE
	xPortSysTickHandler();
    SysTick_ISR();
#endif
}
