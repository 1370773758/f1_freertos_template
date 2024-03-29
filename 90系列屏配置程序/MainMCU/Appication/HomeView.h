/*****************************************************************
 * File: HomeView.h
 * Date: 2018/6/30 11:32
 * Copyright@2018-2028: MedRena, All Right Reserved
 *
 * Note:
 *
******************************************************************/
#ifndef _HOMEVIEW_H_
#define _HOMEVIEW_H_

#include "Typedef.h"
#include "WM.h"


void HomeViewCreate(void);
void HomeViewDestory(void);

void HomeViewSetCurrentJumpView(BOOL isleftview);
BOOL HomeViewGetCurrentJumpView(void);
#endif // _HOMEVIEW_H_
