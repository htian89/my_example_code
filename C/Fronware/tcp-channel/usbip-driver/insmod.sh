#! /bin/sh
#fpath="/mnt/usbip-driver2.0"
#if [ -e $fpath/stub_main.c ];then
#echo $fpath is mount.
#else
#mount -t cifs -o username=tianh,password=mima //192.168.11.1/Insight4/usbip_all2.0/usbip-driver2.0 $fpath
#fi
#echo ""
#cp -rfuv $fpath/* /home/usbip-driver/
#echo ""
#make
TEST=`lsmod | grep usbip`
if [ "x$TEST" != "x" ];then
rmmod usbip.ko
rmmod usbip-core.ko
fi
insmod usbip-core.ko
insmod usbip.ko

