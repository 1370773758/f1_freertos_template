#include "stm32f10x.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "bsp_touch.h"
#include "LCD_RA8889.h"
#include "bsp_timer.h"

#include "stm32f10x.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "bsp_touch.h"
#include "bsp_timer.h"

#include "GT911_i2c.h"
//#include "Sensor.h"
//#include "MemoryConfig.h"
//#include "Memeory.h"
//#include "xpt2046_spi.h"

/*

	RA8875���ô������ӿڣ����ֱ��ͨ��FSMC����RA8875��ؼĴ������ɡ�

	������δʹ�ô����жϹ��ܡ���1ms���ڵ� Systick��ʱ�жϷ�������жԴ���ADCֵ���в�����
	�˲����������ʰ��³���40ms�󣬿�ʼ�ɼ�ADCֵ��ÿ1ms�ɼ�1�Σ������ɼ�10�Σ���Ȼ���
	��10������������Ͳ�����ƽ��ֵ��

	���ǲ�����������tslib 5�㴥����У���㷨��У׼�󱣴�6��У׼ϵ����
	��������Ҫ������ʾ��δ�����湦�ܡ���ҿ����Լ��޸�  TOUCH_SaveParam() �� TOUCH_LoadParam() ��������ʵ�ֱ��湦�ܡ�

*/

typedef struct
{
	int x[5], xfb[5];   //xfb[5]��yfb[5]���Ԥ���趨��5��LCD����ֵ
	int y[5], yfb[5];   //x[5]��y[5]��Ŵ�RA8875�������Ĵ�������ֵ
	int a[7];
}calibration;

calibration cal;

#define SAMPLE_COUNT	16	/* ÿ�������ɼ����ٸ����� */
//				uint16_t s_us_Buf[SAMPLE_COUNT];

	/* ����ϵͳʾ��ͼ�� ��������
			 -----------------------------
			|(0,0)          X             |
			|     --------->              |
			|         |                   |
			|         |                   |
			| Y       |                   |
			|         V                   |
			|     --------->              |
			|                   (1424,280)|
			 -----------------------------

		���Ͻ����������, ɨ�跽���ȴ����ң��ٴ��ϵ��¡�

		�����Ҫ��������ʽ������Ҫ��������������߼������ת��
	*/
#define CALIB_OFFSET	20

#define TP_X1	(CALIB_OFFSET)                         //Top left
#define TP_Y1	(CALIB_OFFSET)

#define TP_X2	(LCD_GetWidth() - CALIB_OFFSET)        //Top right
#define TP_Y2	(CALIB_OFFSET)

#define TP_X3	(LCD_GetWidth() - CALIB_OFFSET)        //Bot right
#define TP_Y3	(LCD_GetHeight() - CALIB_OFFSET)

#define TP_X4	(CALIB_OFFSET)                         //Bot left
#define TP_Y4	(LCD_GetHeight() - CALIB_OFFSET)

#define TP_X5	(LCD_GetWidth() / 2)                   //Center
#define TP_Y5	(LCD_GetHeight() / 2)

#define TOUCH_CALIB_OFFSET              (30)

#define TOUCH_PRESS_FILTER              (5) // ���±��ֵ��˲�����


/* ��ЧADCֵ���ж�����. ̫�ӽ�ADC�ٽ�ֵ��������Ϊ��Ч */
#define ADC_VALID_OFFSET	5

//#define WaitTPReady() while(GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_5))
#define WaitTPReady() {}

/* ����ģ���õ���ȫ�ֱ��� */
TOUCH_T g_tTP;
static BOOL gTouchIsRelease = FALSE;

static uint8_t	TOUCH_PressValid(uint16_t _usX, uint16_t _usY);
static uint16_t TOUCH_DataFilter(uint16_t *_pBuf, uint8_t _ucCount);
static uint16_t TOUCH_DataFilter2(uint16_t *_pBuf, uint8_t _ucCount);
static void TOUCH_LoadParam(void);
void TOUCH_SaveParam(void);
static int16_t TOUCH_TransX(uint16_t _usAdcX, uint16_t _usAdcY);
static int16_t TOUCH_TransY(uint16_t _usAdcX, uint16_t _usAdcY);
int32_t TOUCH_Abs(int32_t x);
static uint8_t perform_calibration(calibration *cal);
static void TOUCH_WaitRelease(void);
static void TOUCH_Scan(void);    

 	
/*
*********************************************************************************************************
*	�� �� ��: bsp_InitTouch
*	����˵��: 
*	��    �Σ���
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void TOUCH_InitHard(void)
{
	g_tTP.Enable = 1;
	GT911_Init();
}

/*
*********************************************************************************************************
*	�� �� ��: TOUCH_ReadAdcX
*	����˵��: ��ô�����X����ADC����ֵ�� �ѽ����˲�����
*	��    �Σ���
*	�� �� ֵ: X ����ADCֵ
*********************************************************************************************************
*/
uint16_t TOUCH_ReadAdcX(void)
{
	uint16_t usAdc;
    
    TOUCH_Scan();
	__set_PRIMASK(1);  		/* ���ж� */
	usAdc = g_tTP.usAdcNowX;
	__set_PRIMASK(0);  		/* ���ж� */

	return usAdc;
}

/*
*********************************************************************************************************
*	�� �� ��: TOUCH_ReadAdcY
*	����˵��: ��ô�����Y����ADC����ֵ�� �ѽ����˲�����
*	��    �Σ���
*	�� �� ֵ: Y ����ֵ������ֵ
*********************************************************************************************************
*/
uint16_t TOUCH_ReadAdcY(void)
{
	uint16_t usAdc;

	__set_PRIMASK(1);  		/* ���ж� */
	usAdc = g_tTP.usAdcNowY;
	__set_PRIMASK(0);  		/* ���ж� */

	return usAdc;
}

/*
*********************************************************************************************************
*	�� �� ��: TOUCH_PutKey
*	����˵��: ��1������������ֵѹ�봥��FIFO��������������ģ��һ��������
*						����������У׼
*	��    ��: _usX, _usY ����ֵ
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void TOUCH_PutKey(uint8_t _ucEvent, uint16_t _usX, uint16_t _usY)
{
	g_tTP.Event[g_tTP.Write] = _ucEvent;
	g_tTP.XBuf[g_tTP.Write] = _usX;//TOUCH_TransX(_usX, _usY);
	g_tTP.YBuf[g_tTP.Write] = _usY;//TOUCH_TransY(_usX, _usY);

	if (++g_tTP.Write  >= TOUCH_FIFO_SIZE)
	{
		g_tTP.Write = 0;
	}
}

/*
*********************************************************************************************************
*	�� �� ��: TOUCH_GetKey
*	����˵��: �Ӵ���FIFO��������ȡһ������ֵ��
*	��    �Σ���
*	�� �� ֵ: 1��ʾ��δ���������
*********************************************************************************************************
*/
uint8_t TOUCH_GetKey(int16_t *_pX, int16_t *_pY)
{
	uint8_t ret;

	if (g_tTP.Read == g_tTP.Write)
	{
		return TOUCH_NONE;
	}
	else
	{
		ret = g_tTP.Event[g_tTP.Read];
		*_pX = g_tTP.XBuf[g_tTP.Read];
		*_pY = g_tTP.YBuf[g_tTP.Read];

		if (++g_tTP.Read >= TOUCH_FIFO_SIZE)
		{
			g_tTP.Read = 0;
		}
		return ret;
	}
}

/*
*********************************************************************************************************
*	�� �� ��: TOUCH_InRect
*	����˵��: �жϵ�ǰ�����Ƿ�λ�ھ��ο���
*	��    �Σ�_usX, _usY: ��������
*			_usRectX,_usRectY: �������
*			_usRectH��_usRectW : ���θ߶ȺͿ��
*	�� �� ֵ: 1 ��ʾ�ڷ�Χ��
*********************************************************************************************************
*/
uint8_t TOUCH_InRect(uint16_t _usX, uint16_t _usY,
	uint16_t _usRectX, uint16_t _usRectY, uint16_t _usRectH, uint16_t _usRectW)
{
	if ((_usX > _usRectX) && (_usX < _usRectX + _usRectW)
		&& (_usY > _usRectY) && (_usY < _usRectY + _usRectH))
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

/*
*********************************************************************************************************
*	�� �� ��: TOUCH_MoveValid
*	����˵��: �жϵ�ǰ������ϴ������Ƿ�ƫ��̫��
*	��    �Σ�_usX1, _usY1: ����1
*			  _usX2, _usY2: ����2
*	�� �� ֵ: 1 ��ʾ��Ч�㣬 0 ��ʾ�ϵ�
*********************************************************************************************************
*/
uint8_t TOUCH_MoveValid(uint16_t _usX1, uint16_t _usY1, uint16_t _usX2, uint16_t _usY2)
{
	int16_t iX, iY;
	static uint8_t s_invalid_count = 0;

	iX = TOUCH_Abs(_usX1 - _usX2);
	iY = TOUCH_Abs(_usY1 - _usY2);

	if ((iX < 25) && (iY < 25))
	{
		s_invalid_count = 0;
		return 1;
	}
	else
	{
		if (++s_invalid_count >= 3)
		{
			s_invalid_count = 0;
			return 1;
		}
		return 0;
	}
}

#define TOUCH_MAX	10
/**************************************************************************
   * Function Name		: TOUCH_ReadXY
   * Description		: ��ȡ�������� X �� Y �����������ֵ
   * Input				: *xValue�������ȡ�� X ����������ֵ�ĵ�ַ
   *			  ����������*yValue�������ȡ�� Y ����������ֵ�ĵ�ַ
   * Output 			: None
   * Return 			: 0����ȡ�ɹ���0xFF����ȡʧ��
   **************************************************************************/
static uint8_t TOUCH_ReadXY(uint16_t *xValue, uint16_t *yValue)
{
//	uint16_t xValue1, yValue1, xValue2, yValue2;
//	
//	//100Hz��ɨ��Ƶ�� �������û��
//	ENGT911_Scan();
//	xValue1 = gGT911_Get().X1;
//	yValue1 = gGT911_Get().Y1;
//	ENGT911_Scan();
//	xValue2 = gGT911_Get().X1;
//	yValue2 = gGT911_Get().Y1;

//	/* �鿴������֮���ֻ����ֵ��� */
//	if(xValue1 > xValue2)
//	{   
//		*xValue = xValue1 - xValue2;
//	}
//	else
//	{   
//		*xValue = xValue2 - xValue1;
//	}
//	   
//	if(yValue1 > yValue2)
//	{
//		*yValue = yValue1 - yValue2;	
//	}
//	else
//	{
//		*yValue = yValue2 - yValue1;
//	}
//	   	  
//	   /* �жϲ�����ֵ�Ƿ��ڿɿط�Χ�� */
//	   if((*xValue > TOUCH_MAX) || (*yValue > TOUCH_MAX))
//	   {
//		   return 0xFF;
//	   }
//	   
//	   /* ��ƽ��ֵ */
//	   *xValue = (xValue1 + xValue2) / 2;
//	   *yValue = (yValue1 + yValue2) / 2;
//  
//	   /* �жϵõ���ֵ���Ƿ���ȡֵ��Χ֮�� */
//	   if((*xValue > TOUCH_X_MAX) || (*xValue < TOUCH_X_MIN)
//	   || (*yValue > TOUCH_Y_MAX) || (*yValue < TOUCH_Y_MIN))
//	   {
//		   return 0xFF;
//	   }
//	   
//	   return 0;
}


/*
*********************************************************************************************************
*	�� �� ��: TOUCH_Scan
*	����˵��: �������¼������򡣸ú����������Ե��ã�ÿ10ms����1��. �� bsp_Timer.c  100Hz
*	��    �Σ���
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void TOUCH_Scan(void)
{
	if (g_tTP.Enable == 0)
	{
		return;
	}
	ENGT911_Scan(); //���ݸ���

	//����ȥ����ADC�˲����� оƬ���Դ�ȡƽ���˲�����Ư����
	//��Ч�������ٽ����˲�
}

/*
*********************************************************************************************************
*	�� �� ��: perform_calibration
*	����˵��: tslib 5�㴥����У�������㷨����
*	��    �Σ�calibration *cal
*	�� �� ֵ: 0��1
*********************************************************************************************************
*/
uint8_t perform_calibration(calibration *cal) 
{
	uint8_t j;
	float n, x, y, x2, y2, xy, z, zx, zy;
	float det, a, b, c, e, f, i;
	float scaling = 65536.0;

// Get sums for matrix
	n = x = y = x2 = y2 = xy = 0;
	for(j = 0;j < 5;j++) 
	{
		n += 1.0;
		x += (float)cal->x[j];
		y += (float)cal->y[j];
		x2 += (float)(cal->x[j]*cal->x[j]);
		y2 += (float)(cal->y[j]*cal->y[j]);
		xy += (float)(cal->x[j]*cal->y[j]);
	}

// Get determinant of matrix -- check if determinant is too small
	det = n*(x2*y2 - xy*xy) + x*(xy*y - x*y2) + y*(x*xy - y*x2);
	if(det < 0.1 && det > -0.1)                                       //ts_calibrate: determinant is too small
	{
		//printf("ts_calibrate: determinant is too small -- %f\n\r",det);		
		return 0;
	}

// Get elements of inverse matrix
	a = (x2*y2 - xy*xy)/det;
	b = (xy*y - x*y2)/det;
	c = (x*xy - y*x2)/det;
	e = (n*y2 - y*y)/det;
	f = (x*y - n*xy)/det;
	i = (n*x2 - x*x)/det;

// Get sums for x calibration
	z = zx = zy = 0;
	for(j = 0;j < 5;j++) 
	{
		z += (float)cal->xfb[j];
		zx += (float)(cal->xfb[j]*cal->x[j]);
		zy += (float)(cal->xfb[j]*cal->y[j]);
	}

// Now multiply out to get the calibration for framebuffer x coord
	cal->a[0] = (int)((a*z + b*zx + c*zy)*(scaling));
	cal->a[1] = (int)((b*z + e*zx + f*zy)*(scaling));
	cal->a[2] = (int)((c*z + f*zx + i*zy)*(scaling));

// Get sums for y calibration
	z = zx = zy = 0;
	for(j = 0;j < 5;j++) 
	{
		z += (float)cal->yfb[j];
		zx += (float)(cal->yfb[j]*cal->x[j]);
		zy += (float)(cal->yfb[j]*cal->y[j]);
	}

// Now multiply out to get the calibration for framebuffer y coord
	cal->a[3] = (int)((a*z + b*zx + c*zy)*(scaling));
	cal->a[4] = (int)((b*z + e*zx + f*zy)*(scaling));
	cal->a[5] = (int)((c*z + f*zx + i*zy)*(scaling));

// If we got here, we're OK, so assign scaling to a[6] and return
	cal->a[6] = (int)scaling;
	return 1;
}

/*
*********************************************************************************************************
*	�� �� ��: TOUCH_TransX
*	����˵��: ������ADCֵת��Ϊ��������
*	��    �Σ�uint16_t _usAdcX, uint16_t _usAdcY
*	�� �� ֵ: X ����ֵ
*********************************************************************************************************
*/
static int16_t TOUCH_TransX(uint16_t _usAdcX, uint16_t _usAdcY)
{
	int16_t x;

	if((_usAdcX == 0) && (_usAdcY == 0))
	{
		x = 0;
	}
	else
	{
	  x = ( cal.a[0] + cal.a[1]*_usAdcX + cal.a[2]*_usAdcY ) / cal.a[6];
	}

	//if(x >= 480 || x <= 0)
	//	x = 0;
    
	return x;
}

/*
*********************************************************************************************************
*	�� �� ��: TOUCH_TransY
*	����˵��: ������ADCֵת��Ϊ��������
*	��    �Σ�uint16_t _usAdcX, uint16_t _usAdcY
*	�� �� ֵ: Y ����ֵ
*********************************************************************************************************
*/
static int16_t TOUCH_TransY(uint16_t _usAdcX, uint16_t _usAdcY)
{
	int16_t y;
	
	if((_usAdcX == 0) && (_usAdcY == 0))
	{
		y = 0;
	}
	else
	{
	  y =	( cal.a[3] + cal.a[4]*_usAdcX + cal.a[5]*_usAdcY ) / cal.a[6];  
	}	
	//if(y >= 272 || y <= 0)
	//	y = 0;
	return y;
}

/*
*********************************************************************************************************
*	�� �� ��: TOUCH_GetX
*	����˵��: ��õ�ǰ�ĵĴ�������X
*	��    �Σ���
*	�� �� ֵ: X ����ֵ
*********************************************************************************************************
*/
int16_t TOUCH_GetX(void)
{
  return gGT911_GetxPhys();
}
/*
*********************************************************************************************************
*	�� �� ��: TOUCH_GetY
*	����˵��: ��õ�ǰ�ĵĴ�������Y
*	��    �Σ���
*	�� �� ֵ: Y ����ֵ
*********************************************************************************************************
*/
int16_t TOUCH_GetY(void)
{
  return gGT911_GetyPhys();
}
/*
*********************************************************************************************************
*	�� �� ��: TOUCH_DataFilter
*	����˵��: �Բ������ݽ����˲�
*	��    �Σ�_pBuf:���˲�����
					 	_ucCount�����˲�����
*	�� �� ֵ: X ����ֵ
*********************************************************************************************************
*/
static uint16_t TOUCH_DataFilter(uint16_t *_pBuf, uint8_t _ucCount)
{
	uint8_t i;
	uint32_t uiSum = 0;
	uint32_t uiMax = 0, uiMin = 4095;

	for (i = 0; i < _ucCount; i++)
	{
		uiSum += _pBuf[i];
		if(uiMax < _pBuf[i])
			uiMax = _pBuf[i];
		if(uiMin > _pBuf[i])
			uiMin = _pBuf[i];
	}
	
	return ((uiSum - uiMax - uiMin) / (_ucCount - 2));
}

/*
*********************************************************************************************************
*	�� �� ��: TOUCH_DataFilter2
*	����˵��: �Բ������ݽ����˲�
*	��    �Σ���
*	�� �� ֵ: X ����ֵ
*********************************************************************************************************
*/
static uint16_t TOUCH_DataFilter2(uint16_t *_pBuf, uint8_t _ucCount)
{
	uint8_t i,j;
	uint16_t temp;
	uint32_t uiSum = 0;
	uint32_t uiMax = 0, uiMin = 4095;
  
	if (_ucCount <= 20) return 0;
		
	for (j=0;j<_ucCount-1;j++)
   {
      for (i=0;i<_ucCount-1-j;i++)
      {
         if ( _pBuf[i]>_pBuf[i+1] )
         {
            temp = _pBuf[i];
            _pBuf[i] = _pBuf[i+1]; 
             _pBuf[i+1] = temp;
         }
      }
   }
	 
	for (i = 10; i < _ucCount - 10; i++)
	{
		uiSum += _pBuf[i];
	}
	
	return ((uiSum) / (_ucCount - 20));
}

/*
*********************************************************************************************************
*	�� �� ��: TOUCH_InitFixedLcdCoord()
*	����˵��: ���������Ե�ֵ����
*	��    �Σ���
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void TOUCH_InitFixedLcdCoord(void)
{
 	cal.xfb[0] = TP_X1; 
	cal.yfb[0] = TP_Y1;
	cal.xfb[1] = TP_X2; 
	cal.yfb[1] = TP_Y2;	
	cal.xfb[2] = TP_X3; 
	cal.yfb[2] = TP_Y3;	
	cal.xfb[3] = TP_X4; 
	cal.yfb[3] = TP_Y4;	
	cal.xfb[4] = TP_X5; 
	cal.yfb[4] = TP_Y5;			
}
/*
*********************************************************************************************************
*	�� �� ��: TOUCH_CalibrationByIndex(int index)
*	����˵��: У׼��i�����ֵ
*	��    �Σ���
*	�� �� ֵ: ��
*********************************************************************************************************
*/
BOOL TOUCH_CalibrationByIndex(int index)
{
	uint16_t usAdcX;
	uint16_t usAdcY;
	uint8_t usCount;
    BOOL result = TRUE;

	/* ��ĻУ׼�ĵ�һ���㲻�ȴ������ͷ� */
      if(index != 2)
	  TOUCH_WaitRelease(); 	    /* �ȴ������ͷ� */
	
	  usCount = 0;
		while(1)
		{
			usAdcX = TOUCH_ReadAdcX();
			usAdcY = TOUCH_ReadAdcY();

			if(TOUCH_PressValid(usAdcX, usAdcY))
			{
				if (++usCount > 5)
				{
					/* ��ѹ��Ч, ����У׼��ADC����ֵ */
					
                      cal.x[index]	= usAdcX;
                      cal.y[index]	= usAdcY;	
					
					/* ��ĻУ׼�����һ�����жϵȴ��ͷ���Ϣ */
					 if(index == 4)
						TOUCH_WaitRelease(); 
					return result;
				}
			}
			else
			{
				usCount = 0;
			}
			bsp_DelayMS(10);
		}
    return FALSE;
//	TOUCH_WaitRelease(); 	/* �ȴ������ͷ� */
}


uint8_t  TOUCH_Calibrate(void)
{
	
	uint8_t i;	
	TOUCH_WaitRelease(); 	/* �ȴ������ͷ� */
	
//�õ�5����Ĳ�������֮������Ϳ���ͨ��perform_calibration��������Ĳ���A��B��C��D��E��F��	
	i = perform_calibration (&cal);

	//TOUCH_SaveParam();        /* У׼����Խ�У׼����������Flash ����EEPROM */
	return i;	
}




/*
*********************************************************************************************************
*	�� �� ��: TOUCH_Calibration
*	����˵��: ������У׼
*	��    �Σ���
*	�� �� ֵ: ��
*********************************************************************************************************
*/
uint8_t TOUCH_Calibration(void)
{
	uint16_t usAdcX;
	uint16_t usAdcY;
	uint8_t usCount;
	uint8_t index;
	uint8_t i;	

	for(index = 0; index < 5; index++)
	{
		//TOUCH_DispPoint(index);		/* ��ʾУ׼�� */

		TOUCH_WaitRelease(); 	    /* �ȴ������ͷ� */

		usCount = 0;
		while(1)
		{
			usAdcX = TOUCH_ReadAdcX();
			usAdcY = TOUCH_ReadAdcY();

			if(TOUCH_PressValid(usAdcX, usAdcY))
			{
				if (++usCount > 5)
				{
					/* ��ѹ��Ч, ����У׼��ADC����ֵ */
					
          cal.x[index]	= usAdcX;
          cal.y[index]	= usAdcY;					
										
					break;
				}
			}
			else
			{
				usCount = 0;
			}
			bsp_DelayMS(10);
		}
	}

//��Ԥ���趨��LCD����ֵ�����xfb[]��yfb[]������	


	TOUCH_WaitRelease(); 	/* �ȴ������ͷ� */
	
//�õ�5����Ĳ�������֮������Ϳ���ͨ��perform_calibration��������Ĳ���A��B��C��D��E��F��	
	i = perform_calibration (&cal);

	//TOUCH_SaveParam();        /* У׼����Խ�У׼����������Flash ����EEPROM */
	return i;
}

/*
*********************************************************************************************************
*	�� �� ��: TOUCH_PressValid
*	����˵��: �жϰ�ѹ�Ƿ���Ч������X, Y��ADCֵ���д����ж�
*	��    �Σ���
*	�� �� ֵ: 1 ��ʾ��Ч�� 0 ��ʾ��Ч
*********************************************************************************************************
*/
static uint8_t	TOUCH_PressValid(uint16_t _usX, uint16_t _usY)
{
	if ((_usX <= ADC_VALID_OFFSET) || (_usY <= ADC_VALID_OFFSET) || (_usX >= g_tTP.usMaxAdc - ADC_VALID_OFFSET) || (_usY >= g_tTP.usMaxAdc - ADC_VALID_OFFSET))
	{
		return 0;    //�ް�ѹ
	}
	else
	{
		return 1;    //�а�ѹ
	}
}

/*
*********************************************************************************************************
*	�� �� ��: TOUCH_WaitRelease
*	����˵��: �ȴ������ͷ�
*	��    �Σ���
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void TOUCH_WaitRelease(void)
{
	uint8_t usCount;

	while(1)
	{
		if (TOUCH_PressValid(TOUCH_ReadAdcX(), TOUCH_ReadAdcY()) == 0)  //���û�а�ѹ
		{
			if (++usCount > 5)       //���û�а�ѹ�ﵽ5�Σ�ÿ�μ��10ms��
			{
				break;                 //��ô��ʾ�����ͷţ��˳�whileѭ��
			}
		}
		else
		{
			usCount = 0;
		}
		bsp_DelayMS(10);
	}
}

/*
*********************************************************************************************************
*	�� �� ��: TOUCH_Abs
*	����˵��: �������ֵ
*	��    �Σ���
*	�� �� ֵ: ��
*********************************************************************************************************
*/
int32_t TOUCH_Abs(int32_t x)
{
	if (x >= 0)
	{
		return x;
	}
	else
	{
		return -x;
	}
}

/*
*********************************************************************************************************
*	�� �� ��: TOUCH_SaveParam
*	����˵��: ����У׼����	s_usAdcX1 s_usAdcX2 s_usAdcY1 s_usAdcX2
*	��    �Σ���
*	�� �� ֵ: ��
*********************************************************************************************************
*/

void TOUCH_SaveParam(void)
{
#if 0
	//����6���������ɣ�cal.a[0]��cal.a[1]��cal.a[2]��cal.a[3]��cal.a[4]��cal.a[5]
	uint32_t usBuf[7];
    
//  usBuf[0] = 0xFF69E3E0;0xFF5EC4C0
//	usBuf[1] = 0x0000C49F;0x0000BFCE
//	usBuf[2] = 0xFFFFF99A;0x0000075A
//	usBuf[3] = 0xFEC6A780;0xFEA4F610
//	usBuf[4] = 0x000001F4;	0x000009E5
//	usBuf[5] = 0x0000E122;	0x0000E441
//	usBuf[6] = 0x00010000;	0x00010000
    
	usBuf[0] = cal.a[0];
	usBuf[1] = cal.a[1];
	usBuf[2] = cal.a[2];
	usBuf[3] = cal.a[3];
	usBuf[4] = cal.a[4];	
	usBuf[5] = cal.a[5];
	usBuf[6] = 0x00010000;
	
  ENMemoryWriteBuffer((BYTE*)&usBuf, (EN_MEMEORY_SYSTEM_ADDR + sizeof(ENSensorCalibParam)), sizeof(usBuf));
#endif 
}

/*
*********************************************************************************************************
*	�� �� ��: TOUCH_LoadParam
*	����˵��: ��ȡУ׼����
*	��    �Σ���
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void TOUCH_LoadParam(void)
{
#if 0
	//��ȡ6���������ɣ�cal.a[0]��cal.a[1]��cal.a[2]��cal.a[3]��cal.a[4]��cal.a[5]
	
	uint32_t usBuf[7];
    BOOL  isCalib = TRUE;

	ENMemoryReadBuffer((BYTE*)&usBuf, (EN_MEMEORY_SYSTEM_ADDR + sizeof(ENSensorCalibParam)), sizeof(usBuf));
    
	switch (LCD_Ctrl_Type())
	{
		case LCD_CTRL_RA8875: 
    if(usBuf[6] != 0x00010000)
    {
        usBuf[0] = 0x02F1E3E0;
        usBuf[1] = 0xFFFF1CF8;
        usBuf[2] = 0xFFFFF7C1;
        usBuf[3] = 0x023FCEC0;
        usBuf[4] = 0x0000006D;
        usBuf[5] = 0xFFFF28D8;
        usBuf[6] = 0x00010000;
        isCalib = FALSE;
    }
			break;
		
		case LCD_CTRL_RA8871: 
	if(usBuf[6] != 0x00010000)
    {
        usBuf[0] = 0x0293EC20;
        usBuf[1] = 0xFFFF2C13;
        usBuf[2] = 0x000001F3;
        usBuf[3] = 0x02414FD0;
        usBuf[4] = 0x0000009E;
        usBuf[5] = 0xFFFF2472;
        usBuf[6] = 0x00010000;
        isCalib = FALSE;
    }
			XPT2046_Configuration();
			break;
	}
		
	cal.a[0] = usBuf[0];
	cal.a[1] = usBuf[1];
	cal.a[2] = usBuf[2];
	cal.a[3] = usBuf[3];
	cal.a[4] = usBuf[4];
	cal.a[5] = usBuf[5];
	cal.a[6] = usBuf[6];
    
    if(isCalib == FALSE)
        TOUCH_SaveParam();

#endif
}

// �������Ƿ����ͷŲ���
BOOL TouchIsRelease(void)
{
    if(gTouchIsRelease)
    {
        gTouchIsRelease = FALSE;
        return TRUE;
    }
    return FALSE;
}

