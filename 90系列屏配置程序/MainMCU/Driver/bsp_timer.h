/*
*********************************************************************************************************
*
*	ģ������ : ��ʱ��ģ��
*	�ļ����� : bsp_timer.h
*
*********************************************************************************************************
*/

#ifndef __BSP_TIMER_H
#define __BSP_TIMER_H

/* Ŀǰ�ǿղ������û����Զ�����CPU����IDLE״̬�ĺ�����ι������ */
#define CPU_IDLE()

/* ��ʱ���ṹ�壬��Ա���������� volatile, ����C�������Ż�ʱ���������� */
enum
{
	TMR_ONCE_MODE = 0,	/* һ�ι���ģʽ */
	TMR_AUTO_MODE = 1		/* �Զ���ʱ����ģʽ */
};

/* ��ʱ���ṹ�壬��Ա���������� volatile, ����C�������Ż�ʱ���������� */
typedef struct
{
	volatile uint8_t Mode;		/* ������ģʽ��1���� */
	volatile uint8_t Flag;		/* ��ʱ�����־  */
	volatile uint32_t Count;	/* ������ */
	volatile uint32_t PreLoad;	/* ������Ԥװֵ */
}SOFT_TMR;

/* ���ⲿ���õĺ������� */
void bsp_InitTimer(void);
void bsp_DelayMS(uint32_t n);
void bsp_StartTimer(uint8_t _id, uint32_t _period);
void bsp_StartAutoTimer(uint8_t _id, uint32_t _period);
void bsp_StopTimer(uint8_t _id);
uint8_t bsp_CheckTimer(uint8_t _id);
int32_t bsp_GetRunTime(void);

#endif
