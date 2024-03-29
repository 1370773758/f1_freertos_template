#ifndef _GT911_I2C_H
#define _GT911_I2C_H

#include "stm32f10x.h"
#include "Typedef.h"

#define GT_VERSION			0x00		//版本号 'A'~'Z' 写入0x00默认为0x41 芯片大于或者等于才写入有效 当前软件取消等于
//默认起始方向不一致 需要X/Y颠倒
#define GT_X_OUTPUT_MAX     1424    //X轴输出最大值
#define GT_Y_OUTPUT_MAX     280     //Y轴输出最大值
#define GT_TOUCH_NUMBER     0x05    //输出触点个数 1~5

#define GT_SHAKE_COUNT      0x01       //手指按下/松开消抖次数
#define GT_LOWPOWER_TIMER   0x03       //进低功耗时间 0~15s 没操作时间进进
#define GT_REFRESH_RATE     0x00       //坐标更新周期 5+X s       

#define GT_CONFIG_FREAH     0x01       //写入falsh配置 id需要高于或等于当前id
//参数需要写入falsh才能使用

typedef struct
{
    uint8_t TouchpointFlag;   //是否按下

    uint8_t Touchkey1trackid;
    uint16_t X1;
    uint16_t Y1;
    uint16_t S1;

    uint8_t Touchkey2trackid;
    uint16_t X2;
    uint16_t Y2;
    uint16_t S2;

    uint8_t Touchkey3trackid;
    uint16_t X3;
    uint16_t Y3;
    uint16_t S3;

    uint8_t Touchkey4trackid;
    uint16_t X4;
    uint16_t Y4;
    uint16_t S4;

    uint8_t Touchkey5trackid;
    uint16_t X5;
    uint16_t Y5;
    uint16_t S5;
} GT911_T;


void GT911_Init(void);

void ENGT911_Scan(void);

uint16_t gGT911_GetxPhys(void);
uint16_t gGT911_GetyPhys(void);
#endif
