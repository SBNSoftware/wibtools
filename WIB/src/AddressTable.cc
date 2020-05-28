#include <AddressTable.hh>
#include <fstream>
#include <AddressTableException.hh>
#include <boost/tokenizer.hpp> //tokenizer
#include <stdlib.h>  //strtoul & getenv
#include <boost/regex.hpp> //regex
#include <boost/algorithm/string/case_conv.hpp> //to_upper


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
