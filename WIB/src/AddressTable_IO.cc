#include <AddressTable.hh>
#include <fstream>
#include <AddressTableException.hh>
#include <boost/tokenizer.hpp> //tokenizer
#include <stdlib.h>  //strtoul & getenv
#include <boost/regex.hpp> //regex
#include <boost/algorithm/string/case_conv.hpp> //to_upper
#include <iostream>
#include "trace.h"

uint32_t AddressTable::Read(uint16_t address)
{
  uint32_t value;
  value = io->Read(address);
  return(value);
}

uint32_t AddressTable::ReadWithRetry(uint16_t address)
{
  uint32_t value = io->ReadWithRetry(address);
  return(value);
}


void AddressTable::Write(uint16_t address, uint32_t data)
{
  io->Write(address,data);
}

void AddressTable::WriteWithRetry(uint16_t address, uint32_t data)
{
  io->WriteWithRetry(address,data);
}

void AddressTable::Write(uint16_t address, 
			 std::vector<uint32_t> const & values)
{
  io->Write(address,values);
}

void AddressTable::Write(uint16_t address,
			 uint32_t const * values, size_t word_count)
{
  io->Write(address,values,word_count);
}


uint32_t AddressTable::Read(std::string registerName)
{
  std::map<std::string,Item *>::iterator itNameItem
    = nameItemMap.find(registerName);
  if(itNameItem == nameItemMap.end())
  {
    WIBException::INVALID_NAME e;
    e.Append("Can't find item with name \"");
    e.Append(registerName.c_str());
    e.Append("\"");
    throw e;    
  }

  Item * item = itNameItem->second;
  uint32_t value;
  value = io->ReadWithRetry(item->address);
  value &= (item->mask);
  value >>= item->offset;

  return value;
}

uint32_t AddressTable::ReadWithRetry(std::string registerName)
{
  std::map<std::string,Item *>::iterator itNameItem 
    = nameItemMap.find(registerName);

  if(itNameItem == nameItemMap.end())
  {
    WIBException::INVALID_NAME e;
    e.Append("Can't find item with name \"");
    e.Append(registerName.c_str());
    e.Append("\"");
    throw e;    
  }

  Item * item = itNameItem->second;
  uint32_t value;
  value = io->ReadWithRetry(item->address);
  value &= (item->mask);
  value >>= item->offset;

  return value;
}

void AddressTable::Write(std::string registerName,uint32_t val)
{
  std::map<std::string,Item *>::iterator itNameItem 
    = nameItemMap.find(registerName);

  if(itNameItem == nameItemMap.end())
  {
    WIBException::INVALID_NAME e;
    e.Append("Can't find item with name \"");
    e.Append(registerName.c_str());
    e.Append("\"");
    throw e;    
  }

  Item * item = itNameItem->second;
  //Check if this entry controls all the bits 
  uint32_t buildingVal =0;
  if(item->mask != 0xFFFFFFFF)
  {
    // Since there are bits this register we don't control, we need to 
    //  see what they currently are
    buildingVal = io->ReadWithRetry(item->address);
    buildingVal &= ~(item->mask);    
  }
  buildingVal |= (item->mask & (val << item->offset));
  io->Write(item->address,buildingVal);
}

void AddressTable::WriteWithRetry(std::string registerName,uint32_t val)
{
  const std::string identification = "AddressTable::WriteWithRetry";
  TLOG_INFO(identification)<<"AddressTable::WriteWithRetry: "<<registerName<<"  "<<val<<TLOG_ENDL;
  std::map<std::string,Item *>::iterator itNameItem 
    = nameItemMap.find(registerName);

  if(itNameItem == nameItemMap.end())
  {
    WIBException::INVALID_NAME e;
    e.Append("Can't find item with name \"");
    e.Append(registerName.c_str());
    e.Append("\"");
    throw e;    
  }

  Item * item = itNameItem->second;
  //Check if this entry controls all the bits 
  uint32_t buildingVal =0;
  if(item->mask != 0xFFFFFFFF)
  {
    // Since there are bits this register we don't control, we need to 
    //  see what they currently are
    buildingVal = io->ReadWithRetry(item->address);
    buildingVal &= ~(item->mask);    
  }
  buildingVal |= (item->mask & (val << item->offset));
  TLOG_INFO(identification)<<"io->WriteWithRetry: "<<item->address
			   <<" "<<buildingVal<<TLOG_ENDL;
  io->WriteWithRetry(item->address,buildingVal);
}


void AddressTable::Write(std::string registerName,
			 std::vector<uint32_t> const & values)
{
  Write(registerName,values.data(),values.size());
}
void AddressTable::Write(std::string registerName,
			 uint32_t const * values, size_t word_count)
{
  std::map<std::string,Item *>::iterator itNameItem 
    = nameItemMap.find(registerName);

  if(itNameItem == nameItemMap.end())
  {
    WIBException::INVALID_NAME e;
    e.Append("Can't find item with name \"");
    e.Append(registerName.c_str());
    e.Append("\"");
    throw e;    
  }

  Item * item = itNameItem->second;
  //Check if this entry controls all the bits 
  if(item->mask != 0xFFFFFFFF)
  {
    WIBException::BAD_BLOCK_WRITE e;
    e.Append("Mask is not 0xFFFFFFFF\n");
    throw e;
  }
  io->Write(item->address,values,word_count);
}

