/**
  ******************************************************************************
  * @file    main.c
  * @author  MCD Application Team
  * @version V1.1.2
  * @date    28-January-2014
  * @brief   This file provides main program functions
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT 2014 STMicroelectronics</center></h2>
  *
  * Licensed under MCD-ST Liberty SW License Agreement V2, (the "License");
  * You may not use this file except in compliance with the License.
  * You may obtain a copy of the License at:
  *
  *        http://www.st.com/software_license_agreement_liberty_v2
  *
  * Unless required by applicable law or agreed to in writing, software 
  * distributed under the License is distributed on an "AS IS" BASIS, 
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  * See the License for the specific language governing permissions and
  * limitations under the License.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/


#include "LCD_RA8889.h"
#include "RA8889_API.h"
#include "bsp_touch.h"

#include "HomeView.h"



#include "Timer.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#include <stdio.h>
#include "stm32f10x_iwdg.h"

#include "Driver_SSD2828.h"
#include "GT911_i2c.h"

#include "HomeView.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
// 优先级configMAX_PRIORITIES最高为6
#define TASK_PRIO_BACKGROUND        (tskIDLE_PRIORITY  + 4)
#define TASK_PRIO_UI                (tskIDLE_PRIORITY  + 3)
#define TASK_PRIO_DOMAIN            (tskIDLE_PRIORITY  + 6)
#define TASK_PRIO_WIFI              (tskIDLE_PRIORITY  + 5)
#define TASK_PRIO_PROTOCOL          (tskIDLE_PRIORITY  + 7)

// 任务栈分配--N*4  Byte
#define STACK_SIZE_DOMAIN            (256)
#define STACK_SIZE_UI                (2048+512)
#define STACK_SIZE_BACKGROUND        (128+128)
#define STACK_SIZE_WIFI              (256)
#define STACK_SIZE_PROTOCOL          (128)

xQueueHandle  hQueueMsg; //用于传递消息的一个公共Handle变量

xTaskHandle taskProtocolHandle=NULL;
xTaskHandle taskWifiHandle=NULL;
xTaskHandle taskDomainHandle=NULL;
xTaskHandle taskBkGroundHandle=NULL;
xTaskHandle taskUIHandle=NULL;

/* Private function prototypes -----------------------------------------------*/
static void AppTaskBackground(void * pvParameters);
static void AppTaskUI(void * pvParameters);
static void AppTaskDomain(void * pvParameters);
static void AppTaskWifi(void * pvParameters);
static void AppTaskProtocol(void * pvParameters); 

/* Private functions ---------------------------------------------------------*/

static BOOL gIsSystemInit = FALSE;

/**
 * prer:0~7
 * 分频：4*2^prer max 256!
 * rlr:重装载寄存器 11位
 * 时间计算：Tout=((4*2^prer)*rlr)/40 (ms).
 */
void IWDG_Init(UINT8 prer,UINT16 rlr)
{
    IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable); 
    IWDG_SetPrescaler(prer);    
    IWDG_SetReload(rlr);     
    IWDG_ReloadCounter();    
    IWDG_Enable();        
}


void IWDG_Feed(void)
{
    IWDG_ReloadCounter();    /*reload*/
}

static void SoftwareInit(void)
{   
//  ENBmpInit(); 
//  ENFontInit();

//   KeypadInit();// 输入法
//   ENViewInit();
	
}



static void HardwareInit(void)
{
    ENSysTickInit();

		
    /* Activate the use of memory device feature */
  //  WM_SetCreateFlags(WM_CF_MEMDEV);
	ENTimerInit();
  
  	  
	
	LCD_InitHard();
	SSD2828_Configuration(); //顺序颠倒会不出画面
	ENSleep(1); //加延时 不然gt911有时写不进
	TOUCH_InitHard();
}

/**
  * @brief  Main program.
  * @param  None
  * @retval None
  */ 
int main(void)
{
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);

	if(hQueueMsg == 0)
	{
		/*没有创建成功，可在这里加入失败处理机制*/
	}
	
    xTaskCreate((pdTASK_CODE)AppTaskBackground,
              (signed char const*)"AppTaskBackground",
              STACK_SIZE_BACKGROUND,
              NULL,
              TASK_PRIO_BACKGROUND,
              &taskBkGroundHandle);

    // 初始化在此任务中执行，创建时优先级最高，初始化完成后调低
  xTaskCreate((pdTASK_CODE)AppTaskUI,
            (signed char const*)"AppTaskUI",
            STACK_SIZE_UI,
            NULL,
            configMAX_PRIORITIES,
            &taskUIHandle);
              
//    // ProtocolProcess
    xTaskCreate((pdTASK_CODE)AppTaskProtocol,
 		 (signed char const*)"AppTask PROTOCOL",
 		 STACK_SIZE_PROTOCOL,
 		 NULL,
 	   	 TASK_PRIO_PROTOCOL,
 		 &taskProtocolHandle);

    vTaskStartScheduler();

    while(1)
		{
			;
		}
}


xTaskHandle GetTaskProtocolHandle(void)
{
    return taskProtocolHandle;
}

xTaskHandle GetTaskWifiHandle(void)
{
    return taskWifiHandle;
}

xTaskHandle GetTaskBkGroundHandle(void)
{
    return taskBkGroundHandle;
}

xTaskHandle GetTaskDomainHandle(void)
{
    return taskDomainHandle;
}

xTaskHandle GetTaskUIHandle(void)
{
    return taskUIHandle;
}

/**
  * @brief  Background task
  * @param  pvParameters not used
  * @retval None
  */
// 后台任务，处理按键及业务消息
GUI_ALLOC_DATATYPE guiMemLeft=64*1024;
//GUI_ALLOC_DATATYPE guiMemUsed=0;
static void AppTaskBackground(void * pvParameters)
{  
 
//    GUI_ALLOC_DATATYPE tmpMemSize;
  /* Run the background task */
    while (1)
    {
//        tmpMemSize = GUI_ALLOC_GetNumFreeBytes();
//        guiMemLeft = (guiMemLeft > tmpMemSize) ? tmpMemSize : guiMemLeft ;
//        tmpMemSize = GUI_ALLOC_GetNumUsedBytes();
//        guiMemUsed = (guiMemUsed < tmpMemSize) ? tmpMemSize : guiMemUsed ; 
        while(gIsSystemInit != TRUE)
        {
            vTaskDelay(10);
        }

        GUI_TOUCH_Exec();
        //IWDG_Feed();
        vTaskDelay(1);
    }
}

GUI_ALLOC_DATATYPE GetGuiMemLeft(void)
{
    return guiMemLeft;
}


static void AppTaskProtocol(void *pvParameters)
{
    while(1)
   	{
        while(gIsSystemInit != TRUE)
        {
            vTaskDelay(10);
        }
	    // 协议业务循环，更新参数
			ENGT911_Scan();
				
	   	//ENDriverProtocolProcess();
	    vTaskDelay(10);
   	}

}


/**
  * @brief  Demonstration task
  * @param  pvParameters not used
  * @retval None
  */
GUI_PID_STATE TouchState;
static void AppTaskUI(void * pvParameters)
{
    HardwareInit();
    
    /* Init the STemWin GUI Library */
//	Canvas_Image_Start_address(Layer2);//specified layer2 for emWin,only need set once 
	
	GUI_Init();
	
	// WM_SetCreateFlags(WM_CF_MEMDEV);
    // /* 设置皮肤**************************************/
//    BUTTON_SetDefaultSkin(BUTTON_SKIN_FLEX);
//    SLIDER_SetDefaultSkin(SLIDER_SKIN_FLEX);
    SoftwareInit();

    vTaskPrioritySet(taskUIHandle,TASK_PRIO_UI);
    gIsSystemInit = TRUE;
		
//	//---------------------//

//      HomeViewCreate();
	GUI_SetBkColor(0x0000ff00);
	GUI_Clear();
	PWM0(1,0,1,255,255); 
  GUI_TOUCH_Exec();
  GUI_SetColor(0X00FF0000);
  GUI_SetPenSize(10);
    while(1)
    {	
			
        GUI_TOUCH_GetState(&TouchState);
        GUI_DrawPoint(TouchState.x,TouchState.y);
//	    GUI_Delay(1);
//       copyLayer2BufferDataToLayer1DisplayArea();
    }
}

void vApplicationStackOverflowHook( xTaskHandle *pxTask, signed portCHAR *pcTaskName )
{
    while(1)
    {}
}
/**
  * @brief  Error callback function
  * @param  None
  * @retval None
  */
void vApplicationMallocFailedHook( void )
{
  while (1)
  {}
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *   where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t* file, uint32_t line)
{
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

  /* Infinite loop */
  while (1)
  {}
}
#endif

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
