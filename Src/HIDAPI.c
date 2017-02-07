

#include <stdio.h>
#include <string.h>   
#include "WLRERRNO.h"   
#include "HIDAPI.h"
#include "Common.h"
//#include "usbd_hid.h"


uint8_t CommDataTypeFlag=0;
uint8_t UserBuf[1*1024];


DISPATCH_ITEM  HID_table[]=
{
	// 读取版本---[ 0x01]
	{ USB_GETVERSIONINFO,GetVersion},			

	// 仪复位---[0x02]
	{ USB_RESET,Reset },

};

uint8_t UsbCmdProcess(uint8_t function_no, uint8_t *cpRecBuf,uint8_t *cpSendBuf, uint32_t *rlen,uint8_t cComType)
{
	int i;

	for(i = 0; i < (sizeof(HID_table)/sizeof(HID_table[0])); i++)
	{
		if(function_no == HID_table[i].ucCommand)
		{
			HID_table[i].func_ptr(cpRecBuf,cpSendBuf, rlen, cComType);
			return C_OK;
		}
	}

	ProcessFailed(cpSendBuf,rlen, cComType);
	return C_NOTOK;
}



//************************************     
// 函数名称: HID_EVENT     
// 函数说明：     
// 作    者: qianxh
// 作成日期：2016/02/26     
// 返 回 值: void     
// 参    数: void     
//************************************ 
void HID_EVENT(void)
{
	uint32_t len=0;
	uint8_t *cpSendBuf;
	
		cpSendBuf = &UserBuf[0];
		//uint8_t ComCode;
		//uint8_t SubComCode;
		//uint8_t *cpRecBuf;
		//uint8_t *cpSendBuf;

		//if (CommDataTypeFlag == USB_MODE)
		//{                 
			//ComCode = UserBuf[3];
			//SubComCode = UserBuf[4];
			//cpRecBuf = &UserBuf[0];
			//cpSendBuf = &UserBuf[0];
	//printf("\n\rHID_EVENT UserBuf:%d,%d,%d,%d,%d\n\r",UserBuf[0],UserBuf[1],UserBuf[2],UserBuf[3],UserBuf[4]);	
		wellcom_SendHID_Data(cpSendBuf, len);

}




//************************************     
// 函数名称: HIDPortdata     
// 函数说明：     
// 作    者: qianxh
// 作成日期：2016/02/26     
// 返 回 值: uint8_t     
// 参    数: unsigned char * pRXData     
// 参    数: unsigned char * pData     
// 参    数: uint32_t u16DataLen     
//************************************ 	 
uint8_t HIDPortdata(uint8_t* pData,uint32_t u16DataLen,uint8_t* pRXData,uint32_t *RDataLen)
{
	uint8_t u8Ret = HID_FAIL;

	static uint8_t HID_OUT_STATE =STATE_02_START;
	static uint8_t volatile u8Byte;				  // 接收到的数据
	static uint8_t volatile u8Count = 0;			  // 接收计数值
	uint8_t u8Buffer[64];		         // 接收临时缓冲区

	static uint32_t volatile u32Len_NoSplit;		  // 不拆分格式数据长度
	static uint32_t volatile u32Rx_cnt;		        //缓存接收计数值
	static uint32_t volatile TimeOutEventCount =0;		//状态超时时间

	//pRecvStruct = (StruCommRcvStruct *)((uint32_t)* pData);
	//pRecvStruct = &stRecvStruct;
	
	if (CommDataTypeFlag != 0)
	{
			return u8Ret;
	}

	while(u16DataLen--)
	{
		u8Byte = *pData++;
		switch (HID_OUT_STATE)
		{
//-----------包头----------------------------------			
			case STATE_02_START:			
				
				u32Rx_cnt = 0;				
				TimeOutEventCount =0; //开始记录时间				
				if(u8Byte == 0x02)
				{					
					pRXData[u32Rx_cnt++] = u8Byte;			
					u8Count = 0;	
					HID_OUT_STATE = STATE_02_CMD_LEN;
				}
				break;
				
//-----------长度----------------------------------				
			case STATE_02_CMD_LEN:
				
				u8Buffer[u8Count++] = u8Byte;
				if(u8Count == 2)
				{		
					pRXData[u32Rx_cnt++]  = u8Buffer[0] ;
					pRXData[u32Rx_cnt++]  = u8Buffer[1] ;					
					u32Len_NoSplit = u8Buffer[0] * 256 + u8Buffer[1] ;	

					HID_OUT_STATE = STATE_02_CMD_DATA;			
					if(u32Len_NoSplit > HID_PKTBUF_SIZE)
					{
						HID_OUT_STATE = STATE_02_START;
					}		
				}
				break;
				
//-----------数据----------------------------------								
			case STATE_02_CMD_DATA:
				pRXData[u32Rx_cnt++]  = u8Byte;
				u32Len_NoSplit--;
				if(u32Len_NoSplit ==0)
				{					
					u32Len_NoSplit =2;
					HID_OUT_STATE =STATE_02_CMD_END;
				}
				break;

//-----------03----------------------------------								
			case STATE_02_CMD_END:
	
				pRXData[u32Rx_cnt++]  = u8Byte;
				u32Len_NoSplit--;
				if(u32Len_NoSplit ==0)
				{					
					if(u8Byte ==0x03)
					{
						u8Ret = HID_OK;
						CommDataTypeFlag = USB_MODE;
						*RDataLen = u32Rx_cnt;	
					}
					HID_OUT_STATE =STATE_02_START;
					return u8Ret;
				}
				break;
			
			default:
				HID_OUT_STATE =STATE_02_START;
				break;
		}
		
		if(TimeOutEventCount >HID_RCV_TIMEOUT)
		{
			u8Ret = HID_TIMEOUT;
			return u8Ret;
		}	
	}
	return u8Ret;
	
}
//延迟1us：
/*******************************************************************************
* 函 数 名         : delay_us
* 函数功能           : 延时函数，延时us
* 输    入         : i
* 输    出         : 无
*******************************************************************************/
void delay_us(uint32_t i)
{
	uint32_t temp;
    SysTick->LOAD=9*i;          //设置重装数值, 72MHZ时
    SysTick->CTRL=0X01;         //使能，减到零是无动作，采用外部时钟源
    SysTick->VAL=0;             //清零计数器
    do
    {
        temp=SysTick->CTRL;           //读取当前倒计数值
    }
    while((temp&0x01)&&(!(temp&(1<<16))));     //等待时间到达
    SysTick->CTRL=0;       //关闭计数器
    SysTick->VAL=0;        //清空计数器
}

//延迟1ms：
/*******************************************************************************
* 函 数 名         : delay_ms
* 函数功能           : 延时函数，延时ms
* 输    入         : i
* 输    出         : 无
*******************************************************************************/
void delay_ms(uint32_t i)
{
	uint32_t temp;
    SysTick->LOAD=9000*i;      //设置重装数值, 72MHZ时
    SysTick->CTRL=0X01;        //使能，减到零是无动作，采用外部时钟源
    SysTick->VAL=0;            //清零计数器
    do
    {
        temp=SysTick->CTRL;       //读取当前倒计数值
    }
    while((temp&0x01)&&(!(temp&(1<<16))));    //等待时间到达
    SysTick->CTRL=0;       //关闭计数器
    SysTick->VAL=0;        //清空计数器
}





