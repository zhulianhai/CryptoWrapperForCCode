#!/bin/bash

CMD=pgrep
PPROC=relayserver


set_coresize()
{
ulimit -c unlimited
}

check_alive()
{
while :
do
	if [ -n "`$CMD $PPROC`" ]
    then  echo "relayserver is ok"
    else
    	echo "relayserver is killed"
		./relayserver
    fi
 #the time of check the process if alive
    sleep 5
 done
}

echo ${1}ing...

#check the argv[]...
if [ -n "$1" ];then
	if [ "$1" == "start" ];then
		if [ -n "`$CMD $PPROC`" ];then
			echo "relayserver already running..."
		else
			./relayserver
		fi
	elif [ "$1" == "stop" ];then
		if [ -n "`$CMD $PPROC`" ];then
			pkill relayserver
		else
			echo "relayserver not running..."
		fi
	elif [ "$1" == "restart" ];then
			pkill relayserver
			./relayserver
	else
		echo "usage...[start] [stop] [restart]"
	fi
else
	echo "Usage...[start] [stop] [restart]"
	return
fi
#set core file size
set_coresize


#start relay-server
#echo  "Start relayserver"
#./relayserver


#trace the process ,if down then restart
#check_alive

