/*
 * @Description: In User Settings Edit
 * @Author: your name
 * @Date: 2019-09-16 02:10:22
 * @LastEditTime: 2019-09-16 02:39:04
 * @LastEditors: Please set LastEditors
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

#include "sysfs_io.h"
#include "syslog.h"
#include "gtypedef.h"

//导出引脚标号文件
int gpio_export(int pin)
{
    char buf[12];
    int fd;
    if((fd=open(SYSFS_GPIO_EXPORT,O_WRONLY|O_EXCL))==-1){
            logMsg(logErr,"Open export error!");
            return ERROR;
    }
    sprintf(buf,"%d",pin);
    if(write(fd,buf,strlen(buf))<0){
        logMsg(logWarn,"Export%d file already exist!",pin);
    }
    close(fd);
    return OK;  
}

//取消导出引脚
int gpio_unexport(int pin)
{
    char buf[12];
    int fd;
    if((fd=open(SYSFS_GPIO_UNEXPORT,O_WRONLY))==-1){
        logMsg(logErr,"Open unxeport error!");
        return ERROR;
    }
    sprintf(buf,"%d",pin);
    if(write(fd,buf,strlen(buf))<0){
        logMsg(logWarn,"Unexport%d file already exist!",pin);
        return ERROR;
    }
    close(fd);
    return OK;
}
//设置引脚方向 0-->IN,1-->OUT
int gpio_direction(int pin,int dir)
{
    char buf[64];
    int fd;
    sprintf(buf,"/sys/class/gpio/gpio%d/direction",pin);
    if((fd=open(buf,O_WRONLY))==-1){
        logMsg(logErr,"Open direction error.");
        return ERROR;
    }
    if(dir==0){
        sprintf(buf,"in");
    }
    else sprintf(buf,"out");

    if(write(fd,buf,strlen(buf))<0){
        logMsg(logErr,"Write direction error.");
        return ERROR;
    }
    close(fd);
    return OK;
}
//设置引脚高低电平；val 0：低电平，1：高电平
int gpio_write(int pin,int val)
{
    char buf[64];
    int fd;
    int status;
    status=gpio_direction(pin,1);
    if(status==EXIT_FAILURE){
        return EXIT_FAILURE;
    }
    sprintf(buf,"/sys/class/gpio/gpio%d/value",pin);
    if((fd=open(buf,O_WRONLY))==-1){
        logMsg(logErr,"Open val error.");
        return ERROR;
    }

    if(val==0){
       sprintf(buf,"0");
    }
    else sprintf(buf,"1");

    if(write(fd,buf,strlen(buf))<0){
        logMsg(logErr,"Write value error.");
        return ERROR;
    }
    close(fd);
    return OK;
}
//读取引脚电平状态
int gpio_read(int pin)
{
    char buf[64];
    int fd;
    int status;
    status=gpio_direction(pin,0);
    if(status==ERROR){
        return ERROR;
    }
    sprintf(buf,"/sys/class/gpio/gpio%d/value",pin);
    if((fd=open(buf,O_RDONLY))==-1){
        printf("Open gpio%d value error!",pin);
        return ERROR;
    }
    if(read(fd,buf,3)<0){
        logMsg(logErr,"Read value error.\n");
        return ERROR;
    }
    close(fd);
    return(atoi(buf));
}
//wifi复位模块
void wifi_Rst(int pin)
{
    gpio_export(pin);
    gpio_write(pin,0);
    usleep(200000);
    gpio_write(pin,1);
    sleep(2);
}
//wifi模式脚设置
void set_wifiMd(int pin,int val)
{
    gpio_export(pin);
    gpio_write(pin,val);
}