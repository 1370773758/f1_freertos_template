/*
����IC GT911
*/
#include "GT911_i2c.h"

#include "stm32f10x_i2c.h"
#include "Timer.h"

//I2C��INT����T2Ϊ��ʱ�Ķ�д����
#define TP_CMD_WR       0xBA
#define TP_CMD_RD       (TP_CMD_WR+0x01)
//GT911 ���ּĴ�������
#define GT911_READ_XY_REG 	0x814E	/* ����Ĵ��� */
#define GT911_CLEARBUF_REG 	0x814E	/* �������Ĵ��� */
#define GT911_CONFIG_REG	0x8047	/* ���ò����Ĵ��� */
#define GT911_COMMAND_REG   0x8040  /* ʵʱ���� */
#define GT911_PRODUCT_ID_REG 0x8140 /*productid*/
#define GT911_VENDOR_ID_REG   0x814A  /* ��ǰģ��ѡ����Ϣ */
#define GT911_CONFIG_VERSION_REG   0x8047  /* �����ļ��汾�� */
#define GT911_CONFIG_CHECKSUM_REG   0x80FF  /* �����ļ�У���� */
#define GT911_FIRMWARE_VERSION_REG   0x8144  /* �̼��汾�� */

#define TP_NORMAL 	0
#define TP_ERR			1

#define GPIO_PORT           GPIOC
#define TP_SDA_GPIO         GPIO_Pin_1
#define TP_SCL_GPIO         GPIO_Pin_2
#define TP_INT_GPIO         GPIO_Pin_4
#define TP_RST_GPIO         GPIO_Pin_5

#define SET_TP_SDA()        GPIO_SetBits(GPIO_PORT,TP_SDA_GPIO)
#define RESET_TP_SDA()      GPIO_ResetBits(GPIO_PORT,TP_SDA_GPIO)
#define SET_TP_SCL()        GPIO_SetBits(GPIO_PORT,TP_SCL_GPIO)
#define RESET_TP_SCL()      GPIO_ResetBits(GPIO_PORT,TP_SCL_GPIO)    
#define SET_TP_INT()        GPIO_SetBits(GPIO_PORT,TP_INT_GPIO)
#define RESET_TP_INT()      GPIO_ResetBits(GPIO_PORT,TP_INT_GPIO)   
#define SET_TP_RST()        GPIO_SetBits(GPIO_PORT,TP_RST_GPIO)
#define RESET_TP_RST()      GPIO_ResetBits(GPIO_PORT,TP_RST_GPIO)   

#define GET_SDA_STATE()     GPIO_ReadInputDataBit(GPIO_PORT,TP_SDA_GPIO)
#define GET_SCL_STATE()			GPIO_ReadInputDataBit(GPIO_PORT,TP_SCL_GPIO)
#define GET_INT_STATE()     GPIO_ReadInputDataBit(GPIO_PORT,TP_INT_GPIO)

#define WAIT_US_TIME        5       //i2c ��϶ʱ��us
#define GTDelayus(x)				ENDelayus(x)
#define GTDelay(x)					ENSleep(x)


GT911_T g_GT911;

//��������ʼ��������һ�㳧�ҵ������Դ�����  ֱ�ӽ��ж�ȡ���У�����Ҫ���Ĳ����Ž���д��
const uint8_t GT911_CFG_TBL[]=
{ 
//	GT_VERSION,				//0x8047		�汾��
//	(GT_X_OUTPUT_MAX&0xff),(GT_X_OUTPUT_MAX>>8),	//0x8048/8049	
//	(GT_Y_OUTPUT_MAX&0xff),(GT_Y_OUTPUT_MAX>>8),	//0x804a/804b	
//	GT_TOUCH_NUMBER,	//0x804c	��������������
//	0x05,//0x0D,//0x35,	//0x804d	������룬�½��ش���
//	0x00,	//0x804e	reserved
//	GT_SHAKE_COUNT,	//0x804f	��ָ����ȥ��������
//	0x08,	//0x8050	ԭʼ���괰���˲�ֵ	
//	0x28,	//0x8051	������������
//	0x05,	//0x8052	��������ֵ
//	0x50,	//0x8053	���ϴ�������޵��е���ֵ
//	0x32,	//0x8054	���ϴ�������е��޵���ֵ
//	GT_LOWPOWER_TIMER,	//0x8055	���͹���ʱ��	s
//	0x05,	//0x8056	�����ϱ���
//	0x00,	//0x8057	X�������������
//	0x00,	//0x8058	Y�������������
//	0x00,0X00,	//0x8059-0x805a	reserved
//	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x8C,0x2E,0x06,
//0x17,0x15,0x31,0x0D,0x00,0x00,0x00,0x9A,0x03,0x1C,
//0x00,0x00,0x00,0x00,0x00,0x03,0x64,0x32,0x00,0x00,
//0x00,0x00,0x00,0x00,0x40,0x00,0x00,0x00,0x00,0x00,
//0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
//0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
//0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
//0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
//0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
//0x00,0x00,0x02,0x04,0x06,0x08,0x0A,0x0C,0xFF,0xFF,
//0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
//0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
//0xFF,0xFF,0x00,0x02,0x04,0x06,0x08,0x0A,0x0C,0x0F,
//0x10,0x12,0x13,0x14,0x16,0x2A,0x29,0x28,0x26,0x24,
//0x22,0x21,0x20,0x1F,0x1E,0x1D,0x1C,0x18,0xFF,0xFF,
//0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
//0xFF,0xFF,0xFF,0xFF//,0x6F,0x01

0x00,
0x90,0x05,0x18,0x01,
0x0A,
0x0D,0x00,
0x01,
0x08
,0x28,0x05,0x50,0x32,0x03,0x05,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x8C,0x2E,0x06,0x17,0x15,0x31,0x0D,0x00,0x00,0x00,0x9A,0x03,0x1C,0x00,0x00,0x00,0x00,0x00,0x03,0x64,0x32,0x00,0x00,0x00,0x00,0x00,0x00,0x40,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x02,0x04,0x06,0x08,0x0A,0x0C,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x00,0x02,0x04,0x06,0x08,0x0A,0x0C,0x0F,0x10,0x12,0x13,0x14,0x16,0x2A,0x29,0x28,0x26,0x24,0x22,0x21,0x20,0x1F,0x1E,0x1D,0x1C,0x18,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF//,0x6F,0x01
};

uint8_t GT911_I2cWrite_nBytes(uint16_t reg,uint8_t *date,uint8_t len);
uint8_t GT911_I2cRead_nBytes(uint16_t reg,uint8_t *date,uint8_t len);
uint8_t GT911_ReadVersion(void);

void GT911_I2cStop(void);
void GT911_Soft_Reset(void);

void GT911_SendCfg(void);
/*
*********************************************************************************************************
*	�� �� ��: 
*	����˵��: gt911���ų�ʼ��
*	��    �Σ���
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void GT911_Configuration(void)
{
	GPIO_InitTypeDef  GPIO_InitStructure;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC,ENABLE);

	RESET_TP_RST();
  RESET_TP_INT();
	
	GPIO_InitStructure.GPIO_Pin = TP_SDA_GPIO | TP_SCL_GPIO;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOC,&GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin = TP_INT_GPIO | TP_RST_GPIO;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOC,&GPIO_InitStructure);
	
	RESET_TP_RST();
  RESET_TP_INT();
	GTDelay(1);
	SET_TP_RST();
	RESET_TP_INT();
	GTDelay(6);
	RESET_TP_INT();
	GTDelay(10);
	//SET_TP_INT();
	
	GPIO_InitStructure.GPIO_Pin = TP_INT_GPIO;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOC,&GPIO_InitStructure);
		
	//GT911_Soft_Reset(); /*��λ*/
	GTDelay(200);
}


/*
*********************************************************************************************************
*	�� �� ��: GT911_ReadVersion
*	����˵��: ���GT911�İ汾��
*	��    ��: ��
*	�� �� ֵ: 16λ�汾
*********************************************************************************************************
*/
void GT911_Init(void)
{
	GT911_Configuration();

	if(GT911_ReadVersion() != GT_VERSION) //ħ�������
	{
		GT911_SendCfg();
	}
}
/*
*********************************************************************************************************
*	�� �� ��: GT911_ReadVersion
*	����˵��: ���GT911�İ汾��
*	��    ��: ��
*	�� �� ֵ: 16λ�汾
*********************************************************************************************************
*/
uint8_t version;  
uint8_t GT911_ReadVersion(void)
{
	GT911_I2cRead_nBytes(GT911_CONFIG_REG, &version, 1); 

	return version; 
}


/*
*********************************************************************************************************
*	�� �� ��: 
*	����˵��: I2C���� GT911Э�鷢�ͽ���
*	��    �Σ���
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void GT911_I2cStart(void)
{
	SET_TP_SDA();
	SET_TP_SCL();
	GTDelayus(WAIT_US_TIME);
	RESET_TP_SDA();
	GTDelayus(WAIT_US_TIME);
	RESET_TP_SCL();
	GTDelayus(WAIT_US_TIME);
}
void GT911_I2cStop(void)
{
	RESET_TP_SDA();
	SET_TP_SCL();
	GTDelayus(WAIT_US_TIME);
	SET_TP_SDA();
}
uint8_t GT911_I2cWaitACK(void)
{
    uint8_t waitTime = 0;
	uint8_t ack = 0;
	
	SET_TP_SDA();GTDelayus(1);
	SET_TP_SCL();GTDelayus(1);
	GTDelayus(WAIT_US_TIME);
	
    if(GET_SDA_STATE())
					ack = 1;
    else ack = 0;

    RESET_TP_SCL();
		GTDelayus(WAIT_US_TIME);
    return ack;
}
void GT911_I2cSendACK(void)
{
	RESET_TP_SDA();
	GTDelayus(WAIT_US_TIME);
	SET_TP_SCL();
	GTDelayus(WAIT_US_TIME);
	RESET_TP_SCL();
	GTDelayus(WAIT_US_TIME);
	SET_TP_SDA();
}
void GT911_I2cSendNACK(void)
{
	SET_TP_SDA();
	GTDelayus(WAIT_US_TIME);
	SET_TP_SCL();
	GTDelayus(WAIT_US_TIME);
	RESET_TP_SCL();
	GTDelayus(WAIT_US_TIME);
}
void GT911_I2cWriteByte(uint8_t date)
{
    uint8_t i;
//	RESET_TP_SCL();
    for(i=0;i<8;i++)
    {  
        if(date&0x80)
            SET_TP_SDA();
        else RESET_TP_SDA();
        GTDelayus(WAIT_US_TIME);
        SET_TP_SCL();
        GTDelayus(WAIT_US_TIME);
				RESET_TP_SCL();
				if(i==7) SET_TP_SDA();
        date = date << 1;
				GTDelayus(WAIT_US_TIME);
    }
}
uint8_t GT911_I2cReadByte(void)
{
    uint8_t i , date;
	SET_TP_SDA();
    for(i=0;i<8;i++)
    {
			SET_TP_SCL();
			GTDelayus(WAIT_US_TIME);
			date = date << 1;
        if(GET_SDA_STATE()) date++; 
			RESET_TP_SCL();
			GTDelayus(WAIT_US_TIME);
    }
    return date;
}

uint8_t GT911_I2cWrite_nBytes(uint16_t reg,uint8_t *date,uint8_t len)
{
    uint16_t i;
	
	GT911_I2cStop();
	
	for(i=0;i<1000;i++)
	{
		GT911_I2cStart();
    GT911_I2cWriteByte(TP_CMD_WR);//д����
    if(GT911_I2cWaitACK() == 0) break;
	}
    
    GT911_I2cWriteByte(reg >> 8);
    if(GT911_I2cWaitACK()) return TP_ERR;
    GT911_I2cWriteByte(reg & 0xff);
    if(GT911_I2cWaitACK()) return TP_ERR;

    for(i=0;i<len;i++)
    {
        GT911_I2cWriteByte(date[i]);
        if(GT911_I2cWaitACK())
				{
					GT911_I2cStop();
					return TP_ERR;
				}					
    }
    GT911_I2cStop();
		return TP_NORMAL;
}

uint8_t GT911_I2cRead_nBytes(uint16_t reg,uint8_t *date,uint8_t len)
{
    uint8_t i;

    GT911_I2cStart();
    GT911_I2cWriteByte(TP_CMD_WR);//д����
    GT911_I2cWaitACK();
    GT911_I2cWriteByte(reg >> 8);
    GT911_I2cWaitACK();
    GT911_I2cWriteByte(reg & 0xff);
    GT911_I2cWaitACK();
		GT911_I2cStop();
	
    GT911_I2cStart();
    GT911_I2cWriteByte(TP_CMD_RD);//������
    if(GT911_I2cWaitACK()!= 0)
		{
			goto cmd_fail;
		}

    for ( i = 0; i < len; i++)
    {
        date[i] = GT911_I2cReadByte();
        if(i == len-1)
            GT911_I2cSendNACK();
        else
            GT911_I2cSendACK();
    }
		GT911_I2cStop();
		return 0;
cmd_fail: /* ����ִ��ʧ�ܺ��мǷ���ֹͣ�źţ�����Ӱ��I2C�����������豸 */
	/* ����I2C����ֹͣ�ź� */
    GT911_I2cStop();
		return 1;
}

/*
*mode:�Ƿ񱣴����� 1���� 0�����浽flash
*/
void GT911_SendCfg(void)
{
	uint8_t buf[2],i;
	buf[0] = 0;
	buf[1] = GT_CONFIG_FREAH;
	for(i=0;i<sizeof(GT911_CFG_TBL);i++) buf[0] += GT911_CFG_TBL[i];	//�����У��
	buf[0] = (~buf[0])+1;
	GT911_I2cWrite_nBytes(0x8047,(uint8_t *)GT911_CFG_TBL,sizeof(GT911_CFG_TBL));
	GT911_I2cWrite_nBytes(0x80FF,(uint8_t *)buf,1);
	GT911_I2cWrite_nBytes(0x8100,&buf[1],1);
}
/*
*********************************************************************************************************
*	�� �� ��: GT911_Soft_Reset
*	����˵��: G911��λ����,�ָ��������á�
*	��    ��:  ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void GT911_Soft_Reset(void)
{
    uint8_t buf[1];
    buf[0] = 0x02;
    GT911_I2cWrite_nBytes(GT911_COMMAND_REG, (uint8_t *)buf, 1);
}
/*
*********************************************************************************************************
*	�� �� ��: 
*	����˵��: ��ȡ��λ
*	�� ��: ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void ENGT911_Scan(void)
{
	uint8_t buf[40];
    uint8_t Clearbuf = 0;

    GT911_I2cRead_nBytes(GT911_READ_XY_REG, buf, 1);
    if ((buf[0] & 0x80) == 0)
    {
			//����Ļ���λ
			g_GT911.X1 = 0xfff;
			g_GT911.Y1 = 0xfff;
			g_GT911.S1 = 0;
			GT911_I2cWrite_nBytes(GT911_CLEARBUF_REG, (uint8_t *)&Clearbuf, 1);
			return ;
    }
    GT911_I2cRead_nBytes(GT911_READ_XY_REG + 1, &buf[1], 39);
    GT911_I2cWrite_nBytes(GT911_CLEARBUF_REG, (uint8_t *)&Clearbuf, 1);
		
    g_GT911.TouchpointFlag = buf[0];
    g_GT911.Touchkey1trackid = buf[1];
    g_GT911.X1 = ((uint16_t)buf[3] << 8) + buf[2];
    g_GT911.Y1 = ((uint16_t)buf[5] << 8) + buf[4];
    g_GT911.S1 = ((uint16_t)buf[7] << 8) + buf[6];
		
//		g_GT911.Touchkey2trackid = buf[8];
//    g_GT911.X2 = ((uint16_t)buf[9] << 8) + buf[10];
//    g_GT911.Y2 = ((uint16_t)buf[11] << 8) + buf[12];
//    g_GT911.S2 = ((uint16_t)buf[13] << 8) + buf[14];
//		
//		g_GT911.Touchkey3trackid = buf[16];
//    g_GT911.X3 = ((uint16_t)buf[17] << 8) + buf[18];
//    g_GT911.Y3 = ((uint16_t)buf[19] << 8) + buf[20];
//    g_GT911.S3 = ((uint16_t)buf[21] << 8) + buf[22];
//		
//		g_GT911.Touchkey4trackid = buf[24];
//    g_GT911.X4 = ((uint16_t)buf[25] << 8) + buf[26];
//    g_GT911.Y4 = ((uint16_t)buf[27] << 8) + buf[28];
//    g_GT911.S4 = ((uint16_t)buf[29] << 8) + buf[30];
//		
//		g_GT911.Touchkey5trackid = buf[32];
//    g_GT911.X5 = ((uint16_t)buf[33] << 8) + buf[34];
//    g_GT911.Y5 = ((uint16_t)buf[35] << 8) + buf[36];
//    g_GT911.S5 = ((uint16_t)buf[37] << 8) + buf[38];
		
}

GT911_T gGT911_Get(void)
{
	return g_GT911;
}

uint16_t gGT911_GetxPhys(void)
{
	return g_GT911.X1;
}
uint16_t gGT911_GetyPhys(void)
{
	return g_GT911.Y1;
}


