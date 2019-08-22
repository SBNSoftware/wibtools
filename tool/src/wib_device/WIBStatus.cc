#include <wib_device/WIBStatus.hh>

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
  wib->Write(0x05,0x0);

  // check if board is on
  if( (g->mask & wib->Read(g->address))==g->mask ) FEMB_PWR[iFEMB]=1;
 
  std::string pwrMesSel("PWR_MES_SEL");
  std::string pwrMesSelBrd = pwrMesSel; 
  pwrMesSelBrd.append("_BRD");
  pwrMesSelBrd.append(1,'0'+FEMB);

  // get temp and Vcc
  wib->Write(pwrMesSel,wib->GetItem(pwrMesSelBrd+"_VCC_TEMP")->mask);

  /* 
  std::cout
  <<"\nGetting Vcc and Temp for board "<<int(FEMB)<<":\n"
  <<"  setting reg 5: "<<std::hex<<wib->Read(0x05)<<std::dec<<"\n"
  <<"  reading reg 6: "<<std::hex<<wib->Read(0x06)<<std::dec<<"\n"
  <<"  - upper bits : "<<wib->Read("PWR_MES_OUT_V")<<"\n"
  <<"  - lower bits : "<<wib->Read("PWR_MES_OUT_C_TEMP")<<"\n";
  */

  //uint32_t upperBits = ConvertSignedInt(wib->Read("PWR_MES_OUT_V"));
  uint32_t upperBits = (wib->Read("PWR_MES_OUT_V"));
  uint32_t lowerBits = ConvertSignedInt(wib->Read("PWR_MES_OUT_C_TEMP"));
  FEMB_VCC[iFEMB]  = double(upperBits) * 305.18e-6 + 2.5;
  FEMB_TEMP[iFEMB] = double(lowerBits) * 0.0625;
  
  // get voltages and currents
  for(int i=0; i<6; i++){
    FEMB_V[iFEMB][i]=0;
    FEMB_C[iFEMB][i]=0;
    uint8_t iv = i+1;
    wib       ->Write(pwrMesSel,wib->GetItem(pwrMesSelBrd+"_"+std::to_string(iv))->mask);
    //upperBits = ConvertSignedInt(wib->Read("PWR_MES_OUT_V"));
    upperBits = (wib->Read("PWR_MES_OUT_V"));
    lowerBits = ConvertSignedInt(wib->Read("PWR_MES_OUT_C_TEMP"));

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
  uint32_t linkStatBits = wib->Read("LINK_SYNC_STATUS_BRD"+std::to_string(FEMB)); 
  //for(int i=0; i<4; i++) LINK_STATUS[FEMB][i] = 0;
  LINK_STATUS[iFEMB][0] = ( 0x3  & linkStatBits );     
  LINK_STATUS[iFEMB][1] = ( 0xC  & linkStatBits );     
  LINK_STATUS[iFEMB][2] = ( 0x30 & linkStatBits );     
  LINK_STATUS[iFEMB][3] = ( 0xC0 & linkStatBits );     

  // Get 4 equalizer statuses (1 bit per link)
  uint32_t eqStatBits = wib->Read("EQ_LOS_BRD"+std::to_string(FEMB)+"_RX");
  //for(int i=0; i<4; i++) EQUALIZER_STATUS[FEMB][i] = 0;
  EQUALIZER_STATUS[iFEMB][0] = ( 0x1 & eqStatBits );     
  EQUALIZER_STATUS[iFEMB][1] = ( 0x2 & eqStatBits );     
  EQUALIZER_STATUS[iFEMB][2] = ( 0x4 & eqStatBits );     
  EQUALIZER_STATUS[iFEMB][3] = ( 0x8 & eqStatBits );     

  // Get checksum error, timestamp, timestmap error, frame error
  // (these all require using 0x12 to set the FEMB and the link)
  wib->Write("FEMB_STAT_SEL",uint32_t(iFEMB));
  for(int i=0; i<4; i++){
    wib->Write("LINK_STAT_SEL",uint32_t(i));
    CHKSUM_ERROR_COUNT[iFEMB][i]  = wib->Read("CHKSUM_ERROR");
    TIME_STAMP[iFEMB][i]          = wib->Read("TIME_STAMP");
    TS_ERROR_COUNT[iFEMB][i]      = wib->Read("TS_ERROR"); 
    FRAME_ERROR_COUNT[iFEMB][i]   = wib->Read("FRAME_ERROR"); 
  }

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
//  std::string sel = pwrMesSel; 
//  sel.append("_WIB_VCC_TEMP");
  wib->Write(pwrMesSel,wib->GetItem(pwrMesSel+"_WIB_VCC_TEMP")->mask);
  
  uint32_t upperBits = (wib->Read("PWR_MES_OUT_V"));
  uint32_t lowerBits = ConvertSignedInt(wib->Read("PWR_MES_OUT_C_TEMP"));
  WIB_VCC  = double(upperBits) * 305.18e-6 + 2.5;
  WIB_TEMP = double(lowerBits) * 0.0625;
  
  // get voltages and currents
  for(int i=0; i<4; i++){
    WIB_V[i]=0;
    WIB_C[i]=0;
    uint8_t iv = i+1;
//    sel = pwrMesSel;
//    sel.append("_WIB_");
//    sel.append(1,'0'+iv);
    wib       ->Write(pwrMesSel,wib->GetItem((pwrMesSel+"_WIB_"+std::to_string(iv)).c_str())->mask);
    upperBits = (wib->Read("PWR_MES_OUT_V"));
    lowerBits = ConvertSignedInt(wib->Read("PWR_MES_OUT_C_TEMP"));
    WIB_V[i]              = double(upperBits) * 3.0518/10. * 1e-3; // convert mV to V
    if( iv==3 ) WIB_C[i]  = double(lowerBits) * 1.9075 * 1e-3; // convert mA -> A
    else        WIB_C[i]  = double(lowerBits) * 190.75 * 1e-6; // convert uA --> A
  }

}


//void WIBTool::WIBStatus::Process(std::string const & singleTable){  
void WIBTool::WIBStatus::Process(std::string const & singleTable){  
  // Main control function for the 'status' command in wibtools.
  // Need to output:
  //  [x] FEMB V and C readout for each board (PWR_MES_SEL, read out by 0x06 PWR_MES_OUT 31:0)
  //  [-] WIB parameters (Vcc, V1-4)
  //  [-] FE Vcc / temp (what is this?)
  //  [-] Bias Vcc / temp 
  //  [-] FEMB link status
 
  //Build WIB tables
  //std::vector<std::string> WIBNames = wib->GetNames("*"); 
  std::cout<<singleTable<<"\n";
  
  //Prompt measurements
  StartPowerMes();
 
  //Get FEMB and WIB power measurements
  ProcessWIB();
  for(uint8_t i=1; i<=FEMB_COUNT;i++) ProcessFEMB(i);


  // =======================================================================================
  // Print the power / temperature monitoring table for WIB and FEMBs
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
  // =======================================================================================



  // =======================================================================================
  // Link status table
  printf("\n\n%20s","");    
  for(uint8_t i=0;i<FEMB_COUNT;i++) { printf("%10s",("FEMB_"+std::to_string(i+1)).c_str());  }           
  printf("\n");
  printf(" ===================");   
  for(uint8_t i=0;i<FEMB_COUNT;i++) printf("==========");      
  printf("\n");
  
  for(int iLink=0; iLink<4; iLink++){

    printf("%20s",("HS LINK "+std::to_string(iLink+1)).c_str());
    printf("\n");

    printf("%20s","SYNC STATUS:");
    for(uint8_t i=0;i<FEMB_COUNT;i++) printf("%10d",LINK_STATUS[i][iLink]);
    printf("\n");   
    
    printf("%20s","EQUALIZER STATUS:");
    for(uint8_t i=0;i<FEMB_COUNT;i++) printf("%10d",EQUALIZER_STATUS[i][iLink]);
    printf("\n");   
    
    printf("%20s","TIMESTAMP:");
    for(uint8_t i=0;i<FEMB_COUNT;i++) printf("%10d",TIME_STAMP[i][iLink]);
    printf("\n");   
    
    printf("%20s","TIMESTAMP ERROR CNT:");
    for(uint8_t i=0;i<FEMB_COUNT;i++) printf("%10d",TS_ERROR_COUNT[i][iLink]);
    printf("\n");   
    
    printf("%20s","CHKSUM ERROR CNT:");
    for(uint8_t i=0;i<FEMB_COUNT;i++) printf("%10d",CHKSUM_ERROR_COUNT[i][iLink]);
    printf("\n");   
    
    printf("%20s","FRAME ERROR CNT:");
    for(uint8_t i=0;i<FEMB_COUNT;i++) printf("%10d",FRAME_ERROR_COUNT[i][iLink]);
    printf("\n");   
    
    printf(" -------------------\n");
  }
  // =======================================================================================





  printf("\n\n");
}

void WIBTool::WIBStatus::StartPowerMes(){
  // disable filter
  wib->Write("FILTER_EN",0x0);
  // repeatedly toggle bit 16 to enable LTC2991 CMS conversion
  for(int i=0; i<100; i++){
    wib->Write("PWR_MES_START",0x0);
    wib->Write("PWR_MES_START",0x1); 
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


