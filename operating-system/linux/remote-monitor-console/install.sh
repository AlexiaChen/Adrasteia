#!/bin/bash


path=`pwd`

chmod +x ./deamon.sh

cd QtTRMSConsole

make

ln -s $path/QtTRMSConsole/qt-console /usr/local/bin/qt-console

cd ../


echo "installation finished"




