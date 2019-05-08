if [[ $_ == $0 ]]; then  
  echo "$0 is meant to be sourced:"
  echo "  source $0"
  exit 0
fi

CACTUS_ROOT=/opt/cactus
DTS_CONFIG_PATH=$PWD/DTS
DUNETS_ADDRESS_TABLE_PATH=$PWD/DTS/addr_tables

LD_LIBRARY_PATH="${CACTUS_ROOT}/lib/:${LD_LIBRARY_PATH}"

export CACTUS_ROOT LD_LIBRARY_PATH DTS_CONFIG_PATH DUNETS_ADDRESS_TABLE_PATH
