#!/bin/bash
if [[ $_ == $0 ]]; then  
  echo "$0 is meant to be sourced:"
  echo "  source $0"
  exit 0
fi

#Excetpion library
EXCEPTION_BASE_PREFIX=$PWD/exception

#WIB library stff
WIB_PREFIX=$PWD/WIB
WIB_ADDRESS_TABLE_PATH=$WIB_PREFIX/tables
WIB_CONFIG_PATH=$WIB_PREFIX/config

#tool stuff
TOOL_PREFIX=$PWD/tool
TOOL_EXE_PATH=$TOOL_PREFIX/bin/tool/


PATH="${TOOL_EXE_PATH}:${PATH}"
LD_LIBRARY_PATH="${EXCEPTION_BASE_PREFIX}/lib:${TOOL_PREFIX}/lib/:${WIB_PREFIX}/lib/:${LD_LIBRARY_PATH}"



export WIB_PREFIX LD_LIBRARY_PATH WIB_ADDRESS_TABLE_PATH PATH EXCEPTION_BASE_PREFIX WIB_CONFIG_PATH
