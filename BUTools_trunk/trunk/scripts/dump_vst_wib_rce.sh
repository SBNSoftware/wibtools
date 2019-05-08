#!/bin/bash

echo "WIB Debug Log"
echo $(date)
echo $USER"@"$(hostname)

shopt -s expand_aliases 
wd=$(pwd)
echo "CWD: `pwd`"

## WIB

cd ../
source env.sh
cd $wd
ip=192.168.200.2
echo "################################################"
echo "############ WIB Info for $ip ############"
echo "################################################"
BUTool.exe -w $ip -X status/dump_all_noIP.script

## RCE

echo "#######################################"
echo "############ RCE Info #################"
echo "#######################################"

source /nfs/sw/rce/setup.sh dev

read -r -d '' remotercewibcheck << 'EOM'
shopt -s expand_aliases 
cd $wd
pwd
source /nfs/sw/rce/setup.sh dev
rce_wib_check
EOM

read -r -d '' remotecommandsbase << 'EOM'
shopt -s expand_aliases 
cd CURRENTDIR
pwd
source /nfs/sw/rce/setup.sh dev
rce_wib_debug
EOM

remotecommandsbase=${remotecommandsbase/CURRENTDIR/$wd}

ssh np04-srv-011 'bash -s' <<< "$remotercewibcheck"

# coldbox cob 1 dpm 0-3, cob 2 dpm 0
# VST cob 2 dpm 3
cobs="2"
dpms="3"
for cob in $cobs; do
  for dpm in $dpms; do
    echo "##############################################"
    echo "############ RCE RAW $rcefn ############"
    echo "##############################################"
    remotecommands="$remotecommandsbase"" --cob $cob --dpm $dpm --oneshot"
    ssh np04-srv-011 'bash -s' <<< "$remotecommands"
    sleep 5;
    remotecommands="$remotecommandsbase"" --cob $cob --dpm $dpm"
    ssh np04-srv-011 'bash -s' <<< "$remotecommands"
    for rce in 0 2; do
      rcefn="CERN-"$cob$dpm$rce".dat"
      #echo "##########################################"
      #echo "############ RCE $rcefn ############"
      #echo "##########################################"
      #hexdump $rcefn
      echo "####################################################"
      echo "############ RCE Formatted $rcefn ############"
      echo "####################################################"
      PdReaderTest $rcefn
    done
  done
done

echo $(date)
echo "done."
