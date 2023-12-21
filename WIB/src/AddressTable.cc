#include <AddressTable.hh>
#include <fstream>
#include <AddressTableException.hh>
#include <boost/tokenizer.hpp> //tokenizer
#include <stdlib.h>  //strtoul & getenv
#include <boost/regex.hpp> //regex
#include <boost/algorithm/string/case_conv.hpp> //to_upper
#include <set>
#include "trace.h"

AddressTable::AddressTable(std::string const & addressTableName, 
			   std::string const & deviceAddress,
			   uint16_t offset,
			   bool isMBB)
{
  fileLevel = 0;
  io = new BNL_UDP(isMBB);
  io->Setup(deviceAddress,offset);
  io->SetWriteAck(true);
  LoadFile(addressTableName);
}

AddressTable::~AddressTable(){
  TLOG(TLVL_INFO)
    <<"Called AddressTable::~AddressTable() " << TLOG_ENDL;
  if(io!=nullptr) delete io;

  for (auto& [key, vec] : addressItemMap){
     for (auto& item : vec) {
       delete item->sc_conv;
       delete item;
     }
  }
}
