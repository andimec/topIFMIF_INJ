#!/bin/bash
#cd /home/pmattei/IFMIF/topIFMIF/emitApp/gui
#cd /home/pmattei/IFMIF/scripts
cd /home/ifmifmgr/topIFMIF_INJ/bin/linux-x86_64

# change 24.05.2017 to allow executing the epics code compiled under CentOS7/glibc2.14 in workstation running CentOS6/glibc2.12 
if [ $HOSTNAME == "injector-wks" ]
   then export LD_LIBRARY_PATH=/opt/glibc-2.14/lib:$LD_LIBRARY_PATH
fi

if [ "`ps -ef | grep xterm | grep -v grep | wc -l`" == 0 ]; then
  /usr/bin/xterm -g 110x60+995+80 -j +ah -hold -fg green -bg black -rightbar \
  -e ./emitSnlEmu_v2 -s &
fi
