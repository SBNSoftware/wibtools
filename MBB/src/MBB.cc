#include <MBB.hh>
#include <BNL_UDP_Exception.hh>
#include <stdio.h> //snprintf
#include <iostream>
#include <MBBException.hh>
#include <unistd.h>
#include <sstream>
#include <iomanip>
#include <bitset>

#define sleep(x) usleep((useconds_t) x * 1e6)


MBB::MBB(std::string const & address, std::string const & MBBAddressTable, bool fullStart): started(false), ContinueOnMBBRegReadError(false)
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

/*uint32_t MBB::ReadMBB(uint16_t address){
  return mbb->Read(address);
}
uint32_t MBB::ReadMBB(std::string const & address){
  return mbb->Read(address);    
  }*/

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

void MBB::EnableWIBs(uint8_t icrate,uint32_t value)
{
  WritePTC(icrate,2,value);
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

/** \Setup MBB
 *
 *  Sets up MBB
 *  mbb_config: list of options to configure the mbb:
 *          PLL clock type: 0,1 for External clock input and Internal 16MHz clock respectively. 
 *          Pulse source: 0,1 for LEMO input and MBB internal pulse generator respectively.
 *          Pulse Period: 0,1,2 for 0ns, 10ns, 20ns and so on.
 */

void MBB::ConfigMBB(uint32_t PLL_CLOCK_TYPE, uint32_t PULSE_SOURCE, uint32_t PULSE_PERIOD){

     if (PLL_CLOCK_TYPE > 1){
         MBBException::MBB_BAD_ARGS e;
         std::stringstream expstr;
         expstr << "ConfigMBB: PLL_CLOCK_TYPE is allowed to be 0 (external), 1 (internal) but is: " << int(PLL_CLOCK_TYPE);
         e.Append(expstr.str().c_str());
         throw e;
         }

     if (PULSE_SOURCE > 1){
         MBBException::MBB_BAD_ARGS e;
         std::stringstream expstr;
         expstr << "ConfigMBB: PULSE_SOURCE is allowed to be 0 (LEMO), 1 (MBB) but is: " << int(PULSE_SOURCE);
         e.Append(expstr.str().c_str());
         throw e;
         }

     if (PULSE_PERIOD > 10000000){
         MBBException::MBB_BAD_ARGS e;
         std::stringstream expstr;
         expstr << "ConfigMBB: PULSE_PERIOD is allowed to be 0 (0ns), 1 (10ns) and so on.. but is: " << int(PULSE_PERIOD);
         e.Append(expstr.str().c_str());
         throw e;
         }

     std::cout << "PLL_CLOCK_TYPE: " << int(PLL_CLOCK_TYPE) << " and PULSE_SOURCE" << int(PULSE_SOURCE) << std::endl;

     if(Read("FIRMWARE_VERSION") == Read("SYS_RESET")) { // can't read register if equal
       if(ContinueOnMBBRegReadError){
           std::cout << "Error: Can't read registers from MBB"<< std::endl;
           return;
           }
        MBBException::MBB_REG_READ_ERROR e;
        std::stringstream expstr;
        expstr << " Register Read Error" << std::endl; 
	e.Append(expstr.str().c_str());
        throw e;
        }

     std::cout << "FW VERSION " << std::hex << Read("FIRMWARE_VERSION") << std::dec << std::endl;

     Write("REG_RESET", 1);
     sleep(1);
    
 }

void MBB::SetContinueOnMBBRegReadError(bool enable){
  ContinueOnMBBRegReadError = enable;
}
