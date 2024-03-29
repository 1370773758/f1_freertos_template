#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "stm32f10x.h"

#define Set_SCK  		GPIO_SetBits(GPIOB,GPIO_Pin_3)
#define Reset_SCK		GPIO_ResetBits(GPIOB,GPIO_Pin_3)

#define	Set_CS  		GPIO_SetBits(GPIOA,GPIO_Pin_15)
#define	Reset_CS		GPIO_ResetBits(GPIOA,GPIO_Pin_15)

#define	Set_MISO  	GPIO_SetBits(GPIOB,GPIO_Pin_4)
#define	Reset_MISO	GPIO_ResetBits(GPIOB,GPIO_Pin_4)

#define	Set_MOSI  	GPIO_SetBits(GPIOB,GPIO_Pin_5)
#define	Reset_MOSI	GPIO_ResetBits(GPIOB,GPIO_Pin_5)

#define LCD_XSIZE_TFT 280	
#define LCD_YSIZE_TFT 1424

//#define LINE 4

//#define LCD_HSPW 20//15
//#define LCD_HBPD 40//30
//#define LCD_HFPD 80//30

//#define LCD_VSPW 18
//#define LCD_VBPD 6
//#define LCD_VFPD 35//6

//=================================================
/*--LCD部分定义--------------------------*/
//#define hsize 280
//#define vsize 1424

#define HBP        30
#define HFP        40
#define HSPW       30
#define VBP        12
#define VFP        15																								
#define VSPW       3

/*---SSD2828 PCLK 输出设置-----------------------*/	

//#define MIPI_CLK    0X8220	//MIPI信号输出CLK


/*--通道数的选择--------------------------*/
//#define MIPI_LANES	 0X00	//1 lane
//#define MIPI_LANES	 0X01	//2 lane
//#define MIPI_LANES	 0X02	//3lane
#define MIPI_LANES	 0X0003	    //4 lane




void SSD2828_Configuration(void);
void ENSSD2828Init(void);




