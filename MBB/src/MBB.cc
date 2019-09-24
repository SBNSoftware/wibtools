#include <MBB.hh>
#include <BNL_UDP_Exception.hh>
#include <stdio.h> //snprintf
#include <iostream>
#include <MBBException.hh>
#include <unistd.h>


MBB::MBB(std::string const & address, std::string const & MBBAddressTable, bool fullStart): started(false)
{
  mbb = new AddressTable(MBBAddressTable,address,0);
  if(fullStart){
     //Turn on write acknowledgments
     mbb->SetWriteAck(false);
     Write("UDP_EN_WR_RDBK", 1);
     mbb->SetWriteAck(true);                                            
     started = true; 
     }
}

std::string MBB::GetAddress(){
  return mbb->GetRemoteAddress();
}

uint32_t MBB::Read(uint16_t address)
{
  return mbb->Read(address);    
}

uint32_t MBB::Read(std::string const & address)
{
  return mbb->Read(address);    
}

void MBB::Write(uint16_t address,uint32_t value)
{
  mbb->Write(address,value);    
}

void MBB::Write(std::string const & address,uint32_t value)
{
  mbb->Write(address,value);    
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
void MBB::WritePTC(uint8_t icrate, uint16_t address, uint32_t value)
{
  Write("PTC_CRATE_ADDRESS", icrate);
  Write("PTC_DATA_ADDRESS", address);
  Write("PTC_DATA", value);
  Write("PTC_WR_REG", 0);
  usleep(1000);
  Write("PTC_WR_REG", 1);
  usleep(1000);
  Write("PTC_WR_REG", 0);
}

void MBB::ConfigPTC(uint8_t icrate)
  {
    if (icrate < 1 || icrate > CRATE_COUNT)
       {
	MBBException::MBB_BAD_ARGS e;
	std::stringstream expstr;
	expstr << "ConfigPTC: icrate should be between 1 and CRATE_COUNT: "
	       << int(icrate);
	e.Append(expstr.str().c_str());
	throw e;
       }
    // to get the bitmask for PTC_DATA (more subtelities can be added to throw errors! analogous to FEMBPower!)
    std::string reg = "PTC_DATA";
    const Item *g = GetItem(reg);
    std::cout<<"Mask: "<<std::hex<<g->mask<<std::dec<<"\n";
    // include the turnOn case like FEMBPower?
    // the bitmask is passed as an integer argument in WritePTC()
    WritePTC(icrate, 0x2, g->mask);
   
  }

void MBB::ConfigAllPTCs()
   {
      for(uint8_t icrate=1; icrate <= CRATE_COUNT; icrate++)
      {
	ConfigPTC(icrate); 
      }
   }
