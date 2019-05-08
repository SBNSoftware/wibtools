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

nameBase=np04-wib-00
for wibNum in 1 2 3 4 5; do
  name=$nameBase$wibNum
  echo "################################################"
  echo "############ WIB Info for $name #########"
  echo "################################################"
  BUTool.exe -w $name -X status/dump_all_noIP.script
done
