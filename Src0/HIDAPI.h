#ifndef _HIDAPI_H_
#define _HIDAPI_H_

#include <stdint.h>
#include "Common.h"


//USB命令的命令码
#define USB_GETVERSIONINFO   			0x01				// 读版本号及厂家标识
#define USB_RESET									0x02				// 指纹仪复位
#define USB_SETSECURITYLEVEL			0x04				// 设置指纹仪安全等级命令
#define USB_GETSECURITYLEVEL			0x05				// 读取识别模块安全等级命令
#define USB_GETSTATUS  						0x07				// 读指纹模块的状态字命令
#define USB_CALIBRATE  						0x08				// 指纹传感器校准命令（Calibrate）
#define USB_SETDEVICEID  					0x0B				// 设置指纹仪序列号
#define USB_GETDEVICEID  					0x0C				// 读取指纹仪序列号
#define USB_SETCACHE 							0x0D				// 写内容到用户数据区数据页
#define USB_GETCACHE 							0x0E				// 读取用户数据区数据页
#define USB_ENROLL  							0x10				// 用户录入3次指纹，生成指纹模板并上传
#define USB_FEATURE  							0x12				// 采集指纹图像，生成指纹特征值，并上传。
#define USB_GATHERENROLL					0x1B		        // 分次采集指纹模板数据
#define USB_SYNCENROLL						0x1C			   	// 分次合成指纹模板
#define USB_IMMEDIATELYFEATURE			0x1D				// 立即采集指纹特征
#define USB_IMMEDIATELYGETIMAGEUPLOAD  	0x1E                // 采集指纹图像并上传命令
#define USB_IMMEDIATELYGETIMAGE 		0x1F                // 立即采集指纹图像不上传
#define USB_UPLOADMB  							0x2A				// 将保存在MB_BUF中的指纹模板上传
#define USB_UPLOADTZ  							0x2B				// 将保存在TZ_BUF中的指纹特征上传
#define USB_DOWNLOADMB  						0x2C				// 下载一枚指纹模板到MB_BUF中
#define USB_DOWNLOADTZ  						0x2D				// 下载一枚指纹特征到TZ_BUF中
#define USB_VERIFY 			    				0x2E				// 将TZ_BUF和MB_BUF中的特征和模板进行比较
#define USB_GETIMAGE								0x40				// 采集指纹图像命令
#define USB_UPLOADIMAGE							0x41				// 上传指纹图像命令
#define USB_CHECKFINGER							0x43				// 检测手指是否放下
#define USB_UPDATEPROGRAM						0x50				// 程序升级命令
#define USB_VERIFYFINGER 						0xA0				// 指纹比对命令

#define USB_CLEARMB									0x20				// 清空指纹库
#define USB_INSERTMB								0x21				// 向指纹库中插入一枚指纹模板
#define USB_WRITEMBLIB							0x22				// 丛指纹库中删除一枚指纹模板
#define USB_GETMBLIBCOUNTER					0x23				// 读取指纹库个数
#define USB_SEARCH									0x31				// 指纹搜索命令


#define HID_PKTBUF_SIZE 2048
#define HID_RCV_TIMEOUT 5000

/* USB Device status */
typedef enum {
  HID_OK   = 0,
	HID_TIMEOUT,
  HID_FAIL,
}HID_ReturnStatus;

typedef enum 
{
  STATE_02_START = 0U, 
	STATE_02_CMD_LEN,
  STATE_02_CMD_DATA, 
	STATE_02_CMD_CHECK,  
	STATE_02_CMD_END,  
}HID_OUTStatus;


extern uint8_t volatile HID_CLASS_OUT ,HID_CLASS_IN ;

///////////////////////////////////////////////////////////////////////////


//通信结构
typedef struct
{
	uint8_t *cpRecBuf;
	uint8_t *cpSendBuf; 
	uint32_t *rlen;
	uint8_t cComType;
	uint8_t function_no;
}StruCommStruct;



typedef uint8_t (*ProcessFuncPtr)(uint8_t *cpRecBuf,uint8_t *cpSendBuf, uint32_t *rlen,uint8_t cComType);

typedef struct tagDispatchItem
{
	uint8_t ucCommand;
	ProcessFuncPtr func_ptr;
}DISPATCH_ITEM;


extern uint8_t CommDataTypeFlag;
extern uint8_t UserBuf[1*1024];

extern uint8_t USBD_HID_SendData(uint8_t *sendData,uint32_t sendLen);
extern void HID_EVENT(void);
extern uint8_t HIDPortdata(uint8_t* pData,uint32_t u16DataLen,uint8_t* pRXData,uint32_t *RDataLen);
extern uint8_t wellcom_SendHID_Data(uint8_t *SendData,uint32_t len);

extern uint8_t GetVersion(uint8_t *cpRecBuf,uint8_t *cpSendBuf, uint32_t *rlen,uint8_t cComType);

extern void delay_us(uint32_t i);
extern void delay_ms(uint32_t i);
#endif


