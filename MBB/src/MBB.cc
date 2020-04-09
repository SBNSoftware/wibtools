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
     Write("UDP_EN_WR_RDBK", 1);//set to enable write echo mode.
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

void MBB::EnableWIBs(uint8_t icrate,uint32_t value)
{
  WritePTC(icrate,0x02,value);
}

//MBB defaults to send the correct signals to the WIBs on power up.

void MBB::WritePTC(uint8_t icrate, uint16_t address, uint32_t value){
  Write("PTC_DATA_ADDRESS", address);
  Write("PTC_CRATE_ADDRESS", icrate); //set by PTC dipswitch. Allows system to control each PTC individually.
  Write("PTC_DATA", value);//If set to 0 will turn ON all WIBs. By default we turn ON all the WIBs on a particular PTC. 
  Write("PTC_WR_REG", 0);
  usleep(1000);
  Write("PTC_WR_REG", 1);
  usleep(1000);
  Write("PTC_WR_REG", 0);
  }

void MBB::ConfigPTC(uint8_t icrate){
  if(icrate < 0 || icrate > CRATE_COUNT){
     MBBException::MBB_BAD_ARGS e;
     std::stringstream expstr;
     expstr << "ConfigPTC: icrate should be between 1 and 4, but is: "
	    <<  int(icrate);
     e.Append(expstr.str().c_str());
     throw e;
     }

  WritePTC(icrate, 0x02, 0);
}

void MBB::ConfigAllPTCs(){
  for(uint8_t icrate=1; icrate <= CRATE_COUNT; icrate++){
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
 *          WIB Power: 0,1 for OFF, ON respectively.
 */

void MBB::ConfigMBB(uint32_t PULSE_SOURCE, uint32_t PULSE_PERIOD, uint32_t wib_pwr1, uint32_t wib_pwr2, uint32_t wib_pwr3, uint32_t wib_pwr4, uint32_t wib_pwr5, uint32_t wib_pwr6){

     if (PULSE_SOURCE > 1){
         MBBException::MBB_BAD_ARGS e;
         std::stringstream expstr;
         expstr << "ConfigMBB: PULSE_SOURCE is allowed to be 0 (LEMO), 1 (MBB) but is: " << int(PULSE_SOURCE);
         e.Append(expstr.str().c_str());
         throw e;
         }

     // 4294967295 the decimal corresponding to 0xFFFFFFFF.
     if (PULSE_PERIOD > 4294967295){
         MBBException::MBB_BAD_ARGS e;
         std::stringstream expstr;
         expstr << "ConfigMBB: PULSE_PERIOD is allowed to be 0 (0ns), 1 (10ns) and so on.. but is: " << int(PULSE_PERIOD);
         e.Append(expstr.str().c_str());
         throw e;
         }

     if (wib_pwr1 > 1){
         MBBException::MBB_BAD_ARGS e;
         std::stringstream expstr;
         expstr << "ConfigMBB: wib_pwr1 is allowed to be 0 (OFF), 1 (ON) but is: " << int(wib_pwr1);
         e.Append(expstr.str().c_str());
         throw e;
         }

     if (wib_pwr2 > 1){
         MBBException::MBB_BAD_ARGS e;
         std::stringstream expstr;
         expstr << "ConfigMBB: wib_pwr2 is allowed to be 0 (OFF), 1 (ON) but is: " << int(wib_pwr2);
         e.Append(expstr.str().c_str());
         throw e;
         }

     if (wib_pwr3 > 1){
         MBBException::MBB_BAD_ARGS e;
         std::stringstream expstr;
         expstr << "ConfigMBB: wib_pwr3 is allowed to be 0 (OFF), 1 (ON) but is: " << int(wib_pwr3);
         e.Append(expstr.str().c_str());
         throw e;
         }
  
     if (wib_pwr4 > 1){
         MBBException::MBB_BAD_ARGS e;
         std::stringstream expstr;
         expstr << "ConfigMBB: wib_pwr4 is allowed to be 0 (OFF), 1 (ON) but is: " << int(wib_pwr4);
         e.Append(expstr.str().c_str());
         throw e;
         }

     if (wib_pwr5 > 1){
         MBBException::MBB_BAD_ARGS e;
         std::stringstream expstr;
         expstr << "ConfigMBB: wib_pwr5 is allowed to be 0 (OFF), 1 (ON) but is: " << int(wib_pwr5);
         e.Append(expstr.str().c_str());
         throw e;
         }

     if (wib_pwr6 > 1){
         MBBException::MBB_BAD_ARGS e;
         std::stringstream expstr;
         expstr << "ConfigMBB: wib_pwr6 is allowed to be 0 (OFF), 1 (ON) but is: " << int(wib_pwr6);
         e.Append(expstr.str().c_str());
         throw e;
         }

     /* Jack sid not to play around with toggle between External/Internal Clock.
     if(PLL_CLOCK_TYPE==0){
        if(Read("PLL_ACTIVE_CLK")==1){
           Write("PLL_CLOCK_SELECT",0);
           Write("PLL_CLOCK_SELECT",1);
           }
        else if(Read("PLL_ACTIVE_CLK")==0){
	        Write("PLL_CLOCK_SELECT",0);
	        }
        }

     if(PLL_CLOCK_TYPE==1){
        if(Read("PLL_ACTIVE_CLK")==0){
	  Write("PLL_CLOCK_SELECT",0);
	  Write("PLL_CLOCK_SELECT",1);
          }
        else if(Read("PLL_ACTIVE_CLK")==1){
                Write("PLL_CLOCK_SELECT",1);
	        }
		}*/ 


     Write("PULSE_SRC_SELECT", PULSE_SOURCE);
     Write("PULSE_PERIOD", PULSE_PERIOD);

     // controlling power on WIBs.
     mask = (wib_pwr6<<5)|(wib_pwr5<<4)|(wib_pwr4<<3)|(wib_pwr3<<2)|(wib_pwr2<<1)|(wib_pwr1);
     mask = (~mask) & 0x3F;
     std::cout<<" *************mask = "<<mask<<std::endl;
     Write("PTC_DATA", mask);
     Write("PTC_CRATE_ADDRESS", 0); // Temporary for testing, must fix
     Write("PTC_DATA_ADDRESS", 0x2); // Only one register awailable
     
     //Transition from 0 to 1 in PTC_WR_REG will send data to the PTC.
     Write("PTC_WR_REG", 0);
     usleep(1000);
     Write("PTC_WR_REG", 1);
     usleep(1000);
     Write("PTC_WR_REG", 0);       

  //storing the WIB Power values corresponding to each PTC.
     crate_num = Read("PTC_CRATE_ADDRESS");
     if(crate_num == 0x1){p1 = Read("PTC_DATA");}
     if(crate_num == 0x2){p2 = Read("PTC_DATA");}
     if(crate_num == 0x3){p3 = Read("PTC_DATA");}
     if(crate_num == 0x4){p4 = Read("PTC_DATA");}
     
	/*
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
     */

     //Write("REG_RESET", 1);
     
     sleep(1);
}


void MBB::SetContinueOnMBBRegReadError(bool enable){
  ContinueOnMBBRegReadError = enable;
}


void MBB::TimeStampReset(){
  Write("TIMESTAMP_RESET", 1);
  sleep(1);
  Write("TIMESTAMP_RESET", 0);
  }
