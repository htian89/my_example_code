#!/bin/sh
cd /opt/vclient
./vClient &
if [ -f /etc/rc.d/rc.delay ];then
    exec /etc/rc.d/rc.delay &
fi
xset s off
./WebsocketClient &
