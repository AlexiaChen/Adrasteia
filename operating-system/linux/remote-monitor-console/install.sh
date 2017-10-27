#!/bin/bash


path=`pwd`

chmod +x ./deamon.sh

cd QtTRMSConsole

make

pkill -f "qt-console"
rm -f /usr/local/bin/qt-console
ln -s $path/QtTRMSConsole/qt-console /usr/local/bin/qt-console

cd ../


echo "installation finished"




