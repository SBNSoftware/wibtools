#include <MBB.hh>
#include <BNL_UDP_Exception.hh>
#include <stdio.h>
#include <iostream>
#include <MBBException.hh>
#include <unistd.h>
#include <sstream>
#include <iomanip>
#include <bitset>

#define sleep(x) usleep((useconds_t) x * 1e6)

MBB::MBB(std::string const & address, std::string const & MBBAddressTable, bool fullStart): started(false), ContinueOnMBBRegReadError(false)
{
  mbb = new AddressTable(MBBAddressTable,address,0,true);

  //Turn on write acknowledgments
  mbb->SetWriteAck(false);
  Write("UDP_EN_WR_RDBK", 1);//set to enable write echo mode.
  mbb->SetWriteAck(true);                                            
  started = true; 
}

std::string MBB::GetAddress(){
  return mbb->GetRemoteAddress();
}

uint32_t MBB::Read(uint16_t address){
  return mbb->Read(address);    
}

uint32_t MBB::Read(std::string const & address){
  return mbb->Read(address);    
}

void MBB::Write(uint16_t address,uint32_t value){
  mbb->Write(address,value);    
}

void MBB::Write(std::string const & address,uint32_t value){
  mbb->Write(address,value);    
}

void MBB::FullStart(){
  started = true;
}

void MBB::InitializeMBB(){

}

//void MBB::ResetMBB(bool cntrlRegister=true, bool global=false, bool daq_path=false, bool udp=false){
void MBB::ResetMBB(bool reset_udp){
}

void MBB::EnableWIBs(uint8_t icrate,uint32_t value){
  WritePTC(icrate,0x02,value);
}


// ----------------------------------------------------------------------------------------------------------
// ----------------------------------------------------------------------------------------------------------
// ----------------------------------------Configuring the PTC-----------------------------------------------
// ----------------------------------------------------------------------------------------------------------                                               
// ----------------------------------------------------------------------------------------------------------

void MBB::WritePTC(uint8_t icrate, uint16_t address, uint32_t value){
  Write("PTC_DATA_ADDRESS", address); //set to 0x02 to control power on WIBs.
  Write("PTC_CRATE_ADDRESS", icrate); //set by PTC dipswitch. Allows system to control each PTC individually.
  Write("PTC_DATA", value); //If set to 0 will turn ON all WIBs. 
  Write("PTC_WR_REG", 0); // This toggle sends the data to PTC.
  usleep(1000);
  Write("PTC_WR_REG", 1);
  usleep(1000);
  Write("PTC_WR_REG", 0);
  }

void MBB::ConfigPTC(uint8_t icrate, uint32_t wib_pwr0, uint32_t wib_pwr1, uint32_t wib_pwr2, uint32_t wib_pwr3, uint32_t wib_pwr4, uint32_t wib_pwr5){
  
  if(icrate !=14){
     if(icrate < 0 || icrate > CRATE_COUNT){
        MBBException::MBB_BAD_ARGS e;
        std::stringstream expstr;
        expstr << "ConfigPTC: icrate should be between 0 and 4, or 14 but is: "
	    <<  int(icrate);
        e.Append(expstr.str().c_str());
        throw e;
        }
     }

  if(wib_pwr0 > 1){
     MBBException::MBB_BAD_ARGS e;
     std::stringstream expstr;
     expstr << "ConfigPTC: wib_pwr0 is allowed to be 0 (OFF), 1 (ON) but is: " << int(wib_pwr0);
     e.Append(expstr.str().c_str());
     throw e;
     }

  if(wib_pwr1 > 1){
     MBBException::MBB_BAD_ARGS e;
     std::stringstream expstr;
     expstr << "ConfigPTC: wib_pwr1 is allowed to be 0 (OFF), 1 (ON) but is: " << int(wib_pwr1);
     e.Append(expstr.str().c_str());
     throw e;
     }

  if(wib_pwr2 > 1){
     MBBException::MBB_BAD_ARGS e;
     std::stringstream expstr;
     expstr << "ConfigPTC: wib_pwr2 is allowed to be 0 (OFF), 1 (ON) but is: " << int(wib_pwr2);
     e.Append(expstr.str().c_str());
     throw e;
     }

  if(wib_pwr3 > 1){
     MBBException::MBB_BAD_ARGS e;
     std::stringstream expstr;
     expstr << "ConfigPTC: wib_pwr3 is allowed to be 0 (OFF), 1 (ON) but is: " << int(wib_pwr3);
     e.Append(expstr.str().c_str());
     throw e;
     }

  if(wib_pwr4 > 1){
     MBBException::MBB_BAD_ARGS e;
     std::stringstream expstr;
     expstr << "ConfigPTC: wib_pwr4 is allowed to be 0 (OFF), 1 (ON) but is: " << int(wib_pwr4);
     e.Append(expstr.str().c_str());
     throw e;
     }

  if(wib_pwr5 > 1){
     MBBException::MBB_BAD_ARGS e;
     std::stringstream expstr;
     expstr << "ConfigPTC: wib_pwr5 is allowed to be 0 (OFF), 1 (ON) but is: " << int(wib_pwr5);
     e.Append(expstr.str().c_str());
     throw e;
     }

  // mask to control Power on WIBs.                                                                                                                         
  mask = (wib_pwr5<<5)|(wib_pwr4<<4)|(wib_pwr3<<3)|(wib_pwr2<<2)|(wib_pwr1<<1)|(wib_pwr0);
  mask = (~mask) & 0x3F;
  std::cout<<" *************mask = "<<mask<<std::endl;

  WritePTC(icrate, 0x02, mask);
  
  wibpoweroncrate[icrate] = mask; //storing the mask to be printed out in status table.
}

// ----------------------------------------------------------------------------------------------------------                                               
// ----------------------------------------------------------------------------------------------------------                                               
// ----------------------------Configuring the MBB with pulse source and pulse period------------------------                                               
// ----------------------------------------------------------------------------------------------------------                                               
// ----------------------------------------------------------------------------------------------------------

void MBB::ConfigMBB(uint32_t PULSE_SOURCE, uint32_t PULSE_PERIOD){

 if(PULSE_SOURCE > 1){
    MBBException::MBB_BAD_ARGS e;
    std::stringstream expstr;
    expstr << "ConfigMBB: PULSE_SOURCE is allowed to be 0 (LEMO), 1 (MBB) but is: " << int(PULSE_SOURCE);
    e.Append(expstr.str().c_str());
    throw e;
    }

 if(PULSE_PERIOD > 4294967295){// 4294967295 the decimal corresponding to 0xFFFFFFFF.
    MBBException::MBB_BAD_ARGS e;
    std::stringstream expstr;
    expstr << "ConfigMBB: PULSE_PERIOD is allowed to be 0 (0ns), 1 (10ns) and so on.. but is: " << int(PULSE_PERIOD);
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
