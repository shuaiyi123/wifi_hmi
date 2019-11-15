#!/bin/sh


i=903 #(低电平复位)

if [ ! -d "/sys/class/gpio/gpio$i" ];then
    echo $i > /sys/class/gpio/export
    echo out > /sys/class/gpio/gpio$i/direction
fi

echo $1 > /sys/class/gpio/gpio$i/value
