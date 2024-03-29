

#ifndef __BSP_TOUCH_H
#define __BSP_TOUCH_H

#include "stm32f10x.h"
#include "typedef.h"

#define TOUCH_FIFO_SIZE		20

typedef struct
{
	/* tslib 5点触摸屏校正算法 */
	uint16_t usAdcX1;	/* 左上角 */
	uint16_t usAdcY1;

	uint16_t usAdcX2;	/* 右下角 */
	uint16_t usAdcY2;

	uint16_t usAdcX3;	/* 左下角 */
	uint16_t usAdcY3;

	uint16_t usAdcX4;	/* 右上角 */
	uint16_t usAdcY4;

	uint16_t usAdcX5;	/* 中心点 */
	uint16_t usAdcY5;

	uint16_t usMaxAdc;	/* 触摸板最大ADC值，用于有效点判断. 最小ADC = 0  */
	uint16_t usAdcNowX;
	uint16_t usAdcNowY;

	uint8_t Enable;		/* 触摸检测使能标志 */

	uint8_t Event[TOUCH_FIFO_SIZE];	/* 触摸事件 */
	int16_t XBuf[TOUCH_FIFO_SIZE];	/* 触摸坐标缓冲区 */
	int16_t YBuf[TOUCH_FIFO_SIZE];	/* 触摸坐标缓冲区 */
	uint8_t Read;					          /* 缓冲区读指针 */
	uint8_t Write;					        /* 缓冲区写指针 */
}TOUCH_T;

/* 触摸事件 */
enum
{
	TOUCH_NONE = 0,		/* 无触摸 */
	TOUCH_DOWN = 1,		/* 按下 */
	TOUCH_MOVE = 2,		/* 移动 */
	TOUCH_RELEASE = 3	/* 释放 */
};


/* 供外部调用的函数声明 */
void TOUCH_InitHard(void);
uint8_t TOUCH_Calibration(void);

uint16_t TOUCH_ReadAdcX(void);
uint16_t TOUCH_ReadAdcY(void);

int16_t TOUCH_GetX(void);
int16_t TOUCH_GetY(void);

uint8_t TOUCH_GetKey(int16_t *_pX, int16_t *_pY);

uint8_t TOUCH_InRect(uint16_t _usX, uint16_t _usY,
uint16_t _usRectX, uint16_t _usRectY, uint16_t _usRectH, uint16_t _usRectW);

void TOUCH_DispPoint(uint8_t _ucIndex);

void TOUCH_InitFixedLcdCoord(void);
BOOL TOUCH_CalibrationByIndex(int index);
uint8_t  TOUCH_Calibrate(void);
void TOUCH_SaveParam(void);

BOOL TouchIsRelease(void);
	
#endif


