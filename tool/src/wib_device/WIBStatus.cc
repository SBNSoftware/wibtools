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
    
  std::string regName("PWR_EN_BRD"); 
  regName.append(1,'0'+FEMB);
  const Item *g = wib->GetItem(regName);

  // reset register
  wib->Write(0x05,0x0);

  // check if board is on
  if( (g->mask & wib->Read(g->address))==g->mask ) FEMB_PWR[iFEMB]=1;
 
  std::string pwrMesSel("PWR_MES_SEL");
  std::string pwrMesSelBrd = pwrMesSel; 
  pwrMesSelBrd.append("_BRD");
  pwrMesSelBrd.append(1,'0'+FEMB);


  // get temp and Vcc
  std::string sel = pwrMesSelBrd; 
  sel.append("_VCC_TEMP");
  wib->Write(pwrMesSel,wib->GetItem(sel)->mask);

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
    sel = pwrMesSelBrd;
    sel.append("_");
    sel.append(1,'0'+iv);
    wib       ->Write(pwrMesSel,wib->GetItem(sel)->mask);
    //upperBits = ConvertSignedInt(wib->Read("PWR_MES_OUT_V"));
    upperBits = (wib->Read("PWR_MES_OUT_V"));
    lowerBits = ConvertSignedInt(wib->Read("PWR_MES_OUT_C_TEMP"));

    /*
    if( FEMB == 1 || FEMB == 2) {
    std::cout
    <<"Getting V/C "<<i+1<<" ("<<sel<<")\n"
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
    if( FEMB_PWR[iFEMB] == 0 && upperBits == 0x3fff ) FEMB_V[iFEMB][i] = 0.;
  }

}

void WIBTool::WIBStatus::ProcessWIB(){  

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
 
  //Get FEMB measurements
  for(uint8_t i=1; i<=FEMB_COUNT;i++) ProcessFEMB(i);

  //Get WIB measurements
  //ProcessWIB();

  

  // =======================================================================================
  // Print the FEMB table
  char label[100];
  printf("\n\n%10s","FEMB:"); for(uint8_t i=0;i<FEMB_COUNT;i++) printf("%10d",i+1);             printf("\n");
  printf(" =========");      for(uint8_t i=0;i<FEMB_COUNT;i++) printf("==========");      printf("\n");
  printf("%10s","ON/OFF");        for(uint8_t i=0;i<FEMB_COUNT;i++) printf("%10d",FEMB_PWR[i]);     printf("\n");
  printf("%10s","TEMP [C]");      for(uint8_t i=0;i<FEMB_COUNT;i++) printf("%10.2f",FEMB_TEMP[i]);  printf("\n");
  printf("%10s","Vcc [V]");       for(uint8_t i=0;i<FEMB_COUNT;i++) printf("%10.2f",FEMB_VCC[i]);   printf("\n");
  for(int iv=0; iv<6; iv++){
    printf("   -------");      for(uint8_t i=0;i<FEMB_COUNT;i++) printf("----------");      printf("\n");
    sprintf(label,"V%d [V]",iv+1);
    printf("%10s",label);        for(uint8_t i=0;i<FEMB_COUNT;i++) printf("%10.2f",FEMB_V[i][iv]); printf("\n");
    sprintf(label,"C%d [A]",iv+1);
    printf("%10s",label);        for(uint8_t i=0;i<FEMB_COUNT;i++) printf("%10.3f",FEMB_C[i][iv]); printf("\n");
  }
  printf("\n");
  // =======================================================================================

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


