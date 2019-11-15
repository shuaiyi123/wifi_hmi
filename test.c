/*
 * @Description: In User Settings Edit
 * @Author: your name
 * @Date: 2019-09-16 02:59:38
 * @LastEditTime: 2019-11-14 21:46:18
 * @LastEditors: Please set LastEditors
 */
#include <stdlib.h>
#include <stdbool.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <termios.h>
#include <time.h>
#include <sys/select.h>

#include "hmi.h"
#include "wifi_drive.h"
#include "syslog.h"
#include "sysfs_io.h"

int main(int argc,char **argv)
{
    struct termios opt;

    //串口初始化
    serial_Init(&gfd,&opt);
    //申请报文收发缓冲区
    m_Txd.buf=(BYTE*)malloc(sizeof(BYTE)*FILE_LENTH);
    m_Rxd.buf=(BYTE*)malloc(sizeof(BYTE)*FILE_LENTH);
    //申请文件缓冲区
    fileBuff =(BYTE*)malloc(sizeof(BYTE)*FILE_LEN_MAX);
    //初始化
    memset((void *)m_Txd.buf,0,sizeof(BYTE)*FILE_LENTH);
    memset((void *)m_Rxd.buf,0,sizeof(BYTE)*FILE_LENTH);
    memset((void *)fileBuff,0,sizeof(BYTE)*FILE_LEN_MAX);
    while(1){
        if(recvData(m_Rxd.buf,FILE_LENTH)>0){//接收到报文
            RxdMonitor();//报文处理
        }
    }
    //释放内存
    free(m_Rxd.buf);
    free(m_Txd.buf);
    free(fileBuff);
    //关闭串口
    close(gfd);
}