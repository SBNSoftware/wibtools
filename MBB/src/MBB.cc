#include <MBB.hh>
#include <BNL_UDP_Exception.hh>
#include <stdio.h> //snprintf
#include <iostream>
#include <MBBException.hh>
#include <unistd.h>

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

void MBB::WritePTC(uint8_t icrate, uint16_t address, uint32_t value)
{
  //map->WritePTC(icrate,address,value)
}

void MBB::WritePTC(uint8_t icrate, std::string const & address, uint32_t value)
{
  //map->WritePTC(icrate,address,value)
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



// to get the desired mask for PTC_DATA
/*void MBB::MBBPower(uint8_t icrate,bool turnOn)
  {
  std::string reg = "PTC_DATA";
  reg.push_back(MBBChar(icrate));
  std::cout<<"MBBPower: register string "<<reg<<"    turnOn: "<<turnOn<<"\n";
  }
*/
//MBB defaults to send the correct signals to the WIBs on power up.
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

    WritePTC(icrate, "PTC_DATA_ADDRESS", 0x2);
    
    WritePTC(icrate, "PTC_CRATE_ADDRESS", 0x2);
    
    /*getting the bitmask from PTC_DATA
        const Item *g = GetItem(reg);
	std::cout<<"Mask: "<<std::hex<<g->mask<<std::dec<<"\n";
        

          if(turnOn)
	   {
	    WritePTC(icrate, "PTC_DATA", g->mask);           
           }
          
         else
           {
	    WritePTC(icrate,"PTC_DATA",0x0);
	   }
    */
    WritePTC(icrate, "PTC_WR_REG", 0);
    usleep(1000);
    WritePTC(icrate, "PTC_WR_REG", 1);
    usleep(1000);
    WritePTC(icrate, "PTC_WR_REG", 0);
      
  
  }

void MBB::ConfigAllPTCs()

   {
      for(uint8_t icrate=1; icrate <= CRATE_COUNT; icrate++)
      {
	ConfigPTC(icrate); 
      }
   
   }
