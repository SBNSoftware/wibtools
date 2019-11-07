#include <WIB.hh>
#include <WIBException.hh>
#include <BNL_UDP_Exception.hh>
#include <stdio.h> //snprintf
#include <iostream>



WIB::WIB(std::string const & address, 
	 std::string const & WIBAddressTable, 
	 std::string const & FEMBAddressTable, 
	 bool fullStart): 
  WIBBase(address,WIBAddressTable,FEMBAddressTable),
  DAQMode(UNKNOWN),
  FEMBStreamCount(4),
  FEMBCDACount(2),
  ContinueOnFEMBRegReadError(false),
  ContinueOnFEMBSPIError(false),
  ContinueOnFEMBSyncError(true),
  ContinueIfListOfFEMBClockPhasesDontSync(true)
{

  if(fullStart)
  {
    // Turn on write acknowledgments
    int flag= Read("UDP_EN_WR_RDBK");
    if ( flag ) wib->SetWriteAck(true);
    else  wib->SetWriteAck(false);

    Write("UDP_EN_WR_RDBK", 1);
    wib->SetWriteAck(true);
    FullStart();
  }
}

WIB::~WIB(){
}

void WIB::FullStart(){
  //SBND has fixed mode, not readable in firmware (wfb)
  FEMBCount = 4;
  DAQLinkCount = 4;
  //TODO check FEMBStreamCount and FEMCDACount from registers on the WIB
  //TODO create those registers
  started = true;
}

void WIB::configWIB(uint8_t clockSource){
  // check communication
  int fw_version = Read("FW_VERSION");
  int crate = Read("CRATE_ADDR");
  int slot = Read("SLOT_ADDR");
 
  std::cout << "Configure WIB in crate " << std::hex << crate << ", slot " << slot << " with fw version " << fw_version << std::dec << std::endl;

  // setup 
  Write("UDP_FRAME_SIZE",0xEFB); // 0xEFB = jumbo, 0x1FB = regular
  Write("UDP_BURST_MODE",0); // normal UDP operation
  Write(0x10,0x7F00); // FIFO size for "burst mode" (N/A)

  // clock select
  if(clockSource == 0){
    std::cout << "  Configure the Si5344 PLL" << std::endl;

    //check PLL status
    bool lol_flag = false;
    //Write(0xA,0xFF0); // bit 8 resets the Si5344, dunno the others
    Write("SILABS_RST",1);  // resets the SI5344
    Write("I2C_WR_STRB",1); // start SI5344 write
    Write("I2C_RD_STRB",1); // start SI5344 read

    usleep(10000);

    // wait for PLL to lock
    for(int i=0; i<100; i++){
      usleep(10000);

      if(Read("SI5344_LOL")) lol_flag = true;
    }
    if(lol_flag){
      std::cout << "  Si5344 PLL locked!" << std::endl;
      Write("FEMB_CLK_SEL",1);
      Write("FEMB_CMD_SEL",1);
      Write("FEMB_INT_CLK_SEL",0);

      usleep(10000);
    }
    else{
      // gotta do more complicated stuff to load the Si5344
    }

    int clk_sel = Read(0x4) & 0xF;
    std::cout << "  Clock bits (Reg 4) are " << std::hex << clk_sel << std::dec << std::endl;
  }
  else if(clockSource == 1){
    std::cout << "  Using 100MHz from oscillator (bypass Si5344)" << std::endl;

    Write("FEMB_CLK_SEL",0);
    Write("FEMB_CMD_SEL",0);
    Write("FEMB_INT_CLK_SEL",0x2);

    usleep(10000);

    int clk_sel = Read(0x4) & 0xF;
    std::cout << "  Clock bits (Reg 4) are " << std::hex << clk_sel << std::dec << std::endl;
  }
  else{
    WIBException::WIB_FEATURE_NOT_SUPPORTED e;
    e.Append("Unknown clock source! Use 0 for Si5344 and 1 for local.\n");
    throw e;
  }


  // now do WIB_PLL_cfg():
  //   - need to open file wibtools/WIB/config/Si53440-RevD...v2
  //   -   

  

}

void WIB::EnableDAQLink(uint8_t iDAQLink){

  // 0x01, bit 3, "SBND_START_DAQ"
  // 1 = begin sending data from FEMBs->WIB
  // (must be flipped back to 0)
  
  // 0x14, bit 1, "TX_PACK_Stream_EN"
  // 0 = enable data stream from WIB->Nevis


  //Build the base string for this DAQLINK
  std::string base("DAQ_LINK_");
  base.push_back(GetDAQLinkChar(iDAQLink));
  base.append(".CONTROL.");
  printf("%s\n",base.c_str());

  //set the CD stream enable mask from that
  uint32_t enable_mask = 0;
  for(size_t iStream = 0; iStream < (4 * FEMBCount/DAQLinkCount);iStream++){
    enable_mask <<= 0x1;
  }
  
  Write(base+"ENABLE_CDA_STREAM",enable_mask);

  Write(base+"ENABLE",0x1);
}

void WIB::EnableDAQLink_Lite(uint8_t iDAQLink, uint8_t enable){
  
  //Build the base string for this DAQLINK
  std::string base("DAQ_LINK_");
  base.push_back(GetDAQLinkChar(iDAQLink));
  base.append(".CONTROL.");
  
  uint8_t stream = 0;
  if(enable){
    if(DAQMode == RCE) stream = 0xF;
    else stream = 0xFF;   
  }

  Write(base+"ENABLE_CDA_STREAM",stream);
  Write(base+"ENABLE",enable);
}


void WIB::InitializeWIB(){
  //run resets
  Write("SYSTEM.RESET",0xFF);  
  //Set clock settings
  Write("POWER.ENABLE.MASTER_BIAS",0x1);//Turn on DC/DC converter
}

//void WIB::ResetWIB(bool cntrlRegister=true, bool global=false, bool daq_path=false, bool udp=false){
void WIB::ResetWIB(bool reset_udp){

  if(reset_udp){
    //Resetting the UDP will stop the reply packet which will cause an error. 
    try{
      Write("SYSTEM.RESET.UDP_RESET",1);
    }catch(WIBException::BAD_REPLY & e){
      //do nothing
    }
    //Since we don't know this happened since we lack a udp response, do it again.
    //This could be extended to read register REG
    usleep(10000);
    try{
      Write("SYSTEM.RESET.UDP_RESET",1);
    }catch(WIBException::BAD_REPLY & e){
      //do nothing
    }
    usleep(10000);
  }
  

  //Reset the control register
  WriteWithRetry("SYSTEM.RESET.CONTROL_REGISTER_RESET",1);
  usleep(1000);

  //If this is felix, make sure we configure the SI5342
  if(DAQMode == FELIX){
    Write("DAQ.SI5342.RESET",1);
    usleep(10000);
    Write("DAQ.SI5342.RESET",0);
    usleep(10000);
    printf("Configuring SI5342 for FELIX\n");
    LoadConfigDAQ_SI5342("default"); //use the default config file for the SI5342
    usleep(10000);      // Wait for 
    SelectSI5342(1,     // Set input to be local oscillator for FELIX clock
		 1);    // enable the output of the SI5342
  }

  //Reset the Eventbuilder PLL
  Write("SYSTEM.RESET.EB_PLL_RESET",1);
  usleep(10000);
  
  //Reset the DAQ path
  Write("SYSTEM.RESET.DAQ_PATH_RESET",1);  

  usleep(10000);
  
  //Set clock settings
  //Write("DTS.CMD_COUNT_RESET", 0xFFFFFFFF);
  //Write("DTS.CMD_COUNT_RESET", 0);
  
  //Halt signals
  //Write("DTS.CONVERT_CONTROL.HALT", 1);
  //Write("DTS.CONVERT_CONTROL.ENABLE", 0);

  //Make sure DC/DC is on
  //Write("POWER.ENABLE.MASTER_BIAS",1);
}

void WIB::ResetWIBAndCfgDTS(uint8_t localClock, uint8_t PDTS_TGRP, uint8_t PDTSsource, uint32_t PDTSAlignment_timeout){
  if(DAQMode == UNKNOWN){
    WIBException::WIB_DAQMODE_UNKNOWN e;
    throw e;    
  }
 // retain localClock input, get rid of DAQ mode stuff
  if(localClock > 1){
    WIBException::WIB_BAD_ARGS e;
    e.Append("localClock > 1; must be 0 (for DTS) or 1 (for local clock)\n");
    throw e;    
  }
  if(PDTSsource > 1){
    WIBException::WIB_BAD_ARGS e;
    e.Append("PDTSsource > 1; must be 0 (for backplane) or 1 (for front panel)\n");
    throw e;    
  }
  if(16 <= PDTS_TGRP){
    WIBException::WIB_BAD_ARGS e;
    e.Append("PDTS TGRP > 15; must be 0 to 15\n");
    throw e;    
  }

  // get this register so we can leave it in the state it started in
  //uint32_t slow_control_dnd = Read("SYSTEM.SLOW_CONTROL_DND");
 
  ResetWIB();
  //Write("SYSTEM.SLOW_CONTROL_DND",1);
  sleep(1);
 
  for (size_t iFEMB=1; iFEMB<=4; iFEMB++){
    FEMBPower(iFEMB,0);
  }
  
  //make sure everything DTS is off
  Write("DTS.CONVERT_CONTROL.HALT",1);  
  Write("DTS.CONVERT_CONTROL.ENABLE",0);  
  Write("DTS.CONVERT_CONTROL.START_SYNC",0);  
  sleep(1);
 
  if(localClock > 0){
    printf("Configuring local clock\n");
    //Configure the SI5344 to use the local oscillator instead of the PDTS
    LoadConfig_SI5344("default"); 
    sleep(1);
    SelectSI5344(1,1);
    sleep(1);
    Write("DTS.CONVERT_CONTROL.EN_FAKE",1);  
    Write("DTS.CONVERT_CONTROL.LOCAL_TIMESTAMP",1);  
    Write("FEMB_CNC.CNC_CLOCK_SELECT",1);  // <-- change to proper reg name? 0x3? (external = 0)
    //    Write("FEMB_CNC.ENABLE_DTS_CMDS",1);  
    sleep(1);
  }
  else {
    //Configure the clocking for the PDTS (assumes the PDTS is sending idle or something)
    printf("Configuring DTS\n");
    Write("DTS.PDTS_TGRP",PDTS_TGRP);
    printf("Using timing group 0x%X\n",PDTS_TGRP);
    //InitializeDTS(PDTSsource,0,PDTSAlignment_timeout);
    sleep(1);
    Write("FEMB_CNC.CNC_CLOCK_SELECT",1);  
    sleep(1);
    //We are ready for the PDTS, start searching
    Write("DTS.PDTS_ENABLE",1);  
    sleep(1);
  }
 
  //Now we have the 128MHz clock
  Write("FEMB1.DAQ.ENABLE",0);  
  Write("FEMB2.DAQ.ENABLE",0);  
  Write("FEMB3.DAQ.ENABLE",0);  
  Write("FEMB4.DAQ.ENABLE",0);  
 
  //Write("SYSTEM.SLOW_CONTROL_DND",slow_control_dnd);

}

void WIB::CheckedResetWIBAndCfgDTS(uint8_t localClock, uint8_t PDTS_TGRP, uint8_t PDTSsource,  uint32_t PDTSAlignment_timeout){
  if(DAQMode == UNKNOWN){
    WIBException::WIB_DAQMODE_UNKNOWN e;
    throw e;    
  }
  if(localClock > 1){
    WIBException::WIB_BAD_ARGS e;
    e.Append("localClock > 1; must be 0 (for DTS) or 1 (for local clock)\n");
    throw e;    
  }
  if(PDTSsource > 1){
    WIBException::WIB_BAD_ARGS e;
    e.Append("PDTSsource > 1; must be 0 (for backplane) or 1 (for front panel)\n");
    throw e;    
  }
  if(16 <= PDTS_TGRP){
    WIBException::WIB_BAD_ARGS e;
    e.Append("PDTS TGRP > 15; must be 0 to 15\n");
    throw e;    
  }


  bool reset_check = false;
  // Check if we are already in a good state
  if(localClock > 0){
    printf("Checking if locked on local clock\n");
    reset_check = (  (Read("DTS.CONVERT_CONTROL.EN_FAKE") != 1) 
                  || (Read("DTS.CCONVERT_CONTROL.LOCAL_TIMESTAMP") != 1)
                  || (Read("FEMB_CNC.CNC_CLOCK_SELECT") != 1) 
		     //                  || (Read("FEMB_CNC.ENABLE_DTS_CMDS") != 1) 
                  || (Read("DTS.SI5344.INPUT_SELECT") != 1)
                  || (Read("DTS.SI5344.ENABLE") != 1) );                  
    if(!reset_check){
      printf("Already in a good state\n"); 
    }
    else{
      printf("Need to reset for local clocking\n");
    }
  }
  else{
    printf("Checking if locked on PDTS\n");
    reset_check = (  (Read("DTS.PDTS_TGRP") != PDTS_TGRP) 
                  || (Read("FEMB_CNC.CNC_CLOCK_SELECT") != 1)
                  || (Read("DTS.PDTS_ENABLE") != 1)
                  || (Read("DTS.CDS.LOL") != 0)
                  || (Read("DTS.CDS.LOS") != 0)
                  || (ReadWithRetry("DTS.SI5344.INPUT_SELECT") != 0)
                  || (ReadWithRetry("DTS.SI5344.LOS") != 0)
                  || (ReadWithRetry("DTS.SI5344.LOL") != 0)
                  || (ReadWithRetry("DTS.SI5344.ENABLE") != 1)
                  || (ReadWithRetry("DTS.PDTS_STATE") != 0x8) );
    if(!reset_check){
      printf("Already in a good state\n");      
    }
    else{
      printf("Need to reset for PDTS\n");
    }
  }

  //Check the SI5342 if we're attached to FELIX
  if(DAQMode == FELIX){
         
    if( 
        (Read("DAQ.SI5342.ENABLE") == 0)
        || (Read("DAQ.SI5342.INPUT_SELECT") != 1) 
        || (Read("DAQ.SI5342.LOL") == 1) 
        || (Read("DAQ.SI5342.LOS_XAXB") == 1) 
        || (Read("DAQ.SI5342.LOS_2") == 1) ){
      printf("Need to reset for SI5342\n"); 
      reset_check = true;
    }
    else{
      printf("SI5342 in good state\n");
    }
  }

  if(reset_check){
    // get this register so we can leave it in the state it started in
//    uint32_t slow_control_dnd = Read("SYSTEM.SLOW_CONTROL_DND");
  
    ResetWIB();
    //Write("SYSTEM.SLOW_CONTROL_DND",1);
  
    for (size_t iFEMB=1; iFEMB<=4; iFEMB++){
      FEMBPower(iFEMB,0);
    }
    
    //make sure everything DTS is off
    Write("DTS.CONVERT_CONTROL.HALT",1);  
    Write("DTS.CONVERT_CONTROL.ENABLE",0);  
    Write("DTS.CONVERT_CONTROL.START_SYNC",0);  
    sleep(1);
  
    if(localClock > 0){
      printf("Configuring local clock\n");
      //Configure the SI5344 to use the local oscillator instead of the PDTS
      LoadConfig_SI5344("default");
      sleep(1);
      SelectSI5344(1,1);
      sleep(1);
      Write("DTS.CONVERT_CONTROL.EN_FAKE",1);  
      Write("DTS.CONVERT_CONTROL.LOCAL_TIMESTAMP",1);  
      Write("FEMB_CNC.CNC_CLOCK_SELECT",1);  
      //      Write("FEMB_CNC.ENABLE_DTS_CMDS",1);  
      sleep(1);
    }
    else {
      //Configure the clocking for the PDTS (assumes the PDTS is sending idle or something)
      printf("Configuring DTS\n");
      Write("DTS.PDTS_TGRP",PDTS_TGRP);
      printf("Using timing group 0x%X\n",PDTS_TGRP);
      //InitializeDTS(PDTSsource,0,PDTSAlignment_timeout);
      sleep(1);
      Write("FEMB_CNC.CNC_CLOCK_SELECT",1);  
      sleep(1);
      //We are ready for the PDTS, start searching
      Write("DTS.PDTS_ENABLE",1);  
      sleep(1);
    }
  
  
    //Write("SYSTEM.SLOW_CONTROL_DND",slow_control_dnd);
  }
  //Now we have the 128MHz clock
  std::cout << "Resetting DAQ Links" << std::endl;
  size_t nLinks = 4;
  if(DAQMode == FELIX){ nLinks = 2; }
  for (size_t iLink=1; iLink <= nLinks; ++iLink){
    std::cout << iLink << std::endl;
    EnableDAQLink_Lite(iLink, 0);
  }

  Write("FEMB1.DAQ.ENABLE",0);  
  Write("FEMB2.DAQ.ENABLE",0);  
  Write("FEMB3.DAQ.ENABLE",0);  
  Write("FEMB4.DAQ.ENABLE",0);  

}

void WIB::StartStreamToDAQ(){
  /*if(DAQMode == UNKNOWN){
    WIBException::WIB_DAQMODE_UNKNOWN e;
    throw e;    
  }
  WriteWithRetry("DTS.CONVERT_CONTROL.HALT",1);
  WriteWithRetry("DTS.CONVERT_CONTROL.ENABLE",0);
*/

  // get this register so we can leave it in the state it started in
  /*
  uint32_t slow_control_dnd = Read("SYSTEM.SLOW_CONTROL_DND");
  Write("SYSTEM.SLOW_CONTROL_DND",1);

  sleep(1);
  Write("FEMB_CNC.FEMB_STOP",1);  
  sleep(1);
  Write("SYSTEM.RESET.DAQ_PATH_RESET",1);  
  sleep(1);
  */

  // Enable DAQ links
  size_t nLinks = 4;
  //if (DAQMode == FELIX){
  //  nLinks = 2;
  //}
  for (size_t iLink=1; iLink <= nLinks; iLink++){
    EnableDAQLink_Lite(iLink,1); // TX_PAC_STREAM_EN?
  }

  // Enable the FEMB to align to idle and wait for convert
  Write("FEMB1.DAQ.ENABLE",0xF);  
  Write("FEMB2.DAQ.ENABLE",0xF);  
  Write("FEMB3.DAQ.ENABLE",0xF);  
  Write("FEMB4.DAQ.ENABLE",0xF);  

  // Start sending characters from the FEMB
  Write("FEMB_CNC.ENABLE_DTS_CMDS",1);  
  //StartSyncDTS();
  //  Write("FEMB_CNC.TIMESTAMP_RESET",1);  
  //Write("FEMB_CNC.FEMB_START",1);  
  //  Write("SYSTEM.RESET.FEMB_COUNTER_RESET",1);  

  //Write("SYSTEM.SLOW_CONTROL_DND",slow_control_dnd);
}


void WIB::FEMBPower(uint8_t iFEMB,bool turnOn){
  
  std::string reg = "PWR_EN_BRD";
  reg.push_back(GetFEMBChar(iFEMB));
  const Item *g = GetItem(reg);
 
  //std::cout
  //<<"  "<<reg<<" ("<<std::hex<<g->mask<<std::dec<<")\n" 
  //<<"  Before: reg "<<g->address<<" = "<<std::hex<<Read(g->address)<<std::dec<<"\n";
  
  if(turnOn){
    // if turning on, life is easy
    Write(reg, 0xFFFFFFFF);
  }else{
    // if turning off, keep in mind other boards might still be on, 
    // so only turn off the bits unique to *this* board while keeping
    // the common CLK_IN bit on (this enables the 5V supply)
    uint32_t mask     = Read(g->address); // current bit pattern
    uint32_t maskCom  = GetItem("PWR_CLK_IN_1")->mask;
    uint32_t maskBrd  = g->mask;
    maskBrd &= ~(maskCom);
    mask &= ~(maskBrd);
    // if this is the last board we're turning off, switch off the CLK_IN bit.
    // we can tell this is the case if all 16 lower bits are 0.
    if( (mask & 0xFFFF) == 0x0 ) mask &= ~(maskCom);
    Write(g->address,mask);
  }

  // verify that it's been turned on/off
  //std::cout<<"  After : reg "<<g->address<<" = "<<std::hex<<Read(g->address)<<std::dec<<"\n";

}

//void WIB::PowerOnFEMB(uint8_t iFEMB){
//  //Turn on power
//  std::string reg = "POWER.ENABLE.FEMB";
//  reg.push_back(GetFEMBChar(iFEMB));
//
//
//  const int REG_COUNT = 6;
//  uint8_t mon_reg_val = 0x1+REG_COUNT*(iFEMB-1); // starts at 0x1 and there are 6 per FEMB
//  
//  for(uint8_t iReg=0;iReg < REG_COUNT;iReg++){
//    //Go measure the reg value
//    //    int iTries = 0;
//    //    const int NUM_TRIES = 100;
//    //Set the value to be read
//    Write("POWER.MON_CONTROL.ADDRESS",mon_reg_val+iReg);
//    //Wait till the output is valid
////    for(;iTries < NUM_TRIES;iTries++){
////      sleep(1);
////      if(0x1 == Read("POWER.MON_CONTROL.VALID")){
////	break;
////      }
////    }
////    if (NUM_TRIES == iTries){
////      printf("Timeout on reading FEMB: %d power reg %d\n",iFEMB,iReg);
////    }else{
//      uint32_t reg_val = Read("POWER.MON_VALUE.MEASUREMENT");
//      printf("Reg %u 0x%04X 0x%04X\n",iReg,reg_val >> 16,reg_val&0xFFFF);
//      //    }
//  }
//  
//}
//
void WIB::EnableFEMBCNC(){
  //Enable the clock and control stream to the FEMBs
  Write("FEMB_CNC.CNC_CLOCK_SELECT",1);
  Write("FEMB_CNC.CNC_COMMAND_SELECT",1);
}
void WIB::DisableFEMBCNC(){
  //Enable the clock and control stream to the FEMBs
  Write("FEMB_CNC.CNC_CLOCK_SELECT",0);
  Write("FEMB_CNC.CNC_COMMAND_SELECT",0);
}

bool WIB::CheckDAQLinkInRange(uint8_t iDAQLink){
  if(!((iDAQLink > 0) && (iDAQLink <= DAQLinkCount))){
    WIBException::WIB_INDEX_OUT_OF_RANGE e;
    e.Append("DAQ Link\n");
    throw e;    
  } 
  return true;
}

char WIB::GetDAQLinkChar(uint8_t iDAQLink){
  char c = '0';
  //Check if the link is in range given DAQLinkCount (throws)
  CheckDAQLinkInRange(iDAQLink);
  //Convert the numeric daq link number to a char
  switch (iDAQLink){
  case 1:    
    c = '1';
    break;
  case 2:
    c = '2';
    break;
  case 3:
    c = '3';
    break;
  case 4:
    c = '4';
    break;
  default:
    WIBException::WIB_INDEX_OUT_OF_RANGE e;    
    e.Append("DAQ Link\n");
    char estr[] = "0";
    estr[0] = c;
    e.Append(estr);      
    throw e;
  }
  return c;
}

bool WIB::CheckFEMBInRange(uint8_t iFEMB){
  if(!((iFEMB > 0) && (iFEMB <= FEMBCount))){
    WIBException::WIB_INDEX_OUT_OF_RANGE e;
    e.Append("FEMB\n");
    throw e;    
  } 
  return true;
}

char WIB::GetFEMBChar(uint8_t iFEMB){
  char c = '0';
  //Check if the link is in range given FEMBCount (throws)
  CheckFEMBInRange(iFEMB);
  //Convert the numeric daq link number to a char
  switch (iFEMB){
  case 1:    
    c = '1';
    break;
  case 2:
    c = '2';
    break;
  case 3:
    c = '3';
    break;
  case 4:
    c = '4';
    break;
  default:
    WIBException::WIB_INDEX_OUT_OF_RANGE e;
    e.Append("FEMB\n");
    char estr[] = "0";
    estr[0] = c;
    e.Append(estr);      
    throw e;
  }
  return c;
}

bool WIB::CheckFEMBStreamInRange(uint8_t iStream){
  if(!((iStream > 0) && (iStream <= FEMBStreamCount))){
    WIBException::WIB_INDEX_OUT_OF_RANGE e;
    e.Append("FEMB Stream");
    throw e;    
  } 
  return true;
}

bool WIB::CheckFEMBCDInRange(uint8_t iCDA){
  if(!((iCDA > 0) && (iCDA <= FEMBCDACount))){
    WIBException::WIB_INDEX_OUT_OF_RANGE e;
    e.Append("FEMB CDA");
    throw e;    
  } 
  return true;
}

char WIB::GetFEMBCDChar(uint8_t iCD){
  char c = '0';
  //Check if the link is in range given FEMBCount (throws)
  CheckFEMBCDInRange(iCD);
  //Convert the numeric daq link number to a char
  switch (iCD){
  case 1:    
    c = '1';
    break;
  case 2:
    c = '2';
    break;
  default:
    WIBException::WIB_INDEX_OUT_OF_RANGE e;
    e.Append("FEMB CDA\n");
    char estr[] = "0";
    estr[0] = c;
    e.Append(estr);      
    throw e;
  }
  return c;
}

void WIB::SourceFEMB(uint64_t iFEMB, uint64_t real){
  if(iFEMB < 1){
    printf("FEMB index out of range < 1\n");
    return;
  }
  
/*  if(DAQMode == RCE && iFEMB > 4){
    printf("FEMB index out of range > 4 (RCE) \n");
    return;}
  if(DAQMode == FELIX && iFEMB > 2){
    printf("FEMB index out of range > 2 (FELIX) \n");
    return;}
  */
  std::string address("FEMB");
  address.push_back(GetFEMBChar(iFEMB));
  address.append(".DAQ.FAKE_CD.FAKE_SOURCE");
  Write(address,real); 
}
