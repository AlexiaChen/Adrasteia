#!/bin/bash

#change ip port below,to connect server
   
deamon(){
 result=`pgrep qt-console`
 if [[ -z $result ]]; then
    echo "qt-console not exists"
    nohup "/usr/local/bin/qt-console" "192.168.1.106" "9559" &
 else
    echo "qt-console exists"
	sleep 60
	pkill -f "qt-console"
 fi
}

while true
do
     deamon;
     sleep 5   
done

