#ifndef _LCD_RA8889_H
#define _LCD_RA8889_H


#include "stm32f10x.h"
#include "Typedef.h"

#include "UserDef.h"
#include "RA8889_MCU_IF.h"
#include "RA8889.h"
#include "RA8889_API.h"

/* ±³¾°¹â¿ØÖÆ */
#define BRIGHT_MAX		255
#define BRIGHT_MIN		0
#define BRIGHT_DEFAULT	200
#define BRIGHT_STEP		5



void LCD_InitHard(void);

uint16_t LCD_GetHeight(void);
uint16_t LCD_GetWidth(void);

void LCD_ClrScr(unsigned long _usColor);
void LCD_SetBackLight(uint8_t _bright);
void LCD_DrawHColorLine(uint16_t _usX1 , uint16_t _usY1, uint16_t _usWidth, uint16_t *_pColor);

void LCD_DrawLineV(uint16_t _usX1 , uint16_t _usY1 , uint16_t _usY2, uint32_t _usColor);
void LCD_DrawLineH(uint16_t _usX1 , uint16_t _usY1 , uint16_t _usX2, uint32_t _usColor);
void LCD_PutPixel(uint16_t _usX, uint16_t _usY, uint32_t _usColor);
uint16_t LCD_GetPixel(uint16_t _usX, uint16_t _usY);
void ENDisplaySetBackLight(UINT8 level);
void LCD_DrawRect(uint16_t _usX, uint16_t _usY, uint16_t _usHeight, uint16_t _usWidth, unsigned long _usColor);


#endif
