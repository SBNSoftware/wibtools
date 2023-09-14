#include <WIBStatus.hh>

void WIBTool::WIBStatus::ProcessFEMB(uint8_t FEMB){  
  // Conversions from Jack: 
  // Vcc      = (data) * 305.18 uV + 2.5 V
  // TEMP     = (data) * 0.0625 C
  // V1-6     = (data) * 3.0518 mV
  // C1-2,4-6 = (data) * 190.75 uA
  // C3       = (data) * 1.9075 mA  
  
  int   iFEMB = FEMB-1; // FEMB index (0-3)
  FEMB_PWR[iFEMB]   = 0;
  FEMB_TEMP[iFEMB]  = 0;
  FEMB_VCC[iFEMB]   = 0;
    
  const Item *g = wib->GetItem("PWR_EN_BRD"+std::to_string(FEMB));

  // reset register
  wib->WriteWithRetry(0x05,0x0);

  // check if board is on
  if( (g->mask & wib->ReadWithRetry(g->address))==g->mask ) FEMB_PWR[iFEMB]=1;
 
  std::string pwrMesSel("PWR_MES_SEL");
  std::string pwrMesSelBrd = pwrMesSel; 
  pwrMesSelBrd.append("_BRD");
  pwrMesSelBrd.append(1,'0'+FEMB);

  // get temp and Vcc
  wib->WriteWithRetry(pwrMesSel,wib->GetItem(pwrMesSelBrd+"_VCC_TEMP")->mask);

  /* 
  std::cout
  <<"\nGetting Vcc and Temp for board "<<int(FEMB)<<":\n"
  <<"  setting reg 5: "<<std::hex<<wib->Read(0x05)<<std::dec<<"\n"
  <<"  reading reg 6: "<<std::hex<<wib->Read(0x06)<<std::dec<<"\n"
  <<"  - upper bits : "<<wib->Read("PWR_MES_OUT_V")<<"\n"
  <<"  - lower bits : "<<wib->Read("PWR_MES_OUT_C_TEMP")<<"\n";
  */

  //uint32_t upperBits = ConvertSignedInt(wib->Read("PWR_MES_OUT_V"));
  uint32_t upperBits = (wib->ReadWithRetry("PWR_MES_OUT_V"));
  uint32_t lowerBits = ConvertSignedInt(wib->ReadWithRetry("PWR_MES_OUT_C_TEMP"));
  FEMB_VCC[iFEMB]  = double(upperBits) * 305.18e-6 + 2.5;
  FEMB_TEMP[iFEMB] = double(lowerBits) * 0.0625;
  
  // get voltages and currents
  for(int i=0; i<6; i++){
    FEMB_V[iFEMB][i]=0;
    FEMB_C[iFEMB][i]=0;
    uint8_t iv = i+1;
    wib->WriteWithRetry(pwrMesSel,wib->GetItem(pwrMesSelBrd+"_"+std::to_string(iv))->mask);
    //upperBits = ConvertSignedInt(wib->Read("PWR_MES_OUT_V"));
    upperBits = (wib->ReadWithRetry("PWR_MES_OUT_V"));
    lowerBits = ConvertSignedInt(wib->ReadWithRetry("PWR_MES_OUT_C_TEMP"));

    /*
    if( FEMB == 1 || FEMB == 2 || FEMB == 4 ) {
    std::cout
    <<"Getting V/C "<<i+1<<" ("<<pwrMesSelBrd+"_"+std::to_string(iv)<<")\n"
    <<"  setting reg 5: "<<std::hex<<wib->Read(0x05)<<std::dec<<"\n"
    <<"  reading reg 6: "<<std::hex<<wib->Read(0x06)<<std::dec<<"\n"
    <<"  - upper bits : "<<std::hex<<upperBits<<std::dec<<"\n"
    <<"  - lower bits : "<<std::hex<<lowerBits<<std::dec<<"\n";
    }
    */

    FEMB_V[iFEMB][i]              = double(upperBits) * 3.0518/10. * 1e-3; // convert mV to V
    if( iv==3 ) FEMB_C[iFEMB][i]  = double(lowerBits) * 1.9075 * 1e-3; // convert mA -> A
    else        FEMB_C[iFEMB][i]  = double(lowerBits) * 190.75 * 1e-6; // convert uA --> A
  
    // if the board is off, set V to 0 if it shows 3FFF:
    if( FEMB_PWR[iFEMB] == 0 
      && ( upperBits == 0x3fff || upperBits == 0x3ffe ) ) FEMB_V[iFEMB][i] = 0.;
  }

  // Get 4 link statuses for this board (2bits per link)
  uint32_t linkStatBits = wib->ReadWithRetry("LINK_SYNC_STATUS_BRD"+std::to_string(FEMB));
  for(int i=0; i<4; i++) LINK_STATUS[iFEMB][i] = 0;
  if( (0x3  & linkStatBits) != 0x0 ) LINK_STATUS[iFEMB][0] = 1;     
  if( (0xC  & linkStatBits) != 0x0 ) LINK_STATUS[iFEMB][1] = 1;     
  if( (0x30 & linkStatBits) != 0x0 ) LINK_STATUS[iFEMB][2] = 1;     
  if( (0xC0 & linkStatBits) != 0x0 ) LINK_STATUS[iFEMB][3] = 1;     

  // Get 4 equalizer statuses (1 bit per link)
  uint32_t eqStatBits = wib->ReadWithRetry("EQ_LOS_BRD"+std::to_string(FEMB)+"_RX");
  for(int i=0; i<4; i++) EQUALIZER_STATUS[iFEMB][i] = 0;
  if( (0x1  & eqStatBits) != 0x0 ) EQUALIZER_STATUS[iFEMB][0] = 1;     
  if( (0x2  & eqStatBits) != 0x0 ) EQUALIZER_STATUS[iFEMB][1] = 1;     
  if( (0x4 & eqStatBits) != 0x0 ) EQUALIZER_STATUS[iFEMB][2] = 1;     
  if( (0x8 & eqStatBits) != 0x0 ) EQUALIZER_STATUS[iFEMB][3] = 1;     

  // Get checksum error, timestamp, timestmap error, frame error
  // (these all require using 0x12 to set the FEMB and the link)
  wib->WriteWithRetry("FEMB_STAT_SEL",uint32_t(iFEMB));
  for(int i=0; i<4; i++){
    wib->WriteWithRetry("LINK_STAT_SEL",uint32_t(i));
    CHKSUM_ERROR_COUNT[iFEMB][i]  = wib->ReadWithRetry("CHKSUM_ERROR");
    TIME_STAMP[iFEMB][i]          = wib->ReadWithRetry("TIME_STAMP");
    TS_ERROR_COUNT[iFEMB][i]      = wib->ReadWithRetry("TS_ERROR"); 
    FRAME_ERROR_COUNT[iFEMB][i]   = wib->ReadWithRetry("FRAME_ERROR"); 
  }


  // Get everything else we care to look at 
  ADC_READOUT_EN[iFEMB] = wib->ReadFEMB(FEMB, "ADC_DISABLE_REG");
  CLOCK_SWITCH[iFEMB]   = wib->ReadFEMB(FEMB, "FEMB_SYSTEM_CLOCK_SWITCH");
  ACTIVE_CLOCK[iFEMB]   = wib->ReadFEMB(FEMB, "FEMB_SYSTEM_ACTIVE_CLOCK");
  CLOCK_STATUS[iFEMB][0]= wib->ReadFEMB(FEMB, "FEMB_SYSTEM_CLOCK_STATUS_0"); // 0=good, 1=bad
  CLOCK_STATUS[iFEMB][1]= wib->ReadFEMB(FEMB, "FEMB_SYSTEM_CLOCK_STATUS_1"); // 0=good, 1=bad
  STREAM_EN[iFEMB]      = wib->ReadFEMB(FEMB, "STREAM_EN");
  ADC_SEND_EN[iFEMB]    = wib->ReadFEMB(FEMB,  "ADC_DATA_EN");
  ADC_SYNC_MODE[iFEMB]  = wib->ReadFEMB(FEMB, "ADC_SYNC_MODE");
  FEMB_TEST_SEL[iFEMB]  = wib->ReadFEMB(FEMB, "FEMB_TST_SEL");
  FIRMWARE_VER[iFEMB]   = wib->ReadFEMB(FEMB, "VERSION_ID");
  ID[iFEMB]             = wib->ReadFEMB(FEMB, "BOARD_ID");
  COMPILED_VER[iFEMB]   = wib->ReadFEMB(FEMB, "COMPILED_VERSION");
  DATE_COMPILED[iFEMB]  = wib->ReadFEMB(FEMB, "DATE_COMPILED"); 
  TIME_COMPILED[iFEMB]  = wib->ReadFEMB(FEMB, "TIME_COMPILED"); 
  
}

void WIBTool::WIBStatus::ProcessWIB(){  
  // Conversions from Jack: 
  // Vcc      = (data) * 305.18 uV + 2.5 V
  // TEMP     = (data) * 0.0625 C
  // V1-6     = (data) * 3.0518 mV
  // C1-2,4-6 = (data) * 190.75 uA
  // C3       = (data) * 1.9075 mA  
 
  // reset measurements
  WIB_TEMP  = 0; 
  WIB_VCC   = 0;
  for(int i=0; i<4; i++) WIB_V[i] = 0; 
    
  std::string pwrMesSel("PWR_MES_SEL");
  
  // Vcc and temp
  wib->WriteWithRetry(pwrMesSel,wib->GetItem(pwrMesSel+"_WIB_VCC_TEMP")->mask);
  
  uint32_t upperBits = (wib->ReadWithRetry("PWR_MES_OUT_V"));
  uint32_t lowerBits = ConvertSignedInt(wib->ReadWithRetry("PWR_MES_OUT_C_TEMP"));
  WIB_VCC  = double(upperBits) * 305.18e-6 + 2.5;
  WIB_TEMP = double(lowerBits) * 0.0625;
  
  // get voltages and currents
  for(int i=0; i<4; i++){
    WIB_V[i]=0;
    WIB_C[i]=0;
    uint8_t iv = i+1;
    wib->WriteWithRetry(pwrMesSel,wib->GetItem((pwrMesSel+"_WIB_"+std::to_string(iv)).c_str())->mask);
    upperBits = (wib->ReadWithRetry("PWR_MES_OUT_V"));
    lowerBits = ConvertSignedInt(wib->ReadWithRetry("PWR_MES_OUT_C_TEMP"));
    WIB_V[i]              = double(upperBits) * 3.0518/10. * 1e-3; // convert mV to V
    if( iv==3 ) WIB_C[i]  = double(lowerBits) * 1.9075 * 1e-3; // convert mA -> A
    else        WIB_C[i]  = double(lowerBits) * 190.75 * 1e-6; // convert uA --> A
  }
  
  // WIB clock status (register 0x04, bits 0:3)
  //    b0 ("FEMB_CLK_SEL") controls system clock (100MHz)
  //      = 0 --> from WIB FPGA (default)
  //      = 1 --> from SI5344
  //    b1 ("FEMB_CMD_SEL") controls system CMD clock (2MHz)
  //      = 0 --> from WIB FPGA (default)
  //      = 1 --> from PTC (MBB->PTC->WIB)
  //    b2-3 ("FEMB_INT_CLK_SEL") 
  //      = 00 or 01 --> select SBND_CLK from SI5344
  //      = 10 --> select CLK_100MHz from PLL driven by onboard 50MHz osc 
  FEMB_CLK      = wib->ReadWithRetry("FEMB_CLK_SEL");
  FEMB_CMD      = wib->ReadWithRetry("FEMB_CMD_SEL");
  FEMB_INT_CLK  = wib->ReadWithRetry("FEMB_INT_CLK_SEL");
  PLL_CLK_LOL   = wib->ReadWithRetry("SI5344_LOL");
  PLL_CLK_LOS   = wib->ReadWithRetry("SI5344_LOSXAXB");
  
}


void WIBTool::WIBStatus::Process(std::string const & option){  
 
  //Prompt measurements
  StartPowerMes();
 
  //Get FEMB and WIB power measurements
  ProcessWIB();
  for(uint8_t i=1; i<=FEMB_COUNT;i++) ProcessFEMB(i);

  if( option == "power" || option == "pwr" || option == "p" || option == "" )
    PrintPowerTable();

  if( option == "clk" || option == "c" || option == "" )
    PrintWIBClockTable();

  if( option == "femb" || option == "" ) 
    PrintFEMBTable();
  
  printf("\n\n");
}

void WIBTool::WIBStatus::StartPowerMes(){
  // disable filter
  wib->WriteWithRetry("FILTER_EN",0x0);
  // repeatedly toggle bit 16 to enable LTC2991 CMS conversion
  for(int i=0; i<100; i++){
    wib->WriteWithRetry("PWR_MES_START",0x0);
    wib->WriteWithRetry("PWR_MES_START",0x1); 
  }
  usleep(5000); // sleep for 5ms
}

uint32_t WIBTool::WIBStatus::ConvertSignedInt(uint32_t in){
  // Assumes input is a 'signed' 14-bit integer cast into
  // an unsigned int. This checks if bit 14 is set; if so,
  // value is set to 0x0
  in = ( ~0xC000 & in );
  if( in & 0x2000 ) in = 0x0;
  return in;
}

void WIBTool::WIBStatus::PrintPowerTable(){
  // =======================================================================================
  // Print the power / temperature monitoring table for WIB and FEMBs
  // =======================================================================================
  printf("\n%10s","");    
  printf("%10s","WIB"); 
  for(uint8_t i=0;i<FEMB_COUNT;i++) { printf("%10s",("FEMB_"+std::to_string(i+1)).c_str());  }           
  printf("\n");

  printf(" =========");   
  printf("==========");   
  for(uint8_t i=0;i<FEMB_COUNT;i++) printf("==========");      
  printf("\n");
  
  printf("%10s","ON/OFF");
  printf("%10s","");   
  for(uint8_t i=0;i<FEMB_COUNT;i++) printf("%10d",FEMB_PWR[i]);     
  printf("\n");
  
  printf("%10s","TEMP [C]");     
  printf("%10.2f",WIB_TEMP); 
  for(uint8_t i=0;i<FEMB_COUNT;i++) printf("%10.2f",FEMB_TEMP[i]);  
  printf("\n");
  
  printf("%10s","Vcc [V]");    
  printf("%10.2f",WIB_VCC);   
  for(uint8_t i=0;i<FEMB_COUNT;i++) printf("%10.2f",FEMB_VCC[i]);   
  printf("\n");
  
  for(int iv=0; iv<6; iv++){
  //    printf("   -------");      for(uint8_t i=0;i<FEMB_COUNT;i++) printf("----------");      printf("\n");
  printf("%10s",("V"+std::to_string(iv+1)+" [V]").c_str());  
  if( iv+1 <= 4 ) { printf("%10.2f",WIB_V[iv]); } else { printf("%10s",""); }
  for(uint8_t i=0;i<FEMB_COUNT;i++) printf("%10.2f",FEMB_V[i][iv]); 
  printf("\n");
  
  printf("%10s",("C"+std::to_string(iv+1)+" [A]").c_str());  
  if( iv+1 <= 4 ) { printf("%10.3f",WIB_C[iv]); } else { printf("%10s",""); }
  for(uint8_t i=0;i<FEMB_COUNT;i++) printf("%10.3f",FEMB_C[i][iv]); 
  printf("\n");
  }
  printf("\n");
}

void WIBTool::WIBStatus::PrintWIBClockTable(){
  
  // =======================================================================================
  // WIB clock parameters
  // =======================================================================================
  
  std::string femb_clk[2]       ={"WIB FPGA", "SI5344"};
  std::string femb_cmd[2]       ={"WIB FPGA", "PTC"};
  std::string femb_int_clk[3]   ={"SI5344","SI5344","PLL"};
  if( FEMB_INT_CLK > 2 ) FEMB_INT_CLK = 2;
  
  printf("\n\n");   
  printf("%20s","WIB clock settings"); printf("%20s",""); printf("\n");
  printf("========================================\n");
  printf("%20s","SYSTEM CLOCK:");   printf("%10d",FEMB_CLK);      printf("%10s",femb_clk[FEMB_CLK].c_str()); printf("\n");
  printf("%20s","CMD CLOCK:");      printf("%10d",FEMB_CMD);      printf("%10s",femb_cmd[FEMB_CMD].c_str()); printf("\n");
  printf("%20s","INT CLOCK:");      printf("%10d",FEMB_INT_CLK);  printf("%10s",femb_int_clk[FEMB_INT_CLK].c_str()); printf("\n");
  printf("%20s","PLL CLK LOCK (LOL):");      printf("%10d",PLL_CLK_LOL);  printf("%10s",""); printf("\n");
  printf("%20s","PLL CLK LOCK (LOS):");      printf("%10d",PLL_CLK_LOS);  printf("%10s",""); printf("\n");
  

}

void WIBTool::WIBStatus::PrintFEMBTable(){
  
  // =======================================================================================
  // Link status table (& other goodies)
  // =======================================================================================
  
  std::string enable_disable[2] ={"disabled","enabled"};
  std::string on_off[2]         ={"off","on"};
  std::string badclk[2]         ={"good","bad"};
  std::string clkmode[2]        ={"system","osc"};
  std::string syncmode[4]       ={"normal","free run","follow","disabled"};

  printf("\n\n%20s","");    
  for(uint8_t i=0;i<FEMB_COUNT;i++) { printf("%12s",("FEMB_"+std::to_string(i+1)).c_str());  }           
  printf("\n");
  printf(" ===================");   
  for(uint8_t i=0;i<FEMB_COUNT;i++) printf("============");      
  printf("\n");
  
  printf("%20s","BOARD ID:");   for(uint8_t i=0;i<FEMB_COUNT;i++) printf("%12d",ID[i]); printf("\n");   
  printf("%20s","FW VERSION:");       for(uint8_t i=0;i<FEMB_COUNT;i++) printf("%12d",FIRMWARE_VER[i]); printf("\n");   
  printf("%20s","COMPILED VERSION:"); for(uint8_t i=0;i<FEMB_COUNT;i++) printf("%12d",COMPILED_VER[i]); printf("\n");   
  printf("%20s","DATE COMPILED:"); for(uint8_t i=0;i<FEMB_COUNT;i++) printf("%12d",DATE_COMPILED[i]); printf("\n");   
  printf("%20s","TIME COMPILED:"); for(uint8_t i=0;i<FEMB_COUNT;i++) printf("%12d",TIME_COMPILED[i]); printf("\n");   
  
  printf(" -------------------\n");
    
  printf("%20s","DATA STREAM:");
  for(uint8_t i=0;i<FEMB_COUNT;i++) printf("%12s",enable_disable[ADC_READOUT_EN[i]].c_str());
  printf("\n");   
  
  printf("%20s","ACTIVE CLK:");
  for(uint8_t i=0;i<FEMB_COUNT;i++) printf("%12d",ACTIVE_CLOCK[i]);
  printf("\n");   
  
  printf("%20s","CLK SWITCH:");
  for(uint8_t i=0;i<FEMB_COUNT;i++) printf("%12s",clkmode[CLOCK_SWITCH[i]].c_str());
  printf("\n");   
  
  printf("%20s","CLK 0 STATUS:");
  for(uint8_t i=0;i<FEMB_COUNT;i++) printf("%12s",badclk[CLOCK_STATUS[i][0]].c_str());
  printf("\n");   
  printf("%20s","CLK 1 STATUS:");
  for(uint8_t i=0;i<FEMB_COUNT;i++) printf("%12s",badclk[CLOCK_STATUS[i][1]].c_str());
  printf("\n");   
  
  printf("%20s","ADC SEND:");
  for(uint8_t i=0;i<FEMB_COUNT;i++) printf("%12s",enable_disable[ADC_SEND_EN[i]].c_str());
  printf("\n");   
  
  printf("%20s","HIGH-SPEED DATA:");
  for(uint8_t i=0;i<FEMB_COUNT;i++) printf("%12s",enable_disable[STREAM_EN[i]].c_str());
  printf("\n");   

  printf("%20s","ADC SYNC MODE:");
  for(uint8_t i=0;i<FEMB_COUNT;i++) printf("%12s",syncmode[ADC_SYNC_MODE[i]].c_str());
  printf("\n");   
 
  printf("%20s","TEST MODE:");
  for(uint8_t i=0;i<FEMB_COUNT;i++) printf("%12d",FEMB_TEST_SEL[i]);
  printf("\n");   
  
  printf(" -------------------\n");
  
  for(int iLink=0; iLink<4; iLink++){

    printf("%20s",("HS LINK "+std::to_string(iLink+1)).c_str());
    printf("\n");

    printf("%20s","SYNC STATUS:");
    for(uint8_t i=0;i<FEMB_COUNT;i++) printf("%12d",LINK_STATUS[i][iLink]);
    printf("\n");   
    
    printf("%20s","EQUALIZER STATUS:");
    for(uint8_t i=0;i<FEMB_COUNT;i++) printf("%12d",EQUALIZER_STATUS[i][iLink]);
    printf("\n");   
 
    printf("%20s","TIMESTAMP:");
    for(uint8_t i=0;i<FEMB_COUNT;i++) printf("%12d",TIME_STAMP[i][iLink]);
    printf("\n");   
    
    printf("%20s","TIMESTAMP ERROR CNT:");
    for(uint8_t i=0;i<FEMB_COUNT;i++) printf("%12d",TS_ERROR_COUNT[i][iLink]);
    printf("\n");   
    
    printf("%20s","CHKSUM ERROR CNT:");
    for(uint8_t i=0;i<FEMB_COUNT;i++) printf("%12d",CHKSUM_ERROR_COUNT[i][iLink]);
    printf("\n");   
    
    printf("%20s","FRAME ERROR CNT:");
    for(uint8_t i=0;i<FEMB_COUNT;i++) printf("%12d",FRAME_ERROR_COUNT[i][iLink]);
    printf("\n");   
    
    printf(" -------------------\n");
  }
  

}

// Returns status variables for archiver
std::map<std::string,double> WIBTool::WIBStatus::RetrieveStatusVars(){
  StartPowerMes();
  ProcessWIB();
  std::map<std::string,double> wib_map = wib->WIB_STATUS();
  for(uint8_t i=1; i<=FEMB_COUNT;i++) ProcessFEMB(i);
  
  std::map<std::string,double> map;
  map.insert( std::pair<std::string,double>("WIB_TEMP",WIB_TEMP) );
  map.insert( std::pair<std::string,double>("WIB_VCC",WIB_VCC) );
  map.insert( std::pair<std::string,double>("WIB_V1",WIB_V[0]) );
  map.insert( std::pair<std::string,double>("WIB_V2",WIB_V[1]) );
  map.insert( std::pair<std::string,double>("WIB_V3",WIB_V[2]) );
  map.insert( std::pair<std::string,double>("WIB_V4",WIB_V[3]) );
  map.insert( std::pair<std::string,double>("WIB_C1",WIB_C[0]) );
  map.insert( std::pair<std::string,double>("WIB_C2",WIB_C[1]) );
  map.insert( std::pair<std::string,double>("WIB_C3",WIB_C[2]) );
  map.insert( std::pair<std::string,double>("WIB_C4",WIB_C[3]) );

  for(int i=0; i<FEMB_COUNT; i++){
    int ibrd = i+1;
    map.insert( std::pair<std::string,double>( "FEMB"+std::to_string(ibrd)+"_PWR", FEMB_PWR[i] ));
    map.insert( std::pair<std::string,double>( "FEMB"+std::to_string(ibrd)+"_TEMP", FEMB_TEMP[i]) );
    map.insert( std::pair<std::string,double>( "FEMB"+std::to_string(ibrd)+"_VCC", FEMB_VCC[i] ));
    map.insert( std::pair<std::string,double>( "FEMB"+std::to_string(ibrd)+"_V1", FEMB_V[i][0] ));
    map.insert( std::pair<std::string,double>( "FEMB"+std::to_string(ibrd)+"_V2", FEMB_V[i][1] ));
    map.insert( std::pair<std::string,double>( "FEMB"+std::to_string(ibrd)+"_V3", FEMB_V[i][2] ));
    map.insert( std::pair<std::string,double>( "FEMB"+std::to_string(ibrd)+"_V4", FEMB_V[i][3] ));
    map.insert( std::pair<std::string,double>( "FEMB"+std::to_string(ibrd)+"_V5", FEMB_V[i][4] ));
    map.insert( std::pair<std::string,double>( "FEMB"+std::to_string(ibrd)+"_V6", FEMB_V[i][5] ));
    map.insert( std::pair<std::string,double>( "FEMB"+std::to_string(ibrd)+"_C1", FEMB_C[i][0] ));
    map.insert( std::pair<std::string,double>( "FEMB"+std::to_string(ibrd)+"_C2", FEMB_C[i][1] ));
    map.insert( std::pair<std::string,double>( "FEMB"+std::to_string(ibrd)+"_C3", FEMB_C[i][2] ));
    map.insert( std::pair<std::string,double>( "FEMB"+std::to_string(ibrd)+"_C4", FEMB_C[i][3] ));
    map.insert( std::pair<std::string,double>( "FEMB"+std::to_string(ibrd)+"_C5", FEMB_C[i][4] ));
    map.insert( std::pair<std::string,double>( "FEMB"+std::to_string(ibrd)+"_C6", FEMB_C[i][5] ));
    
    map.insert( std::pair<std::string,double>( "FEMB"+std::to_string(ibrd)+"_BOARD_ID", ID[i] ));
    map.insert( std::pair<std::string,double>( "FEMB"+std::to_string(ibrd)+"_FIRMWARE_VER", FIRMWARE_VER[i] ));
    map.insert( std::pair<std::string,double>( "FEMB"+std::to_string(ibrd)+"_COMPILED_VER", COMPILED_VER[i] ));
    map.insert( std::pair<std::string,double>( "FEMB"+std::to_string(ibrd)+"_DATE_COMPILED", DATE_COMPILED[i] ));
    map.insert( std::pair<std::string,double>(  "FEMB"+std::to_string(ibrd)+"_TIME_COMPILED", TIME_COMPILED[i] ));

    map.insert( std::pair<std::string,double>(  "FEMB"+std::to_string(ibrd)+"_ADC_READOUT_ENABLE", ADC_READOUT_EN[i] ));
    map.insert( std::pair<std::string,double>(  "FEMB"+std::to_string(ibrd)+"_ADC_SEND_ENABLE", ADC_SEND_EN[i] ));
    map.insert( std::pair<std::string,double>(  "FEMB"+std::to_string(ibrd)+"_ACTIVE_CLOCK", ACTIVE_CLOCK[i] ));
    map.insert( std::pair<std::string,double>(  "FEMB"+std::to_string(ibrd)+"_CLOCK_SWITCH", CLOCK_SWITCH[i] ));
    map.insert( std::pair<std::string,double>(  "FEMB"+std::to_string(ibrd)+"_CLOCK0_STATUS", CLOCK_STATUS[i][0] ));
    map.insert( std::pair<std::string,double>(  "FEMB"+std::to_string(ibrd)+"_CLOCK1_STATUS", CLOCK_STATUS[i][1] ));
    map.insert( std::pair<std::string,double>(  "FEMB"+std::to_string(ibrd)+"_HIGHSPEED_DATA_ENABLE", STREAM_EN[i] ));
    map.insert( std::pair<std::string,double>(  "FEMB"+std::to_string(ibrd)+"_ADC_SYNC_MODE", ADC_SYNC_MODE[i] ));
    map.insert( std::pair<std::string,double>(  "FEMB"+std::to_string(ibrd)+"_TEST_SEL", FEMB_TEST_SEL[i] ));
    
    for(int iLink=0; iLink<4; iLink++){
      map.insert( std::pair<std::string,double>(  "FEMB"+std::to_string(ibrd)+"_LINK"+std::to_string(iLink+1)+"_LINK_STATUS", LINK_STATUS[i][iLink] ));
      map.insert( std::pair<std::string,double>(  "FEMB"+std::to_string(ibrd)+"_LINK"+std::to_string(iLink+1)+"_EQUALIZER_STATUS", EQUALIZER_STATUS[i][iLink] ));
      map.insert( std::pair<std::string,double>(  "FEMB"+std::to_string(ibrd)+"_LINK"+std::to_string(iLink+1)+"_TIME_STAMP", TIME_STAMP[i][iLink] ));
      map.insert( std::pair<std::string,double>(  "FEMB"+std::to_string(ibrd)+"_LINK"+std::to_string(iLink+1)+"_TIME_STAMP_ERRCOUNT", TS_ERROR_COUNT[i][iLink] ));
      map.insert( std::pair<std::string,double>(  "FEMB"+std::to_string(ibrd)+"_LINK"+std::to_string(iLink+1)+"_CHKSUM_ERRCOUNT", CHKSUM_ERROR_COUNT[i][iLink] ));
      map.insert( std::pair<std::string,double>(  "FEMB"+std::to_string(ibrd)+"_LINK"+std::to_string(iLink+1)+"_FRAME_ERRCOUNT", FRAME_ERROR_COUNT[i][iLink] ));
    }
    
    map.insert( std::pair<std::string,double>(  "FEMB"+std::to_string(ibrd)+"_AMV28_I", wib_map.find("FEMB"+std::to_string(ibrd)+"_AMV28_I")->second ));
         
  }  

  return map;

}

