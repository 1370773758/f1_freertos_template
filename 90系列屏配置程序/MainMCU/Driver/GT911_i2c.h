#ifndef _GT911_I2C_H
#define _GT911_I2C_H

#include "stm32f10x.h"
#include "Typedef.h"

#define GT_VERSION			0x00		//�汾�� 'A'~'Z' д��0x00Ĭ��Ϊ0x41 оƬ���ڻ��ߵ��ڲ�д����Ч ��ǰ���ȡ������
//Ĭ����ʼ����һ�� ��ҪX/Y�ߵ�
#define GT_X_OUTPUT_MAX     1424    //X��������ֵ
#define GT_Y_OUTPUT_MAX     280     //Y��������ֵ
#define GT_TOUCH_NUMBER     0x05    //���������� 1~5

#define GT_SHAKE_COUNT      0x01       //��ָ����/�ɿ���������
#define GT_LOWPOWER_TIMER   0x03       //���͹���ʱ�� 0~15s û����ʱ�����
#define GT_REFRESH_RATE     0x00       //����������� 5+X s       

#define GT_CONFIG_FREAH     0x01       //д��falsh���� id��Ҫ���ڻ���ڵ�ǰid
//������Ҫд��falsh����ʹ��

typedef struct
{
    uint8_t TouchpointFlag;   //�Ƿ���

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
