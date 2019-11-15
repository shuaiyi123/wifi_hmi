/*
 * @Description: In User Settings Edit
 * @Author: your name
 * @Date: 2019-09-16 02:09:38
 * @LastEditTime: 2019-09-16 02:39:10
 * @LastEditors: Please set LastEditors
 */
#ifndef _SYSFS_IO__H
#define _SYSFS_IO__H

#define SYSFS_GPIO_EXPORT       "/sys/class/gpio/export"
#define SYSFS_GPIO_UNEXPORT     "/sys/class/gpio/unexport" 
#define SYSFS_GPIO_DIR          "/sys/class/gpio/gpio899/direction"
#define SYSFS_GPIO_DIR_IN       "in"
#define SYSFS_GPIO_DIR_OUT      "out"  
#define SYSFS_GPIO_VAL          "/sys/class/gpio/gpio899/value"
#define SYSFS_GPIO_VAL_H        "1"
#define SYSFS_GPIO_VAL_L        "0"

int gpio_export(int pin);
int gpio_unexport(int pin);
int gpio_direction(int pin,int dir);
int gpio_write(int pin,int val);
int gpio_read(int pin);
void wifi_Rst(int pin);
void set_wifiMd(int pin,int val);
#endif 