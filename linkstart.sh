#!/bin/sh
#make sure a process is always running.

export DISPLAY=:0 #needed if you are running a simple gui app.

process=ServerComm.py
makerun="python /root/ServerComm.py"

if ps | grep -v grep | grep $process > /dev/null
then
	echo $process "already running"
    exit
else
    $makerun &
fi
        
exit
