#include <MMB.hh>
#include <BNL_UDP_Exception.hh>
#include <stdio.h> //snprintf
#include <iostream>



MMB::MMB(std::string const & address, 
	 std::string const & MMBAddressTable, 
	 bool fullStart): started(false)
{
  map = new AddressTable(MMBAddressTable,address,0);
  if(fullStart)
  {
    FullStart();
  }
}

uint32_t MMB::Read(uint16_t address)
{
  return map->Read(address);    
}

uint32_t MMB::Read(std::string const & address)
{
  return map->Read(address);    
}

void MMB::Write(uint16_t address,uint32_t value)
{
  map->Write(address,value);    
}

void MMB::Write(std::string const & address,uint32_t value)
{
  map->Write(address,value);    
}

void MMB::FullStart()
{
  started = true;
}

void MMB::InitializeMMB()
{

}

//void MMB::ResetMMB(bool cntrlRegister=true, bool global=false, bool daq_path=false, bool udp=false){
void MMB::ResetMMB(bool reset_udp)
{
}
