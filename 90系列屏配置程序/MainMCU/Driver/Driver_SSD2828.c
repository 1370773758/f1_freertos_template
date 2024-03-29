#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "stm32f10x.h"

#include "Driver_SSD2828.h"

#include "Timer.h"

#define ENSSD2828_DELAY			3

// cs PA15
// CLK PB3

void SSD2828_Configuration(void)
{
  GPIO_InitTypeDef GPIO_InitStructure;
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB, ENABLE);
	
	 GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;	
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;	
  GPIO_Init(GPIOB, &GPIO_InitStructure);			
		
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO,ENABLE);
	GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE); 
	
	
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;	
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;	
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIOB, &GPIO_InitStructure);					 
	
	 GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;	
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;	
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIOB, &GPIO_InitStructure);			
	
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_15;	
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;	
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIOA, &GPIO_InitStructure);					

	ENSSD2828Init();
}


/**
  * @brief  SSD2828发送寄存器命令数据
  * 先发0再发寄存器地址
  * @param  None
  * @retval None
  */ 
void SPI_3W_SET_Cmd(unsigned char cmd)
{
	unsigned char kk;
	
	Reset_MOSI;			//Set DC=0, for writing to Command register
	Reset_SCK;
	ENDelayus(ENSSD2828_DELAY);
	Set_SCK;
	ENDelayus(ENSSD2828_DELAY);	

	Reset_SCK;
	for(kk=0;kk<8;kk++)
	{
		if((cmd&0x80)==0x80) Set_MOSI;
		else         Reset_MOSI;
		
		Reset_SCK;
		ENDelayus(ENSSD2828_DELAY);
		Set_SCK;
		ENDelayus(ENSSD2828_DELAY);
		cmd = cmd<<1;	
	}
}
/**
  * @brief  SSD2828发送数据格式
  * 先发1再发数据
  * @param  None
  * @retval None
  */ 
void SPI_3W_SET_PAs(unsigned char value)
{
	unsigned char kk;

	Set_MOSI;		//Set DC=1, for writing to Data register
	Reset_SCK;
	ENDelayus(ENSSD2828_DELAY);
	Set_SCK;
	ENDelayus(ENSSD2828_DELAY);	
	
	Reset_SCK;
	for(kk=0;kk<8;kk++)
	{
		if((value&0x80)==0x80) Set_MOSI;
		else         Reset_MOSI;
		
		Reset_SCK;
		ENDelayus(ENSSD2828_DELAY);
		Set_SCK;
		ENDelayus(ENSSD2828_DELAY);
		value = value<<1;	
	}	
}
/**
  * @brief  SSD2828读地址
  * 先发1再发数据
  * @param  None
  * @retval None
  */ 
unsigned char SPI_READ_new(void)
{
	unsigned char  reValue,kk;
	
	Reset_CS;
	Set_MISO;
	
	reValue=0;
	for(kk=0;kk<8;kk++)
	{
		reValue = reValue<<1;
		Reset_SCK;
		ENDelayus(ENSSD2828_DELAY);
		if(GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_6)) 
			reValue |= 0x01;
		Set_SCK;
		ENDelayus(ENSSD2828_DELAY);		
	}
	
	Set_CS;
	
	return reValue;			
}
//---------------------------------------------------------------------------------------------------------------

void SPI_2825_WrCmd(unsigned char cmd)
{
	Reset_CS;
	SPI_3W_SET_Cmd(cmd);	
	Set_CS;
}


void SPI_WriteData(unsigned char value)
{
	Reset_CS;
	SPI_3W_SET_PAs(value);
	Set_CS;
}

void SPI_2825_WrReg(unsigned char c,unsigned short value)
{
	Reset_CS;
	SPI_3W_SET_Cmd(c);
	SPI_3W_SET_PAs(value&0xff);
	SPI_3W_SET_PAs((value>>8)&0xff);	
	Set_CS;
}

void GP_COMMAD_PA(unsigned short num)
{
	SPI_2825_WrReg(0xbc, num);
	SPI_2825_WrCmd(0xbf);
}

void SSD2828_WritePackageSize(unsigned short num)
{
	SPI_2825_WrReg(0xbc, num);
	SPI_2825_WrCmd(0xbf);	
}

//---------------------------------------------------------------------------------------------------------------
unsigned short SPI_READ(void)
{
	
	unsigned char  rdT;
	unsigned short reValue;

	SPI_2825_WrCmd(0xB0);
	SPI_2825_WrCmd(0xFA);
	
	reValue = SPI_READ_new();
	rdT = SPI_READ_new();
	reValue += (rdT<<8);
	
	return reValue;		
}


// void SPI_READ_ID(void)
// {
// 	uint16_t SSD2828ID = 0;
// 	SPI_2825_WrReg(0xd4, 0x00FA);
// //	Uart_Printf("2828 ID: 0x%x\n",SPI_READ());
// 	SSD2828ID = SPI_READ();
// }

void SSD_SEND(unsigned short num ,unsigned char value1 ,unsigned char value2)
{
	SSD2828_WritePackageSize(num);
	SPI_WriteData(value1);
	SPI_WriteData(value2);
}
/**
  * @brief  LCD初始化数据
  * @param  None
  * @retval None
  */ 
void LCD_init(void)
{
//SPI_WriteData(0x04,0xB9); /// Set EXTC
SSD2828_WritePackageSize(4);
SPI_WriteData(0xB9);
SPI_WriteData(0xF1);   //1
SPI_WriteData(0x12);   //2
SPI_WriteData(0x83);   //3

//SPI_WriteData(0x1C,0xBA); /// Set DSI
SSD2828_WritePackageSize(0x1C); 
SPI_WriteData(0xBA);
SPI_WriteData(0x33);   //1  // 33 4 lane 32 3 lane
SPI_WriteData(0x81);   //2
SPI_WriteData(0x05);   //3
SPI_WriteData(0xF9);   //4
SPI_WriteData(0x0E);   //5
SPI_WriteData(0x0E);   //6
SPI_WriteData(0x20);   //7
SPI_WriteData(0x00);   //8
SPI_WriteData(0x00);   //9
SPI_WriteData(0x00);   //10
SPI_WriteData(0x00);   //11
SPI_WriteData(0x00);   //12
SPI_WriteData(0x00);   //13
SPI_WriteData(0x00);   //14
SPI_WriteData(0x44);   //15
SPI_WriteData(0x25);   //16
SPI_WriteData(0x00);   //17
SPI_WriteData(0x90);   //18
SPI_WriteData(0x0A);   //19
SPI_WriteData(0x00);   //20
SPI_WriteData(0x00);   //21
SPI_WriteData(0x01);   //22
SPI_WriteData(0x4F);   //23
SPI_WriteData(0xD1);   //24
SPI_WriteData(0x00);   //25
SPI_WriteData(0x00);   //26
SPI_WriteData(0x37);   //27



//SPI_WriteData(0x04,0xB2); /// Set RSO
SSD2828_WritePackageSize(0x04); 
SPI_WriteData(0xB2);
SPI_WriteData(0xEC);   //1
SPI_WriteData(0x14);   //2
SPI_WriteData(0x70);   //3

//SPI_WriteData(0x0B,0xB3); /// SET RGB
SSD2828_WritePackageSize(0x0B); 
SPI_WriteData(0xB3);
SPI_WriteData(0x10);   //1 VBP_RGB_GEN 
SPI_WriteData(0x10);   //2 VFP_RGB_GEN 
SPI_WriteData(0x28);   //3 DE_BP_RGB_GEN 
SPI_WriteData(0x28);   //4 DE_FP_RGB_GEN 
SPI_WriteData(0x03);   //5
SPI_WriteData(0xFF);   //6
SPI_WriteData(0x00);   //7
SPI_WriteData(0x00);   //8
SPI_WriteData(0x00);   //9
SPI_WriteData(0x00);   //10

//SPI_WriteData(0x02,0xB4); /// Set Panel Inversion
SSD2828_WritePackageSize(0x02); 
SPI_WriteData(0xB4);
SPI_WriteData(0x80);   //1

//SPI_WriteData(0x03,0xB5); /// Set BGP
SSD2828_WritePackageSize(0x03); 
SPI_WriteData(0xB5);
SPI_WriteData(0x0D);   //1 vref
SPI_WriteData(0x0D);   //2 nvref

//SPI_WriteData(0x03,0xB6); /// Set VCOM
SSD2828_WritePackageSize(0x03); 
SPI_WriteData(0xB6);
SPI_WriteData(0x4E);   //1 F_VCOM
SPI_WriteData(0x4E);   //2 B_VCOM

//SPI_WriteData(0x05,0xB8); ///Set ECP
SSD2828_WritePackageSize(0x05); 
SPI_WriteData(0xB8);
SPI_WriteData(0x26);  //0x76 for 3 Power Mode,0x26 for Power IC Mode   
SPI_WriteData(0x22);  //   
SPI_WriteData(0xF0);  //
SPI_WriteData(0x63);  // 

//SPI_WriteData(0x02,0xBC); /// Set VDC
SSD2828_WritePackageSize(0x02); 
SPI_WriteData(0xBC);
SPI_WriteData(0x46);   //1 defaut=46

//SPI_WriteData(0x04,0xBF); ///Set PCR
SSD2828_WritePackageSize(0x04); 
SPI_WriteData(0xBF);
SPI_WriteData(0x02);  //   
SPI_WriteData(0x11);
SPI_WriteData(0x00);

//SPI_WriteData(0x0A,0xC0); /// Set SCR
SSD2828_WritePackageSize(0x0A); 
SPI_WriteData(0xC0);
SPI_WriteData(0x73);   //1  
SPI_WriteData(0x73);   //2  
SPI_WriteData(0x50);   //3
SPI_WriteData(0x50);   //4
SPI_WriteData(0x00);   //5
SPI_WriteData(0x00);   //6
SPI_WriteData(0x12);   //7
SPI_WriteData(0x50);   //8  
SPI_WriteData(0x00);   //9


//SPI_WriteData(0x0D,0xC1); /// Set POWER	
SSD2828_WritePackageSize(0x0D); 
SPI_WriteData(0xC1);
SPI_WriteData(0x53);   //1 VBTHS VBTLS 	
SPI_WriteData(0x00);	//2 E3
SPI_WriteData(0x32);   //3 VSPR	
SPI_WriteData(0x32);   //4 VSNR	
SPI_WriteData(0x99);   //5 VSP VSN	
SPI_WriteData(0xE4);   //6 APS	
SPI_WriteData(0xFF);   //7 VGH1 VGL1	
SPI_WriteData(0xFF);   //8 VGH1 VGL1	
SPI_WriteData(0xCC);   //9 VGH2 VGL2	
SPI_WriteData(0xCC);   //10 VGH2 VGL2	
SPI_WriteData(0x77);   //11 VGH3 VGL3	
SPI_WriteData(0x77);   //12 VGH3 VGL3


//SPI_WriteData(0x02,0xCC); /// Set Panel
SSD2828_WritePackageSize(0x02); 
SPI_WriteData(0xCC);
SPI_WriteData(0x0B);   //1 Forward:0x0B , Backward:0x07


//SPI_WriteData(0x23,0xE0); /// Set Gamma2.2
SSD2828_WritePackageSize(0x23); 
SPI_WriteData(0xE0);
SPI_WriteData(0x00);  //1
SPI_WriteData(0x0A);  //2
SPI_WriteData(0x11);  //3
SPI_WriteData(0x2B);  //4
SPI_WriteData(0x3B);  //5
SPI_WriteData(0x3F);  //6
SPI_WriteData(0x44);  //7
SPI_WriteData(0x3B);  //8
SPI_WriteData(0x07);  //9
SPI_WriteData(0x0D);  //10
SPI_WriteData(0x0E);  //11
SPI_WriteData(0x12);  //12
SPI_WriteData(0x12);  //13
SPI_WriteData(0x11);  //14
SPI_WriteData(0x13);  //15
SPI_WriteData(0x11);  //16
SPI_WriteData(0x17);  //17
SPI_WriteData(0x00);  //1
SPI_WriteData(0x0A);  //2
SPI_WriteData(0x11);  //3
SPI_WriteData(0x2B);  //4
SPI_WriteData(0x3B);  //5
SPI_WriteData(0x3F);  //6
SPI_WriteData(0x44);  //7
SPI_WriteData(0x3B);  //8
SPI_WriteData(0x07);  //9
SPI_WriteData(0x0D);  //10
SPI_WriteData(0x0E);  //11
SPI_WriteData(0x12);  //12
SPI_WriteData(0x12);  //13
SPI_WriteData(0x11);  //14
SPI_WriteData(0x13);  //15
SPI_WriteData(0x11);  //16
SPI_WriteData(0x17);  //17



//SPI_WriteData(0x0F,0xE3); /// Set EQ
SSD2828_WritePackageSize(0x0F); 
SPI_WriteData(0xE3);
SPI_WriteData(0x03);   //1  PNOEQ                        
SPI_WriteData(0x03);   //2  NNOEQ                        
SPI_WriteData(0x03);   //3  PEQGND                       
SPI_WriteData(0x03);   //4  NEQGND                       
SPI_WriteData(0x00);   //5  PEQVCI                       
SPI_WriteData(0x03);   //6  NEQVCI                       
SPI_WriteData(0x00);   //7  PEQVCI1                      
SPI_WriteData(0x00);   //8  NEQVCI1                      
SPI_WriteData(0x00);   //9  VCOM_PULLGND_OFF             
SPI_WriteData(0x00);   //10 VCOM_PULLGND_OFF             
SPI_WriteData(0xFF);   //11 VCOM_IDLE_ON                 
SPI_WriteData(0x80);   //12                              
SPI_WriteData(0xC0);   //13 defaut C0 ESD detect function
SPI_WriteData(0x10);   //14 SLPOTP                      



//SPI_WriteData(0x40,0xE9); /// Set GIP
SSD2828_WritePackageSize(0x40); 
SPI_WriteData(0xE9);
SPI_WriteData(0xC8);  //1  PANSEL      //   08
SPI_WriteData(0x10);  //2  SHR_0[11:8] //   00
SPI_WriteData(0x0A);  //3  SHR_0[7:0]  //   07
SPI_WriteData(0x10);  //4  SHR_1[11:8]      00
SPI_WriteData(0x0D);  //5  SHR_1[7:0]       04
SPI_WriteData(0x80);  //6  SPON[7:0]   
SPI_WriteData(0x38);  //7  SPOFF[7:0]  
SPI_WriteData(0x12);  //8  SHR0_1[3:0], SHR0_2[3:0]
SPI_WriteData(0x31);  //9  SHR0_3[3:0], SHR1_1[3:0]
SPI_WriteData(0x23);  //10  SHR1_2[3:0], SHR1_3[3:0]
SPI_WriteData(0x4F);  //11  SHP[3:0], SCP[3:0]  
SPI_WriteData(0x86);  //12  CHR[7:0]  //    0B 
SPI_WriteData(0x80);  //13  CON[7:0]  
SPI_WriteData(0x38);  //14  COFF[7:0]  
SPI_WriteData(0x47);  //15  CHP[3:0], CCP[3:0]  
SPI_WriteData(0x08);  //16  USER_GIP_GATE[7:0]
SPI_WriteData(0x00);  //17  CGTS_L[21:16]
SPI_WriteData(0x00);  //18  CGTS_L[15:8]
SPI_WriteData(0x4F);  //19  CGTS_L[7:0]
SPI_WriteData(0x00);  //20  CGTS_INV_L[21:16]
SPI_WriteData(0x00);  //21  CGTS_INV_L[15:8]
SPI_WriteData(0x04);  //22  CGTS_INV_L[7:0]
SPI_WriteData(0x00);  //23  CGTS_R[21:16]
SPI_WriteData(0x00);  //24  CGTS_R[15:8]
SPI_WriteData(0x4F);  //25  CGTS_R[7:0]
SPI_WriteData(0x00);  //26  CGTS_INV_R[21:16]
SPI_WriteData(0x00);  //27  CGTS_INV_R[15:8]
SPI_WriteData(0x04);  //28  CGTS_INV_R[7:0]

SPI_WriteData(0x94);  //29  COS1_R[3:0],  COS2_R[3:0] ,//   GCL  ,  STV0
SPI_WriteData(0xA3);  //30  COS3_R[3:0],  COS4_R[3:0] ,//   GCH  ,  STV4  
SPI_WriteData(0xF8);  //31  COS5_R[3:0],  COS6_R[3:0] ,//   VDS  ,  VSD 
SPI_WriteData(0x18);  //32  COS7_R[3:0],  COS8_R[3:0] ,//   STV2 ,  VGL
SPI_WriteData(0x13);  //33  COS9_R[3:0],  COS10_R[3:0],//   CLK6 ,  CLK8
SPI_WriteData(0x57);  //34  COS11_R[3:0], COS12_R[3:0],//   CLK2 ,  CLK4
SPI_WriteData(0x88);  //35  COS13_R[3:0], COS14_R[3:0],//
SPI_WriteData(0x88);  //36  COS15_R[3:0], COS16_R[3:0],//
SPI_WriteData(0x88);  //37  COS17_R[3:0], COS18_R[3:0],// 
SPI_WriteData(0x88);  //38  COS19_R[3:0], COS20_R[3:0],// 
SPI_WriteData(0x88);  //39  COS21_R[3:0], COS22_R[3:0],// 

SPI_WriteData(0x94);  //40  COS1_L[3:0],  COS2_L[3:0] ,//   GCL  ,  STV0 
SPI_WriteData(0xA2);  //41  COS3_L[3:0],  COS4_L[3:0] ,//   GCH  ,  STV3 
SPI_WriteData(0xF8);  //42  COS5_L[3:0],  COS6_L[3:0] ,//   VDS  ,  VSD
SPI_WriteData(0x08);  //43  COS7_L[3:0],  COS8_L[3:0] ,//   STV1 ,  VGL
SPI_WriteData(0x02);  //44  COS9_L[3:0],  COS10_L[3:0],//   CLK5 ,  CLK7
SPI_WriteData(0x46);  //45  COS11_L[3:0], COS12_L[3:0],//   CLK1 ,  CLK3
SPI_WriteData(0x88);  //46  COS13_L[3:0], COS14_L[3:0],//
SPI_WriteData(0x88);  //47  COS15_L[3:0], COS16_L[3:0],//
SPI_WriteData(0x88);  //48  COS17_L[3:0], COS18_L[3:0],//      
SPI_WriteData(0x88);  //49  COS19_L[3:0], COS20_L[3:0],// 
SPI_WriteData(0x88);  //50  COS21_L[3:0], COS22_L[3:0],//
SPI_WriteData(0x00);  //51  TCONOPTION
SPI_WriteData(0x00);  //52  OPTION
SPI_WriteData(0x00);  //53  OTPION
SPI_WriteData(0x01);  //54  OPTION
SPI_WriteData(0x00);  //55  CHR2
SPI_WriteData(0x80);  //56  CON2
SPI_WriteData(0x38);  //57  COFF2
SPI_WriteData(0x00);  //58  CHP2,CCP2
SPI_WriteData(0x00);  //59  CKS 21 20 19 18 17 16
SPI_WriteData(0x00);  //60  CKS 15 14 13 12 11 10 9 8
SPI_WriteData(0x00);  //61  CKS 7~0
SPI_WriteData(0x00);  //62  COFF[7:6]   CON[5:4]    SPOFF[3:2]    SPON[1:0]
SPI_WriteData(0x00);  //63  COFF2[7:6]    CON2[5:4]   - - - -

//SPI_WriteData(0x3E,0xEA); /// Set GIP2
SSD2828_WritePackageSize(0x3E); 
SPI_WriteData(0xEA);
SPI_WriteData(0x00);  //1  ys2_sel[1:0]
SPI_WriteData(0x1A);  //2  user_gip_gate1[7:0]
SPI_WriteData(0x00);  //3  ck_all_on_width1[5:0]
SPI_WriteData(0x00);  //4  ck_all_on_width2[5:0]
SPI_WriteData(0x00);  //5  ck_all_on_width3[5:0]
SPI_WriteData(0x00);  //6  ys_flag_period[7:0]
SPI_WriteData(0x01);  //7  ys_2
SPI_WriteData(0x0A);  //8  user_gip_gate1_2[7:0]
SPI_WriteData(0x41);  //9  ck_all_on_width1_2[5:0]
SPI_WriteData(0x01);  //10 ck_all_on_width2_2[5:0]
SPI_WriteData(0x02);  //11 ck_all_on_width3_2[5:0]
SPI_WriteData(0x00);  //12 ys_flag_period_2[7:0]
SPI_WriteData(0x94);  //13  COS1_R[3:0],  COS2_R[3:0] ,//   GCL  ,  STV0
SPI_WriteData(0xA0);  //14  COS3_R[3:0],  COS4_R[3:0] ,//   GCH  ,  STV4  
SPI_WriteData(0x8F);  //15  COS5_R[3:0],  COS6_R[3:0] ,//   VDS  ,  VSD 
SPI_WriteData(0x28);  //16  COS7_R[3:0],  COS8_R[3:0] ,//   STV2 ,  VGL
SPI_WriteData(0x64);  //17  COS9_R[3:0],  COS10_R[3:0],//   CLK6 ,  CLK8
SPI_WriteData(0x20);  //18  COS11_R[3:0], COS12_R[3:0],//   CLK2 ,  CLK4
SPI_WriteData(0x88);  //19  COS13_R[3:0], COS14_R[3:0],//
SPI_WriteData(0x88);  //20  COS15_R[3:0], COS16_R[3:0],//
SPI_WriteData(0x88);  //21  COS17_R[3:0], COS18_R[3:0],// 
SPI_WriteData(0x88);  //22  COS19_R[3:0], COS20_R[3:0],// 
SPI_WriteData(0x88);  //23  COS21_R[3:0], COS22_R[3:0],// 
SPI_WriteData(0x94);  //24  COS1_L[3:0],  COS2_L[3:0] ,//   GCL  ,  STV0 
SPI_WriteData(0xA1);  //25  COS3_L[3:0],  COS4_L[3:0] ,//   GCH  ,  STV3 
SPI_WriteData(0x8F);  //26  COS5_L[3:0],  COS6_L[3:0] ,//   VDS  ,  VSD
SPI_WriteData(0x38);  //27  COS7_L[3:0],  COS8_L[3:0] ,//   STV1 ,  VGL
SPI_WriteData(0x75);  //28  COS9_L[3:0],  COS10_L[3:0],//   CLK5 ,  CLK7
SPI_WriteData(0x31);  //29  COS11_L[3:0], COS12_L[3:0],//   CLK1 ,  CLK3
SPI_WriteData(0x88);  //30  COS13_L[3:0], COS14_L[3:0],//
SPI_WriteData(0x88);  //31  COS15_L[3:0], COS16_L[3:0],//
SPI_WriteData(0x88);  //32  COS17_L[3:0], COS18_L[3:0],//      
SPI_WriteData(0x88);  //33  COS19_L[3:0], COS20_L[3:0],// 
SPI_WriteData(0x88);  //34  COS21_L[3:0], COS22_L[3:0],//
SPI_WriteData(0x23);  //35 EQOPT , EQ_SEL
SPI_WriteData(0x10);  //36 EQ_DELAY[7:0]
SPI_WriteData(0x00);  //37 EQ_DELAY_HSYNC [3:0]
SPI_WriteData(0x00);  //38 HSYNC_TO_CL1_CNT9[8]
SPI_WriteData(0xE8);  //39 HSYNC_TO_CL1_CNT9[7:0]
SPI_WriteData(0x00);  //40 HIZ_L
SPI_WriteData(0x00);  //41 HIZ_R
SPI_WriteData(0x00);  //42 CKS_GS[21:16]
SPI_WriteData(0x00);  //43 CKS_GS[15:8]
SPI_WriteData(0x00);  //44 CKS_GS[7:0]
SPI_WriteData(0x00);  //45 CK_MSB_EN[21:16]
SPI_WriteData(0x00);  //46 CK_MSB_EN[15:8]
SPI_WriteData(0x00);  //47 CK_MSB_EN[7:0]
SPI_WriteData(0x00);  //48 CK_MSB_EN_GS[21:16]
SPI_WriteData(0x00);  //49 CK_MSB_EN_GS[15:8]
SPI_WriteData(0x00);  //50 CK_MSB_EN_GS[7:0]
SPI_WriteData(0x05);  //51  SHR2[11:8]
SPI_WriteData(0xAA);  //52  SHR2[7:0]
SPI_WriteData(0x00);  //53  SHR2_1[3:0] SHR2_2
SPI_WriteData(0x00);  //54  SHR2_3[3:0]
SPI_WriteData(0x40);  //55 SHP1[3:0]
SPI_WriteData(0x80);  //56 SPON1[7:0]
SPI_WriteData(0x38);  //57 SPOFF1[7:0]
SPI_WriteData(0x40);  //58 SHP2[3:0]
SPI_WriteData(0x80);  //59 SPON2[7:0]
SPI_WriteData(0x38);  //60 SPOFF2[7:0]
SPI_WriteData(0x00);  //61 SPOFF2[9:8]/SPON2[9:8]/SPOFF1[9:8]/SPON1[9:8]




SSD2828_WritePackageSize(1);SPI_WriteData(0x11);
ENSleep(200);

//SPI_WriteData(0x01,0x29); ///Display On
//DelayX1ms(50);
SSD2828_WritePackageSize(1);SPI_WriteData(0x29);
ENSleep(100);

}

void ENSSD2828Init(void)
{	
//	SSD2825_Initial
SPI_2825_WrCmd(0xb7);
SPI_WriteData(0x50);//50=TX_CLK 70=PCLK 使能DCS
SPI_WriteData(0x00);   //Configuration Register

SPI_2825_WrCmd(0xb8);
SPI_WriteData(0x00);
SPI_WriteData(0x00);   //VC(Virtual ChannelID) Control Register

SPI_2825_WrCmd(0xb9);
SPI_WriteData(0x00);//1=PLL disable
SPI_WriteData(0x00);
		                               //TX_CLK/MS should be between 5Mhz to100Mhz
SPI_2825_WrCmd(0xBA);//PLL=(TX_CLK/MS)*NS 8228=480M 4428=240M  061E=120M 4214=240M 821E=360M 8219=300M
SPI_WriteData(0x14);//D7-0=NS(0x01 : NS=1)
SPI_WriteData(0x42);//D15-14=PLL范围 00=62.5-125 01=126-250 10=251-500 11=501-1000  DB12-8=MS(01:MS=1)

SPI_2825_WrCmd(0xBB);//LP Clock Divider LP clock = 400MHz / LPD / 8 = 240 / 8 / 4 = 7.5MHz
SPI_WriteData(0x03);//D5-0=LPD=0x1 – Divide by 2
SPI_WriteData(0x00);

SPI_2825_WrCmd(0xb9);
SPI_WriteData(0x01);//1=PLL disable
SPI_WriteData(0x00);
//MIPI lane configuration
SPI_2825_WrCmd(0xDE);//通道数
SPI_WriteData(MIPI_LANES);//11=4LANE 10=3LANE 01=2LANE 00=1LANE
SPI_WriteData(0x00);

SPI_2825_WrCmd(0xc9);
SPI_WriteData(0x02);
SPI_WriteData(0x23);//(0x23);   //p1: HS-Data-zero  p2: HS-Data- prepare  --> 8031 issue

	////////////////////LCD Initial  CODE/////////////////////
	LCD_init();
	//============Vedio parats=====================
	//SSD2825_Initial
	SPI_2825_WrCmd(0xb7);
	SPI_WriteData(0x50);
	SPI_WriteData(0x00);   //Configuration Register

	SPI_2825_WrCmd(0xb8);
	SPI_WriteData(0x00);
	SPI_WriteData(0x00);   //VC(Virtual ChannelID) Control Register

	SPI_2825_WrCmd(0xb9);
	SPI_WriteData(0x00);//1=PLL disable
	SPI_WriteData(0x00);

	SPI_2825_WrCmd(0xBA);//PLL=(TX_CLK/MS)*NS 8228=480M 4428=240M  061E=120M 4214=240M 821E=360M 8219=300M
	SPI_WriteData(0x28);//D7-0=NS(0x01 : NS=1)
	SPI_WriteData(0x82);//D15-14=PLL范围 00=62.5-125 01=126-250 10=251-500 11=501-1000  DB12-8=MS(01:MS=1)

	SPI_2825_WrCmd(0xBB);//LP Clock Divider LP clock = 400MHz / LPD / 8 = 300 / 5 / 8 = 7.5MHz
	SPI_WriteData(0x07);//D5-0=LPD=0x1 – Divide by 2
	SPI_WriteData(0x00);

	SPI_2825_WrCmd(0xb9);
	SPI_WriteData(0x01);//1=PLL disable
	SPI_WriteData(0x00);

	SPI_2825_WrCmd(0xc9);
	SPI_WriteData(0x02);
	SPI_WriteData(0x23);   //p1: HS-Data-zero  p2: HS-Data- prepare  --> 8031 issue
	ENSleep(10);

	SPI_2825_WrCmd(0xCA);
	SPI_WriteData(0x01);//CLK Prepare
	SPI_WriteData(0x23);//Clk Zero

	SPI_2825_WrCmd(0xCB); //local_write_reg(addr=0xCB,data=0x0510)
	SPI_WriteData(0x10); //Clk Post
	SPI_WriteData(0x05); //Clk Per

	SPI_2825_WrCmd(0xCC); //local_write_reg(addr=0xCC,data=0x100A)
	SPI_WriteData(0x05); //HS Trail
	SPI_WriteData(0x10); //Clk Trail


	SPI_2825_WrCmd(0xB1); //local_write_reg(addr=0xB2,data=0x1224)
	SPI_WriteData(HSPW); //HSA
	SPI_WriteData(VSPW); //VSA for HX8369A-Ver.B


	SPI_2825_WrCmd(0xB2); //local_write_reg(addr=0xB2,data=0x1224)
	SPI_WriteData(HBP); //HBP
	SPI_WriteData(VBP); //VBP for HX8369A-Ver.B


	SPI_2825_WrCmd(0xB3); //local_write_reg(addr=0xB3,data=0x060C)
	SPI_WriteData(HFP); //HFP
	SPI_WriteData(VFP); //VFP for HX8369A-Ver.B 10

	SPI_2825_WrReg(0xb4, LCD_XSIZE_TFT);		//Horizontal active period 
	SPI_2825_WrReg(0xb5, LCD_YSIZE_TFT);		//Vertical active period 	


	SPI_2825_WrCmd(0xB6);	  //(addr=0xB6,data=0x011F)
	SPI_WriteData(0x03);   //  最低8bit VM VPF  VM= 0x00 sync pulse  0x01 sync events 0x10 burst mode  vpf: 0x11 24bit 0x00 16bit 0x01,0x10 18bit
	SPI_WriteData(0x00);

	//MIPI lane configuration
	SPI_2825_WrCmd(0xDE);//通道数
	SPI_WriteData(MIPI_LANES);//03=4LANE 02=3LANE 01=2LANE 00=1LANE
	SPI_WriteData(0x00);

	SPI_2825_WrCmd(0xD6);//  05=RGB  04=BGR
	SPI_WriteData(0x05);//D0=0=RGB 1:BGR D1=1=Most significant byte sent first
	SPI_WriteData(0x00);

	SPI_2825_WrCmd(0xB7);
	SPI_WriteData(0x4B);
	SPI_WriteData(0x02);
	
	
	SPI_2825_WrCmd(0x2C);
}
