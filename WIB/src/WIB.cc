#include <WIB.hh>
#define TRACE_NAME "WIB"
#include <WIBException.hh>
#include <BNL_UDP_Exception.hh>
#include <stdio.h> //snprintf
#include <iostream>
#include <fstream>
#include <bitset>
#include "trace.h"

#define WIB_CONFIG_PATH "WIB_CONFIG_PATH" 
#define SI5344_CONFIG_FILENAME "Si5344-RevD-SBND_V2_100MHz_REVD_2.txt"

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

  wib->SetWriteAck(false);
  // Select new style reply port numbering scheme and write read backs
  // This must be a direct Write, otherwise Write issues a Read first
  Write(0x1E,0x3);

  wib->SetWriteAck(true);
  FullStart();
  const std::string identification = "WIB:WIB";
  TLOG(TLVL_INFO) << "Constructor for WIB class finished"; 
}

WIB::~WIB(){
  TLOG(TLVL_INFO) << "Destructor for WIB class finished"; 
}

void WIB::FullStart()
{
  //SBND has fixed mode, not readable in firmware (wfb)
  FEMBCount = 4;
  DAQLinkCount = 4;
  //TODO check FEMBStreamCount and FEMCDACount from registers on the WIB
  //TODO create those registers
  started = true;
}

void WIB::configWIB(uint8_t clockSource){
  
  int fw_version = Read("FW_VERSION");
  int crate = Read("CRATE_ADDR");
  int slot = Read("SLOT_ADDR");
  const std::string identification = "WIB::configWIB";
  //TLOG_INFO(identification) << "***** FW VERSION : " << fw_version << "  " << std::bitset<32>(fw_version).to_string() << " *******" << TLOG_ENDL;
  //TLOG_INFO(identification) << "***** FW VERSION 1: " << fw_version << " FW VERSION 2: " << int(Read(0xFF)&0xFFFF) << TLOG_ENDL;
  //TLOG_INFO(identification) << "***** CRATE ADDR. 1: " << crate << " CRATE ADDR. 2: " << int((Read(0xFF)&0xFF000000)>>24) << TLOG_ENDL;
  //TLOG_INFO(identification) << "***** SLOT ADDR. 1: " << slot << " SLOT ADDR. 2: " << int((Read(0xFF)&0xFF0000)>>16) << TLOG_ENDL;
  TLOG_INFO(identification) << "Configure WIB in crate " << std::hex << crate << " slot " << std::hex << slot << " with fw version " << std::hex << fw_version << " clockSource " << int(clockSource) << TLOG_ENDL; 

  // setup
  UDP_enable(true); 
  TLOG_INFO(identification) << "UDP ENABLE VALUE (After enabling) : " << int(Read("UDP_DISABLE")) << TLOG_ENDL;
  Write("UDP_FRAME_SIZE",0xEFB); // 0xEFB = jumbo, 0x1FB = regular
  if (CheckWIB_FEMB_REGs) CheckWIBRegisters(0xEFB, "UDP_FRAME_SIZE", 30);
  TLOG_INFO(identification) << "WRITE VALUE (UDP_FRAME_SIZE) : EFB" << TLOG_ENDL;
  TLOG_INFO(identification) << "READ (UDP_FRAME_SIZE) : " << std::hex << int(Read("UDP_FRAME_SIZE")) << TLOG_ENDL;
  //Write(0x1F,Read(0x1F)|0xEFB);
  //TLOG_INFO(identification) << "****** UDP FRAME SIZE 1 : " << int(Read("UDP_FRAME_SIZE")) << " UDP FRAME SIZE 2 : " << int(Read(0x1F)&0xFFF) << TLOG_ENDL;
  Write("UDP_SAMP_TO_SAVE",0x7F00);
  if (CheckWIB_FEMB_REGs) CheckWIBRegisters(0x7F00, "UDP_SAMP_TO_SAVE", 30);
  TLOG_INFO(identification) << "WRITE VALUE (UDP_SAMP_TO_SAVE) : 0x7F00" << TLOG_ENDL;
  TLOG_INFO(identification) << "READ (UDP_SAMP_TO_SAVE) : " << std::hex << int(Read("UDP_SAMP_TO_SAVE")) << TLOG_ENDL;
  //Write(0x10,Read(0x10)|0x7F00); // Wrong
  //Write(0x10,(Read(0x10)&0xFFFF0000)|0x7F00); // Right
  //TLOG_INFO(identification) << " UDP SAMPLE TO SAVE VALUE : " << int(Read("UDP_SAMP_TO_SAVE")) << TLOG_ENDL;
  Write("UDP_BURST_MODE",0); // normal UDP operation
  if (CheckWIB_FEMB_REGs) CheckWIBRegisters(0, "UDP_BURST_MODE", 30);
  TLOG_INFO(identification) << "WRITE VALUE (UDP_BURST_MODE) : 0" << TLOG_ENDL;
  TLOG_INFO(identification) << "READ (UDP_BURST_MODE) : " << std::hex << int(Read("UDP_BURST_MODE")) << TLOG_ENDL;
  //Write(0x0F,(Read(0x0F)&0xFFFFFFF0)|0x3);
  //TLOG_INFO(identification) << " UDP_BURST_MODE : " << int(Read("UDP_BURST_MODE")) << TLOG_ENDL;
  UDP_enable(false); 
  TLOG_INFO(identification) << "UDP ENABLE VALUE (After disabling) : " << int(Read("UDP_DISABLE")) << TLOG_ENDL;
  // clock select
  if(clockSource == 0){
    TLOG_INFO(identification)  << "--> configuring Si5344 PLL...";

    ResetSi5344();
    
    //UDP_enable(true);// copied from BNL CE ce_runs.py (Varuna)
    //Write(0x0A,0xFF0); // copied from BNL CE ce_runs.py (Varuna)
    //Write(0x0A,0xFF0); // copied from BNL CE ce_runs.py (Varuna)
    //usleep(10000); copied from BNL CE ce_runs.py (Varuna)
    
    // check PLL status; if PLL is not already locked, 
    // then load from the configuration from a file
    bool lol_flag = PLL_check();
    if( !lol_flag ){
      loadConfig(SI5344_CONFIG_FILENAME);
      lol_flag = PLL_check();
    }

    if( lol_flag ){
      TLOG_INFO(identification)  << "** Si5344 PLL locked!! :) **";
      Write("FEMB_CLK_SEL",1);
      if (CheckWIB_FEMB_REGs) CheckWIBRegisters(1, "FEMB_CLK_SEL", 30);
      Write("FEMB_CMD_SEL",1);
      if (CheckWIB_FEMB_REGs) CheckWIBRegisters(1, "FEMB_CMD_SEL", 30);
      Write("FEMB_INT_CLK_SEL",0);
      if (CheckWIB_FEMB_REGs) CheckWIBRegisters(0, "FEMB_INT_CLK_SEL", 30);
      usleep(10000);
    } else {
      TLOG_INFO(identification) <<"PLL failed to lock.";
    }

    //int clk_sel = Read(0x4) & 0xF;
    //std::cout << "  Clock bits (reg 4) are " << std::hex << clk_sel << std::dec << std::endl;
  }
  else if(clockSource == 1){
    TLOG_INFO(identification)  << "--> using 100MHz from oscillator (bypass Si5344)";
    Write("FEMB_CLK_SEL",0);
    if (CheckWIB_FEMB_REGs) CheckWIBRegisters(0, "FEMB_CLK_SEL", 30);
    Write("FEMB_CMD_SEL",0);
    if (CheckWIB_FEMB_REGs) CheckWIBRegisters(0, "FEMB_CMD_SEL", 30);
    Write("FEMB_INT_CLK_SEL",0x2);
    if (CheckWIB_FEMB_REGs) CheckWIBRegisters(0x2, "FEMB_INT_CLK_SEL", 30);
    usleep(10000);
    //int clk_sel = Read(0x4) & 0xF;
    //std::cout << "  Clock bits (reg 4) are " << std::hex << clk_sel << std::dec << std::endl;
  } else {
    WIBException::WIB_FEATURE_NOT_SUPPORTED e;
    e.Append("Unknown clock source! Use 0 for Si5344 and 1 for local.\n");
    throw e;
  }

  UDP_enable(false);
  TLOG_INFO(identification) << "configureing WIB is finished" << TLOG_ENDL;
}

bool WIB::PLL_check(int iTries){
  const std::string identification = "WIB::PLL_check"; 
  bool out = false;
  for(int i=0; i<iTries; i++){
    printf("Checking PLL status... (attempt %d/%d)\n",i+1,iTries);
    usleep(500000);
    if(Read("SI5344_LOL")) {
      out = true;
      break;
    }
  }
  return out;
}

void WIB::UDP_enable(bool enable){
  if( enable == true ){  
      Write("UDP_DISABLE",0);
      if (CheckWIB_FEMB_REGs) CheckWIBRegisters(0, "UDP_DISABLE", 30);
  }
  else {
      Write("UDP_DISABLE",1);
      if (CheckWIB_FEMB_REGs) CheckWIBRegisters(1, "UDP_DISABLE", 30);
  }
}

void WIB::ResetSi5344(){
  const std::string identification = "WIB::ResetSi5344"; 
  TLOG_INFO(identification)<<"Resetting Si5344\n";
  Write(10,0x0);
  if(CheckWIB_FEMB_REGs) CheckWIBRegisters(0x0, 10, 30);
  Write("SILABS_RST",1);
  if(CheckWIB_FEMB_REGs) CheckWIBRegisters(1, "SILABS_RST", 30);   
  Write("SILABS_RST",0); 
  if(CheckWIB_FEMB_REGs) CheckWIBRegisters(0, "SILABS_RST", 30);  
  usleep(100000);
}

void WIB::loadConfig(std::string const & fileName){
   const std::string identification = "WIB::loadConfig";   
  // read in configuration file
  std::string fullPath = getenv(WIB_CONFIG_PATH);
  fullPath += "/";
  fullPath += fileName;
  std::ifstream confFile(fullPath.c_str());
  WIBException::WIB_BAD_ARGS badFile;
  
  TLOG_INFO(identification)<<"Configuring PLL from file: "<<fileName<<TLOG_ENDL;
  if(confFile.fail()){
    //Failed to topen filename, add it to the exception
    badFile.Append("Bad SI5344 config file name:");
    badFile.Append(fullPath.c_str());
    //Try the default
    if(getenv(WIB_CONFIG_PATH) != NULL){      
      std::string fileDefault = getenv(WIB_CONFIG_PATH);
      fileDefault += "/";
      fileDefault += SI5344_CONFIG_FILENAME;
      confFile.open(fileDefault.c_str());
      if(confFile.fail()){
	badFile.Append("Bad env based filename:");
	badFile.Append(fileDefault.c_str());
      }
    }
  }
  if(confFile.fail()){ 
    throw badFile; 
  }

  // create vectors to hold the data in the config file 
  // (copying convention from the python scripts)
  std::vector<uint16_t> adrs_h;
  std::vector<uint16_t> adrs_l;
  std::vector<uint16_t> data_v;
  while(!confFile.eof()){
    std::string line;
    std::getline(confFile,line);
    uint16_t tmp = line.find(',');
    if( line.size() == 0 ) {
      continue;
    } else if( line[0] == '#' || line[0] == 'A'){
      continue;
    } else{
      if( tmp == std::string::npos ){
	printf("Skipping bad line: \"%s\"\n",line.c_str());
	continue;
      }
      uint16_t  adr   = strtoul(line.substr(2,tmp).c_str(),NULL,16);
      uint16_t data   = strtoul(line.substr(tmp+1).c_str(),NULL,16);
      adrs_h.push_back((adr&0xFF00)>>8);
      adrs_l.push_back(adr&0xFF);
      data_v.push_back(data&0xFF);
      //int i = adrs_h.size()-1;
      //std::cout<<"adrs_h = "<<std::hex<<adrs_h.at(i)<<"   adrs_l = "<<adrs_l.at(i)<<"    datass = "<<data_v.at(i)<<std::dec<<"\n";
     
      }
    }

    size_t numWrites = adrs_h.size();

    if( numWrites > 0 ) {
      
      // Adapting procedure from Shanshan's script, ce_runs.py
      UDP_enable(true);
      uint16_t p_addr = 1;
      uint16_t page4 = adrs_h[0];
      PLL_write(p_addr,page4); // set page
      for(size_t i=0; i<adrs_h.size(); i++){
        if( page4 == adrs_h[i] ){
          PLL_write(adrs_l[i],data_v[i]);
        } else {
          page4 = adrs_h[i];
          PLL_write(p_addr,page4); // new page
          PLL_write(adrs_l[i],data_v[i]);
        }
      }
      TLOG_INFO(identification)<<"Finished."<<TLOG_ENDL;
      sleep(1);
    }
 
}

void WIB::PLL_write(uint16_t addr, uint16_t data){  
   const std::string identification = "WIB::PLL_write";
  // Register 11 controls SI5344 I2C
  //  0:3   (0x00000F)  -- Number of bytes to write (set to 1 when writing data, 0 when setting address)
  //  8:15  (0x00FF00)  -- Address to write to
  //  16:23 (0xFF0000)  -- Data being written 
     
  //std::cout<<std::hex<<"Addr: "<<addr<<"   data: "<<data<<"\n";
  size_t value = 0x01 + ((addr&0xFF)<<8) + ((data&0x00FF)<<16);
  Write(0x0B,value);
  if (CheckWIB_FEMB_REGs) CheckWIBRegisters(value, 0x0B, 30);
  usleep(1000);
  Write("I2C_WR_STRB",1); 
  if (CheckWIB_FEMB_REGs) CheckWIBRegisters(1, "I2C_WR_STRB", 30);
  usleep(1000);
  Write("I2C_WR_STRB",0);
  if (CheckWIB_FEMB_REGs) CheckWIBRegisters(0, "I2C_WR_STRB", 30);
  usleep(1000);
}


void WIB::EnableDAQLink(uint8_t iDAQLink){
  const std::string identification = "WIB::EnableDAQLink";
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
  const std::string identification = "WIB::EnableDAQLink_Lite";
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
  const std::string identification = "WIB::InitializeWIB";
  //run resets
  Write("SYSTEM.RESET",0xFF);  
  //Set clock settings
  Write("POWER.ENABLE.MASTER_BIAS",0x1);//Turn on DC/DC converter
}

//void WIB::ResetWIB(bool cntrlRegister=true, bool global=false, bool daq_path=false, bool udp=false){
void WIB::ResetWIB(bool reset_udp){
  const std::string identification = "WIB::ResetWIB";
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
  const std::string identification = "WIB::ResetWIBAndCfgDTS";
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
  const std::string identification = "WIB::CheckedResetWIBAndCfgDTS";
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
  TLOG_INFO(identification)<<"Finished."<<TLOG_ENDL; 
  TLOG_INFO(identification)<< "Resetting DAQ Links" << TLOG_ENDL;
  size_t nLinks = 4;
  if(DAQMode == FELIX){ nLinks = 2; }
  for (size_t iLink=1; iLink <= nLinks; ++iLink){
    TLOG_INFO(identification) << iLink << TLOG_ENDL;
    EnableDAQLink_Lite(iLink, 0);
  }

  Write("FEMB1.DAQ.ENABLE",0);  
  Write("FEMB2.DAQ.ENABLE",0);  
  Write("FEMB3.DAQ.ENABLE",0);  
  Write("FEMB4.DAQ.ENABLE",0);  

}

void WIB::StartStreamToDAQ(){
  const std::string identification = "WIB::StartStreamToDAQ";
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
  const std::string identification = "WIB::FEMBPower";
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

void WIB::EnableFEMBCNC(){
  const std::string identification = "WIB::EnableFEMBCNC"; 
  //Enable the clock and control stream to the FEMBs
  Write("FEMB_CNC.CNC_CLOCK_SELECT",1);
  Write("FEMB_CNC.CNC_COMMAND_SELECT",1);
}
void WIB::DisableFEMBCNC(){
  const std::string identification = "WIB::DisableFEMBCNC";
  //Enable the clock and control stream to the FEMBs
  Write("FEMB_CNC.CNC_CLOCK_SELECT",0);
  Write("FEMB_CNC.CNC_COMMAND_SELECT",0);
}

bool WIB::CheckDAQLinkInRange(uint8_t iDAQLink){
  const std::string identification = "WIB::CheckDAQLinkInRange";
  if(!((iDAQLink > 0) && (iDAQLink <= DAQLinkCount))){
    WIBException::WIB_INDEX_OUT_OF_RANGE e;
    e.Append("DAQ Link\n");
    throw e;    
  } 
  return true;
}

char WIB::GetDAQLinkChar(uint8_t iDAQLink){
  const std::string identification = "WIB::GetDAQLinkChar";
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
  const std::string identification = "WIB::CheckFEMBInRange";
  if(!((iFEMB > 0) && (iFEMB <= FEMBCount))){
    WIBException::WIB_INDEX_OUT_OF_RANGE e;
    e.Append("FEMB\n");
    throw e;    
  } 
  return true;
}

char WIB::GetFEMBChar(uint8_t iFEMB){
  const std::string identification = "WIB::GetFEMBChar";
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
  const std::string identification = "WIB::CheckFEMBStreamInRange";
  if(!((iStream > 0) && (iStream <= FEMBStreamCount))){
    WIBException::WIB_INDEX_OUT_OF_RANGE e;
    e.Append("FEMB Stream");
    throw e;    
  } 
  return true;
}

bool WIB::CheckFEMBCDInRange(uint8_t iCDA){
  const std::string identification = "WIB::CheckFEMBCDInRange";
  if(!((iCDA > 0) && (iCDA <= FEMBCDACount))){
    WIBException::WIB_INDEX_OUT_OF_RANGE e;
    e.Append("FEMB CDA");
    throw e;    
  } 
  return true;
}

char WIB::GetFEMBCDChar(uint8_t iCD){
  const std::string identification = "WIB::GetFEMBCDChar";
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
  const std::string identification = "WIB::SourceFEMB";
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

void WIB::PrintInstructions(){
  std::cerr << "The operation was unsuccessful. Please try the following steps to \
resolve the issue. Terminate any running instances of the FEMBreceiver (femb), \
WIBTool.exe, or WIBBoardReader processes. Then, delete the semaphores /dev/shm/sem.WIB_LCK \
and /dev/shm/sem.WIB_YLD. If you intend to run both FEMBreceiver and WIBTool.exe or \
WIBBoardReader, start with FEMBreceiver first.\n";
}

void WIB::CheckWIBRegisters(uint32_t expected_val, std::string reg_addrs, int tries){
  const std::string identification = "WIB::CheckWIBRegisters";
  bool throw_excpt = true;
  uint32_t reg_value = -9999;
  for (int i = 0; i < tries; i++){
       sleep(0.1);
       reg_value = Read(reg_addrs);
       if (reg_value == expected_val){
           throw_excpt = false;
	   break;
       }
       Write(reg_addrs, expected_val);
  }
  if(throw_excpt){
     WIBException::FEMB_REG_READ_ERROR e;
     std::stringstream expstr;
     expstr << "WIB" << " register " << reg_addrs << " value is " << reg_value << " and expected value is " << expected_val;
     e.Append(expstr.str().c_str());
     throw e;
  }
}

void WIB::CheckWIBRegisters(uint32_t expected_val, uint32_t reg_addrs, int tries){
  const std::string identification = "WIB::CheckWIBRegisters";
  bool throw_excpt = true;
  uint32_t reg_value = -9999;
  for (int i = 0; i < tries; i++){
       sleep(0.1);
       reg_value = Read(reg_addrs);
       if (reg_value == expected_val){
           throw_excpt = false;
	   break;
       }
       Write(reg_addrs, expected_val);
  }
  if(throw_excpt){
     WIBException::FEMB_REG_READ_ERROR e;
     std::stringstream expstr;
     expstr << "WIB" << " register " << reg_addrs << " value is " << reg_value << " and expected value is " << expected_val;
     e.Append(expstr.str().c_str());
     throw e;
  }
}

void WIB::WIB_UDP_CTL(bool WIB_UDP_EN){
    // This function is copied from Shanshan's python script
    // to configure WIB/FEMB.
    // The original function is in cls_config.py module inside the repository CE_LD with same name (git branch name is, Installation_Support)
    const std::string identification = "WIB::WIB_UDP_CTL";
    auto wib_reg_7_value = Read(7);
    if (WIB_UDP_EN) wib_reg_7_value = wib_reg_7_value & 0x00000000; // bit31 of reg7 for disable wib udp control
    else wib_reg_7_value = wib_reg_7_value | 0x80000000; // bit31 of reg7 for disable wib udp control 
    Write(7, wib_reg_7_value);
    CheckWIBRegisters(wib_reg_7_value, 7, 30);
}

void WIB::FEMB_ASIC_CS(int femb_addr, int asic){
   // This function is copied from Shanshan's python script
   // to configure WIB/FEMB.
   // The original function is in cls_config.py module inside the repository CE_LD with same name (git branch name is, Installation_Support)
   const std::string identification = "WIB::FEMB_ASIC_CS";
   auto femb_asic = asic & 0x0F;
   auto wib_asic =  ( ((femb_addr << 16)&0x000F0000) + ((femb_asic << 8) &0xFF00) );
   Write(7, wib_asic | 0x80000000);
   CheckWIBRegisters(wib_asic | 0x80000000, 7, 30);
   Write(7, wib_asic);
   CheckWIBRegisters(wib_asic, 7, 30);
}

void WIB::FEMB_UDPACQ(int femb_addr){
     // This function is a modified version of one of the funcitons available in Shanshan's python script
     // to configure WIB/FEMB.
     // The original function is in cls_config.py module inside the repository CE_LD with same name (git branch name is, Installation_Support)
     const std::string identification = "WIB::FEMB_UDPACQ";
     WIB_UDP_CTL(true);
     for (int i=0; i<8; i++) FEMB_ASIC_CS(femb_addr, i);
     WIB_UDP_CTL(false);
}

void WIB::FEMB_UDPACQ_V2(int femb_addr){
    // This function is a modified version of one of the funcitons available in Shanshan's python script
    // to configure WIB/FEMB.
    // The original function is in cls_config.py module inside the repository CE_LD with name FEMB_UDPACQ(git branch name is, Installation_Support)
    const std::string identification = "WIB::FEMB_UDPACQ_V2";
    Write(0x01,0x2); // Time Stamp Reset command encoded in 2MHz
    CheckWIBRegisters(0x2,0x01,30); 
    Write(0x01,0x0);
    CheckWIBRegisters(0x0,0x01,30);
    Write(18,0x8000);
    CheckWIBRegisters(0x8000,18,30); // reset error counters
    WIB_UDP_CTL(true); // Enable HS data from the WIB to PC through UDP
    for (int i=0; i<8; i++) FEMB_ASIC_CS(femb_addr, i);
    WIB_UDP_CTL(false); // disable HS data from this WIB to PC through UDP
}

std::map<std::string,double> WIB::WIB_STATUS(){
     // This function is copied from Shanshan's python script
     // to configure WIB/FEMB.
     // The original function is in cls_config.py module inside the repository CE_LD with same name (git branch name is, Installation_Support)
     const std::string identification = "WIB::WIB_STATUS";
     Write(0x12, 0x8000);
     CheckWIBRegisters(0x8000, 0x12, 30);
     Write(0x12, 0x100);
     CheckWIBRegisters(0x100, 0x12, 30);
     sleep(0.02);
     
     uint32_t link_status = -1; // earlier int link_status = 
     uint32_t eq_status = -1; // earlier int eq_status = 
     
     for (int i=0; i<5; i++){
        link_status = Read(0x21);
	eq_status = Read(0x24);
	sleep(0.001); 
     }
     
     std::map<std::string,double> map;
     
     map.insert( std::pair<std::string,double>("FEMB1_LINK",link_status&0xFF) );
     map.insert( std::pair<std::string,double>("FEMB1_EQ",eq_status&0x0F) );
     map.insert( std::pair<std::string,double>("FEMB2_LINK",(link_status&0xFF00)>>8) );
     map.insert( std::pair<std::string,double>("FEMB2_EQ",(eq_status&0xF0)>>4) );
     map.insert( std::pair<std::string,double>("FEMB3_LINK",(link_status&0xFF0000)>>16) );
     map.insert( std::pair<std::string,double>("FEMB3_EQ",(eq_status&0xF00)>>8) );
     map.insert( std::pair<std::string,double>("FEMB4_LINK",(link_status&0xFF000000)>>24) );
     map.insert( std::pair<std::string,double>("FEMB4_EQ",(eq_status&0xF000)>>12) );
     
     Write(0x12, 0x000);
     CheckWIBRegisters(0x000, 0x12, 30);
     
     for (int i=0; i<4; i++){
        for (int j=0; j<4; j++){
	     Write(0x12, (i<<2) + j);
	     CheckWIBRegisters((i<<2) + j, 0x12, 30);
	     auto reg34 = Read(0x22);
	     auto femb_ts_cnt = (reg34&0xFFFF0000)>>16;
	     auto chkerr_cnt = (reg34&0xFFFF);
	     auto reg35 = Read(0x25);
	     auto frameerr_cnt =(reg35&0xFFFF);
	     map.insert( std::pair<std::string,double>("FEMB"+std::to_string(i+1)+"_TS_LINK"+std::to_string(j+1),femb_ts_cnt) );
	     map.insert( std::pair<std::string,double>("FEMB"+std::to_string(i+1)+"_CHK_ERR_LINK"+std::to_string(j+1),chkerr_cnt) );
	     map.insert( std::pair<std::string,double>("FEMB"+std::to_string(i+1)+"_FRAME_ERR_LINK"+std::to_string(j+1),frameerr_cnt) );
	}
     }
     
     std::vector<int> vcts;
     for (int j=0; j<3; j++){
        for (int k=0; k<5; k++){
	   Write(5, 0x00000);
	   CheckWIBRegisters(0x00000, 5, 30);
	   Write(5, 0x00000 | 0x10000);
	   CheckWIBRegisters(0x00000 | 0x10000, 5, 30);
	   sleep(0.01);
	   Write(5, 0x00000);
	   CheckWIBRegisters(0x00000, 5, 30);
	}
 
	for (int i=0; i<35; i++){
	   Write(5, i);
	   CheckWIBRegisters(i, 5, 30);
	   sleep(0.001);
	   auto tmp = Read(6) & 0xFFFFFFFF;
	   if ( (tmp&0x40000000)>>16 == 0x4000 ) tmp = tmp & 0x3fff;
	   if ((tmp&0x3f00) == 0x3f00) tmp = tmp & 0x3fff0000;
	   if (j == 2) vcts.push_back(tmp);
	   sleep(0.001);
	} 
     }
     
     auto wib_vcc   = (((vcts[0x19]&0x0FFFF0000) >> 16) & 0x3FFF) * 305.18 * 0.000001  + 2.5;
     auto wib_t     = (((vcts[0x19]&0x0FFFF) & 0x3FFF) * 62.5) * 0.001; 
     auto wib_vbias = (((vcts[0x1A]&0x0FFFF0000) >> 16) & 0x3FFF) * 305.18 * 0.000001;
     auto wib_ibias = ((vcts[0x1A]& 0x3FFF) * 19.075) * 0.000001 / 0.1;
     auto wib_v18   = (((vcts[0x1B]&0x0FFFF0000) >> 16) & 0x3FFF) * 305.18 * 0.000001;
     auto wib_i18   = ((vcts[0x1B]& 0x3FFF) * 19.075) * 0.000001 / 0.01;
     auto wib_v36   = (((vcts[0x1C]&0x0FFFF0000) >> 16) & 0x3FFF) * 305.18 * 0.000001;
     auto wib_i36   = ((vcts[0x1C]& 0x3FFF) * 19.075) * 0.000001 / 0.01;
     auto wib_v28   = (((vcts[0x1D]&0x0FFFF0000) >> 16) & 0x3FFF) * 305.18 * 0.000001;
     auto wib_i28   = ((vcts[0x1D]& 0x3FFF) * 19.075) * 0.000001 / 0.01;
     
     map.insert( std::pair<std::string,double>("WIB_2991_VCC",wib_vcc) );
     map.insert( std::pair<std::string,double>("WIB_2991_T",wib_t) );
     map.insert( std::pair<std::string,double>("WIB_BIAS_V",wib_vbias) ); 
     map.insert( std::pair<std::string,double>("WIB_BIAS_I",wib_ibias) );
     map.insert( std::pair<std::string,double>("WIB_V18_V",wib_v18) );
     map.insert( std::pair<std::string,double>("WIB_V18_I",wib_i18) );
     map.insert( std::pair<std::string,double>("WIB_V36_V",wib_v36) );
     map.insert( std::pair<std::string,double>("WIB_V36_I",wib_i36) ); 
     map.insert( std::pair<std::string,double>("WIB_V28_V",wib_v28) );  
     map.insert( std::pair<std::string,double>("WIB_V28_I",wib_i28) );  
     
     auto bias_vcc  = (((vcts[0x00]&0x0FFFF0000) >> 16) & 0x3FFF) * 305.18 * 0.000001  + 2.5;
     auto bias_t    = (((vcts[0x00]&0x0FFFF) & 0x3FFF) * 62.5) * 0.001;
     
     map.insert( std::pair<std::string,double>("BIAS_2991_V",bias_vcc) ); 
     map.insert( std::pair<std::string,double>("BIAS_2991_T",bias_t) ); 
     map.insert( std::pair<std::string,double>("WIB_PC",(map.find("WIB_BIAS_V")->second * map.find("WIB_BIAS_I")->second) + (map.find("WIB_V18_V")->second * map.find("WIB_V18_I")->second) + (map.find("WIB_V36_V")->second * map.find("WIB_V36_I")->second) + (map.find("WIB_V28_V")->second * map.find("WIB_V28_I")->second)) );
     
     for (int i=0; i<4; i++){
         std::vector<int> vct;
	 for (int j=i*6+1; j<i*6+7; j++) vct.push_back(vcts[j]);
	 auto vc25 = vcts[31+i];
	 map.insert( std::pair<std::string,double>("FEMB"+std::to_string(i+1)+"_2991_VCC",(((vct[0]&0x0FFFF0000) >> 16) & 0x3FFF) * 305.18 * 0.000001 + 2.5) );
	 map.insert( std::pair<std::string,double>("FEMB"+std::to_string(i+1)+"_2991_T",(((vct[0]&0x0FFFF) & 0x3FFF) * 62.5) * 0.001) );
	 map.insert( std::pair<std::string,double>("FEMB"+std::to_string(i+1)+"_FMV39_V",(((vct[1]&0x0FFFF0000) >> 16) & 0x3FFF) * 305.18 * 0.000001) );
	 map.insert( std::pair<std::string,double>("FEMB"+std::to_string(i+1)+"_FMV39_I",((vct[1]& 0x3FFF) * 19.075) * 0.000001 / 0.1) );
	 map.insert( std::pair<std::string,double>("FEMB"+std::to_string(i+1)+"_FMV30_V",(((vct[2]&0x0FFFF0000) >> 16) & 0x3FFF) * 305.18 * 0.000001) );
	 map.insert( std::pair<std::string,double>("FEMB"+std::to_string(i+1)+"_FMV30_I",((vct[2]& 0x3FFF) * 19.075) * 0.000001 / 0.1) );
	 map.insert( std::pair<std::string,double>("FEMB"+std::to_string(i+1)+"_FMV18_V",(((vct[4]&0x0FFFF0000) >> 16) & 0x3FFF) * 305.18 * 0.000001) );
	 map.insert( std::pair<std::string,double>("FEMB"+std::to_string(i+1)+"_FMV18_I",((vct[4]& 0x3FFF) * 19.075) * 0.000001 / 0.1) );
	 map.insert( std::pair<std::string,double>("FEMB"+std::to_string(i+1)+"_AMV33_V",(((vct[3]&0x0FFFF0000) >> 16) & 0x3FFF) * 305.18 * 0.000001) );
	 map.insert( std::pair<std::string,double>("FEMB"+std::to_string(i+1)+"_AMV33_I",((vct[3]& 0x3FFF) * 19.075) * 0.000001 / 0.01) );
	 map.insert( std::pair<std::string,double>("FEMB"+std::to_string(i+1)+"_BIAS_V",(((vct[5]&0x0FFFF0000) >> 16) & 0x3FFF) * 305.18 * 0.000001) );
	 map.insert( std::pair<std::string,double>("FEMB"+std::to_string(i+1)+"_BIAS_I",((vct[5]& 0x3FFF) * 19.075) * 0.000001 / 0.1) );
	 map.insert( std::pair<std::string,double>("FEMB"+std::to_string(i+1)+"_AMV28_V",(((vc25&0x0FFFF0000) >> 16) & 0x3FFF) * 305.18 * 0.000001) );
	 map.insert( std::pair<std::string,double>("FEMB"+std::to_string(i+1)+"_AMV28_I",((vc25& 0x3FFF) * 19.075) * 0.000001 / 0.01) );
	 map.find("FEMB"+std::to_string(i+1)+"_AMV33_I")->second = map.find("FEMB"+std::to_string(i+1)+"_AMV33_I")->second - map.find("FEMB"+std::to_string(i+1)+"_AMV28_I")->second;
	 map.insert( std::pair<std::string,double>("FEMB"+std::to_string(i+1)+"_PC",(map.find("FEMB"+std::to_string(i+1)+"_FMV39_V")->second * map.find("FEMB"+std::to_string(i+1)+"_FMV39_I")->second) + (map.find("FEMB"+std::to_string(i+1)+"_FMV30_V")->second * map.find("FEMB"+std::to_string(i+1)+"_FMV30_I")->second) + (map.find("FEMB"+std::to_string(i+1)+"_FMV18_V")->second * map.find("FEMB"+std::to_string(i+1)+"_FMV18_I")->second) + (map.find("FEMB"+std::to_string(i+1)+"_AMV33_V")->second * map.find("FEMB"+std::to_string(i+1)+"_AMV33_I")->second) + (map.find("FEMB"+std::to_string(i+1)+"_BIAS_V")->second * map.find("FEMB"+std::to_string(i+1)+"_BIAS_I")->second) + (map.find("FEMB"+std::to_string(i+1)+"_AMV28_V")->second * map.find("FEMB"+std::to_string(i+1)+"_AMV28_I")->second)  ) );
     }
     
     return map;
}

void WIB::WIB_PLL_wr(int addr, int din){
   // This function is copied from Shanshan's python script
   // to configure WIB/FEMB.
   // The original function is in cls_config.py module inside the repository CE_LD with same name (git branch name is, Installation_Support)
   const std::string identification = "WIB::WIB_PLL_wr";
   auto value = 0x01 + ((addr&0xFF)<<8) + ((din&0x00FF)<<16);
   Write(11,value);
   CheckWIBRegisters(value, 11, 30);
   sleep(0.001);
   Write(10,1);
   CheckWIBRegisters(1, 10, 30);
   sleep(0.001);
   Write(10,0);
   CheckWIBRegisters(0, 10, 30);
}

void WIB::WIB_PLL_cfg(){
  // This function is copied from Shanshan's python script
  // to configure WIB/FEMB.
  // The original function is in cls_config.py module inside the repository CE_LD with same name (git branch name is, Installation_Support)
  const std::string identification = "WIB::WIB_PLL_cfg";
  
  std::string fullPath = getenv(WIB_CONFIG_PATH);
  fullPath += "/";
  fullPath += SI5344_CONFIG_FILENAME;
  std::ifstream confFile(fullPath.c_str());
  WIBException::WIB_BAD_ARGS e;
  
  if(confFile.fail()){
     std::stringstream expstr;
     expstr << "Cannot find config file  " << fullPath;
     e.Append(expstr.str().c_str());
     throw e;
  }
  
  std::string line;
  std::vector<int> adrs_h; 
  std::vector<int> adrs_l; 
  std::vector<int> datass;
  int cnt = 0;
  while(getline(confFile, line)){
    cnt = cnt + 1;
    int tmp = line.find(",");
    if (tmp > 0){
       auto adr = int(strtoul(line.substr(2,(tmp-2)).c_str(),NULL,16)); // earlier int adr
       adrs_h.push_back((adr&0xFF00)>>8);
       adrs_l.push_back((adr&0xFF));
       datass.push_back(int(strtoul(line.substr(tmp+3,2).c_str(),NULL,16))&0xFF);
    }
  }
  
  confFile.close();
  
  bool lol_flg = false;
  
  for (int i=0; i<5; i++){
    TLOG_INFO(identification) << "check PLL status, please wait..." << TLOG_ENDL;
    sleep(1);
    uint32_t ver_value = Read(12); // earlier int ver_value = Read(12)
    ver_value = Read(12);
    auto lol = (ver_value & 0x10000)>>16; // earlier int lol =
    auto lolXAXB = (ver_value & 0x20000)>>17; // earlier int lolXAXB = 
    auto INTR = (ver_value & 0x40000)>>18; // earlier int INTR = 
    if (lol == 1){
       lol_flg = true;
       break;
    }
  }
  
  if (lol_flg){
     TLOG_INFO(identification) << "PLL of WIB " << wib->GetRemoteAddress() <<  " has locked" << TLOG_ENDL;
     TLOG_INFO(identification) << "Select system clock and CMD from MBB" << TLOG_ENDL;
     Write(4, 0x03);
     CheckWIBRegisters(0x03, 4, 30);  
  }
  
  else{
    Write(10,0xFF0);
    sleep(0.01);
    Write(10,0xFF0);
    sleep(0.2);
    TLOG_INFO(identification) << "configurate PLL of WIB " << wib->GetRemoteAddress() <<  "please wait a few minutes..." << TLOG_ENDL;
    int p_addr = 1;
    //step1
    auto page4 = adrs_h[0]; // earlier int page4
    WIB_PLL_wr(p_addr, page4);
    WIB_PLL_wr(adrs_l[0], datass[0]);
    //step2
    page4 = adrs_h[1];
    WIB_PLL_wr(p_addr, page4);
    WIB_PLL_wr(adrs_l[1], datass[1]);
    //step3
    page4 = adrs_h[2];
    WIB_PLL_wr(p_addr, page4);
    WIB_PLL_wr(adrs_l[2], datass[2]);
    sleep(0.5);
    //step4
    for (unsigned int i=3; i<adrs_h.size(); i++){
       if (page4 == adrs_h[i]){
           WIB_PLL_wr(adrs_l[i], datass[i]);
       }
       else{
          page4 = adrs_h[i];
	  WIB_PLL_wr(p_addr, page4);
	  WIB_PLL_wr(adrs_l[i], datass[i]);
       }
    }
    
    for (int i=0; i<10; i++){
        sleep(3);
	TLOG_INFO(identification) << "check PLL status, please wait..." << TLOG_ENDL;
	uint32_t ver_value = Read(12); // earlier int ver_value
	ver_value = Read(12);
	auto lol = (ver_value & 0x10000)>>16; // earlier int lol
	auto lolXAXB = (ver_value & 0x20000)>>17; // earlier int lolXAXB
	auto INTR = (ver_value & 0x40000)>>18; // earlier int INTR
	if (lol == 1){
	   TLOG_INFO(identification) << "PLL of WIB " << wib->GetRemoteAddress() << " is locked" << TLOG_ENDL;
	   Write(4, 0x01);
	   CheckWIBRegisters(0x01, 4, 30);
	   break; 
	}
	if (i == 9){
	    WIBException::WIB_ERROR e;
	    std::stringstream expstr;
            expstr << "Fail to configurate PLL of WIB  " << wib->GetRemoteAddress() << ". Please check if MBB is on or 16MHz from dAQ.";
            e.Append(expstr.str().c_str());
            throw e;
	}
    }
    
  }
}

void WIB::WIB_CLKCMD_cs(uint8_t clockSource){
  // This function is copied from Shanshan's python script
  // to configure WIB/FEMB.
  // The original function is in cls_config.py module inside the repository CE_LD with same name (git branch name is, Installation_Support)
  
  const std::string identification = "WIB::WIB_CLKCMD_cs";
  
  if(clockSource){
     Write(0x04, 0x08); // select WIB onboard system clock and CMD
     CheckWIBRegisters(0x08, 0x04, 30);
     TLOG_WARNING(identification) << "select WIB onboard system clock and CMD, plese select system clock and CMD from MBB " << TLOG_ENDL;
  }
  
  else{
     WIB_PLL_cfg(); //select system clock and CMD from MBB
     TLOG_INFO(identification) << "select system clock and CMD from MBB" << TLOG_ENDL;
  }
}

void WIB::WIBs_SCAN(uint32_t WIB_ver, uint8_t clockSource){
  // This function is copied from Shanshan's python script
  // to configure WIB/FEMB.
  // The original function is in cls_config.py module inside the repository CE_LD with same name (git branch name is, Installation_Support)
  
  const std::string identification = "WIB::WIBs_SCAN";
  
  for (int i=0; i<5; i++){
     uint32_t wib_ver_rb = Read(0xFF); 
     wib_ver_rb = Read(0xFF);
     if ((wib_ver_rb&0x0FFF) == (WIB_ver&0x0FFF) && ( wib_ver_rb >= 0)){
        WIB_CLKCMD_cs(clockSource); // choose clock source
	break;
     }
     
     if (i == 4){
         WIBException::WIB_ERROR e;
	 std::stringstream expstr;
	 expstr << "WIB with IP  " << wib->GetRemoteAddress() << " readback error with read back value " << wib_ver_rb;
	 e.Append(expstr.str().c_str());
	 throw e;
     }
  }
  WIB_UDP_CTL();
  TLOG_INFO(identification) << "enable data stream and synchronize to Nevis" << TLOG_ENDL;
  Write(20, 0x00); // disable data stream and synchronize to Nevis
  CheckWIBRegisters(0x00, 20, 30);
  Write(20, 0x03); // disable data stream and synchronize to Nevis
  CheckWIBRegisters(0x03, 20, 30);
  Write(20, 0x00); // disable data stream and synchronize to Nevis
  CheckWIBRegisters(0x00, 20, 30);
  TLOG_INFO(identification) << "WIB scanning is done" << TLOG_ENDL;
}

void WIB::WIB_PWR_FEMB(int FEMB_NO, bool pwr_int_f, int power){
  // This function is copied from Shanshan's python script
  // to configure WIB/FEMB.
  // The original function is in cls_config.py module inside the repository CE_LD with same name (git branch name is, Installation_Support)
  // This function is called on only a single FEMB at once (FEMB_NO = 1, 2, 3, 4)
  
  const std::string identification = "WIB::WIB_PWR_FEMB";
  
  std::vector<uint32_t> pwr_ctl = {0x31000F, 0x5200F0, 0x940F00, 0x118F000};
  
  if (pwr_int_f){
    int pwr_wr = 0;
    if (power == 1){
       pwr_wr = pwr_wr | pwr_ctl[FEMB_NO-1];
    }
    else{
      pwr_wr = pwr_wr | 0;
    }
    Write(0x8, 0); // All off
    CheckWIBRegisters(0, 0x8, 30);
    sleep(5);
    Write(0x8, pwr_wr);
    CheckWIBRegisters(pwr_wr, 0x8, 30);
    sleep(5);
    Write(0x8, 0); // All off
    CheckWIBRegisters(0, 0x8, 30);
    sleep(0.1);
    Write(0x8, pwr_wr);
    CheckWIBRegisters(pwr_wr, 0x8, 30);
  } // pwr_int_f == true
  
  else{
    auto pwr_status = Read(0x8);
    if (power == 1){
      pwr_status = pwr_status | pwr_ctl[FEMB_NO-1];
      Write(0x8, pwr_status);
      CheckWIBRegisters(pwr_status, 0x8, 30);
      sleep(0.5);
    }
    else{
      if ((FEMB_NO-1 == 3) && (((Read(0x8) & 0xFFF)) == 0xFFF) && ((((Read(0x8) & 0x70000))>>16) == 0x07) && (((Read(0x08) && 0xE00000)>>21) == 0x07)){
         pwr_status = 0x00000000;
      }
      else{
        pwr_status = pwr_status & ((~(pwr_ctl[FEMB_NO-1])) | 0x00100000);
      }
      TLOG_INFO(identification) << "FEMB # : " << FEMB_NO << " register 8 value : " << std::hex << pwr_status << TLOG_ENDL;
      Write(0x8, pwr_status);
      CheckWIBRegisters(pwr_status, 0x8, 30);
      //sleep(0.5);
    }
  } // pwr_int_f == false
  
  sleep(2);
}

void WIB::WIB_PWR_FEMB(std::vector<bool> &FEMB_NOs, bool pwr_int_f, std::vector<int> power){
  // This function is copied from Shanshan's python script
  // to configure WIB/FEMB.
  // The original function is in cls_config.py module inside the repository CE_LD with same name (git branch name is, Installation_Support)
  // This function is called on all 4 FEMBs at once
  
  const std::string identification = "WIB::WIB_PWR_FEMB";
  
  std::vector<uint32_t> pwr_ctl = {0x31000F, 0x5200F0, 0x940F00, 0x118F000};
  
  if (pwr_int_f){
     int pwr_wr = 0;
     for (unsigned int i=0; i<FEMB_NOs.size(); i++){
         if (FEMB_NOs[i]){
	   if (power[i] == 1){
	     pwr_wr = pwr_wr | pwr_ctl[i];
	   }
	   else{
	    pwr_wr = pwr_wr | 0; 
	   }
	 }
     }
     Write(0x8, 0); // All off
     CheckWIBRegisters(0, 0x8, 30);
     sleep(5);
     Write(0x8, pwr_wr);
     CheckWIBRegisters(pwr_wr, 0x8, 30);
     sleep(5);
     Write(0x8, 0); // All off
     CheckWIBRegisters(0, 0x8, 30);
     sleep(0.1);
     Write(0x8, pwr_wr);
     CheckWIBRegisters(pwr_wr, 0x8, 30);
  } // pwr_int_f == true
  
  else{
    auto pwr_status = Read(0x8);
    for (unsigned int i=0; i<FEMB_NOs.size(); i++){
        if (FEMB_NOs[i]){
	  if (power[i] == 1){
	     pwr_status = pwr_status | pwr_ctl[i];
             Write(0x8, pwr_status);
             CheckWIBRegisters(pwr_status, 0x8, 30);
             sleep(0.5);
	  }
	  
	  else{
	    if ((i == 3) && (((Read(0x8) & 0xFFF)) == 0xFFF) && ((((Read(0x8) & 0x70000))>>16) == 0x07) && (((Read(0x08) && 0xE00000)>>21) == 0x07)){
	      pwr_status = 0x00000000;
	    }
	    else{
	      pwr_status = pwr_status & ((~(pwr_ctl[i])) | 0x00100000);
	    }
	    Write(0x8, pwr_status);
            CheckWIBRegisters(pwr_status, 0x8, 30);
	    sleep(0.5);
	  }
	}
    }
  } // pwr_int_f == false
  
  if (std::count(power.begin(), power.end(), 1)) sleep(3);
  else sleep(1);
}

void WIB::WIBs_CFG_INIT(bool jumbo_flag){
  // This function is copied from Shanshan's python script
  // to configure WIB/FEMB.
  // The original function is in cls_config.py module inside the repository CE_LD with same name (git branch name is, Installation_Support)
  
  const std::string identification = "WIB::WIB_PWR_FEMB";
  
  WIB_UDP_CTL();
  if (jumbo_flag){
     Write(0x1F, 0xEFB); // normal operation
     CheckWIBRegisters(0xEFB, 0x1F, 30);
  }
  else{
    Write(0x1F, 0x1FB); // normal operation
    CheckWIBRegisters(0x1FB, 0x1F, 30);
  }
  
  Write(0x0F, 0x0); // normal operation
  CheckWIBRegisters(0x0, 0x0F, 30);
}
