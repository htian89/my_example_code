#! /bin/sh
#no need this file, so exit now
exit 0 
fpath="/mnt/tcp-usbip2.0"
if [ -e $fpath/fusb_main.c ];then
echo $fpath is mount.
else
mount -t cifs -o username=tianh,password=mima //192.168.11.1/Insight4/usbip_all2.0/tcp-usbip2.0 $fpath
fi
echo ""
cp -rfuv $fpath/* /home/tcp-usbip/
echo ""
make
