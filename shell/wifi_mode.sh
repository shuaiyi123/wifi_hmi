#!/bin/sh

i=904 #低电平进入烧录模式

if [ ! -d "/sys/class/gpio/gpio$i" ];then
    echo $i > /sys/class/gpio/export
    echo out > /sys/class/gpio/gpio$i/direction
fi

echo $1 > /sys/class/gpio/gpio$i/value
