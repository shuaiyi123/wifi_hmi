/*
 * @Description: In User Settings Edit
 * @Author: your name
 * @Date: 2019-09-16 00:17:03
 * @LastEditTime: 2019-11-14 22:01:24
 * @LastEditors: Please set LastEditors
 */
#include <stdlib.h>
#include <stdbool.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <time.h>
#include <sys/select.h>

#include "syslog.h"
#include "wifi_drive.h"

//全局变量,串口文件描述符
int gfd;
/**
 * @description:打开串口 
 * @param {type} 
 * @return: 
 */
int open_serial(char *dev) 
{
	int fd;
/*
    先以非阻塞方式打开一个串口设备文件：
    O_RDWR：可读可写
    O_NOCTTY：不以终端设备方式打开
    O_NDELAY：非阻塞方式读，无数据时直接返回0
*/
	fd = open(dev, O_RDWR|O_NOCTTY|O_NDELAY, 0);
	if ( fd < 1 ) {
		logMsg(logErr,"open %s error!",dev);
		return ERROR;
	}
	return fd;
}
/**
 * @description: 设置串口参数
 * @param {fd：设备文件，databits：数据位，stopbits：停止位，parity：校验位} 
 * @return: 返回true，串口设置成功。
 */
int set_termios(int fd,struct termios *options,int databits,int stopbits,int parity)
{ 
    if ( tcgetattr(fd,options)  !=  0){ 
         logMsg(logErr,"Get serial attibuttion failed!");     
         return ERROR;  
    }
    options->c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
    options->c_iflag &= ~(BRKINT | ICRNL  | ISTRIP | IXON);//串口接收到0x11/0x13不忽略
    options->c_oflag &= ~OPOST;
    options->c_cflag &= ~CSIZE; 
        /* No hardware flow control */
    options->c_cflag &= ~CRTSCTS;
    switch (databits){   /*设置数据位数*/
        case 7:        
            options->c_cflag |= CS7; 
            break;
        case 8:     
            options->c_cflag |= CS8;
            break;   
        default:    
            logMsg(logErr,"\n"); 
            options->c_cflag |= CS8;
            logMsg(logInfo,"Invalid data size,defautly set for 8 bits data size!");
            break;  
    }
    switch (parity) {   
        case 'n':
        case 'N':    
            options->c_cflag &= ~PARENB;   /* Clear parity enable */
            options->c_iflag &= ~INPCK;     /* disnable parity checking */ 
            break;  
        case 'o':   
        case 'O':     
            options->c_cflag |= (PARODD | PARENB); /* 设置为奇效验*/  
            options->c_iflag |= INPCK;             /* enable parity checking */ 
            break;  
        case 'e':  
        case 'E':   
            options->c_cflag |= PARENB;     /* Enable parity */    
            options->c_cflag &= ~PARODD;   /* 转换为偶效验*/     
            options->c_iflag |= INPCK;       /* enable parity checking */
            break; 
        default:   
            options->c_cflag &= ~PARENB;   /* Clear parity enable */
            options->c_iflag &= ~INPCK;     /* disnable parity checking */ 
            logMsg(logInfo,"Invalid parity,set for no parity");
            break;
     }  
    /* 设置停止位*/  
    switch (stopbits){   
        case 1:    
            options->c_cflag &= ~CSTOPB;  
            break;  
        case 2:    
            options->c_cflag |= CSTOPB;  
            break;
        default: 
            options->c_cflag &= ~CSTOPB;
            logMsg(logInfo,"Invalid stopBits, set for one stopBit!");  
            break;  
    } 
    /* Output mode */
    options->c_oflag = 0;
    /* No active terminal mode */
    options->c_lflag = 0;
    /* Overflow data can be received, but not read */
    if (tcflush(fd, TCIFLUSH) < 0){
        logMsg(logErr,"tcflush failed"); 
        return ERROR;
    }
    if (tcsetattr(fd, TCSANOW, options) < 0){
        logMsg(logErr,"SetupSerial failed"); 
        return ERROR;
    }
    return OK;  
}
/**
 * @description: 设置串口波特率
 * @param {type} 
 * @return: 
 */
int set_baudrate(int fd,struct termios *opt,int baudrate)
{
           	/* Input baud rate */
        if (cfsetispeed(opt, baudrate) < 0)
            return ERROR;
        /* Output baud rate */
        if (cfsetospeed(opt, baudrate) < 0)
            return ERROR;
        /* Overflow data can be received, but not read */
        if (tcflush(fd, TCIFLUSH) < 0)
            return ERROR;
        if (tcsetattr(fd, TCSANOW, opt) < 0)
            return ERROR;
        
        return OK;
}
/**
 * @description: 查找对应的波特率
 * @param {type} 
 * @return: 
 */
int find_baudrate(int rate)
{
	int baudr;
    switch(rate)
    {
        case    4800 :baudr  = B4800;
                     break;
		case    9600 : baudr = B9600;
                     break;
		case   19200 : baudr = B19200;
                     break;
		case   38400 : baudr = B38400;
                     break;
		case   57600 : baudr = B57600;
                     break;
		case  115200 : baudr = B115200;
                    break;
		case  230400 : baudr = B230400;
                    break;
        case  460800 : baudr = B460800;
                    break;
        case  500000 : baudr = B500000;
                    break;
        case  576000 : baudr = B576000;
                    break;
		default      : logMsg(logInfo,"invalid baudrate, set baudrate for 115200\n");
					   baudr = B115200;
                   break;
    }
	return baudr;
}
/**
 * @description:写串口数据 
 * @param {type} 
 * @return: 
 */
int serial_write(int *fd, const void *buf, int bufLen,int timeout_sec) 
{
	int 	count=0;
	int 	ret=0;
	fd_set	output;
	struct timeval timeout;

	timeout.tv_sec = timeout_sec;
	timeout.tv_usec = 0;
  
	FD_ZERO(&output);
	FD_SET(*fd, &output);
	
	do{	/* listen */	
		ret = select(*fd + 1, NULL, &output, NULL, &timeout);
		if (ret == -1) { /* error */
			logMsg(logErr,"select() failed!");
			return ERROR;
		} 
        else if (ret) { /* write buffer */
			ret = write(*fd, (BYTE*) buf + count, bufLen);
			if (ret < 1) {	
				logMsg(logErr, "write error %s\n", strerror(errno));
                return ERROR;
			}
			count += ret;
			bufLen -= ret;
		}	
		else { /* timeout */
			logMsg(logErr,"time out.\n");
			return ERROR;
		}
	} while (bufLen > 0);
	return count;
}
/**
 * @description:发送报文 
 * @param {type} 
 * @return: 返回发送数据的长度，返回-1错误
 */
int transData(BYTE *buff, int bufLen)
{
    int write_size;

    write_size = serial_write(&gfd,buff, bufLen,2);

    if (write_size <= 0 || write_size!=bufLen)
    {
        logMsg(logErr,"Transmited data error,data size of transmission=%d\n",write_size);
        return ERROR;
    }
    logMsg(logInfo,"Transmited data size:%d",write_size);
    return write_size;
}
/**
 * @description:读串口不定长数据读,假设两个字节之间最大时间间隔100us,等待1us后下一数据还没到来说明一帧数据读完
 * @param {type} 
 * @return: 
//  */
int serial_read(int *fd, BYTE *data, int bufLen,int timeout_sec)
{
    int cnt=0;
    int read_size = 0;
    struct timeval timeout;
    fd_set rfds;

    timeout.tv_sec=timeout_sec;
    timeout.tv_usec=0;
    FD_ZERO(&rfds);
    FD_SET(*fd, &rfds);

    switch (select(*fd + 1, &rfds, NULL, NULL, &timeout))
    {
        case -1:
            logMsg(logErr,"select()");
            return ERROR; 
        case 0:
            //logMsg(logErr,"timeout and retry");
            return ERROR;
        default:
        
            if (FD_ISSET(*fd, &rfds))//判断fd是否可读。
            { 
                do
                {
                    read_size = read(*fd, data+cnt, bufLen);
                    if(read_size>0){
                        cnt +=read_size;
                        bufLen -=read_size;
                    }
                    usleep(100);//延时100us等待下一个数据到来
                } while (read_size>0);
                
            }
    }
    return cnt;
    
}
/**
 * @description: 接收报文
 * @param {type} 
 * @return: 返回-1，接收错误，，，否则返回接收到的数据长度
 */
int recvData(BYTE *buff,int bufLen)
{
    int read_size;
    
    read_size=serial_read(&gfd,buff,bufLen,2);
    if(read_size >1)//接收到数据
        logMsg(logInfo,"Recviced size=%d",read_size);
    return read_size;
}

/**
 * @description: 串口初始化
 * @param {type} 
 * @return: 
 */
int serial_Init(int *fd,struct termios *opt)
{
    int state;
    /* open serial */
	*fd = open_serial(SERIAL_NAME);
	if(*fd < 1)
		return ERROR;
	
    //串口设置，8位数据位，1位停止位，无校验
	state=set_termios(*fd,opt,8,1,'n');
    if(state==ERROR){
        logMsg(logErr,"Set termios failed\n");
        return ERROR;
    }
    //串口终端波特率设置
    state=set_baudrate(*fd,opt,B230400);
    if(state==ERROR){
        logMsg(logInfo,"Set baudrate faild");
        return ERROR;
    }
    return OK;
}