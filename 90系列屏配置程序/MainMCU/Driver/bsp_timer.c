/*
*********************************************************************************************************
*
*	模块名称 : 定时器模块
*	文件名称 : bsp_timer.c
*	版    本 : V1.0
*	说    明 : 配置TIM4定时器作为系统滴答定时器。TIM1和TIM2都是高级定时器，TIM4是基本定时器，为了保留TIM1
*			   和TIM2高级定时器功能（可用于捕获，比较），我们选择 TIM4作为滴答定时器，定时周期为2ms。
*
*				实现了4个软件定时器供应用软件使用(精度1ms)， 可以通过修改 TMR_COUNT 增减定时器个数
*				实现了ms级别延迟函数（精度1ms）
*				实现了系统运行时间函数（1ms单位）
*
*********************************************************************************************************
*/

#include "stm32f10x.h"
#include <stdio.h>
#include "bsp_timer.h"

#define ENABLE_SYSTICK_INT()	__set_PRIMASK(0)	/* 使能中断 */
#define DISABLE_SYSTICK_INT()	__set_PRIMASK(1)	/* 禁止中断 */

/*
	在此定义若干个软件定时器全局变量
	注意，必须增加__IO 即 volatile，因为这个变量在中断和主程序中同时被访问，有可能造成编译器错误优化。
*/
#define TMR_COUNT	4		/* 软件定时器的个数，第1个保留给bsp_DelayMS()使用 */

static __IO uint32_t s_uiDelayCount = 0;
static __IO uint8_t s_ucTimeOutFlag = 0;

SOFT_TMR g_Tmr[TMR_COUNT];

/* 全局运行时间，单位10ms，用于uIP */
__IO int32_t g_iRunTime = 0;

static void bsp_SoftTimerDec(SOFT_TMR *_tmr);

extern __IO uint8_t s_ucRA8875BusyNow;

/*
*********************************************************************************************************
*	函 数 名: bsp_InitTimer
*	功能说明: 配置systick中断，并初始化软件定时器变量
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
void bsp_InitTimer(void)
{
	uint8_t i;

	/* 清零所有的软件定时器 */
	for (i = 0; i < TMR_COUNT; i++)
	{
		g_Tmr[i].Count = 0;
		g_Tmr[i].PreLoad = 0;
		g_Tmr[i].Flag = 0;
		g_Tmr[i].Mode = TMR_ONCE_MODE;	/* 缺省是1次性工作模式 */
	}

	/*
		配置systic中断周期为1ms，并启动systick中断。
    	这个函数在 \Libraries\CMSIS\CM3\CoreSupport\core_cm3.h

    	Systick中断向量在(\Libraries\CMSIS\CM3\DeviceSupport\ST\STM32F10x\startup\arm\
    		startup_stm32f10x_hd.s 文件中定义为 SysTick_Handler。
    	SysTick_Handler函数的实现在stm32f10x_it.c 文件。
    	SysTick_Handler函数调用了SysTick_ISR()函数，在本文件末尾。
    */
	SysTick_Config(SystemCoreClock / 1000);
}

/*
*********************************************************************************************************
*	函 数 名: SysTick_ISR
*	功能说明: SysTick中断服务程序，每隔1ms进入1次
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
extern void bsp_RunPer10ms(void);
void SysTick_ISR(void)
{
	static uint8_t s_count = 0;
	uint8_t i;

	/* 每隔1ms进来1次 */
	if (s_uiDelayCount > 0)
	{
		if (--s_uiDelayCount == 0)
		{
			s_ucTimeOutFlag = 1;
		}
	}

	for (i = 0; i < TMR_COUNT; i++)
	{
		bsp_SoftTimerDec(&g_Tmr[i]);
	}

	g_iRunTime++;	/* 全局运行时间每1ms增1 */
	if (g_iRunTime == 0x7FFFFFFF)
	{
		g_iRunTime = 0;
	}

	if (++s_count >= 10)
	{
		s_count = 0;
		//bsp_RunPer10ms();	/* 此函数在 bsp.c 中实现(可以为空) */
	}
}

/*
*********************************************************************************************************
*	函 数 名: bsp_SoftTimerDec
*	功能说明: 每隔1ms对所有定时器变量减1。必须被SysTick_ISR周期性调用。
*	形    参：_tmr : 定时器变量指针
*	返 回 值: 无
*********************************************************************************************************
*/
static void bsp_SoftTimerDec(SOFT_TMR *_tmr)
{
	if (_tmr->Count > 0)
	{
		/* 如果定时器变量减到1则设置定时器到达标志 */
		if (--_tmr->Count == 0)
		{
			_tmr->Flag = 1;

			/* 如果是自动模式，则自动重装计数器 */
			if(_tmr->Mode == TMR_AUTO_MODE)
			{
				_tmr->Count = _tmr->PreLoad;
			}
		}
	}
}

/*
*********************************************************************************************************
*	函 数 名: bsp_DelayMS
*	功能说明: ms级延迟，延迟精度为正负1ms
*	形    参：n : 延迟长度，单位1 ms。 n 应大于2
*	返 回 值: 无
*********************************************************************************************************
*/
void bsp_DelayMS(uint32_t n)
{
	if (n == 0)
	{
		return;
	}
	else if (n == 1)
	{
		n = 2;
	}

	DISABLE_SYSTICK_INT();  			/* 关中断 */

	s_uiDelayCount = n;
	s_ucTimeOutFlag = 0;

	ENABLE_SYSTICK_INT();  				/* 开中断 */

	while (1)
	{
		CPU_IDLE();			/* 此处是空操作。用户可以定义，让CPU进入IDLE状态，以降低功耗；或实现喂狗 */

		/* 等待延迟时间到 */
		/* 注意：编译器认为 s_ucTimeOutFlag = 0，所以可能优化错误
		因此 s_ucTimeOutFlag 变量必须申明为_IO_型 */
		if (s_ucTimeOutFlag == 1)
		{
			break;
		}
	}
}

/*
*********************************************************************************************************
*	函 数 名: bsp_StartTimer
*	功能说明: 启动一个定时器，并设置定时周期。
*	形    参：	_id     : 定时器ID，值域【0,TMR_COUNT-1】。用户必须自行维护定时器ID，以避免定时器ID冲突。
*				_period : 定时周期，单位1ms
*	返 回 值: 无
*********************************************************************************************************
*/
void bsp_StartTimer(uint8_t _id, uint32_t _period)
{
	if (_id >= TMR_COUNT)
	{
		/* while(1); 死机 */
		return;
	}

	DISABLE_SYSTICK_INT();  			/* 关中断 */

	g_Tmr[_id].Count = _period;			/* 实时计数器初值 */
	g_Tmr[_id].PreLoad = _period;		/* 计数器自动重装值，仅自动模式起作用 */
	g_Tmr[_id].Flag = 0;				/* 定时时间到标志 */
	g_Tmr[_id].Mode = TMR_ONCE_MODE;	/* 1次性工作模式 */

	ENABLE_SYSTICK_INT();  				/* 开中断 */
}

/*
*********************************************************************************************************
*	函 数 名: bsp_StartAutoTimer
*	功能说明: 启动一个自动定时器，并设置定时周期。
*	形    参：	_id     : 定时器ID，值域【0,TMR_COUNT-1】。用户必须自行维护定时器ID，以避免定时器ID冲突。
*				_period : 定时周期，单位10ms
*	返 回 值: 无
*********************************************************************************************************
*/
void bsp_StartAutoTimer(uint8_t _id, uint32_t _period)
{
	if (_id >= TMR_COUNT)
	{
		/* while(1); 死机 */
		return;
	}

	DISABLE_SYSTICK_INT();  		/* 关中断 */

	g_Tmr[_id].Count = _period;			/* 实时计数器初值 */
	g_Tmr[_id].PreLoad = _period;		/* 计数器自动重装值，仅自动模式起作用 */
	g_Tmr[_id].Flag = 0;				/* 定时时间到标志 */
	g_Tmr[_id].Mode = TMR_AUTO_MODE;	/* 自动工作模式 */

	ENABLE_SYSTICK_INT();  			/* 开中断 */
}

/*
*********************************************************************************************************
*	函 数 名: bsp_StopTimer
*	功能说明: 停止一个定时器
*	形    参：	_id     : 定时器ID，值域【0,TMR_COUNT-1】。用户必须自行维护定时器ID，以避免定时器ID冲突。
*	返 回 值: 无
*********************************************************************************************************
*/
void bsp_StopTimer(uint8_t _id)
{
	if (_id >= TMR_COUNT)
	{
		/* while(1); 死机 */
		return;
	}

	DISABLE_SYSTICK_INT();  	/* 关中断 */

	g_Tmr[_id].Count = 0;				/* 实时计数器初值 */
	g_Tmr[_id].Flag = 0;				/* 定时时间到标志 */
	g_Tmr[_id].Mode = TMR_ONCE_MODE;	/* 自动工作模式 */

	ENABLE_SYSTICK_INT();  		/* 开中断 */
}

/*
*********************************************************************************************************
*	函 数 名: bsp_CheckTimer
*	功能说明: 检测定时器是否超时
*	形    参：	_id     : 定时器ID，值域【0,TMR_COUNT-1】。用户必须自行维护定时器ID，以避免定时器ID冲突。
*				_period : 定时周期，单位1ms
*	返 回 值: 返回 0 表示定时未到， 1表示定时到
*********************************************************************************************************
*/
uint8_t bsp_CheckTimer(uint8_t _id)
{
	if (_id >= TMR_COUNT)
	{
		return 0;
	}

	if (g_Tmr[_id].Flag == 1)
	{
		g_Tmr[_id].Flag = 0;
		return 1;
	}
	else
	{
		return 0;
	}
}

/*
*********************************************************************************************************
*	函 数 名: bsp_GetRunTime
*	功能说明: 获取CPU运行时间，单位1ms
*	形    参：无
*	返 回 值: CPU运行时间，单位1ms
*********************************************************************************************************
*/
int32_t bsp_GetRunTime(void)
{
	int32_t runtime;

	DISABLE_SYSTICK_INT();  	/* 关中断 */

	runtime = g_iRunTime;	/* 由于在Systick中断被改写，因此关中断进行保护 */

	ENABLE_SYSTICK_INT();  		/* 开中断 */

	return runtime;
}

