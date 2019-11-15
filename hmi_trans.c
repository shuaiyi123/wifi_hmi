/*
 * @Description: In User Settings Edit
 * @Author: your name
 * @Date: 2019-09-16 05:00:34
 * @LastEditTime: 2019-11-14 19:47:19
 * @LastEditors: Please set LastEditors
 */
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <sys/time.h>
#include <time.h>
#include <string.h>

#include "hmi.h"
#include "wifi_drive.h"
#include "syslog.h"
/**
 * @description:链路处理 
 * @param {type} 
 * @return: 
 */
int TxdCheLink(BYTE msg)
{
    BYTE cheSum;
    BYTE *pBuf;
    int TdLen;

    pBuf=m_Txd.buf;
    //填充启动字符
    pBuf[0]=pBuf[5]=0x10;
    //定位到功能码
    m_Txd.len=6;
    pBuf[m_Txd.len++]=0x01;
    //填充信息域
    pBuf[m_Txd.len++]=msg;
    //填充报文长度
    pBuf[1]=pBuf[3]=LOBYTE(m_Txd.len-6);
    pBuf[2]=pBuf[4]=HIBYTE(m_Txd.len-6);
    //填充检验和
    cheSum=ChkSum(pBuf+6,m_Txd.len-6);
    pBuf[m_Txd.len++]=cheSum;
    //填充结束字符
    pBuf[m_Txd.len++]=0x16;
    //发送报文
    if((TdLen=transData(m_Txd.buf,m_Txd.len))==ERROR){
        return ERROR;
    }   
    else{
        return TdLen;//返回发送的报文长度
    }      
}
/**
 * @description:装置操作 
 * @param {type} 
 * @return: 
 */
int TdDevSet(BYTE msg)
{
    BYTE cheSum;
    BYTE *pBuf;
    int TdLen;

    pBuf=m_Txd.buf;
    //填充启动字符
    pBuf[0]=pBuf[5]=0x10;
    //定位到功能码
    m_Txd.len=6;
    pBuf[m_Txd.len++]=0x10;
    //填充信息域
    pBuf[m_Txd.len++]=msg;
    //填充报文长度
    pBuf[1]=pBuf[3]=LOBYTE(m_Txd.len-6);
    pBuf[2]=pBuf[4]=HIBYTE(m_Txd.len-6);
    //填充校验和
    cheSum=ChkSum(pBuf+6,m_Txd.len-6);
    pBuf[m_Txd.len++]=cheSum;
    //填充结束字符
    pBuf[m_Txd.len++]=0x16;
    //发送报文
    if((TdLen=transData(m_Txd.buf,m_Txd.len))==ERROR){
        return ERROR;
    }   
    else{
        return TdLen;//返回发送的报文长度
    } 
}
/**
 * @description:发送时钟
 * @param {type} 
 * @return: 
 */
int TdClock(BYTE funCode)
{
    BYTE cheSum;
    BYTE *pBuf;
    int TdLen;
    DWORD time;
    struct timeval tm;

    //获取系统时间戳
    gettimeofday(&tm,NULL);
    pBuf=m_Txd.buf;
    //填充启动字符
    pBuf[0]=pBuf[5]=0x68;
    //固定到功能码
    m_Txd.len=6;
    pBuf[m_Txd.len++]=funCode;
    //填充信息域
    time=(DWORD)tm.tv_sec;
    pBuf[m_Txd.len++]=LLBYTE(time);
    pBuf[m_Txd.len++]=LHBYTE(time);
    pBuf[m_Txd.len++]=HLBYTE(time);
    pBuf[m_Txd.len++]=HHBYTE(time);
    time=(DWORD)tm.tv_usec;
    pBuf[m_Txd.len++]=LLBYTE(time);
    pBuf[m_Txd.len++]=LHBYTE(time);
    pBuf[m_Txd.len++]=HLBYTE(time);
    pBuf[m_Txd.len++]=HHBYTE(time);
    //填充报文长度
    pBuf[1]=pBuf[3]=LOBYTE(m_Txd.len-6);
    pBuf[2]=pBuf[4]=HIBYTE(m_Txd.len-6);
    //填充检验和
    cheSum=ChkSum(pBuf+6,m_Txd.len-6);
    pBuf[m_Txd.len++]=cheSum;
    //填充结束字符
    pBuf[m_Txd.len++]=0x16;
    //发送报文
    if((TdLen=transData(m_Txd.buf,m_Txd.len))==ERROR){
        return ERROR;
    }   
    else{
        return TdLen;//返回发送的报文长度
    } 
}
/**
 * @description: 上送文件属性
 * @param {type} 
 * @return: 
 */
int TdJsonFileAttr(const char *flieName,BYTE *filebuff)
{
    BYTE cheSum; //校验和
    BYTE *pBuf;
    int TdLen; //报文长度
    int ll,lh,hl;
    
    //获取json文件校验和
    flAttr.crc16=GetJSONCrc16(flieName,filebuff);
    if(flAttr.crc16==ERROR)//文件打开错误或文件内容为空
        return ERROR;
    pBuf=m_Txd.buf;
    //填充启动字符
    pBuf[0]=pBuf[5]=0x68;
    //定位到功能码
    m_Txd.len=6;
    pBuf[m_Txd.len++]=0x20;
    //填充文件名长度
    flAttr.fNameLen=(BYTE)(strlen(flAttr.fileName)+1);
    pBuf[m_Txd.len++]=flAttr.fNameLen;
    //填充文件名
    for(BYTE cnt=0;cnt<flAttr.fNameLen;cnt++){
        pBuf[m_Txd.len++]=flAttr.fileName[cnt];
    }
    //填充文件大小
    pBuf[m_Txd.len++]=LLBYTE((DWORD)flAttr.fileLen);
    pBuf[m_Txd.len++]=LHBYTE((DWORD)flAttr.fileLen);
    pBuf[m_Txd.len++]=HLBYTE((DWORD)flAttr.fileLen);
    pBuf[m_Txd.len++]=HHBYTE((DWORD)flAttr.fileLen);
    //填充版本号
    sprintf(flAttr.fv,"1.0.0");
    sscanf(flAttr.fv,"%d.%d.%d",&hl,&lh,&ll);
    pBuf[m_Txd.len++]=(BYTE)ll;
    pBuf[m_Txd.len++]=(BYTE)lh;
    pBuf[m_Txd.len++]=(BYTE)hl;
    pBuf[m_Txd.len++]=0x00;
    //填充文件crc16校验码
    pBuf[m_Txd.len++]=LOBYTE(flAttr.crc16);
    pBuf[m_Txd.len++]=HIBYTE(flAttr.crc16);
    //填充文件改变标志
    pBuf[m_Txd.len++]=0x00;
    //填充传输帧数
    //填充传输帧数
    if(flAttr.fileLen%1024==0){
        flAttr.fmNum=(flAttr.fileLen/1024); 
    }
    else{
        flAttr.fmNum=(flAttr.fileLen/1024)+1;
    }
    flAttr.fmNum_bak=flAttr.fmNum;
    pBuf[m_Txd.len++]=flAttr.fmNum;
    //填充报文长度
    pBuf[1]=pBuf[3]=LOBYTE(m_Txd.len-6);
    pBuf[2]=pBuf[4]=HIBYTE(m_Txd.len-6);
    //填充检验和
    cheSum=ChkSum(pBuf+6,m_Txd.len-6);
    pBuf[m_Txd.len++]=cheSum;
    //填充结束字符
    pBuf[m_Txd.len++]=0x16;
    //发送报文
    if((TdLen=transData(m_Txd.buf,m_Txd.len))==ERROR){
        return ERROR;
    }   
    else{
        return TdLen;//返回发送的报文长度
    } 
}
/**
 * @description:求Json文件校验和 
 * @param {type} 
 * @return: 
 */
int GetJSONSum(const char *flieName,BYTE *filebuff)
{
    int ch;
    int fileCheSum=0;
    long cnt=0;
    FILE *fd = NULL;
    char filePath[FILE_NAME_LEN_MAX];
    //打开文件
    sprintf(filePath,"configs/%s",flAttr.fileName);
    if(NULL == (fd=fopen(filePath,"r"))){
        logMsg(logErr,"[Error]fopen %s  failed",flAttr.fileName);
        return ERROR;
    }
    // 将文件指针移到文件末尾
    fseek(fd, 0, SEEK_END);
    flAttr.fileLen = ftell(fd); // 获取文件长度
    if(flAttr.fileLen==0){
        logMsg(logErr,"[Error]%s is empty!",flAttr.fileName);
        return ERROR;
    }
    //定位到文件首
    fseek(fd, 0, SEEK_SET);
    //读取文件内容到存储区
    while((ch = fgetc(fd)) !=EOF)
        filebuff[cnt++]=(BYTE)ch;   
   // fread(filebuff,sizeof(BYTE),flAttr.fileLen,fd);
    //求文件校验和
    for(long num=0;num<flAttr.fileLen;num++){
        fileCheSum += filebuff[num];
    }
    fclose(fd);
    return fileCheSum;
}
/**
 * @description:求Json文件crc16校验码 
 * @param {type} 
 * @return: 
 */
WORD GetJSONCrc16(const char *flieName,BYTE *filebuff)
{
    int ch;
    WORD fileCrc16 = 0;
    long cnt=0;
    FILE *fd = NULL;
    char filePath[FILE_NAME_LEN_MAX];
    //打开文件
    sprintf(filePath,"configs/%s",flAttr.fileName);
    if(NULL == (fd=fopen(filePath,"r"))){
        logMsg(logErr,"[Error]fopen %s  failed",flAttr.fileName);
        return ERROR;
    }
    // 将文件指针移到文件末尾
    fseek(fd, 0, SEEK_END);
    flAttr.fileLen = ftell(fd); // 获取文件长度
    if(flAttr.fileLen==0){
        logMsg(logErr,"[Error]%s is empty!",flAttr.fileName);
        return ERROR;
    }
    //定位到文件首
    fseek(fd, 0, SEEK_SET);
    //读取文件内容到存储区
    while((ch = fgetc(fd)) !=EOF)
        filebuff[cnt++]=(BYTE)ch;   
   // fread(filebuff,sizeof(BYTE),flAttr.fileLen,fd);
    //求文件CRC校验码
    fileCrc16 = CRC16_X25(filebuff,flAttr.fileLen);
    fclose(fd);
    return fileCrc16;
}
/**
 * @description: 从动方发送json文件内容
 * @param {type} 
 * @return: 
 */
int TdJsonFile(BYTE msg,const BYTE *filebuff)
{  
    long fileseek=0; 
    BYTE cheSum; //校验和
    BYTE *pBuf;
    int TdLen;
    
    //判断文件传输帧数
    if(flAttr.fmNum > 0){
        //定位到信息域
        if(msg==1){ //上一帧发送成功，进入发送下一帧
            if(flAttr.fmNum>1){
                 flAttr.fileLen -= BYTENUM;
            }
            //包数自-1
            flAttr.fmNum -= 1;
            if(flAttr.fmNum==0){//传输完成
                RST(FG_STRWAVEFILE);//将传输录波文件标志清零
                 return OK;
            }    
        }
        pBuf=m_Txd.buf;
        //填充启动字符
        pBuf[0]=pBuf[5]=0x68;
        m_Txd.len=6;//固定报文头，6字节
        //填充功能码
        pBuf[m_Txd.len++]=0x21;
        //填充文件内容
        if(flAttr.fmNum>1){ //大于1包
            fileseek=flAttr.fmNum_bak - flAttr.fmNum;
            fileseek *=BYTENUM;
            for(int num=0;num<BYTENUM;num++){
                pBuf[m_Txd.len++]=filebuff[fileseek++];
            }
        }
        else{ //剩余最后1包
            fileseek=flAttr.fmNum_bak-flAttr.fmNum;
            fileseek *=BYTENUM;
            for(int num=0;num<flAttr.fileLen;num++){
                 pBuf[m_Txd.len++]=filebuff[fileseek++];
            }
        }
        //填充报文长度
        pBuf[1]=pBuf[3]=LOBYTE(m_Txd.len-6);
        pBuf[2]=pBuf[4]=HIBYTE(m_Txd.len-6);
        //填充检验和
        cheSum=ChkSum(pBuf+6,m_Txd.len-6);
        pBuf[m_Txd.len++]=cheSum;
        //填充结束字符
        pBuf[m_Txd.len++]=0x16;
        //发送报文
        if((TdLen=transData(m_Txd.buf,m_Txd.len))==ERROR){
            return ERROR;
        }
    }
    return OK; 
}
/**
 * @description:启动下发应答 
 * @param {type} 
 * @return: 
 */
int TdLowHairAck(BYTE msg)
{
    BYTE cheSum; //校验和
    BYTE *pBuf;
    int TdLen;

    pBuf=m_Txd.buf;
    //填充启动字符
    pBuf[0]=pBuf[5]=0x68;
    m_Txd.len=6;//固定报文头，6字节
    //填充功能码
    pBuf[m_Txd.len++]=0x22;
    //文件名长度=文件名字符串+字符串结束符
    pBuf[m_Txd.len++]=flAttr.fNameLen;
    //填充反校文件名
    for(BYTE cnt=0;cnt<flAttr.fNameLen;cnt++){
        pBuf[m_Txd.len++]=flAttr.fileName[cnt];
    }
     pBuf[m_Txd.len++] = msg;
    //填充报文长度
    pBuf[1]=pBuf[3]=LOBYTE(m_Txd.len-6);
    pBuf[2]=pBuf[4]=HIBYTE(m_Txd.len-6);
    //填充检验和
    cheSum=ChkSum(pBuf+6,m_Txd.len-6);
    pBuf[m_Txd.len++]=cheSum;
    //填充结束字符
    pBuf[m_Txd.len++]=0x16;
    //发送报文
    if((TdLen=transData(m_Txd.buf,m_Txd.len))==ERROR){
        return ERROR;
    }   
    else{
        return TdLen;//返回发送的报文长度
    } 

}
/**
 * @description:接收下发应答
 * @param {type} 
 * @return: 
 */
int TdWdJsonAck(BYTE msg)
{
    BYTE cheSum; //校验和
    BYTE *pBuf;
    int TdLen;

    pBuf=m_Txd.buf;
    //填充启动字符
    pBuf[0]=pBuf[5]=0x10;
    m_Txd.len=6;//固定报文头，6字节
    //填充功能码
    pBuf[m_Txd.len++]=0x23;
    //填充信息域
    pBuf[m_Txd.len++]=msg;
    //填充报文长度
    pBuf[1]=pBuf[3]=LOBYTE(m_Txd.len-6);
    pBuf[2]=pBuf[4]=HIBYTE(m_Txd.len-6);
    //填充检验和
    cheSum=ChkSum(pBuf+6,m_Txd.len-6);
    pBuf[m_Txd.len++]=cheSum;
    //填充结束字符
    pBuf[m_Txd.len++]=0x16;
    //发送报文
    if((TdLen=transData(m_Txd.buf,m_Txd.len))==ERROR){
        return ERROR;
    }   
    else{
        return TdLen;//返回发送的报文长度
    } 
}
/**
 * @description: 上送历史事件或soe事件或运行信息或无事件
 * @param {type} 
 * @return: 
 */
int TdQtyEvent(BYTE msg)
{
    BYTE cheSum; //校验和
    BYTE *pBuf;
    int TdLen;

    pBuf=m_Txd.buf;
    //填充启动字符
    pBuf[0]=pBuf[5]=0x68;
    m_Txd.len=6;//固定报文头，6字节
    //填充功能码
    pBuf[m_Txd.len++]=0x30;
    if(msg==1){//历史事件
        pBuf[m_Txd.len++]=00;
    }
    else if(msg==2){//SOE事件
        pBuf[m_Txd.len++]=00;
    }
    else if(msg==3){//运行信息
        pBuf[m_Txd.len++]=00;
    }
    else //无事件
        pBuf[m_Txd.len++]=00;
    //填充报文长度
    pBuf[1]=pBuf[3]=LOBYTE(m_Txd.len-6);
    pBuf[2]=pBuf[4]=HIBYTE(m_Txd.len-6);
    //填充检验和
    cheSum=ChkSum(pBuf+6,m_Txd.len-6);
    pBuf[m_Txd.len++]=cheSum;
    //填充结束字符
    pBuf[m_Txd.len++]=0x16;
    //发送报文
    if((TdLen=transData(m_Txd.buf,m_Txd.len))==ERROR){
        return ERROR;
    }   
    else{
        return TdLen;//返回发送的报文长度
    } 
}
/**
 * @description:上送所有遥测 
 * @param {type} 
 * @return: 
 */
int TdYCAll(BYTE ycNum)
{
    BYTE cheSum; //校验和
    BYTE *pBuf;
    int TdLen;

    pBuf=m_Txd.buf;
    //填充启动字符
    pBuf[0]=pBuf[5]=0x68;
    m_Txd.len=6;//固定报文头，6字节
    //填充功能码
    pBuf[m_Txd.len++]=0x40;
    //填充遥测总数量
    pBuf[m_Txd.len++]=ycNum;
    //填充遥测数值
    for(BYTE cnt=0;cnt<ycNum;cnt++){
        pBuf[m_Txd.len++]=LOBYTE((WORD)ycData[cnt]);//遥测低位
        pBuf[m_Txd.len++]=HIBYTE((WORD)ycData[cnt]);//遥测高位
    }
    //填充报文长度
    pBuf[1]=pBuf[3]=LOBYTE(m_Txd.len-6);
    pBuf[2]=pBuf[4]=HIBYTE(m_Txd.len-6);
    //填充检验和
    cheSum=ChkSum(pBuf+6,m_Txd.len-6);
    pBuf[m_Txd.len++]=cheSum;
    //填充结束字符
    pBuf[m_Txd.len++]=0x16;
    //发送报文
    if((TdLen=transData(m_Txd.buf,m_Txd.len))==ERROR){
        return ERROR;
    }   
    else{
        return TdLen;//返回发送的报文长度
    } 
}
/**
 * @description:上送越限遥测 
 * @param {type} 
 * @return: 
 */
int TdYCOver()
{
    BYTE cheSum; //校验和
    BYTE *pBuf;
    int TdLen;

    pBuf=m_Txd.buf;
    //填充启动字符
    pBuf[0]=pBuf[5]=0x68;
    m_Txd.len=6;//固定报文头，6字节
    //填充功能码
    pBuf[m_Txd.len++]=0x41;
    //填充变化遥测总数
    pBuf[m_Txd.len++]=2;
    //填充变化遥测点号
    pBuf[m_Txd.len++]=0; //遥测点号0~15
    //填充遥测数据
    pBuf[m_Txd.len++]=LOBYTE((WORD)ycData[0]);//遥测低位
    pBuf[m_Txd.len++]=HIBYTE((WORD)ycData[0]);//遥测高位
    //填充遥测点号
    pBuf[m_Txd.len++]=1;
    //填充变化遥测数据
    pBuf[m_Txd.len++]=LOBYTE((WORD)ycData[1]);//遥测低位
    pBuf[m_Txd.len++]=HIBYTE((WORD)ycData[1]);//遥测高位
    //填充报文长度
    pBuf[1]=pBuf[3]=LOBYTE(m_Txd.len-6);
    pBuf[2]=pBuf[4]=HIBYTE(m_Txd.len-6);
    //填充检验和
    cheSum=ChkSum(pBuf+6,m_Txd.len-6);
    pBuf[m_Txd.len++]=cheSum;
    //填充结束字符
    pBuf[m_Txd.len++]=0x16;
    //发送报文
    if((TdLen=transData(m_Txd.buf,m_Txd.len))==ERROR){
        return ERROR;
    }   
    else{
        return TdLen;//返回发送的报文长度
    } 
}
/**
 * @description: 上送所有遥信数据
 * @param {type} 
 * @return: 
 */
int TdYXAll(BYTE yxNum)
{
    BYTE yxTemp;
    BYTE cheSum; //校验和
    BYTE *pBuf;
    int TdLen;

    pBuf=m_Txd.buf;
    //填充启动字符
    pBuf[0]=pBuf[5]=0x68;
    m_Txd.len=6;//固定报文头，6字节
    //填充功能码
    pBuf[m_Txd.len++]=0x42;
    //填充遥信总数量
    pBuf[m_Txd.len++]=yxNum;
    //填充遥信数值,最大8路遥信
    for(BYTE cnt=0;cnt<yxNum;cnt++){
        //取最低位
        yxTemp=yxData[cnt]&0x01;
        yxTemp <<= cnt;
        pBuf[m_Txd.len] |= yxTemp;
    }
    //logMsg(logInfo,"所有摇信:%d",pBuf[m_Txd.len]);
    m_Txd.len++;
    //填充报文长度
    pBuf[1]=pBuf[3]=LOBYTE(m_Txd.len-6);
    pBuf[2]=pBuf[4]=HIBYTE(m_Txd.len-6);
    //填充检验和
    cheSum=ChkSum(pBuf+6,m_Txd.len-6);
    pBuf[m_Txd.len++]=cheSum;
    //填充结束字符
    pBuf[m_Txd.len++]=0x16;
    //发送报文
    if((TdLen=transData(m_Txd.buf,m_Txd.len))==ERROR){
        return ERROR;
    }   
    else{
        return TdLen;//返回发送的报文长度
    } 
}
/**
 * @description:上送变位遥信 
 * @param {type} 
 * @return: 
 */
int TdYXChange()
{
    BYTE cheSum; //校验和
    BYTE *pBuf;
    int TdLen;

    pBuf=m_Txd.buf;
    //填充启动字符
    pBuf[0]=pBuf[5]=0x68;
    m_Txd.len=6;//固定报文头，6字节
    //填充功能码
    pBuf[m_Txd.len++]=0x43;
    //填充变位遥信总数
    pBuf[m_Txd.len++]=2;
    //填充变位遥信点号1
    pBuf[m_Txd.len++]=0; //摇信点号0~1
    //填充遥信数据
    pBuf[m_Txd.len++]=yxData[0];
    //logMsg(logInfo,"0摇号摇信:%d",pBuf[m_Txd.len-1]);
    //填充变位遥信点号2
    pBuf[m_Txd.len++]=1;
    //填充变位遥信数据
    pBuf[m_Txd.len++]=yxData[1];
    //logMsg(logInfo,"1摇信:%d",pBuf[m_Txd.len-1]);
    //填充报文长度
    pBuf[1]=pBuf[3]=LOBYTE(m_Txd.len-6);
    pBuf[2]=pBuf[4]=HIBYTE(m_Txd.len-6);
    //填充检验和
    cheSum=ChkSum(pBuf+6,m_Txd.len-6);
    pBuf[m_Txd.len++]=cheSum;
    //填充结束字符
    pBuf[m_Txd.len++]=0x16;
    //发送报文
    if((TdLen=transData(m_Txd.buf,m_Txd.len))==ERROR){
        return ERROR;
    }   
    else{
        return TdLen;//返回发送的报文长度
    } 
}
/**
 * @description:查询录波文件列表 
 * @param {type} 
 * @return: 
 */
int TdWaveList()
{
    BYTE fNLen; //文件名长度
    BYTE fileNum;//文件个数
    BYTE cheSum; //校验和
    BYTE *pBuf;
    int TdLen;

    pBuf=m_Txd.buf;
    //填充启动字符
    pBuf[0]=pBuf[5]=0x68;
    m_Txd.len=6;//固定报文头，6字节
    //填充功能码
    pBuf[m_Txd.len++]=0x50;
    //扫描录波目录
    fileNum=scanDir("./wavefiles");
    if(fileNum<1){//没有录波文件
        pBuf[m_Txd.len++]=fileNum;
    }
    else{//有录波文件
        //录波文件条目数
        pBuf[m_Txd.len++]=fileNum;
        //填充所有录波文件名
        for(BYTE cnt=0;cnt<fileNum;cnt++){
            //文件名长度=文件名字符串+字符串结束符
            fNLen=strlen(fWaveName[cnt]);
            pBuf[m_Txd.len++]=(fNLen+1);
            //填充本条录波文件名
            for(BYTE num=0;num<(fNLen+1);num++){
                pBuf[m_Txd.len++]=fWaveName[cnt][num];
            }
        }
    }
    //填充报文长度
    pBuf[1]=pBuf[3]=LOBYTE(m_Txd.len-6);
    pBuf[2]=pBuf[4]=HIBYTE(m_Txd.len-6);
    //填充检验和
    cheSum=ChkSum(pBuf+6,m_Txd.len-6);
    pBuf[m_Txd.len++]=cheSum;
    //填充结束字符
    pBuf[m_Txd.len++]=0x16;
    //发送报文
    if((TdLen=transData(m_Txd.buf,m_Txd.len))==ERROR){
        return ERROR;
    }   
    else{
        return TdLen;//返回发送的报文长度
    } 
}
/**
 * @description:扫描波录目录下的所有录波文件 
 * @param {type} 
 * @return:  返回录波文件个数
 */
int scanDir(const char *dir)
{
    BYTE flag;
    int num,cnt=0;
    char fName[30];
    DIR *dp;
    struct dirent *entry;
    //打开保存录波文件目录
  
    if((dp = opendir(dir))==NULL){
        logMsg(logErr,"cannot open directory:%s",dir);
        return ERROR;
    }
    //进入录波文件目录
    chdir(dir);
    while((entry=readdir(dp)) != NULL){
        if (strcmp(".", entry->d_name) == 0 || strcmp("..", entry->d_name) == 0) //跳过本目录和上级目录 
              continue;   
        sscanf(entry->d_name,"%[^.]",fName);//剔除文件后缀         
        for(num=0;num<cnt;num++){
            if(strcmp(fWaveName[num],fName)==0){//找到相同的则跳出
                flag=0;
                goto tag;
            }    
        }
        flag=1;  
    tag://前面没出现过的文件名
        if(flag==1){
            flag=0;
            strcpy(fWaveName[cnt],fName);
           //logMsg(logInfo,"%s",fWaveName[cnt]);
            cnt++;
            if(cnt==WAVE_NUM_MAX){//最大传送录波个数
                 return cnt;
            }     
        }
          
    }
    //返回上一级目录
    chdir("..");
    //关闭目录
    closedir(dp);
    return cnt;
}
/**
 * @description: 上送录波文件属性
 * @param {type} 
 * @return: 
 */
int TdWaveAttr(BYTE msg)
{
    FILE *fd;
    char fName[40];
    BYTE cheSum; //校验和
    BYTE *pBuf;
    int TdLen;

    pBuf=m_Txd.buf;
    switch(msg){ //判断文件类型
        case 0: //.cfg文件
        case 1: //.dat文件
        case 2: //.hdr文件
        case 3: //.inf文件
            sprintf(fName,"wavefiles/%s",flAttr.fileName);
            fd=fopen(fName,"r");
            break;
        default:
            fd = NULL;    
    }
    if(fd==NULL){//文件打开失败,说明无此文件,回复无此文件报文
        logMsg(logErr,"%s open failed!",flAttr.fileName);
         //填充启动字符
        pBuf[0]=pBuf[5]=0x10; 
        m_Txd.len=6;//固定报文头，6字节
        //填充功能码
        pBuf[m_Txd.len++]=0x51;
        //填充信息域
        pBuf[m_Txd.len++]=0;
    }
    else{ //有此文件,回复文件属性
        flAttr.crc16=GetWaveCrc16(fd,fileBuff);
        //填充启动字符
        pBuf[0]=pBuf[5]=0x68; 
        m_Txd.len=6;//固定报文头，6字节
        //填充功能码
        pBuf[m_Txd.len++]=0x51;
        //填充文件名长度
        flAttr.fNameLen=(BYTE)(strlen(flAttr.fileName)+1);
        pBuf[m_Txd.len++]=flAttr.fNameLen;
        //填充文件名
        for(BYTE cnt=0;cnt<flAttr.fNameLen;cnt++){
            pBuf[m_Txd.len++]=flAttr.fileName[cnt];
        }
        //填充文件大小
        pBuf[m_Txd.len++]=LLBYTE((DWORD)flAttr.fileLen);
        pBuf[m_Txd.len++]=LHBYTE((DWORD)flAttr.fileLen);
        pBuf[m_Txd.len++]=HLBYTE((DWORD)flAttr.fileLen);
        pBuf[m_Txd.len++]=HHBYTE((DWORD)flAttr.fileLen);
        //填充文件校验和
        pBuf[m_Txd.len++]=LOBYTE(flAttr.crc16);
        pBuf[m_Txd.len++]=HIBYTE(flAttr.crc16);
        //填充传输帧数
        if(flAttr.fileLen%1024==0){
           flAttr.fmNum=(flAttr.fileLen/1024); 
        }
        else{
            flAttr.fmNum=(flAttr.fileLen/1024)+1;
        }
        flAttr.fmNum_bak=flAttr.fmNum;
        pBuf[m_Txd.len++]=LOBYTE(flAttr.fmNum);
        pBuf[m_Txd.len++]=HIBYTE(flAttr.fmNum);
    }
    //填充报文长度,回复此文件不存在报文
    pBuf[1]=pBuf[3]=LOBYTE(m_Txd.len-6);
    pBuf[2]=pBuf[4]=HIBYTE(m_Txd.len-6);
    //填充检验和
    cheSum=ChkSum(pBuf+6,m_Txd.len-6);
    pBuf[m_Txd.len++]=cheSum;
    //填充结束字符
    pBuf[m_Txd.len++]=0x16;
    //发送报文
    if((TdLen=transData(m_Txd.buf,m_Txd.len))==ERROR){
        return ERROR;
    }   
    else{
        return TdLen;//返回发送的报文长度
    } 
}
/**
 * @description:求录波文件校验和 
 * @param {type} 
 * @return: 
 */
int GetWaveSum(FILE *fd,BYTE *filebuff)
{
    int ch;
    int fileCheSum=0;
    long cnt=0;
  
    //将文件指针移到文件末尾
    fseek(fd, 0, SEEK_END);
    flAttr.fileLen = ftell(fd); // 获取文件长度
    if(flAttr.fileLen==0){
        logMsg(logErr,"[Error]%s is empty!",flAttr.fileName);
        return 0;
    }
    //定位到文件首
    fseek(fd, 0, SEEK_SET);
    //读取文件内容到存储区
    while((ch = fgetc(fd)) !=EOF)
        filebuff[cnt++]=(BYTE)ch;   
   // fread(filebuff,sizeof(BYTE),flAttr.fileLen,fd);
    //求文件校验和
    for(long num=0;num<flAttr.fileLen;num++){
        fileCheSum += (BYTE)filebuff[num];
    }
    fclose(fd);
    return fileCheSum;
}
/**
 * @description:求录波文件CRC16校验码 
 * @param {type} 
 * @return: 
 */
WORD GetWaveCrc16(FILE *fd,BYTE *filebuff)
{
    int ch;
    WORD fileCrc16=0;
    long cnt=0;
  
    //将文件指针移到文件末尾
    fseek(fd, 0, SEEK_END);
    flAttr.fileLen = ftell(fd); // 获取文件长度
    if(flAttr.fileLen==0){
        logMsg(logErr,"[Error]%s is empty!",flAttr.fileName);
        return 0;
    }
    //定位到文件首
    fseek(fd, 0, SEEK_SET);
    //读取文件内容到存储区
    while((ch = fgetc(fd)) !=EOF)
        filebuff[cnt++]=(BYTE)ch;   
   // fread(filebuff,sizeof(BYTE),flAttr.fileLen,fd);
    //求文件CRC16校验码
    fileCrc16 = CRC16_X25(filebuff,flAttr.fileLen);
    fclose(fd);
    return fileCrc16;
}
/**
 * @description: 读录波文件
 * @param {type} 
 * @return: 
 */
int TdWaveFile(BYTE msg,const BYTE *filebuff)
{
    long fileseek=0; 
    BYTE cheSum; //校验和
    BYTE *pBuf;
    int TdLen;
     
    //判断文件传输帧数
    if(flAttr.fmNum>0){
        //定位到信息域
        if(msg==1){ //上一帧发送成功，进入发送下一帧
            if(flAttr.fmNum>1){
                flAttr.fileLen -= BYTENUM;
            }     
            //包数自减1
            flAttr.fmNum -= 1;
            if(flAttr.fmNum==0){//传输完成
                 RST(FG_STRWAVEFILE);//将传输录波文件标志清零
                 return OK;
            }     
        }
        pBuf=m_Txd.buf;
        //填充启动字符
        pBuf[0]=pBuf[5]=0x68;
        m_Txd.len=6;//固定报文头，6字节
        //填充功能码
        pBuf[m_Txd.len++]=0x52;
        //填充文件内容
        if(flAttr.fmNum>1){ //大于1包
            fileseek=flAttr.fmNum_bak - flAttr.fmNum;
            fileseek *=BYTENUM;
            for(int num=0;num<BYTENUM;num++){
                pBuf[m_Txd.len++]=filebuff[fileseek++];
            }
        }
        else{ //剩余最后1包
            fileseek=flAttr.fmNum_bak-1;
            fileseek *=BYTENUM;
            for(int num=0;num<flAttr.fileLen;num++){
                 pBuf[m_Txd.len++]=filebuff[fileseek++];
            }
        }
        //填充报文长度
        pBuf[1]=pBuf[3]=LOBYTE(m_Txd.len-6);
        pBuf[2]=pBuf[4]=HIBYTE(m_Txd.len-6);
        //填充检验和
        cheSum=ChkSum(pBuf+6,m_Txd.len-6);
        pBuf[m_Txd.len++]=cheSum;
        //填充结束字符
        pBuf[m_Txd.len++]=0x16;
        //发送报文
        if((TdLen=transData(m_Txd.buf,m_Txd.len))==ERROR){
            return ERROR;
        }   
        else{
            return TdLen;//返回发送的报文长度
        } 
    } 
    return OK; 
}
/**
***************************************************************************************************
* @Brief    Single byte data inversion        
* @Param    
*            @DesBuf: destination buffer
*            @SrcBuf: source buffer
* @RetVal    None
* @Note      (MSB)0101_0101 ---> 1010_1010(LSB)
**************************************************************************************************
*/
void InvertUint8(BYTE *DesBuf, BYTE *SrcBuf)
{
    int i;
    unsigned char temp = 0;
      
    for(i = 0; i < 8; i++)
    {
        if(SrcBuf[0] & (1 << i))
        {
            temp |= 1<<(7-i);
        }
    }
    DesBuf[0] = temp;
}
 /**
 **************************************************************************************************
 * @Brief    double byte data inversion        
 * @Param    
 *            @DesBuf: destination buffer
 *            @SrcBuf: source buffer
 * @RetVal    None
 * @Note      (MSB)0101_0101_1010_1010 ---> 0101_0101_1010_1010(LSB)
 **************************************************************************************************
 */
void InvertUint16(WORD *DesBuf, WORD *SrcBuf)  
{  
    int i;  
    unsigned short temp = 0;   

    for(i = 0; i < 16; i++)  
    {  
        if(SrcBuf[0] & (1 << i))
        {          
            temp |= 1<<(15 - i);  
        }
    }  
    DesBuf[0] = temp;  
}
/*二、CRC16校验码计算方法
　　1.根据CRC16的标准选择初值CRCIn的值。
　　2.将数据的第一个字节与CRCIn高8位异或。
　　3.判断最高位，若该位为 0 左移一位，若为 1 左移一位再与多项式Hex码异或。
　　4.重复3直至8位全部移位计算结束。
　　5.重复将所有输入数据操作完成以上步骤，所得16位数即16位CRC校验码。*/
/**
 * @description: 获取crc16校验码
 * @param {puchMsg:信息,usDataLen:信息长度} 
 * @return: 
 */
WORD CRC16_X25(BYTE *puchMsg, DWORD usDataLen)  
{  
    WORD wCRCin = 0xFFFF;   //初始值
    WORD wCPoly = 0x1021;  //多项式
    BYTE wChar = 0;  //中间值
     
    while (usDataLen--)     
    {  
        wChar = *(puchMsg++);  
        InvertUint8(&wChar, &wChar);  //反转位
        wCRCin ^= (wChar << 8); 
        for(int i = 0;i < 8;i++)  
        {  
            if(wCRCin & 0x8000)
            {              
                wCRCin = (wCRCin << 1) ^ wCPoly; 
            }            
            else  
            {
                wCRCin = wCRCin << 1; 
            }            
        }  
    }  
    InvertUint16(&wCRCin, &wCRCin);  
    return (wCRCin^0xFFFF);  //返回crc16码
}  