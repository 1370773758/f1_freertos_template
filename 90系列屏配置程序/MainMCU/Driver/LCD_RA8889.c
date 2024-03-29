/*
RA8889���������
*/
#include "LCD_RA8889.h"
#include "stm32f10x.h"
#include "Typedef.h"

#include "RA8889.h"
#include "RA8889_API.h"
#include "RA8889_MCU_IF.h"
#include "UserDef.h"

#include "Timer.h"

static void LCD_CtrlLinesConfig(void);
static void LCD_FSMCConfig(void);
/*
*********************************************************************************************************
*	�� �� ��: LCD_CtrlLinesConfig
*	����˵��: ����LCD���ƿ��ߣ�FSMC�ܽ�����Ϊ���ù���
*	��    �Σ���
*	�� �� ֵ: ��
*********************************************************************************************************
*/
//��λ��Ĭ������ ûIO����
#define CON_PORT  GPIOD
#define LCD_RST   GPIO_Pin_12
#define SET_PIN(pin,x)   CON_PORT->ODR = (CON_PORT->ODR & ~pin)|(x ? pin : 0)

void Delayms(UINT16 ms)    
{ 
    UINT16 i,j; 
    for( i = 0; i < ms; i++ )
    { 
        for( j = 0; j < 5600; j++ );
    }
} 

/*
*********************************************************************************************************
*	�� �� ��: LCD_InitHard
*	����˵��: ��ʼ��LCD
*	��    ��: ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void LCD_InitHard(void)
{
	/* ����LCD���ƿ���GPIO */
	LCD_CtrlLinesConfig();
	ENSleep(30);
	/* ����FSMC�ӿڣ��������� */
	LCD_FSMCConfig();
	/* FSMC���ú������ӳٲ��ܷ��������豸  */
   ENSleep(30);
    RA8889_Initial();
	
	//
}

/*
PD0(D2),PD1(D3),PD4(RD),PD5(WR),PD8(D13),PD9(D14),PD10(D15),PD14(D0),PD15(D1)
Set PE.07(D4), PE.08(D5), PE.09(D6), PE.10(D7), PE.11(D8), PE.12(D9), PE.13(D10),
     PE.14(D11), PE.15(D12) 
	 CS PD7
	 RS PD11
*/
static void LCD_CtrlLinesConfig(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_FSMC, ENABLE);
	ENSleep(1);
	/* ʹ�� FSMC, GPIOD, GPIOE, GPIOF, GPIOG �� AFIO ʱ�� */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC | RCC_APB2Periph_GPIOD | RCC_APB2Periph_GPIOE | RCC_APB2Periph_AFIO, ENABLE);
	

	/* ���� PD.00(D2), PD.01(D3), PD.04(NOE), PD.05(NWE), PD.08(D13), PD.09(D14), PD11 RS PD7 CS
	 PD.10(D15), PD.14(D0), PD.15(D1) Ϊ����������� */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_7 | 
	                            GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_10 | GPIO_Pin_11 | GPIO_Pin_14 |
	                            GPIO_Pin_15;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_Init(GPIOD, &GPIO_InitStructure);


  /* Set PE.07(D4), PE.08(D5), PE.09(D6), PE.10(D7), PE.11(D8), PE.12(D9), PE.13(D10),
     PE.14(D11), PE.15(D12) as alternate function push pull */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7 | GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_10 |
                                GPIO_Pin_11 | GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14 |
                                GPIO_Pin_15;  
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
  GPIO_Init(GPIOE, &GPIO_InitStructure);

	/* ���� PF.00(A0 (RS))  Ϊ����������� */
	
    //RA8889_wait = 1 ׼���ô������� 0 δ׼����
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init(GPIOD,&GPIO_InitStructure);
	
    GPIO_SetBits(GPIOD, GPIO_Pin_7);          //CS=1 
    GPIO_SetBits(GPIOD, GPIO_Pin_14| GPIO_Pin_15 |GPIO_Pin_0 | GPIO_Pin_1);    
    GPIO_SetBits(GPIOE, GPIO_Pin_7 | GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_10);   
    GPIO_SetBits(GPIOD, GPIO_Pin_4);          //RD=1
    GPIO_SetBits(GPIOD, GPIO_Pin_5);          //WR=1
		
		//PD12 RESET
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(GPIOD, &GPIO_InitStructure);
		
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;// LCD���⣬RA8889�汾Ӳ��
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
		GPIO_Init(GPIOC, &GPIO_InitStructure);
		
		GPIO_SetBits(GPIOC, GPIO_Pin_6);//��������
}

/*
*********************************************************************************************************
*	�� �� ��: LCD_FSMCConfig
*	����˵��: ����FSMC���ڷ���ʱ��
*	��    �Σ���
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void LCD_FSMCConfig(void)
{
    
    FSMC_NORSRAMInitTypeDef  FSMC_NORSRAMInitStructure;
    FSMC_NORSRAMTimingInitTypeDef  p;
	
    p.FSMC_AddressSetupTime = 1;
  p.FSMC_AddressHoldTime = 1;
  p.FSMC_DataSetupTime = 1;
  p.FSMC_BusTurnAroundDuration = 0;
  p.FSMC_CLKDivision = 0;
  p.FSMC_DataLatency = 0;
  p.FSMC_AccessMode = FSMC_AccessMode_B;

  FSMC_NORSRAMInitStructure.FSMC_Bank = FSMC_Bank1_NORSRAM1;
  FSMC_NORSRAMInitStructure.FSMC_DataAddressMux = FSMC_DataAddressMux_Disable;
  //FSMC_NORSRAMInitStructure.FSMC_DataAddressMux = FSMC_DataAddressMux_Enable;
  FSMC_NORSRAMInitStructure.FSMC_MemoryType = FSMC_MemoryType_NOR;
   //FSMC_NORSRAMInitStructure.FSMC_MemoryType = FSMC_MemoryType_PSRAM; //�]�w?P�B�D�_��

  FSMC_NORSRAMInitStructure.FSMC_MemoryDataWidth = FSMC_MemoryDataWidth_16b;
  FSMC_NORSRAMInitStructure.FSMC_BurstAccessMode = FSMC_BurstAccessMode_Disable;
  //FSMC_NORSRAMInitStructure.FSMC_BurstAccessMode = FSMC_BurstAccessMode_Enable;//�]�w��?��o�ǿ�Ҧ�

  FSMC_NORSRAMInitStructure.FSMC_WaitSignalPolarity = FSMC_WaitSignalPolarity_Low;
  //FSMC_NORSRAMInitStructure.FSMC_WaitSignalPolarity = FSMC_WaitSignalPolarity_High;
  FSMC_NORSRAMInitStructure.FSMC_WrapMode = FSMC_WrapMode_Disable;
  //FSMC_NORSRAMInitStructure.FSMC_WrapMode = FSMC_WrapMode_Enable;
  //FSMC_NORSRAMInitStructure.FSMC_WaitSignalActive = FSMC_WaitSignalActive_BeforeWaitState;
  FSMC_NORSRAMInitStructure.FSMC_WaitSignalActive = FSMC_WaitSignalActive_DuringWaitState; 
  FSMC_NORSRAMInitStructure.FSMC_WriteOperation = FSMC_WriteOperation_Enable;
  //FSMC_NORSRAMInitStructure.FSMC_WaitSignal = FSMC_WaitSignal_Disable;
  FSMC_NORSRAMInitStructure.FSMC_WaitSignal = FSMC_WaitSignal_Enable;
  FSMC_NORSRAMInitStructure.FSMC_ExtendedMode = FSMC_ExtendedMode_Disable;
  //FSMC_NORSRAMInitStructure.FSMC_ExtendedMode = FSMC_ExtendedMode_Enable;

  //FSMC_NORSRAMInitStructure.FSMC_AsyncWait = FSMC_AsyncWait_Enable;

  FSMC_NORSRAMInitStructure.FSMC_WriteBurst = FSMC_WriteBurst_Disable;
  //FSMC_NORSRAMInitStructure.FSMC_WriteBurst = FSMC_WriteBurst_Enable;
  FSMC_NORSRAMInitStructure.FSMC_ReadWriteTimingStruct = &p;
  FSMC_NORSRAMInitStructure.FSMC_WriteTimingStruct = &p;	  

  FSMC_NORSRAMInit(&FSMC_NORSRAMInitStructure); 

  /* Enable FSMC Bank1_SRAM Bank */
  FSMC_NORSRAMCmd(FSMC_Bank1_NORSRAM1, ENABLE);  
}
/*
*********************************************************************************************************
*	�� �� ��: LCD_GetHeight
*	����˵��: ��ȡLCD�ֱ���֮�߶�
*	��    �Σ���
*	�� �� ֵ: ��
*********************************************************************************************************
*/
uint16_t LCD_GetHeight(void)
{
	return LCD_legth;
}

/*
*********************************************************************************************************
*	�� �� ��: LCD_GetWidth
*	����˵��: ��ȡLCD�ֱ���֮���
*	��    �Σ���
*	�� �� ֵ: ��
*********************************************************************************************************
*/
uint16_t LCD_GetWidth(void)
{
	return LCD_width;
}
/*
*********************************************************************************************************
*	�� �� ��: LCD_ClrScr
*	����˵��: �����������ɫֵ����
*	��    �Σ�_usColor : ����ɫ
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void LCD_ClrScr(unsigned long _usColor)
{
    Active_Window_XY(0,0);
    Active_Window_WH(LCD_GetWidth(),LCD_GetHeight()); //set full LCD size can draw graph
    Draw_Square_Fill(_usColor,0,0,LCD_GetWidth(),LCD_GetHeight());
}
/*
*********************************************************************************************************
*	�� �� ��: LCD_SetBackLight
*	����˵��: ��ʼ������LCD�������GPIO,����ΪPWMģʽ��
*			���رձ���ʱ����CPU IO����Ϊ��������ģʽ���Ƽ�����Ϊ������������������͵�ƽ)����TIM3�ر� ʡ��
*	��    �Σ�_bright ���ȣ�0����255������
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void LCD_SetBackLight(uint8_t _bright)
{
  if (_bright == 0)
	{
		/* �ر�PWM, PWM0����ȱʡ���LOW  */
		PWM0(0,0,2,255,255);// 78.12khz
		
		GPIO_ResetBits(GPIOC, GPIO_Pin_6); /* �رձ���*/
	}
	else
	{
		/* ʹ��PWM1, ����ռ�ձȵ��� */
		PWM0(1,0,2,BRIGHT_MAX,255-_bright);// BRIGHT_MAX Ϊ255 ʱ�� 78.12khz
		
		GPIO_SetBits(GPIOC, GPIO_Pin_6); /* �򿪱���*/
	}
}
/*
*********************************************************************************************************
*	�� �� ��: LCD_DrawHColorLine
*	����˵��: ����һ����ɫˮƽ�� ����Ҫ����UCGUI�Ľӿں�����
*	��    �Σ�_usX1    ����ʼ��X����
*			  _usY1    ��ˮƽ�ߵ�Y����
*			  _usWidth ��ֱ�ߵĿ��
*			  _pColor : ��ɫ������
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void LCD_DrawHColorLine(uint16_t _usX1 , uint16_t _usY1, uint16_t _usWidth, uint16_t *_pColor)
{
//	MPU16_16bpp_Memory_Write(_usX1,_usY1,_usWidth, 1, _pColor);
  MPU16_24bpp_Mode2_Memory_Write(_usX1,_usY1,_usWidth, 1, _pColor);
}
/*
*********************************************************************************************************
*	�� �� ��: LCD_DrawLineV
*	����˵��: ����һ����ֱ���� ����Ҫ����UCGUI�Ľӿں�����
*	��    �Σ� _usX1    : ��ֱ�ߵ�X����
*			  _usY1    : ��ʼ��Y����
*			  _usY2    : ������Y����
*			  _usColor : ��ɫ
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void LCD_DrawLineV(uint16_t _usX1 , uint16_t _usY1 , uint16_t _usY2, uint32_t _usColor)
{
			Draw_Line(_usColor,_usY1,_usX1,_usY2,_usX1);
//    Active_Window_XY(0,0);
//    Active_Window_WH(LCD_GetWidth(),LCD_GetHeight()); //set full LCD size can draw graph
//    Draw_Line(_usColor,_usX1,_usY1,_usX1,_usY2);
}
/*
*********************************************************************************************************
*	�� �� ��: LCD_DrawHLine
*	����˵��: ����һ��ˮƽ�� ����Ҫ����UCGUI�Ľӿں�����
*	��    �Σ�_usX1    ����ʼ��X����
*			  _usY1    ��ˮƽ�ߵ�Y����
*			  _usX2    ��������X����
*			  _usColor : ��ɫ
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void LCD_DrawLineH(uint16_t _usX1 , uint16_t _usY1 , uint16_t _usX2, uint32_t _usColor)
{
	Draw_Line(_usColor,_usX1,_usY1,_usX1,_usX2);
	
    // Active_Window_XY(0,0);
    // Active_Window_WH(LCD_GetWidth(),LCD_GetHeight()); //set full LCD size can draw graph
    // Draw_Line(_usColor,_usX1,_usY1,_usX2,_usY1);
}
/*
*********************************************************************************************************
*	�� �� ��: LCD_PutPixel
*	����˵��: ��1������
*	��    �Σ�
*			_usX,_usY : ��������
*			_usColor  ��������ɫ
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void LCD_PutPixel(uint16_t _usX, uint16_t _usY, uint32_t _usColor)
{
	putPixel(_usX,_usY,_usColor);
}
/*
*********************************************************************************************************
*	�� �� ��: LCD_GetPixel
*	����˵��: ��ȡ1������
*	��    ��:
*			_usX,_usY : ��������
*	�� �� ֵ: RGB��ɫֵ
*********************************************************************************************************
*/
uint16_t LCD_GetPixel(uint16_t _usX, uint16_t _usY)
{
   uint16_t usRGB;

	Graphic_Mode();
   
	Active_Window_XY(_usX,_usY);
	Active_Window_WH(1,1); 	
	Goto_Pixel_XY(_usX,_usY);
	LCD_CmdWrite(0x04);	
  
	usRGB = LCD_DataRead();// ��Ҫ
	Check_Mem_RD_FIFO_not_Empty();
	
	usRGB = LCD_DataRead();
	Check_Mem_RD_FIFO_not_Empty();
	
//	usRGB = LCD_DataRead();
//	Check_Mem_RD_FIFO_not_Empty();
	
	return usRGB;
}

/*
*********************************************************************************************************
*	�� �� ��: 
*	����˵��: ����ȼ�����
*	��    ��:
*			level������ȼ� 0--10
*	�� �� ֵ: RGB��ɫֵ
*********************************************************************************************************
*/
void ENDisplaySetBackLight(UINT8 level)
{
    level = (UINT8)((UINT16)level * BRIGHT_MAX / 10);
    LCD_SetBackLight(level);
}

/*
*********************************************************************************************************
*	�� �� ��: 
*	����˵��: ���ƾ���
*	��    ��:
*			_usX,_usY : ��������
*			_usColor  :������ɫ
*	�� �� ֵ: RGB��ɫֵ
*********************************************************************************************************
*/
void LCD_DrawRect(uint16_t _usX, uint16_t _usY, uint16_t _usHeight, uint16_t _usWidth, unsigned long _usColor)
{
	Draw_Square_Fill(_usColor,_usX,_usY, _usHeight, _usWidth);
  
}





