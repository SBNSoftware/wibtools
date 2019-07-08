#include <MBB.hh>
#include <BNL_UDP_Exception.hh>
#include <stdio.h> //snprintf
#include <iostream>
#include <MBBException.hh>


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


//MBB defaults to send the correct signals to the WIBs on power up.

void MBB::ConfigPTC(uint8_t icrate)

   {
      if (icrate < 1 || icrate > 4)
       {
	 MBBException::MBB_BAD_ARGS e;
	 std::stringstream expstr;
	 expstr << "ConfigPTC: icrate should be between 1 and 4: "
		<< int(icrate);
	 e.Append(expstr.str().c_str());
	 throw e;
       }
     
   }
 

void MBB::ConfigAllPTCs(uint8_t icrate)

{

  WritePTC(icrate, "PTC_DATA_ADDRESS", 0x2);
    
   for (uint8_t bit_value=1; bit_value < 5; bit_value++)
      {
	WritePTC(icrate, "PTC_CRATE_ADDRESS", bit_value);
	WritePTC(icrate, "PTC_DATA", 0);
	WritePTC(icrate, "PTC_WR_REG", 0);
        sleep(1);
        WritePTC(icrate, "PTC_WR_REG", 1);
        sleep(1);
        WritePTC(icrate, "PTC_WR_REG", 0);
      }

}
