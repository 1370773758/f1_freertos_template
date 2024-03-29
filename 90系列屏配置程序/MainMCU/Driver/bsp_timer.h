/*
*********************************************************************************************************
*
*	模块名称 : 定时器模块
*	文件名称 : bsp_timer.h
*
*********************************************************************************************************
*/

#ifndef __BSP_TIMER_H
#define __BSP_TIMER_H

/* 目前是空操作，用户可以定义让CPU进入IDLE状态的函数和喂狗函数 */
#define CPU_IDLE()

/* 定时器结构体，成员变量必须是 volatile, 否则C编译器优化时可能有问题 */
enum
{
	TMR_ONCE_MODE = 0,	/* 一次工作模式 */
	TMR_AUTO_MODE = 1		/* 自动定时工作模式 */
};

/* 定时器结构体，成员变量必须是 volatile, 否则C编译器优化时可能有问题 */
typedef struct
{
	volatile uint8_t Mode;		/* 计数器模式，1次性 */
	volatile uint8_t Flag;		/* 定时到达标志  */
	volatile uint32_t Count;	/* 计数器 */
	volatile uint32_t PreLoad;	/* 计数器预装值 */
}SOFT_TMR;

/* 供外部调用的函数声明 */
void bsp_InitTimer(void);
void bsp_DelayMS(uint32_t n);
void bsp_StartTimer(uint8_t _id, uint32_t _period);
void bsp_StartAutoTimer(uint8_t _id, uint32_t _period);
void bsp_StopTimer(uint8_t _id);
uint8_t bsp_CheckTimer(uint8_t _id);
int32_t bsp_GetRunTime(void);

#endif
