/*****************************************************************
 * File: Timer.h
 * Date: 2018/06/08 17:29
 * Copyright@2018-2028: MedRena, All Right Reserved
 *
 * Note:
 *
*****************************************************************/
#ifndef _TIMER_H_
#define _TIMER_H_

#include "Typedef.h"

typedef INT8 TIMER;
typedef void (*ENTIMER_ISR_CB_PROC)(void);

#define REPEAT_UNLIMITED_TIMES           (0xFFFFFFFF)

void ENTimerInit(void);
TIMER ENTimerCreate(UINT32 intervalByus,ENTIMER_ISR_CB_PROC pTimerCBProc);
void  ENTimerDestory(TIMER timer);
void  ENTimerStart(TIMER timer, UINT32 nRepeatTimes);
void  ENTimerStop(TIMER timer);

void  ENSysTickInit(void);
UINT64 ENSysTickGet(void);

void ENSleep(UINT32 ms);
void ENDelayus(UINT32 Time);

#endif // 
