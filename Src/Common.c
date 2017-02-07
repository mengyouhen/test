
#include <stdio.h>
#include <string.h>  
#include "WLRERRNO.h"   
#include "HIDAPI.h"
#include "Common.h"
#include "SoftwareVersion.h"

StruCommStruct 	stCommStruct;


uint8_t volatile HID_CLASS_OUT =FALSE,HID_CLASS_IN = FALSE;

//*****************************************************     
// 函数名称: CmdGenAck     
// 函数说明�?    维尔数据组织
// �?   者：qianxh    
// 作成日期�?014/08/19 16 : 15     
// �?�?�? void     
// �?   �? unsigned char * cpBuf     
//*****************************************************
void CmdGenAck(unsigned char *cpBuf)
{
	uint32_t wLen;//guoyr
	uint32_t i;//second
	uint8_t cCrc;

	// 取数据部分的长度
	wLen = cpBuf[1] * 256 + cpBuf[2];

	// 计算CRC校验
	cCrc = 0;
	for (i=0; i<wLen+2; i++)
		cCrc ^= cpBuf[i+1];
	cpBuf[wLen+3] = cCrc;

}

//*****************************************************     
// 函数名称: AdaptCommRvcStruct     
// 函数说明�?    调整接收结构
// �?   者：qianxh    
// 作成日期�?014/08/22 14 : 22     
// �?�?�? UINT32     
// �?   �? StruCommRcv * stCommRcv     
// �?   �? UINT8 * cpBuf     
//*****************************************************
uint32_t AdaptCommRvcStruct(StruCommRcv *stCommRcv, uint8_t *cpBuf)
{
	uint32_t rlen;

	stCommRcv->AllNum = cpBuf[1] * 256 + cpBuf[2]; //除规定部分长�?

	stCommRcv->P1 = cpBuf[3];
	stCommRcv->P2 = cpBuf[4];
	stCommRcv->P3 = cpBuf[5];
	stCommRcv->P4 = cpBuf[6];


	if (stCommRcv->AllNum > 4)
	{
		stCommRcv->pBuffer=cpBuf+7;
		rlen = stCommRcv->AllNum - 4;
	}
	else
	{
		rlen = 0;
	}

	return(rlen);
}


//*****************************************************     
// 函数名称: AdaptCommSendStruct     
// 函数说明�?    调整发送结�?
// �?   者：qianxh    
// 作成日期�?014/08/19 16 : 18     
// �?�?�? UINT32     
// �?   �? StruComm * stComm     
// �?   �? UINT8 * cpBuf     
//*****************************************************
uint32_t AdaptCommSendStruct(StruComm *stComm,uint8_t *cpBuf,uint8_t cComType)
{
	uint32_t i;
	uint32_t rlen;
	uint8_t *pos;
	
	pos = cpBuf;

	*cpBuf++ = 0x02;				//包头
	*cpBuf++ = 0;					//从SW1到数据结束的长度
	*cpBuf++ = 0;					//从SW2到数据结束的长度
	*cpBuf++ = stComm->RetCode;		//命令结果
	*cpBuf++ = stComm->ComCode;		//命令�?

	if (0 != stComm->AllNum)
	{
		if (stComm->wLen >= stComm->AllNum)
		{	//上传其他
			for (i = 0; i < stComm->wLen; i++)
			{
				*cpBuf++ = stComm->pBuffer[i];
			}
		}
		else
		{
			for (i = 0; i < stComm->wLen; i++)
			{
				*cpBuf++ = stComm->pBuffer[i];
			}

			if (stComm->AllNum>stComm->wLen)
			{
				for (i = 0; i < stComm->AllNum - stComm->wLen; i++)
				{
					*cpBuf++ = 0x00;
				}
			}
		}
	}

	*cpBuf++ = 0x00;			// CRC
	*cpBuf++ = 0x03;

	rlen = cpBuf - &pos[3] - 2;
	pos[1] = rlen / 256;
	pos[2] = rlen % 256;

	CmdGenAck(&pos[0]);
	rlen = cpBuf - &pos[0];


	//USB模式
	if (cComType == USB_MODE)
	{
		return(rlen);
	}
	//RS232模式
	 else if(cComType == RS232_MODE)
	{
		return(rlen);

	}else if(cComType == RS232_SPLIT_MODE)
	{
		 //// 对命令序列进行拆�?
		 memcpy(&cpBuf[rlen * 2], &pos[0], rlen);

		 pos[0] = cpBuf[rlen * 2 ];
		 for (i = 0; i < rlen - 2; i++)
		 {
			 pos[i * 2 + 1] = ((cpBuf[rlen * 2 + i + 1] >> 4) & 0x0F) + 0x30;
			 pos[i * 2 + 2] = (cpBuf[rlen * 2 + i + 1] & 0x0F) + 0x30;
		 }
		 pos[i * 2 + 1] = cpBuf[rlen * 2 + i + 1];
		 rlen = i * 2 + 2;

		 return rlen;

	} 

	return 0;
}

//*****************************************************     
// 函数名称: UsbProcessFailed     
// 函数说明�?    返回通用错误
// �?   者：qianxh    
// 作成日期�?014/08/19 16 : 20     
// �?�?�? void     
// �?   �? UINT32 * rlen     
//*****************************************************
void ProcessFailed(uint8_t *cpSendBuf,uint32_t *rlen,uint8_t cComType)
{
	StruComm strComm;			

	strComm.RetCode = C_NOTOK; 
	strComm.ComCode = NULL;
	strComm.pBuffer = NULL;
	strComm.wLen =    NULL;
	strComm.AllNum =  NULL;
	*rlen = AdaptCommSendStruct(&strComm, cpSendBuf,cComType);
}



//*****************************************************     
// 函数名称: GetVersion     
// 函数说明�?    读取版本信息
//其中：P2�?x00时返回外部版本信息，即发版信息；
//          0x01时返回内部版本信息，可为发版信息或自定义信息�?
// �?   者：qianxh    
// 作成日期�?014/08/19 16 : 13     
// �?�?�? uint8_t     
// �?   �? uint8_t * cpRecBuf     
// �?   �? uint32_t * rlen     
//*****************************************************
uint8_t GetVersion(uint8_t *cpRecBuf,uint8_t *cpSendBuf, uint32_t *rlen,uint8_t cComType)
{
	
	StruComm strComm;			
	StruCommRcv stCommRcv;
	uint8_t ucInfoLen,SprintfsBuf[128];
	AdaptCommRvcStruct(&stCommRcv, cpRecBuf);

	strComm.RetCode = C_OK;
	strComm.ComCode = stCommRcv.P1;  //CMD_V50_GetInfo
	strComm.AllNum = LENOFVERSION;
	
	//参数类型
	if (stCommRcv.P2 == 0)
	{
		if (USB_MODE !=cComType)
		{
			strComm.pBuffer = (uint8_t*)SOFTWAREVERSION;
			strComm.wLen = strlen((char*)SOFTWAREVERSION);
			*rlen = AdaptCommSendStruct(&strComm, cpSendBuf,cComType);
		} 
		else
		{				
						
			ucInfoLen = (uint8_t)strlen((char*)SOFTWAREVERSION);
			memset(SprintfsBuf, 0, sizeof(SprintfsBuf));   
			SprintfsBuf[0] = ucInfoLen;
			memcpy(&SprintfsBuf[1], SOFTWAREVERSION, ucInfoLen);
			strComm.pBuffer = SprintfsBuf;
			strComm.wLen = ucInfoLen + 1;
			*rlen = AdaptCommSendStruct(&strComm, cpSendBuf,cComType);
		}
	} 
	else if (stCommRcv.P2 == 0x01)
	{
		if (USB_MODE !=cComType)
		{
			strComm.pBuffer = (uint8_t*)WELLSOFTWAREVERSION;
			strComm.wLen = strlen((char*)WELLSOFTWAREVERSION);
			*rlen = AdaptCommSendStruct(&strComm, cpSendBuf,cComType);
		} 
		else
		{
			ucInfoLen = (uint8_t)strlen((char*)WELLSOFTWAREVERSION);
			memset(SprintfsBuf, 0, sizeof(SprintfsBuf));
			SprintfsBuf[0] = ucInfoLen;
			memcpy(&SprintfsBuf[1], WELLSOFTWAREVERSION, ucInfoLen);
			strComm.pBuffer = SprintfsBuf;
			strComm.wLen = ucInfoLen + 1;
			*rlen = AdaptCommSendStruct(&strComm, cpSendBuf,cComType);
		}
	}
	else
	{
			strComm.RetCode = C_CMDERR;
			strComm.ComCode = NULL;
			strComm.pBuffer = NULL;
			strComm.wLen =    NULL;
			strComm.AllNum =  NULL;
			*rlen = AdaptCommSendStruct(&strComm, cpSendBuf,cComType);
	}
	
	return(C_OK);
}



//*****************************************************     
// 函数名称: Reset     
// 函数说明�?    
// �?   者：qianxh    
// 作成日期�?014/08/22 17 : 18     
// �?�?�? uint8_t     
// �?   �? uint8_t * cpRecBuf     
// �?   �? uint32_t * rlen     
//*****************************************************
uint8_t Reset(uint8_t *cpRecBuf,uint8_t *cpSendBuf, uint32_t *rlen,uint8_t cComType)
{
	StruComm strComm;
	StruCommRcv stCommRcv;
	AdaptCommRvcStruct(&stCommRcv, cpRecBuf);

	strComm.RetCode = C_OK;
	strComm.ComCode = stCommRcv.P1;  
	strComm.pBuffer = NULL;
	strComm.wLen = NULL;
	strComm.AllNum = NULL;
	*rlen = AdaptCommSendStruct(&strComm, cpSendBuf, cComType);
	
	return(C_OK);
}


