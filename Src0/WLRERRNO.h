
////维尔金融错误码
#ifndef _WLR_ERRORS_H_
#define _WLR_ERRORS_H_


#define RET_OK						0	//正确响应
#define RET_MATCH_ERROR			    201	//比对失败。
#define RET_MATCH_ERROR2			202	//比对失败。

#define C_OK               			0x00            // 操作成功
#define C_CMDTIMEOUT          		0x01            // 命令超时
#define C_CMDERR           			0x03            // 命令字错误
#define C_CMDPARAERR          		0x04            // 命令参数错误
#define	C_CMDCRCERR           		0x05            // 校验和错误
#define	C_SECURITYOVERFLOW          0x06            // 安全级别越界
#define	C_BUADRATEOVERFLOW       	0x07            // 波特率越界
#define	C_INDEXFULL           		0x26            // 索引越界
#define C_FIGIMGERR        			0x30            // 规定时间未采集到指纹图像
#define C_FIGENRERR        			0x31            // 登录错误
#define C_FIGCMPERR        			0x32            // 不匹配
#define C_FIGEXTERR        			0x33            // 采样错误
#define C_FIGTIMEOUT				0x34			// 超时
#define C_FIGMBBUFEMPTY				0x35			// 模板BUF为空					
#define C_FIGTZBUFEMPTY				0x36			// 特征BUF为空	
#define C_FIGINVALIDMB				0x37			// 无效的模板或特征	
#define C_FIGMBINDEXERROR			0x38			// 模板索引错误	
#define C_FIGNOVALIDMB				0x39			// 指定模板区域中没有有效的模板		
#define C_FIGSEARCHERROR			0x3A			// 模板库搜索错误
#define GFP_VERIFY_ERROR			0x3B			// 验证错误
#define C_IMAGEERROR        		0x40            // 图像宽度错误，不能生成特征
#define C_IMAGEOVERFLOW        		0x60            // 指纹图像上溢出
#define	C_UPDATEDATEERROR           0x61            // 程序升级时接收数据量错误
#define	C_UPDATECRCERROR			0x62            // 程序升级时接收数据校验和错误
#define	C_NOIMAGE					0x63            // 没有采集指纹图像
#define	C_TOOFEWFEATURES           	0x72            // 模板或特征中的特征点太少
#define	C_MEMALLOCERR           	0x80            // 内存分配错误
#define	C_BREAK           			0x81            // 操作中断
#define	C_CRCERROR           		0x82            // 校验和错误
#define	RET_OPER_ERROR           	0x83            // 校验和错误
#define CMD_DELETEFROMMBLIB 		0x22			// 删除库
#define CMD_READMBLIBTOMBBUF 		0x23			// 读取库到MBBUF
#define C_PAGEINDEOVERFLOW        	0x23            // 页号越界

#define C_NOTTCS2ERROR				0x64			// 没有TCS传感器
#define C_INITTCS2ERROR				0x65			// 初始化TCS2参数失败

#define C_NOTOK            			0xff        	// 通用错误


#endif

/**********************************************************************************************************
  End of File
**********************************************************************************************************/
