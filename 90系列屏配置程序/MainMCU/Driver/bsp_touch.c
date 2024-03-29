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

	RA8875内置触摸屏接口，因此直接通过FSMC访问RA8875相关寄存器即可。

	本程序未使用触笔中断功能。在1ms周期的 Systick定时中断服务程序中对触摸ADC值进行采样和
	滤波处理。当触笔按下超过40ms后，开始采集ADC值（每1ms采集1次，连续采集10次），然后对
	这10个样本进行求和并计算平均值。

	我们采用最完美的tslib 5点触摸屏校正算法，校准后保存6个校准系数。
	本程序主要用于演示，未做保存功能。大家可以自己修改  TOUCH_SaveParam() 和 TOUCH_LoadParam() 两个函数实现保存功能。

*/

typedef struct
{
	int x[5], xfb[5];   //xfb[5]、yfb[5]存放预先设定的5个LCD坐标值
	int y[5], yfb[5];   //x[5]、y[5]存放从RA8875读回来的触摸坐标值
	int a[7];
}calibration;

calibration cal;

#define SAMPLE_COUNT	16	/* 每次连续采集多少个样本 */
//				uint16_t s_us_Buf[SAMPLE_COUNT];

	/* 坐标系统示意图： （横屏）
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

		左上角是坐标零点, 扫描方向，先从左到右，再从上到下。

		如果需要做竖屏方式，你需要进行物理坐标和逻辑坐标的转换
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

#define TOUCH_PRESS_FILTER              (5) // 按下保持的滤波次数


/* 有效ADC值的判断门限. 太接近ADC临界值的坐标认为无效 */
#define ADC_VALID_OFFSET	5

//#define WaitTPReady() while(GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_5))
#define WaitTPReady() {}

/* 触屏模块用到的全局变量 */
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
*	函 数 名: bsp_InitTouch
*	功能说明: 
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
void TOUCH_InitHard(void)
{
	g_tTP.Enable = 1;
	GT911_Init();
}

/*
*********************************************************************************************************
*	函 数 名: TOUCH_ReadAdcX
*	功能说明: 获得触摸板X方向ADC采样值， 已进行滤波处理
*	形    参：无
*	返 回 值: X 方向ADC值
*********************************************************************************************************
*/
uint16_t TOUCH_ReadAdcX(void)
{
	uint16_t usAdc;
    
    TOUCH_Scan();
	__set_PRIMASK(1);  		/* 关中断 */
	usAdc = g_tTP.usAdcNowX;
	__set_PRIMASK(0);  		/* 开中断 */

	return usAdc;
}

/*
*********************************************************************************************************
*	函 数 名: TOUCH_ReadAdcY
*	功能说明: 获得触摸板Y方向ADC采样值， 已进行滤波处理
*	形    参：无
*	返 回 值: Y 坐标值，允许负值
*********************************************************************************************************
*/
uint16_t TOUCH_ReadAdcY(void)
{
	uint16_t usAdc;

	__set_PRIMASK(1);  		/* 关中断 */
	usAdc = g_tTP.usAdcNowY;
	__set_PRIMASK(0);  		/* 开中断 */

	return usAdc;
}

/*
*********************************************************************************************************
*	函 数 名: TOUCH_PutKey
*	功能说明: 将1个触摸点坐标值压入触摸FIFO缓冲区。可用于模拟一个按键。
*						电容屏无须校准
*	形    参: _usX, _usY 坐标值
*	返 回 值: 无
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
*	函 数 名: TOUCH_GetKey
*	功能说明: 从触摸FIFO缓冲区读取一个坐标值。
*	形    参：无
*	返 回 值: 1表示有未处理的数据
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
*	函 数 名: TOUCH_InRect
*	功能说明: 判断当前坐标是否位于矩形框内
*	形    参：_usX, _usY: 输入坐标
*			_usRectX,_usRectY: 矩形起点
*			_usRectH、_usRectW : 矩形高度和宽度
*	返 回 值: 1 表示在范围内
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
*	函 数 名: TOUCH_MoveValid
*	功能说明: 判断当前坐标和上次坐标是否偏差太大
*	形    参：_usX1, _usY1: 坐标1
*			  _usX2, _usY2: 坐标2
*	返 回 值: 1 表示有效点， 0 表示废点
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
   * Description		: 读取触摸屏的 X 轴 Y 轴的物理坐标值
   * Input				: *xValue：保存读取到 X 轴物理坐标值的地址
   *			  　　　　　*yValue：保存读取到 Y 轴物理坐标值的地址
   * Output 			: None
   * Return 			: 0：读取成功；0xFF：读取失败
   **************************************************************************/
static uint8_t TOUCH_ReadXY(uint16_t *xValue, uint16_t *yValue)
{
//	uint16_t xValue1, yValue1, xValue2, yValue2;
//	
//	//100Hz的扫描频率 这个可能没用
//	ENGT911_Scan();
//	xValue1 = gGT911_Get().X1;
//	yValue1 = gGT911_Get().Y1;
//	ENGT911_Scan();
//	xValue2 = gGT911_Get().X1;
//	yValue2 = gGT911_Get().Y1;

//	/* 查看两个点之间的只采样值差距 */
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
//	   /* 判断采样差值是否在可控范围内 */
//	   if((*xValue > TOUCH_MAX) || (*yValue > TOUCH_MAX))
//	   {
//		   return 0xFF;
//	   }
//	   
//	   /* 求平均值 */
//	   *xValue = (xValue1 + xValue2) / 2;
//	   *yValue = (yValue1 + yValue2) / 2;
//  
//	   /* 判断得到的值，是否在取值范围之内 */
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
*	函 数 名: TOUCH_Scan
*	功能说明: 触摸板事件检测程序。该函数被周期性调用，每10ms调用1次. 见 bsp_Timer.c  100Hz
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
void TOUCH_Scan(void)
{
	if (g_tTP.Enable == 0)
	{
		return;
	}
	ENGT911_Scan(); //数据更新

	//这里去除了ADC滤波部分 芯片有自带取平均滤波和温漂补偿
	//若效果不好再进行滤波
}

/*
*********************************************************************************************************
*	函 数 名: perform_calibration
*	功能说明: tslib 5点触摸屏校正核心算法函数
*	形    参：calibration *cal
*	返 回 值: 0或1
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
*	函 数 名: TOUCH_TransX
*	功能说明: 将触摸ADC值转换为像素坐标
*	形    参：uint16_t _usAdcX, uint16_t _usAdcY
*	返 回 值: X 坐标值
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
*	函 数 名: TOUCH_TransY
*	功能说明: 将触摸ADC值转换为像素坐标
*	形    参：uint16_t _usAdcX, uint16_t _usAdcY
*	返 回 值: Y 坐标值
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
*	函 数 名: TOUCH_GetX
*	功能说明: 获得当前的的触摸坐标X
*	形    参：无
*	返 回 值: X 坐标值
*********************************************************************************************************
*/
int16_t TOUCH_GetX(void)
{
  return gGT911_GetxPhys();
}
/*
*********************************************************************************************************
*	函 数 名: TOUCH_GetY
*	功能说明: 获得当前的的触摸坐标Y
*	形    参：无
*	返 回 值: Y 坐标值
*********************************************************************************************************
*/
int16_t TOUCH_GetY(void)
{
  return gGT911_GetyPhys();
}
/*
*********************************************************************************************************
*	函 数 名: TOUCH_DataFilter
*	功能说明: 对采样数据进行滤波
*	形    参：_pBuf:待滤波数组
					 	_ucCount：待滤波长度
*	返 回 值: X 坐标值
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
*	函 数 名: TOUCH_DataFilter2
*	功能说明: 对采样数据进行滤波
*	形    参：无
*	返 回 值: X 坐标值
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
*	函 数 名: TOUCH_InitFixedLcdCoord()
*	功能说明: 触摸屏绝对点值导入
*	形    参：无
*	返 回 值: 无
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
*	函 数 名: TOUCH_CalibrationByIndex(int index)
*	功能说明: 校准第i个点的值
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
BOOL TOUCH_CalibrationByIndex(int index)
{
	uint16_t usAdcX;
	uint16_t usAdcY;
	uint8_t usCount;
    BOOL result = TRUE;

	/* 屏幕校准的第一个点不等待触笔释放 */
      if(index != 2)
	  TOUCH_WaitRelease(); 	    /* 等待触笔释放 */
	
	  usCount = 0;
		while(1)
		{
			usAdcX = TOUCH_ReadAdcX();
			usAdcY = TOUCH_ReadAdcY();

			if(TOUCH_PressValid(usAdcX, usAdcY))
			{
				if (++usCount > 5)
				{
					/* 按压有效, 保存校准点ADC采样值 */
					
                      cal.x[index]	= usAdcX;
                      cal.y[index]	= usAdcY;	
					
					/* 屏幕校准的最后一个点判断等待释放消息 */
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
//	TOUCH_WaitRelease(); 	/* 等待触笔释放 */
}


uint8_t  TOUCH_Calibrate(void)
{
	
	uint8_t i;	
	TOUCH_WaitRelease(); 	/* 等待触笔释放 */
	
//得到5个点的采样数据之后，下面就可以通过perform_calibration计算上面的参数A，B，C，D，E，F了	
	i = perform_calibration (&cal);

	//TOUCH_SaveParam();        /* 校准后可以将校准参数保存入Flash 或者EEPROM */
	return i;	
}




/*
*********************************************************************************************************
*	函 数 名: TOUCH_Calibration
*	功能说明: 触摸屏校准
*	形    参：无
*	返 回 值: 无
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
		//TOUCH_DispPoint(index);		/* 显示校准点 */

		TOUCH_WaitRelease(); 	    /* 等待触笔释放 */

		usCount = 0;
		while(1)
		{
			usAdcX = TOUCH_ReadAdcX();
			usAdcY = TOUCH_ReadAdcY();

			if(TOUCH_PressValid(usAdcX, usAdcY))
			{
				if (++usCount > 5)
				{
					/* 按压有效, 保存校准点ADC采样值 */
					
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

//将预先设定的LCD坐标值存放在xfb[]，yfb[]数组里	


	TOUCH_WaitRelease(); 	/* 等待触笔释放 */
	
//得到5个点的采样数据之后，下面就可以通过perform_calibration计算上面的参数A，B，C，D，E，F了	
	i = perform_calibration (&cal);

	//TOUCH_SaveParam();        /* 校准后可以将校准参数保存入Flash 或者EEPROM */
	return i;
}

/*
*********************************************************************************************************
*	函 数 名: TOUCH_PressValid
*	功能说明: 判断按压是否有效，根据X, Y的ADC值进行大致判断
*	形    参：无
*	返 回 值: 1 表示有效； 0 表示无效
*********************************************************************************************************
*/
static uint8_t	TOUCH_PressValid(uint16_t _usX, uint16_t _usY)
{
	if ((_usX <= ADC_VALID_OFFSET) || (_usY <= ADC_VALID_OFFSET) || (_usX >= g_tTP.usMaxAdc - ADC_VALID_OFFSET) || (_usY >= g_tTP.usMaxAdc - ADC_VALID_OFFSET))
	{
		return 0;    //无按压
	}
	else
	{
		return 1;    //有按压
	}
}

/*
*********************************************************************************************************
*	函 数 名: TOUCH_WaitRelease
*	功能说明: 等待触笔释放
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
static void TOUCH_WaitRelease(void)
{
	uint8_t usCount;

	while(1)
	{
		if (TOUCH_PressValid(TOUCH_ReadAdcX(), TOUCH_ReadAdcY()) == 0)  //如果没有按压
		{
			if (++usCount > 5)       //如果没有按压达到5次（每次间隔10ms）
			{
				break;                 //那么表示触笔释放，退出while循环
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
*	函 数 名: TOUCH_Abs
*	功能说明: 计算绝对值
*	形    参：无
*	返 回 值: 无
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
*	函 数 名: TOUCH_SaveParam
*	功能说明: 保存校准参数	s_usAdcX1 s_usAdcX2 s_usAdcY1 s_usAdcX2
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/

void TOUCH_SaveParam(void)
{
#if 0
	//保存6个变量即可：cal.a[0]、cal.a[1]、cal.a[2]、cal.a[3]、cal.a[4]、cal.a[5]
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
*	函 数 名: TOUCH_LoadParam
*	功能说明: 读取校准参数
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
static void TOUCH_LoadParam(void)
{
#if 0
	//读取6个变量即可：cal.a[0]、cal.a[1]、cal.a[2]、cal.a[3]、cal.a[4]、cal.a[5]
	
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

// 触摸屏是否有释放操作
BOOL TouchIsRelease(void)
{
    if(gTouchIsRelease)
    {
        gTouchIsRelease = FALSE;
        return TRUE;
    }
    return FALSE;
}

