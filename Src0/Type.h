/******************************************************************************
@File Name:	TYPE.H
@Created By: Cattle
@Created Date: 2014.4.17
@Description : ���������ͽ����ض��壬�Է�����ֲ����Ӧ��ͬ�Ĵ�����
@Edition:V1.0
@Modify  By:
@Modify  Date:
@Modify  Description :
*******************************************************************************/

#ifndef TYPE_H_
#define TYPE_H_

//**********************************************************
//typedef void                 VOID;	   /*2014.4.28�Ըô����������Σ���ֹ��Щ������ƽ̨������  */
typedef unsigned char        BOOL;
typedef unsigned char        UCHAR;
typedef signed char          CHAR;
typedef unsigned char        UINT8;
typedef signed char          SINT8;
typedef unsigned short       UINT16;
typedef signed short         SINT16;
typedef unsigned int         UINT32;     /*STM32  32λ*/
typedef signed int           SINT32;
typedef unsigned long        ULONG;
typedef signed long          SLONG;
typedef unsigned long long   UINT64;
typedef signed long long     SINT64;
typedef float                FLOAT;
typedef double               DOUBLE;	
//**********************************************************



#ifndef FALSE
#define FALSE 0
#endif

#ifndef TRUE
#define TRUE   (!FALSE)
#endif

//#ifndef NULL
//#ifdef __cplusplus
#define NULL 0
//#else
//#define NULL ((void *)0)
//#endif
//#endif

//#define ENABLE 1
//#define DISABLE (!ENABLE)
//���� ������
typedef union 
{
	FLOAT  value;
	unsigned char byte[4];
} f_bytes;

//���� ������
typedef union 
{
	UINT16  value;
	unsigned char byte[2];
} i_bytes;


#define SUCESS  1
#define FAULT  -1
//**********************************************************
typedef UINT16(*pFun16)(void);
typedef SINT32 (*pFun32)(void);
typedef void (*pFun_void)(void);
//**********************************************************
#define NONE  (-1)
#define EOS   '\0'
//**********************************************************



#endif /*TYPE_H_ */

//------------------End of File-------------------------------------------------

