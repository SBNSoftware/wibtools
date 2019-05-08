#!/bin/bash

echo "WIB Debug Log"
echo $(date)
echo $USER"@"$(hostname)

shopt -s expand_aliases 
wd=$(pwd)

## WIB

cd ../
source env.sh
cd $wd
ip=192.168.200.5
echo "################################################"
echo "############ WIB Info for $ip ############"
echo "################################################"
BUTool.exe -w $ip -X status/dump_all_noIP_FELIX.script
