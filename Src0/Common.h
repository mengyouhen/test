#ifndef _COMMON_H_
#define _COMMON_H_

#include <stdint.h>
#include "stm32f0xx_hal.h"
#include "stm32f0xx_hal_gpio.h"
#include "Common.h"


/*
*********************************************************************************************************
*                                            Common Defines
*********************************************************************************************************
*/
#ifndef     ON
#define     ON              (1U)
#endif

#ifndef     OFF
#define     OFF             (0U)
#endif

#ifndef	NULL
#define	NULL					0				// 空
#endif	//NULL

#ifndef	FILL
#define	FILL					1				// 满
#endif	//FILL

#ifndef	FALSE
#define	FALSE					0				// 错
#endif	//FALSE

#ifndef	TRUE
#define	TRUE					1				// 对
#endif	//TRUE

#ifndef EVEN
#define	EVEN					0				// 偶
#endif	//EVEN

#ifndef ODD
#define	ODD						1				// 奇
#endif	//ODD

#ifndef FREE
#define	FREE					0				// 闲
#endif	//FREE

#ifndef BUSY
#define	BUSY					1				// 忙
#endif	//BUSY

#ifndef BIT
#define BIT(x)					(1uL<<(x))		// 位
#endif	
#ifndef	MASK
#define	MASK(x)					BIT(x)			// 屏蔽位(选择的位为1,其它为0)
#define	UNMASK(x)				(~BIT(x))		// 反屏蔽位(选择的位为0,其它为1)
#endif	


#define BTYE2CHAR(HIGH,LOW) (((b[0] & 0xFF) << 8) | (b[1] & 0xFF))
#define FLIPW(ray) ((((uint16_t)(ray)[0]) * 256) + (ray)[1])

#define  SWAPBYTE(addr)        (((uint16_t)(*((uint8_t *)(addr)))) + \
                               (((uint16_t)(*(((uint8_t *)(addr)) + 1))) << 8))

#define LOBYTE(x)  ((uint8_t)(x & 0x00FF))
#define HIBYTE(x)  ((uint8_t)((x & 0xFF00) >>8))
#define MIN(a, b)  (((a) < (b)) ? (a) : (b))
#define MAX(a, b)  (((a) > (b)) ? (a) : (b))


/* -------------------- 与协议相关常量 ----------------------- */
#define LENOFVERSION			64
#define LENOFUPDATEFRAME		1024			// USB通信在线升级数据报的大小

/* ------------------- 与模板有关 ------------------------- */
#define LENOFMB  				256       		 // 模板的长度,不含CRC
#define LENOFTZ  				256       		 // 特征的长度,不含CRC


/* ---------------- 函数的返回值或命令的返回值 ---------------- */
#define C_INITTCS2ERROR				0x65			// 初始化TCS2参数失败


/* ---------------- GPIO ---------------- */
#define COMMANDBLINKDELAY		40
//#define	CLR_LED_R()	GPIO_ResetBits(GPIOA,GPIO_Pin_8);		
//#define	SET_LED_R()	GPIO_SetBits(GPIOA,GPIO_Pin_8);
//#define	CLR_LED_G()	GPIO_ResetBits(GPIOA,GPIO_Pin_9);		
//#define	SET_LED_G()	GPIO_SetBits(GPIOA,GPIO_Pin_9);



/* ---------------  通信命令的结构  ------------------*/
typedef enum 
{
	RS232_MODE =0x01,		// 设备工作模式 - RS232模式
	USB_MODE,				// 设备工作模式 - USB模式
	RS232_SPLIT_MODE, 		// 设备工作模式 - RS232拆分模式
	RS232_1A_MODE, 		 	// 设备工作模式 - RS232,0x1a模式
	RS232_ABC_MODE  	    // 设备工作模式 - RS232,农总行模式
}MODE_TypeData;

typedef struct
{
	uint8_t P1;						// 命令码
	uint8_t P2;						// 命令码
	uint8_t P3;						// 命令码
	uint8_t P4;						// 命令码
	uint8_t *pBuffer;				// 接收的数据
	uint16_t wLen;						// 实际的数据
	uint16_t AllNum;						// 最大长度

}StruCommRcv;

typedef struct
{
	uint8_t RetCode;						// 应答的结果
	uint8_t ComCode;						// 命令码
	const uint8_t *pBuffer;				// 需要发送的数据
	uint16_t wLen;						// 实际的数据
	uint16_t AllNum;						// 最大长度

}StruComm;



//延时函数
void Delayms(uint16_t ms);

void ProcessFailed(uint8_t *cpSendBuf,uint32_t *rlen,uint8_t cComType);	
uint8_t GetVersion(uint8_t *cpRecBuf,uint8_t *cpSendBuf, uint32_t *rlen,uint8_t cComType);
uint8_t Reset(uint8_t *cpRecBuf,uint8_t *cpSendBuf, uint32_t *rlen,uint8_t cComType);

#endif


