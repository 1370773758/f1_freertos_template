/*****************************************************************
 * File: HomeView.c
 * Date: 2018/6/30 11:32
 * Copyright@2018-2028: MedRena, All Right Reserved
 *
 * Note:
 *
******************************************************************/
/*********************************************************************
*                                                                    *
*                SEGGER Microcontroller GmbH & Co. KG                *
*        Solutions for real time microcontroller applications        *
*                                                                    *
**********************************************************************
*                                                                    *
* C-file generated by:                                               *
*                                                                    *
*        GUI_Builder for emWin version 5.20                          *
*        Compiled Mar 19 2013, 15:01:00                              *
*        (c) 2013 Segger Microcontroller GmbH & Co. KG               *
*                                                                    *
**********************************************************************
*                                                                    *
*        Internet: www.segger.com  Support: support@segger.com       *
*                                                                    *
**********************************************************************
*/

// USER START (Optionally insert additional includes)
// USER END

#include "GUI.h"
#include "DIALOG.h"

#include "HomeView.h"

#include "Typedef.h"

extern GUI_CONST_STORAGE GUI_BITMAP bm80SystemInfo;

#define TEXT_FONT     		(GUI_FONT_16B_ASCII) /*GUI_FONT_16B_ASCII*/
#define TEXT_BKCOLOR  		(EN_RGB(236,250,251))
#define WINDOW_BACK_COLOR (EN_RGB(236,250,251))

#define LEFT_LINE_COLOR		(LEFT_THEME_COLOR)
#define RIGHT_LINE_COLOR	(RIGHT_THEME_COLOR)
/*********************************************************************
*
*       Defines
*
**********************************************************************
*/
#define ID_WINDOW_0 (GUI_ID_USER + 0x00)
#define ID_IMAGE_0  (GUI_ID_USER + 0x01)
#define ID_IMAGE_1  (GUI_ID_USER + 0x02)
#define ID_IMAGE_2  (GUI_ID_USER + 0x03)
#define ID_IMAGE_3  (GUI_ID_USER + 0x04)
#define ID_IMAGE_4  (GUI_ID_USER + 0x05)
#define ID_IMAGE_5  (GUI_ID_USER + 0x06)
#define ID_IMAGE_6  (GUI_ID_USER + 0x07)
#define ID_IMAGE_7  (GUI_ID_USER + 0x08)
#define ID_IMAGE_8  (GUI_ID_USER + 0x09)
#define ID_IMAGE_9  (GUI_ID_USER + 0x0A)

#define ID_TEXT_0  (GUI_ID_USER + 0x10)
#define ID_TEXT_1  (GUI_ID_USER + 0x11)
#define ID_TEXT_2  (GUI_ID_USER + 0x12)
#define ID_TEXT_3  (GUI_ID_USER + 0x13)
#define ID_TEXT_4  (GUI_ID_USER + 0x14)
#define ID_TEXT_5  (GUI_ID_USER + 0x15)
#define ID_TEXT_6  (GUI_ID_USER + 0x16)
#define ID_TEXT_7  (GUI_ID_USER + 0x17)
#define ID_TEXT_8  (GUI_ID_USER + 0x18)
#define ID_TEXT_9  (GUI_ID_USER + 0x19)

#define ID_SCALE_0  (GUI_ID_USER + 0x20)
#define ID_SCALE_1  (GUI_ID_USER + 0x21)
#define ID_SCALE_2  (GUI_ID_USER + 0x22)
#define ID_SCALE_3  (GUI_ID_USER + 0x23)
#define ID_SCALE_4  (GUI_ID_USER + 0x24)
#define ID_SCALE_5  (GUI_ID_USER + 0x25)
#define ID_SCALE_6  (GUI_ID_USER + 0x26)
#define ID_SCALE_7  (GUI_ID_USER + 0x27)
#define ID_SCALE_8  (GUI_ID_USER + 0x28)
#define ID_SCALE_9  (GUI_ID_USER + 0x29)

#define ID_TEXT_11 	(GUI_ID_USER + 0x1B)
#define ID_TEXT_12 	(GUI_ID_USER + 0x1C)
#define ID_SCALE_11	(GUI_ID_USER + 0x2B)
#define ID_SCALE_12 (GUI_ID_USER + 0x2C)
/*********************************************************************
*
*       _aDialogCreate
*/
#define IMG_START_XSIZE						(331)
#define IMG_START_YSIZE						(18)
#define IMG_SIZE 									(75)
#define IMG_MIDGAP								(36)
#define IMG_RIGHT2LEFTSPACE_SIZE	(159)
#define IMG_TOP2DOWNSPACE_SIZE		(36)

#define TEXT_START_XSIZE					(IMG_START_XSIZE)
#define TEXT_START_YSIZE					(IMG_START_YSIZE + IMG_SIZE)
#define TEXT_TOP2DOWNSPACE_SIZE		(IMG_START_YSIZE+IMG_TOP2DOWNSPACE_SIZE+IMG_SIZE+8)
#define TEXT_RIGHT2LEFTSPACE_SIZE	(126)
#define TEXT_XSIZE								(100)
#define TEXT_YSIZE								(39)
static const GUI_WIDGET_CREATE_INFO _aDialogCreate[] = {
	{ WINDOW_CreateIndirect, "HomeView", ID_WINDOW_0, 0,0 , 1424, 280, 0, 0x0, 0 },
	{ IMAGE_CreateIndirect, "Image", ID_IMAGE_0, IMG_START_XSIZE + (IMG_SIZE+IMG_RIGHT2LEFTSPACE_SIZE)*0, IMG_START_YSIZE, 																			IMG_SIZE, IMG_SIZE, 0, 0, 0 },
	{ IMAGE_CreateIndirect, "Image", ID_IMAGE_1, IMG_START_XSIZE + (IMG_SIZE+IMG_RIGHT2LEFTSPACE_SIZE)*1, IMG_START_YSIZE, 																			IMG_SIZE, IMG_SIZE, 0, 0, 0 },
	{ IMAGE_CreateIndirect, "Image", ID_IMAGE_2, IMG_START_XSIZE + (IMG_SIZE+IMG_RIGHT2LEFTSPACE_SIZE)*2, IMG_START_YSIZE, 																			IMG_SIZE, IMG_SIZE, 0, 0, 0 },
	{ IMAGE_CreateIndirect, "Image", ID_IMAGE_3, IMG_START_XSIZE + (IMG_SIZE+IMG_RIGHT2LEFTSPACE_SIZE)*3, IMG_START_YSIZE, 																			IMG_SIZE, IMG_SIZE, 0, 0, 0 },
	{ IMAGE_CreateIndirect, "Image", ID_IMAGE_4, IMG_START_XSIZE + (IMG_SIZE+IMG_RIGHT2LEFTSPACE_SIZE)*4, IMG_START_YSIZE, 																			IMG_SIZE, IMG_SIZE, 0, 0, 0 },
	{ IMAGE_CreateIndirect, "Image", ID_IMAGE_5, IMG_START_XSIZE + (IMG_SIZE+IMG_RIGHT2LEFTSPACE_SIZE)*0, IMG_START_YSIZE + IMG_SIZE + IMG_TOP2DOWNSPACE_SIZE + TEXT_YSIZE, 	IMG_SIZE, IMG_SIZE, 0, 0, 0 },
	{ IMAGE_CreateIndirect, "Image", ID_IMAGE_6, IMG_START_XSIZE + (IMG_SIZE+IMG_RIGHT2LEFTSPACE_SIZE)*1, IMG_START_YSIZE + IMG_SIZE + IMG_TOP2DOWNSPACE_SIZE + TEXT_YSIZE, 	IMG_SIZE, IMG_SIZE, 0, 0, 0 },
	{ IMAGE_CreateIndirect, "Image", ID_IMAGE_7, IMG_START_XSIZE + (IMG_SIZE+IMG_RIGHT2LEFTSPACE_SIZE)*2, IMG_START_YSIZE + IMG_SIZE + IMG_TOP2DOWNSPACE_SIZE + TEXT_YSIZE, 	IMG_SIZE, IMG_SIZE, 0, 0, 0 },
	{ IMAGE_CreateIndirect, "Image", ID_IMAGE_8, IMG_START_XSIZE + (IMG_SIZE+IMG_RIGHT2LEFTSPACE_SIZE)*3, IMG_START_YSIZE + IMG_SIZE + IMG_TOP2DOWNSPACE_SIZE + TEXT_YSIZE, 	IMG_SIZE, IMG_SIZE, 0, 0, 0 },
	{ IMAGE_CreateIndirect, "Image", ID_IMAGE_9, IMG_START_XSIZE + (IMG_SIZE+IMG_RIGHT2LEFTSPACE_SIZE)*4, IMG_START_YSIZE + IMG_SIZE + IMG_TOP2DOWNSPACE_SIZE + TEXT_YSIZE, 	IMG_SIZE, IMG_SIZE, 0, 0, 0 },

	{ TEXT_CreateIndirect, "", ID_TEXT_0, TEXT_START_XSIZE + (TEXT_XSIZE + TEXT_RIGHT2LEFTSPACE_SIZE)*0, TEXT_START_YSIZE,  TEXT_XSIZE, TEXT_YSIZE, 0, 0x0, 0 },
	{ TEXT_CreateIndirect, "", ID_TEXT_1, TEXT_START_XSIZE + (TEXT_XSIZE + TEXT_RIGHT2LEFTSPACE_SIZE)*1, TEXT_START_YSIZE,  TEXT_XSIZE, TEXT_YSIZE, 0, 0x0, 0 },
	{ TEXT_CreateIndirect, "", ID_TEXT_2, TEXT_START_XSIZE + (TEXT_XSIZE + TEXT_RIGHT2LEFTSPACE_SIZE)*2, TEXT_START_YSIZE,  TEXT_XSIZE, TEXT_YSIZE, 0, 0x0, 0 },
	{ TEXT_CreateIndirect, "", ID_TEXT_3, TEXT_START_XSIZE + (TEXT_XSIZE + TEXT_RIGHT2LEFTSPACE_SIZE)*3, TEXT_START_YSIZE,  TEXT_XSIZE, TEXT_YSIZE, 0, 0x0, 0 },
	{ TEXT_CreateIndirect, "", ID_TEXT_4, TEXT_START_XSIZE + (TEXT_XSIZE + TEXT_RIGHT2LEFTSPACE_SIZE)*4, TEXT_START_YSIZE,  TEXT_XSIZE, TEXT_YSIZE, 0, 0x0, 0 },
	{ TEXT_CreateIndirect, "", ID_TEXT_5, TEXT_START_XSIZE + (TEXT_XSIZE + TEXT_RIGHT2LEFTSPACE_SIZE)*0, TEXT_START_YSIZE + TEXT_TOP2DOWNSPACE_SIZE,  TEXT_XSIZE, TEXT_YSIZE, 0, 0x0, 0 },
	{ TEXT_CreateIndirect, "", ID_TEXT_6, TEXT_START_XSIZE + (TEXT_XSIZE + TEXT_RIGHT2LEFTSPACE_SIZE)*1, TEXT_START_YSIZE + TEXT_TOP2DOWNSPACE_SIZE,  TEXT_XSIZE, TEXT_YSIZE, 0, 0x0, 0 },
	{ TEXT_CreateIndirect, "", ID_TEXT_7, TEXT_START_XSIZE + (TEXT_XSIZE + TEXT_RIGHT2LEFTSPACE_SIZE)*2, TEXT_START_YSIZE + TEXT_TOP2DOWNSPACE_SIZE,  TEXT_XSIZE, TEXT_YSIZE, 0, 0x0, 0 },
	{ TEXT_CreateIndirect, "", ID_TEXT_8, TEXT_START_XSIZE + (TEXT_XSIZE + TEXT_RIGHT2LEFTSPACE_SIZE)*3, TEXT_START_YSIZE + TEXT_TOP2DOWNSPACE_SIZE,  TEXT_XSIZE, TEXT_YSIZE, 0, 0x0, 0 },
	{ TEXT_CreateIndirect, "", ID_TEXT_9, TEXT_START_XSIZE + (TEXT_XSIZE + TEXT_RIGHT2LEFTSPACE_SIZE)*4, TEXT_START_YSIZE + TEXT_TOP2DOWNSPACE_SIZE,  TEXT_XSIZE, TEXT_YSIZE, 0, 0x0, 0 },
	
	{ TEXT_CreateIndirect, "", ID_SCALE_0, IMG_START_XSIZE + (IMG_SIZE+IMG_RIGHT2LEFTSPACE_SIZE)*0, IMG_START_YSIZE, 																			IMG_SIZE, IMG_SIZE, 0, 0, 0 },
	{ TEXT_CreateIndirect, "", ID_SCALE_1, IMG_START_XSIZE + (IMG_SIZE+IMG_RIGHT2LEFTSPACE_SIZE)*1, IMG_START_YSIZE, 																			IMG_SIZE, IMG_SIZE, 0, 0, 0 },
	{ TEXT_CreateIndirect, "", ID_SCALE_2, IMG_START_XSIZE + (IMG_SIZE+IMG_RIGHT2LEFTSPACE_SIZE)*2, IMG_START_YSIZE, 																			IMG_SIZE, IMG_SIZE, 0, 0, 0 },
	{ TEXT_CreateIndirect, "", ID_SCALE_3, IMG_START_XSIZE + (IMG_SIZE+IMG_RIGHT2LEFTSPACE_SIZE)*3, IMG_START_YSIZE, 																			IMG_SIZE, IMG_SIZE, 0, 0, 0 },
	{ TEXT_CreateIndirect, "", ID_SCALE_4, IMG_START_XSIZE + (IMG_SIZE+IMG_RIGHT2LEFTSPACE_SIZE)*4, IMG_START_YSIZE, 																			IMG_SIZE, IMG_SIZE, 0, 0, 0 },
	{ TEXT_CreateIndirect, "", ID_SCALE_5, IMG_START_XSIZE + (IMG_SIZE+IMG_RIGHT2LEFTSPACE_SIZE)*0, IMG_START_YSIZE + IMG_SIZE + IMG_TOP2DOWNSPACE_SIZE + TEXT_YSIZE, 	IMG_SIZE, IMG_SIZE, 0, 0, 0 },
	{ TEXT_CreateIndirect, "", ID_SCALE_6, IMG_START_XSIZE + (IMG_SIZE+IMG_RIGHT2LEFTSPACE_SIZE)*1, IMG_START_YSIZE + IMG_SIZE + IMG_TOP2DOWNSPACE_SIZE + TEXT_YSIZE, 	IMG_SIZE, IMG_SIZE, 0, 0, 0 },
	{ TEXT_CreateIndirect, "", ID_SCALE_7, IMG_START_XSIZE + (IMG_SIZE+IMG_RIGHT2LEFTSPACE_SIZE)*2, IMG_START_YSIZE + IMG_SIZE + IMG_TOP2DOWNSPACE_SIZE + TEXT_YSIZE, 	IMG_SIZE, IMG_SIZE, 0, 0, 0 },
	{ TEXT_CreateIndirect, "", ID_SCALE_8, IMG_START_XSIZE + (IMG_SIZE+IMG_RIGHT2LEFTSPACE_SIZE)*3, IMG_START_YSIZE + IMG_SIZE + IMG_TOP2DOWNSPACE_SIZE + TEXT_YSIZE, 	IMG_SIZE, IMG_SIZE, 0, 0, 0 },
	{ TEXT_CreateIndirect, "", ID_SCALE_9, IMG_START_XSIZE + (IMG_SIZE+IMG_RIGHT2LEFTSPACE_SIZE)*4, IMG_START_YSIZE + IMG_SIZE + IMG_TOP2DOWNSPACE_SIZE + TEXT_YSIZE, 	IMG_SIZE, IMG_SIZE, 0, 0, 0 },

	{ TEXT_CreateIndirect, "", ID_TEXT_11, 10, 10,  258, 125, 0, 0x0, 0 },
	{ TEXT_CreateIndirect, "", ID_TEXT_12, 10, 145,  258, 125, 0, 0x0, 0 },

	{ TEXT_CreateIndirect, "", ID_SCALE_11, 10, 10,	258, 125, 0, 0, 0 },
	{ TEXT_CreateIndirect, "", ID_SCALE_12, 10, 145, 258, 125, 0, 0, 0 },

	// USER START (Optionally insert additional widgets)
	// USER END
};

static WM_HWIN hWin = 0;
static int lastId; // 记录按下的编号，以防止重复刷新
static BOOL isLeftView = TRUE;	//当前是否为左界面 默认左

void HomeViewInit(WM_MESSAGE *pMsg)
{
	WM_HWIN      hItem;
	const char *pText[10];
	UINT8 i;
	
	pText[0] = "MENU";	
	pText[1] = "MENU";
	pText[2] = "MENU";
	pText[3] = "MENU";
	pText[4] = "MENU";
	pText[5] = "MENU";
	pText[6] = "MENU";
	pText[7] = "MENU";
	pText[8] = "MENU";
	pText[9] = "MENU";
		
	// Initialization of 'Window'
	//
	 hItem = pMsg->hWin;
	 WINDOW_SetBkColor(hItem, EN_RGB(236,250,251));

	// Initialization of 'Image'
	// Initialization of Text
	for(i=0;i<10;i++)
	{
		hItem = WM_GetDialogItem(pMsg->hWin, ID_IMAGE_0 + i);
		IMAGE_SetBitmap(hItem,&bm80SystemInfo);

		hItem = WM_GetDialogItem(pMsg->hWin, ID_TEXT_0+i);
		TEXT_SetBkColor(hItem, TEXT_BKCOLOR);
		TEXT_SetText(hItem, pText[i]);
		TEXT_SetFont(hItem, TEXT_FONT);
		TEXT_SetTextAlign(hItem, GUI_TA_HCENTER | GUI_TA_VCENTER);
	}

	hItem = WM_GetDialogItem(pMsg->hWin, ID_TEXT_11);
	TEXT_SetTextColor(hItem, LEFT_LINE_COLOR);
	TEXT_SetText(hItem, "01");
	TEXT_SetFont(hItem, GUI_FONT_32B_ASCII);
	TEXT_SetTextAlign(hItem, GUI_TA_HCENTER | GUI_TA_VCENTER);

	hItem = WM_GetDialogItem(pMsg->hWin, ID_TEXT_12);
	TEXT_SetTextColor(hItem, RIGHT_LINE_COLOR);
	TEXT_SetText(hItem, "02");
	TEXT_SetFont(hItem, GUI_FONT_32B_ASCII);
	TEXT_SetTextAlign(hItem, GUI_TA_HCENTER | GUI_TA_VCENTER);

  lastId = 0;
}


static void HomeViewOnRelease(WM_MESSAGE *pMsg)
{
    int Id;
		const GUI_RECT RECT1 = {0,0,300,280};
		const GUI_RECT RECT2 = {301,0,1424,15};
		const GUI_RECT RECT3 = {1400,0,1424,280};
		const GUI_RECT RECT4 = {300,265,1424,280};
    Id = WM_GetId(pMsg->hWinSrc);    

    switch(Id)
    {
	case ID_SCALE_11:
		if(isLeftView == TRUE) break;	//防止重复绘制
		isLeftView = TRUE;

		WM_InvalidateRect(pMsg->hWin,&RECT1);

		WM_InvalidateRect(pMsg->hWin,&RECT2);


		break;
	case ID_SCALE_12:
		if(isLeftView == FALSE) break; //防止重复绘制
		isLeftView = FALSE;

		WM_InvalidateRect(pMsg->hWin,&RECT1);
	
		WM_InvalidateRect(pMsg->hWin,&RECT2);

		break;
    }
    
    lastId = 0;
}

// USER START (Optionally insert additional static code)
// USER END

/*********************************************************************
*
*       _cbDialog
*/
static void _cbDialog(WM_MESSAGE * pMsg) {

	// USER START (Optionally insert additional variables)
    int   NCode;
	//int   Id;

	// USER END
	switch (pMsg->MsgId) {
	case WM_INIT_DIALOG:
		// USER START (Optionally insert additional code for further widget initialization)
		HomeViewInit(pMsg);
		// USER END
		break;
		// USER START (Optionally insert additional message handling)
	case WM_NOTIFY_PARENT:
		NCode = pMsg->Data.v;
		switch(NCode)
		{
		case WM_NOTIFICATION_RELEASED:
        HomeViewOnRelease(pMsg);
			 break;
		}
		break;
	case WM_PAINT:
		GUI_SetColor(WINDOW_BACK_COLOR);
		GUI_Clear();

		if(isLeftView)
		{
			GUI_SetColor(RIGHT_LINE_COLOR);
			GUI_DrawRoundedFrame(10,145,268,270,12,4);
			//外框线条
			GUI_SetColor(LEFT_LINE_COLOR);
			GUI_SetPenSize(4);
			GUI_DrawLine(10+12,10,1414-12,10);
			GUI_DrawLine(1414,10+12,1414,270-12);
			GUI_DrawLine(1414-12,270,278+12,270);
			GUI_DrawLine(278,135+12,278,270-12);
			GUI_DrawLine(10+12,135,278-12,135);
			GUI_DrawLine(10,135-12,10,10+12);

			GUI_DrawArc(10+12,	10+12,12,12,90,180);
			GUI_DrawArc(1414-11,10+12,12,12,0,90);
			GUI_DrawArc(1414-11,270-12,12,12,270,360);
			GUI_DrawArc(278+12,	270-12,12,12,180,270);
			GUI_DrawArc(278-11,	135+12,12,12,0,90);
			GUI_DrawArc(10+12,	135-12,12,12,180,270);
		}
		else
		{
			GUI_SetColor(LEFT_LINE_COLOR);
			GUI_DrawRoundedFrame(10,10,268,135,12,4);
			//外框线条
			GUI_SetColor(RIGHT_LINE_COLOR);
			GUI_SetPenSize(4);
			GUI_DrawLine(278+12,10,1414-12,10);
			GUI_DrawLine(1414,10+12,1414,270-12);
			GUI_DrawLine(1414-12,270,10+12,270);
			GUI_DrawLine(10,270-12,10,145+12);
			GUI_DrawLine(10+12,145,278-12,145);
			GUI_DrawLine(278,145-12,278,10+12);

			GUI_DrawArc(278+12,		10+12,12,12,90,180);
			GUI_DrawArc(1414-11,	10+12,12,12,0,90);
			GUI_DrawArc(1414-11,	270-12,12,12,270,360);
			GUI_DrawArc(10+12,		270-12,12,12,180,270);
			GUI_DrawArc(10+12,		145+12,12,12,90,180);
			GUI_DrawArc(278-11,		145-12,12,12,270,360);
		}
		break;
		// USER END
	default:
		WM_DefaultProc(pMsg);
		break;
	}
}


/*********************************************************************
*
*       Public code
*
**********************************************************************
*/

// USER START (Optionally insert additional public code)
// USER END

/*************************** End of file ****************************/

void HomeViewCreate()
{
	hWin = GUI_CreateDialogBox(_aDialogCreate, GUI_COUNTOF(_aDialogCreate), _cbDialog, WM_HBKWIN, 0, 0);  // 非阻塞模式
}
void HomeViewDestory()
{
	if(hWin)
	{
		GUI_EndDialog(hWin, 0);
	}
	hWin = 0;
}
//设置跳转后优先显示哪个窗口
void HomeViewSetCurrentJumpView(BOOL isleftview)
{
	isLeftView = isleftview;
}
//获取当前跳转界面 调用后可判断是由哪个泵界面进入
BOOL HomeViewGetCurrentJumpView(void)
{
	return isLeftView;
}

