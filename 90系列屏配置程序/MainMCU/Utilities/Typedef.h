/***************************************************************
 * File:  Typedef.h
 * Date:   
 * Copyright@2015-2025MDRNAll Right Reserved
 *
 * Note:�ض����������ͣ�������ֲ
 *
****************************************************************/
#ifndef _TYPEDEF_H_
#define _TYPEDEF_H_

#ifndef INT8
    #define INT8   signed char
#endif

#ifndef UINT8
    #define UINT8  unsigned char
#endif

#ifndef INT16
    #define INT16  signed short int
#endif

#ifndef UINT16
    #define UINT16 unsigned short int
#endif

#ifndef INT32
    #define INT32  long
#endif

#ifndef UINT32
    #define UINT32 unsigned long
#endif

#ifndef INT64
    #define INT64  __int64
#endif

#ifndef UINT64
    #define UINT64 unsigned __int64
#endif

#ifndef CHAR
    #define CHAR  char
#endif

#ifndef BYTE
    #define BYTE    CHAR
#endif

#ifndef FLOAT
    #define FLOAT  float
#endif

#ifndef DOUBLE
    #define DOUBLE double
#endif

#ifndef BOOL
    #define BOOL  INT8
#endif

#ifndef TRUE
    #define TRUE  (1)
#endif

#ifndef FALSE
    #define FALSE (0)
#endif

#ifndef NULL
    #define NULL  (0)
#endif

#ifndef EN_COLOR
	#define EN_COLOR   UINT32
#endif
// emWIN��ɫֵ��ʽΪBGR(����GUI_RED��֪)������Ҫ��RGB��ʽ��ɫֵת��
#define EN_RGB(r,g,b)	(r )|(g<<8)|(b<<16)

// ��ȡ�ṹ�����ݵ����ƫ����
#define EN_MEMBER_OFFSET(TYPE,MEMBER)       ((int)(&((TYPE *)0)->MEMBER))

//������ɫ
#define LEFT_THEME_COLOR	(EN_RGB(0, 176, 191))
#define RIGHT_THEME_COLOR	(EN_RGB(184, 71, 113))

#if defined WIN32 || defined BARE_MACHINE
#define xSemaphoreHandle    UINT32
#endif
#endif // _TYPEDEF_H_
