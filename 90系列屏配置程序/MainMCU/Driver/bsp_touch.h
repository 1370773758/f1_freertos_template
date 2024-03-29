

#ifndef __BSP_TOUCH_H
#define __BSP_TOUCH_H

#include "stm32f10x.h"
#include "typedef.h"

#define TOUCH_FIFO_SIZE		20

typedef struct
{
	/* tslib 5�㴥����У���㷨 */
	uint16_t usAdcX1;	/* ���Ͻ� */
	uint16_t usAdcY1;

	uint16_t usAdcX2;	/* ���½� */
	uint16_t usAdcY2;

	uint16_t usAdcX3;	/* ���½� */
	uint16_t usAdcY3;

	uint16_t usAdcX4;	/* ���Ͻ� */
	uint16_t usAdcY4;

	uint16_t usAdcX5;	/* ���ĵ� */
	uint16_t usAdcY5;

	uint16_t usMaxAdc;	/* ���������ADCֵ��������Ч���ж�. ��СADC = 0  */
	uint16_t usAdcNowX;
	uint16_t usAdcNowY;

	uint8_t Enable;		/* �������ʹ�ܱ�־ */

	uint8_t Event[TOUCH_FIFO_SIZE];	/* �����¼� */
	int16_t XBuf[TOUCH_FIFO_SIZE];	/* �������껺���� */
	int16_t YBuf[TOUCH_FIFO_SIZE];	/* �������껺���� */
	uint8_t Read;					          /* ��������ָ�� */
	uint8_t Write;					        /* ������дָ�� */
}TOUCH_T;

/* �����¼� */
enum
{
	TOUCH_NONE = 0,		/* �޴��� */
	TOUCH_DOWN = 1,		/* ���� */
	TOUCH_MOVE = 2,		/* �ƶ� */
	TOUCH_RELEASE = 3	/* �ͷ� */
};


/* ���ⲿ���õĺ������� */
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


