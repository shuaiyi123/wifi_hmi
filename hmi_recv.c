/*
 * @Description: In User Settings Edit
 * @Author: your name
 * @Date: 2019-09-16 05:00:10
 * @LastEditTime: 2019-11-13 05:37:39
 * @LastEditors: Please set LastEditors
 */
#include <unistd.h>
#include<sys/time.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>

#include "hmi.h"
#include "syslog.h"

/**
 * @description: 接收报文处理
 * @param {type} 
 * @return: 
 */
int RxdMonitor()
{
    BYTE *pBuf;

    pBuf=m_Rxd.buf;
    m_Rxd.len=7; //定位到功能码
    switch(m_Rxd.buf[m_Rxd.len]){
        case 0x23://接收json文件
            break;
        default:
            if(SearchFrm()!=OK){ //检索报文
                logMsg(logErr,"Invalid frame messsage!");
                return ERROR;
            }
    }
    if(pBuf[0]==0x10){
        return RxdFixFrm(); //固定帧处理
    }
    else if(pBuf[0]==0x68){
        return RxdVarFrm(); //可变帧处理
    }
    else{
        return ERROR;
    }   
}
/**
 * @description:解析接收到的报文
 * @param {type} 
 * @return: 
 */
int SearchFrm()
{
    BYTE *pBuf;
    WORD frmLen,Re_frmLen;
    BYTE cheSum;

    pBuf=m_Rxd.buf;
    switch (pBuf[0])
    {
    case 0x10 ://固定帧全帧长度10字节
        //获取长度
        frmLen=MAKEWORD(pBuf[1],pBuf[2]);
        //获取重复长度
        Re_frmLen=MAKEWORD(pBuf[3],pBuf[4]);
        //比较两长度
        if(frmLen!=Re_frmLen){
            logMsg(logErr,"Lenth of frame error!");
            return ERROR;
        }
        //固定帧报文长度==2
        if(frmLen!=2){
            logMsg(logErr,"Lenth of frame error!");
            return ERROR;
        }
        //再次判断启动字符
        if(pBuf[5]!=0x10){
            logMsg(logErr,"Repeat charater for start error!");
            return ERROR;
        }
        //获取校验和
        cheSum=ChkSum(pBuf+6,frmLen);
        //定位到校验和=信息域长度+固定报文头(6)
        frmLen=frmLen+6;
        //判断校验和
        if(cheSum!=pBuf[frmLen]){
            logMsg(logErr,"Check sum error!");
            return ERROR;
        }
        //判断结束符
        if(pBuf[frmLen+1]!=0x16){
            logMsg(logErr,"Character of end error!");
            return ERROR;
        }
        break;
    case 0x68 :
        //获取长度
        frmLen=MAKEWORD(pBuf[1],pBuf[2]);
        //获取重复长度
        Re_frmLen=MAKEWORD(pBuf[3],pBuf[4]);
        //比较长度
        if(frmLen!=Re_frmLen){
            logMsg(logErr,"Lenth of frame error!");
            return ERROR;
        }
        //再次判断启动字符
        if(pBuf[5]!=0x68){
            logMsg(logErr,"Repeat charater for start error!");
            return ERROR;
        }
        //获取校验和，固定报文头6字节
        cheSum=ChkSum(pBuf+6,frmLen);
        //定位到校验和=信息域长度+固定报文头(6)
        frmLen=frmLen+6;
        //判断校验和
        if(cheSum!=pBuf[frmLen]){
            logMsg(logErr,"Check sum=%d,pBuf[chesum]=%d",cheSum,pBuf[frmLen]);
            return ERROR;
        }
        //判断结束符
        if(pBuf[frmLen+1]!=0x16){
            logMsg(logErr,"Character of end error!");
            return ERROR;
        }
        break;
    default:
        logMsg(logErr,"Character of start error!");
        return ERROR;
    }
    return OK;
}
/**
 * @description:固定帧处理 
 * @param {type} 
 * @return: 
 */
int  RxdFixFrm()
{
    BYTE *pBuf=NULL;
    BYTE fCode;
    BYTE msg;

    pBuf = m_Rxd.buf;
    fCode = pBuf[6];//获取功能码
    msg = pBuf[7];

    switch(fCode){
        case 0x21:
            RdJsonFile(msg);//应答 上送Json文件内容
            break;
        case 0x52:
            RdWaveFile(msg);//应答 上送录波文件内容
        default:
            return ERROR;
    }
    return OK;
}
/**
 * @description:可变帧处理 
 * @param {type} 
 * @return: 
 */
int RxdVarFrm()
{
    BYTE *pBuf=NULL;
    BYTE fCode;
    
    pBuf = m_Rxd.buf;
    fCode = pBuf[6];//获取功能码
    
    switch(fCode){
        case 0x01:
            LinkProc();//链路处理
            break;
        case 0x10:
            DeviceSet();//装置设置
            break;
        case 0x11:
            ReqClock();//请求时钟
            break;
        case 0x12:
            ModClock();//修改时钟
            break;
        case 0x20:
            StrUpDeliver();//启动Josn文件上送
            break;
        case 0x21:
            RdJsonFile((BYTE)0);//启动方读json文件
            break;
        case 0x22://启动下发
            StrLowHair(1);//允许下发
            break;
        case 0x23://启动方写json文件
            if(SearchFrm()!=OK){
                logMsg(logErr,"Invalid frame,repeated transmission!");
                WdJsonFile(0);//上一包接收失败，启动方重发上一包
            }
            else{
                WdJsonFile(1);//上一包接收成功，启动方发送下一包
            }    
            break;
        case 0x30:
            QtyEvent();//查询事件记录及运行信息
            break;
        case 0x40:
            CallYCAll();//召唤（读）所有遥测数据
            break;
        case 0x41:
            CallYCOver();//召唤（读）越限遥测数据
            break;
        case 0x42:
            CallYXAll();//召唤（读）所有遥信数据
            break;
        case 0x43:
            CallYXChange();//召唤（读）变位遥信数据
            break;
        case 0x50:
            CheWaveList();//查询录波文件列表
            break;
        case 0x51:
            StrTranWaveFile();//启动录波文件传输
            break;
        case 0x52:
            RdWaveFile((BYTE)0);//读录波数据
            break;
        default:
            return ERROR;
    }
    return OK;
}
/**
 * @description:链路处理 
 * @param {type} 
 * @return: 
 */
void LinkProc()
{
    BYTE *pBuf;
    
    pBuf=m_Rxd.buf;
    if(pBuf[7]==0x55){//测试链路
        RstAllFlag();//将所有标志位清零
        SET(FG_LINKCREATED);//链路建立标志置位
        TxdCheLink((BYTE)0x55);
    }
    else if(pBuf[7]==0xAA){//请求断开
        //TxdCheLink((BYTE)0xAA);
        //RST(FG_LINKCREATED);//链路建立标志清零
    }
    else{
        logMsg(logErr,"Link check error code=%d,expectly it is 0x55 or 0xAA",pBuf[7]);
    } 
}
/**
 * @description:装置设置 
 * @param {type} 
 * @return: 
 */
void DeviceSet()
{
    BYTE msg;
    BYTE *pBuf;

    pBuf=m_Rxd.buf;
    if(pBuf[7]==0x00){//信号复归
        if(SigRst()==OK)//信号复归成功
            msg=0x01;
        else
            msg=0x00;
        TdDevSet(msg); //发送应答报文
    }
    else if(pBuf[7]==0x01){//手动录波
        if(manRecWave()==OK)//录波成功
            msg=0x01;
        else
            msg=0x00;
        TdDevSet(msg);
    }
    else if(pBuf[7]==0x02){//装置复位
        msg=0x01;
        TdDevSet(msg);
        system("reboot");
    }
    else{
        logMsg(logErr,"Device set code error =%d,expectly it is 0x00 or 0x01 or 0x02",pBuf[7]);
    }
}
/**
 * @description: 请求时钟
 * @param {type} 
 * @return: 
 */
void ReqClock()
{
    BYTE funCode=0x11;
    TdClock(funCode);//发送时间
}
/**
 * @description:修改时钟 
 * @param {type} 
 * @return: 
 */
void ModClock()
{
    BYTE funCode=0x12;
    UpdateSysTime();//更新系统时间
    usleep(100000); //延时100ms等待硬件时间更新完成
    TdClock(funCode);//发送时间
}
/**
 * @description: 启动上送json文件
 * @param {type} 
 * @return: 
 */
void StrUpDeliver()
{
    BYTE *pBuf;

    pBuf=m_Rxd.buf;
    m_Rxd.len=7; //定位到文件名长度字节
    flAttr.fNameLen=pBuf[m_Rxd.len++];
    //获取文件名
    for(BYTE cnt=0;cnt<flAttr.fNameLen;cnt++){
        flAttr.fileName[cnt]=(char)pBuf[m_Rxd.len++];
    }
    SET(FG_STRFILEUP);//将上送标志位置1
    TdJsonFileAttr(flAttr.fileName,fileBuff);//发送文件属性
}
/**
 * @description:发送json文件 
 * @param {type} 
 * @return: 
 */
void RdJsonFile(BYTE msg)
{
    if(*FG_STRFILEUP==1){
        TdJsonFile(msg,fileBuff);//发送第一包json文件内容
    }
}
/**
 * @description:启动下发json文件 
 * @param {type} 
 * @return: 
 */
void StrLowHair(BYTE msg)
{
    //定位到文件名长度
    m_Rxd.len=7;
    //获取文件名长度
    flAttr.fNameLen=m_Rxd.buf[m_Rxd.len++];
    //获取文件名
    for(BYTE cnt=0;cnt<flAttr.fNameLen;cnt++){
        flAttr.fileName[cnt]=(char)m_Rxd.buf[m_Rxd.len++];
    }
    //获取文件大小
    flAttr.fileLen=MAKEDWORD(m_Rxd.buf[m_Rxd.len],m_Rxd.buf[m_Rxd.len+1],m_Rxd.buf[m_Rxd.len+2],m_Rxd.buf[m_Rxd.len+3]);
    m_Rxd.len += 4;
    //获取文件版本号
    sprintf(flAttr.fv,"%d.%d.%d.%d",m_Rxd.buf[m_Rxd.len],m_Rxd.buf[m_Rxd.len+1],m_Rxd.buf[m_Rxd.len+2],m_Rxd.buf[m_Rxd.len+3]);
    m_Rxd.len += 4;
    //获取文件crc16
    flAttr.crc16=MAKEWORD(m_Rxd.buf[m_Rxd.len],m_Rxd.buf[m_Rxd.len+1]);
    m_Rxd.len += 2;
    //获取文件修改标志
    flAttr.modifyFlg=m_Rxd.buf[m_Rxd.len++];
    //获取文件传输帧数
    flAttr.fmNum=m_Rxd.buf[m_Rxd.len++];
    flAttr.fmNum_bak=flAttr.fmNum;
    //设置下发标志
    SET(FG_STRFILELOWER);
    TdLowHairAck(msg);//启动下发应答
}
/**
 * @description:启动方写json文件 
 * @param {type} 
 * @return: 
 */
void WdJsonFile(BYTE msg)
{
    char path[40];
    BYTE *pBuf;
    WORD flcrc16=0;
    WORD oneFrmLen;  //每帧文件长度
    FILE *fd;
    long fileseek=0; 
    
    if(*FG_STRFILELOWER==1){
        if(msg==1){//这一帧接收正确
            pBuf=m_Rxd.buf;
            m_Rxd.len=7;//定位到文件内容    
            //提取文件内容
            if(flAttr.fmNum>1){ //大于1包
                fileseek=flAttr.fmNum_bak - flAttr.fmNum;
                fileseek *=BYTENUM;
                for(int num=0;num<BYTENUM;num++){
                    fileBuff[fileseek++]=pBuf[m_Rxd.len++];
                }
                flAttr.fmNum -= 1;
            }
            else { //剩余最后1包
                oneFrmLen = MAKEWORD(pBuf[1],pBuf[2])-1;
                fileseek=flAttr.fmNum_bak-1;
                fileseek *=BYTENUM;//求得数组下标偏移地址
                for(int num=0;num<oneFrmLen;num++){
                    fileBuff[fileseek++]=pBuf[m_Rxd.len++];
                }
                RST(FG_STRFILELOWER);//将下发标志清零
                //求文件crc16
                flcrc16 = CRC16_X25(fileBuff,(DWORD)flAttr.fileLen);
                //文件校验和相同,且已被修改过，则保存文件
                if(flcrc16 == flAttr.crc16 && flAttr.modifyFlg == 1){
                    sprintf(path,"configs_bak/%s",flAttr.fileName);
                    fd=fopen(path,"w");
                    if(NULL==fd){
                        logMsg(logErr,"fopen %s error",flAttr.fileName);
                    }
                    else{
                        fwrite((char*)fileBuff,sizeof(char),flAttr.fileLen,fd);
                        fclose(fd);
                        logMsg(logInfo,"Save %s succeed",flAttr.fileName);
                    }
                }
                else{
                    logMsg(logErr,"file crc16 error!");
                }
            }
        }
    }
    TdWdJsonAck(msg);//接收下发应答
}
/**
 * @description: 录波事件查询及运行信息查询
 * @param {type} 
 * @return: 
 */
void QtyEvent()
{
    BYTE msg=0;//无事件
    TdQtyEvent(msg);
}
/**
 * @description:召唤全遥测数据 
 * @param {type} 
 * @return: 
 */
void CallYCAll()
{
    BYTE ycNum=16;//16路遥测
    TdYCAll(ycNum);
}
/**
 * @description:召唤越限遥测 
 * @param {type} 
 * @return: 
 */
void CallYCOver()
{
    TdYCOver();
}
/**
 * @description:召唤所有遥信数据 
 * @param {type} 
 * @return: 
 */
void CallYXAll()
{
    BYTE yxNum=2;//2路遥信
    TdYXAll(yxNum);
}
/**
 * @description:召唤变位摇信 
 * @param {type} 
 * @return: 
 */
void CallYXChange()
{
    TdYXChange();
}
/**
 * @description:查询录波列表 
 * @param {type} 
 * @return: 
 */
void CheWaveList()
{
    TdWaveList();
}
/**
 * @description: 启动传输录波文件
 * @param {type} 
 * @return: 
 */
void  StrTranWaveFile()
{
    BYTE *pBuf;
    BYTE msg;
    
    pBuf=m_Rxd.buf;
    m_Txd.len=7;//定位到信息域
    msg=pBuf[m_Txd.len++];//获取信息域
    //获取文件名长度
    flAttr.fNameLen=pBuf[m_Txd.len++];
    //获取文件名
    for(BYTE cnt=0;cnt<flAttr.fNameLen;cnt++){
        flAttr.fileName[cnt]=(char)pBuf[m_Txd.len++];
    }
    SET(FG_STRWAVEFILE);
    TdWaveAttr(msg);
}
/**
 * @description:发送录波文件 
 * @param {type} 
 * @return: 
 */
void RdWaveFile(BYTE msg)
{ 
    //提取信息域
    if(*FG_STRWAVEFILE==1){
        TdWaveFile(msg,fileBuff);   
    }
}
/**
 * @description:计算帧的校验和值
 * @param {校验和变量的起始地址，检验和变量的长度} 
 * @return: 校验和值
 */
BYTE ChkSum(BYTE* p_Addr,int chek_len)
{
    int i;
	BYTE checkSum=0;  
	BYTE* headAddr;
	headAddr=p_Addr;
	for(i=0;i<chek_len;i++){
		checkSum += (*headAddr);     //计算校验和，不考虑溢出
		//logMsg(logInfo,"checkSum=%d",checkSum);
		headAddr++;
	}
	return checkSum;
}
/**
 * @description:更新系统时间 
 * @param {type} 
 * @return: 
 */
void UpdateSysTime()
{
    BYTE *pBuf;
    struct timeval tm;
    DWORD time;

    pBuf=m_Rxd.buf;
    //获取秒
    time=MAKEDWORD(pBuf[7],pBuf[8],pBuf[9],pBuf[10]);
    tm.tv_sec=(__time_t)time;
    //获取微秒
    time=MAKEDWORD(pBuf[11],pBuf[12],pBuf[13],pBuf[14]);
    tm.tv_usec=(__suseconds_t)time;
    //更新系统时间
    settimeofday(&tm,NULL);
    //将系统时间同步到硬件时钟
    //system("bash shell/wdhwclock.sh");
    system("hwclock -w");
}
/**
 * @description: 信号复归
 * @param {type} 
 * @return: 
 */
int SigRst()
{
    return OK;
}
/**
 * @description: 手动录波
 * @param {type} 
 * @return: 
 */
int manRecWave()
{
    return OK;
}
