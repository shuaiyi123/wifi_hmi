/*
 * @Description: In User Settings Edit
 * @Author: your name
 * @Date: 2019-09-16 19:02:42
 * @LastEditTime: 2019-11-14 18:21:46
 * @LastEditors: Please set LastEditors
 */
#include <stdio.h>
#include "hmi.h"

char fWaveName[50][30];//保存录波文件名称
//实际遥测值增大100倍
short ycData[16]={10014,1234,20102,8975,20365,10230,8752,6532,10456,18452,17032,15243,12350,20135,15324,16523};//遥测数据
BYTE  yxData[2]={0,1};//遥信数据
BYTE *fileBuff; //1M文件缓冲区
FILEATTRI_TAG flAttr; //文件属性结构体
BYTE UserFlag[10]={0}; //用户标志位
TXD_TAG m_Txd={NULL,0}; //发送报文结构体
RXD_TAG m_Rxd={NULL,0}; //接收报文结构体

/**
 * @description: 将所有用户标志位清零
 * @param {type} 
 * @return: 
 */
void RstAllFlag()
{
    BYTE cnt;
    for(cnt=0;cnt<10;cnt++){
        UserFlag[cnt]=0;
    }
}