/*
 * @Description: In User Settings Edit
 * @Author: your name
 * @Date: 2019-09-15 23:00:23
 * @LastEditTime: 2019-11-14 22:01:25
 * @LastEditors: Please set LastEditors
 */
#ifndef _WIFI_DRIVE__H
#define _WIFI_DRIVE__H

#include <termios.h>
#include "gtypedef.h"

#ifdef __cplusplus
extern "C"
{
#endif

#define SERIAL_NAME "/dev/ttyUL2"

extern int gfd;
int open_serial(char *dev); 
int set_termios(int fd,struct termios *options,int databits,int stopbits,int parity);
int set_baudrate(int fd,struct termios *opt,int baudrate);
int find_baudrate(int rate);
int serial_write(int *fd, const void *buf, int bufLen,int timeout_sec);
int serial_read(int *fd, BYTE *data, int bufLen,int timeout_sec);
int transData(BYTE *buff, int bufLen);
int recvData(BYTE *buff,int bufLen);
int serial_Init(int *fd,struct termios *opt);

#ifdef __cplusplus
}
#endif

#endif 
