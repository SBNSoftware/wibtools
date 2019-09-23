#!/bin/bash +x
if [[ $_ == $0 ]]; then  
  echo "$0 is meant to be sourced:"
  echo "  source $0"
  exit 0
fi

WIBTOOLS_BASE=$PWD

#Excetpion library
EXCEPTION_BASE_PREFIX=$WIBTOOLS_BASE/exception

#WIB library stff
WIB_PREFIX=$WIBTOOLS_BASE/WIB
WIB_ADDRESS_TABLE_PATH=$WIB_PREFIX/tables
WIB_CONFIG_PATH=$WIB_PREFIX/config

#MBB library stff
MBB_PREFIX=$WIBTOOLS_BASE/MBB
MBB_ADDRESS_TABLE_PATH=$MBB_PREFIX/tables
MBB_CONFIG_PATH=$MBB_PREFIX/config

#tool stuff
TOOL_PREFIX=$WIBTOOLS_BASE/tool
TOOL_EXE_PATH=$TOOL_PREFIX/bin/tool/


PATH="${TOOL_EXE_PATH}:${PATH}"
LD_LIBRARY_PATH="${EXCEPTION_BASE_PREFIX}/lib:${TOOL_PREFIX}/lib/:${WIB_PREFIX}/lib/:${MBB_PREFIX}/lib:${LD_LIBRARY_PATH}"

alias wib=$TOOL_EXE_PATH/WIBTool.exe


export WIB_PREFIX MBB_PREFIX MBB_ADDRESS_TABLE_PATH MBB_CONFIG_PATH LD_LIBRARY_PATH WIB_ADDRESS_TABLE_PATH PATH EXCEPTION_BASE_PREFIX WIB_CONFIG_PATH
