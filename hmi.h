/*
 * @Description: In User Settings Edit
 * @Author: your name
 * @Date: 2019-09-15 23:05:45
 * @LastEditTime: 2019-11-14 19:47:08
 * @LastEditors: Please set LastEditors
 */
#ifndef _HMI__H
#define _HMI__H

#include <stdio.h>
#include "gtypedef.h"

#ifdef __cplusplus
extern "C"
{
#endif

#define WAVE_NUM_MAX 40 //传送最大录波个数
#define BYTENUM 1024  //一包文件内容字节数,WiFi模块最大一包发送1042
#define FILE_NAME_LEN_MAX 40 //文件路径长度
#define FILE_LENTH 1048  //报文缓冲区大小
#define FILE_LEN_MAX 1024*1024 //1M文件缓冲区大小

#define SET(flag) *(flag) = 1;  //将用户标志置1
#define RST(flag) *(flag) = 0;   //将用户标志清0
#define FG_LINKCREATED    (UserFlag + 0) //链路建立标志位
#define FG_STRFILEUP      (UserFlag + 1) //Jsonn文件上送标志位
#define FG_STRFILELOWER   (UserFlag + 2) //Json文件下发标志位
#define FG_STRWAVEFILE    (UserFlag + 3) //读录波文件标志位

extern char fWaveName[WAVE_NUM_MAX][30];//保存录波文件名称
extern short ycData[16];//遥测数据
extern BYTE  yxData[2];//遥信数据,每一字节表示一个遥信状态
extern BYTE UserFlag[10]; //用户标志位
extern BYTE *fileBuff;//文件缓冲区

//文件属性结构体
typedef struct file
{
    char fileName[30];  //文件名
    BYTE fNameLen; //文件名长度
    char fv[12]; //文件版本号
    long fileLen; //文件长度
    WORD  fmNum;//文件传输帧数
    WORD  fmNum_bak; //文件传输帧数备份
    WORD cheSum;//文件校验和
    WORD crc16; //文件crc校验和
    BYTE modifyFlg; //文件修改标志
} FILEATTRI_TAG;
extern FILEATTRI_TAG flAttr;
//发送报文结构体
typedef struct txd
{
    BYTE *buf;
    WORD  len;
} TXD_TAG;
extern TXD_TAG m_Txd; 
 //接收报文结构体
typedef struct rxd
{
    BYTE *buf;
    WORD  len;
}RXD_TAG;
extern RXD_TAG m_Rxd;

//将所有标志位清零
void RstAllFlag();

//接收模块
int SigRst();
int manRecWave();

int RxdMonitor();
int SearchFrm();
int RxdFixFrm();
int RxdVarFrm();
void LinkProc();
void DeviceSet();
void ReqClock();
void ModClock();
void StrUpDeliver();
void RdJsonFile(BYTE msg);
void StrLowHair(BYTE msg);
void WdJsonFile(BYTE msg);
void QtyEvent();
void CallYCAll();
void CallYCOver();
void CallYXAll();
void CallYXChange();
void CheWaveList();
void StrTranWaveFile();
void RdWaveFile(BYTE msg);
BYTE ChkSum(BYTE* p_Addr,int chek_len);
void UpdateSysTime();

//发送模块
int TxdCheLink(BYTE msg);
int TdDevSet(BYTE msg);
int TdClock(BYTE funCode);
int TdJsonFileAttr(const char *flieName,BYTE *filebuff);
int GetJSONSum(const char *flieName,BYTE *filebuff);
WORD GetJSONCrc16(const char *flieName,BYTE *filebuff);
int TdJsonFile(BYTE msg,const BYTE *filebuff);
int TdLowHairAck(BYTE msg);
int TdWdJsonAck(BYTE msg);
int TdQtyEvent(BYTE msg);
int TdYCAll(BYTE ycNum);
int TdYCOver();
int TdYXAll(BYTE yxNum);
int TdYXChange();
int TdWaveList();
int scanDir(const char *dir);
int TdWaveAttr(BYTE msg);
int GetWaveSum(FILE *fd,BYTE *filebuff);
WORD GetWaveCrc16(FILE *fd,BYTE *filebuff);
int TdWaveFile(BYTE msg,const BYTE *filebuff);
void InvertUint8(BYTE *DesBuf, BYTE *SrcBuf);
void InvertUint16(WORD *DesBuf, WORD *SrcBuf);
WORD CRC16_X25(BYTE *puchMsg, DWORD usDataLen);
#ifdef __cplusplus
}
#endif

#endif