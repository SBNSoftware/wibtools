#include <MBB.hh>
#include <BNL_UDP_Exception.hh>
#include <stdio.h> //snprintf
#include <iostream>



MBB::MBB(std::string const & address, 
	 std::string const & MBBAddressTable, 
	 bool fullStart): started(false)
{
  map = new AddressTable(MBBAddressTable,address,0);
  map->SetWriteAck(false); // Write ack will be awailable in future firmware versions
  if(fullStart)
  {
    FullStart();
  }
}

uint32_t MBB::Read(uint16_t address)
{
  return map->Read(address);    
}

uint32_t MBB::Read(std::string const & address)
{
  return map->Read(address);    
}

void MBB::Write(uint16_t address,uint32_t value)
{
  map->Write(address,value);    
}

void MBB::Write(std::string const & address,uint32_t value)
{
  map->Write(address,value);    
}

void MBB::FullStart()
{
  started = true;
}

void MBB::InitializeMBB()
{

}

//void MBB::ResetMBB(bool cntrlRegister=true, bool global=false, bool daq_path=false, bool udp=false){
void MBB::ResetMBB(bool reset_udp)
{
}
