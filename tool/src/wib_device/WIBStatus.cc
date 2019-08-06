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

  // set bit 16 to enable LTC2991 CMS conversion operation.
  // disable bit 17 to turn off filter.
  wib->Write("PWR_MES_START",0x1);
  wib->Write("FILTER_EN",0x0);

  // get temp and Vcc
  std::string sel = pwrMesSelBrd; 
  sel.append("_VCC_TEMP");
  //std::cout<<pwrMesSelBrd<<"\n";
  wib->Write(pwrMesSel,wib->GetItem(sel)->mask);
  
  wib->Write("PWR_MES_START",0x0);
  wib->Write("PWR_MES_START",0x1);

  /*
  std::cout<<"\nGetting Vcc and Temp for board "<<int(FEMB)<<":\n";
  std::cout<<"  setting reg 5: "<<std::hex<<wib->Read(0x05)<<std::dec<<"\n";
  std::cout<<"  reading reg 6: "<<std::hex<<wib->Read(0x06)<<std::dec<<"\n";
  std::cout<<"  - upper bits : "<<wib->Read("PWR_MES_OUT_V")<<"\n";
  std::cout<<"  - lower bits : "<<wib->Read("PWR_MES_OUT_C_TEMP")<<"\n";
  */

  // get temperature measurement by taking 10 reads and 
  // throwing out the obvious wrong ones (> ~500) 
 // double sumT = 0;
 // int N = 10;
 // int n = 0;
 // for(int i=0; i<N; i++) {
 //   double T = MeasureTemp(wib->Read("PWR_MES_OUT_C_TEMP"));
 //   std::cout<<"T = "<<T<<"\n";
 //   if( T < 500. ) { 
 //     sumT += T; n++; 
 //   } else {
 //     std::cout<<"!!! T = "<<T<<"\n";
 //   wib->Write("PWR_MES_START",0x0);
 //   wib->Write("PWR_MES_START",0x1);
 //     usleep(1000);
 //   }
 // }
  FEMB_TEMP[iFEMB] = MeasureTemp(wib->Read("PWR_MES_OUT_C_TEMP"));
  FEMB_VCC[iFEMB]   = wib->Read("PWR_MES_OUT_V") * 305.18e-6 + 2.5;
  //FEMB_TEMP[iFEMB]  = wib->Read("PWR_MES_OUT_C_TEMP") * 0.0625; 
  
  // get voltages and currents
  for(int i=0; i<6; i++){
    FEMB_V[iFEMB][i]=0;
    FEMB_C[iFEMB][i]=0;
    uint8_t iv = i+1;
    sel = pwrMesSelBrd;
    sel.append("_");
    sel.append(1,'0'+iv);
    wib->Write(pwrMesSel,wib->GetItem(sel)->mask);
    wib->Write("PWR_MES_START",0x0);
    wib->Write("PWR_MES_START",0x1);
    /*
    std::cout<<"Getting V/C "<<i+1<<" ("<<sel<<")\n";
    std::cout<<"  setting reg 5: "<<std::hex<<wib->Read(0x05)<<std::dec<<"\n";
    std::cout<<"  reading reg 6: "<<std::hex<<wib->Read(0x06)<<std::dec<<"\n";
    std::cout<<"  - upper bits : "<<wib->Read("PWR_MES_OUT_V")<<"\n";
    std::cout<<"  - lower bits : "<<wib->Read("PWR_MES_OUT_C_TEMP")<<"\n";
    */

    // zero out bit 0x2000 to ignore signed component
    //
    // check if bit 0x2000 is set; if so, zero out whole value
    uint32_t readout_u = (wib->Read("PWR_MES_OUT_V"));
    uint32_t readout_l = (wib->Read("PWR_MES_OUT_C_TEMP"));
    
    /*if( (0x2000 & readout_u) != 0x0 ) {
      std::cout<<"Bit 0x2000 is set: "<<std::hex<<readout_u<<std::dec<<"\n";
      readout_u = 0x0;
    }
    
    if( (0x2000 & readout_l) != 0x0 ) readout_l = 0x0;
    */

    FEMB_V[iFEMB][i]  = readout_u * 3.0518/10. * 1e-3; // convert mV to V
    if( iv==3 ) FEMB_C[iFEMB][i]=readout_l * 1.9075; // already in mA
    else        FEMB_C[iFEMB][i]=readout_l * 190.75 * 1e-3; // convert uA to mA
   
    // check for high values, indicating negative bit is set
    //if( FEMB_C[iFEMB][i] > 3000. ) FEMB_C[iFEMB][i] = 0.; 
  }

  /*
  //Append this FEMB to the name
  std::string FEMBName("FEMB");        
  FEMBName.append(1,'0' +iFEMB);
  FEMBName.append(1,'.');

  std::cout<<"ProcessFEMB --> "<<FEMBName<<"\n";

  //Build FEMB tables
  std::vector<std::string> FEMBNames = wib->GetFEMBNames("*"); 
  //process all the nodes and build table structure
  for(std::vector<std::string>::iterator itName = FEMBNames.begin();
      itName != FEMBNames.end();
      itName++){
    //Get the list of parameters for this node
    Item const * item = wib->GetFEMBItem(iFEMB,*itName);
    boost::unordered_map<std::string,std::string> parameters = item->user;
    //Look for a Status parameter
    if(parameters.find("Status") != parameters.end()){	
      //Check for an enabled argument
      boost::unordered_map<std::string,std::string>::iterator itEnabled = parameters.find("Enabled");
      if(itEnabled != parameters.end()){
	if((itEnabled->second)[0] == '!'){
	  //We hvae the negative of our normal control
	  itEnabled->second = (itEnabled->second).substr(1); //Strip the '!'
	  itEnabled->second = (wib->ReadFEMB(iFEMB,itEnabled->second) == 0x0) ? "1" : "0";	  
	}else{
	  //Change the value to 0 or 1 depending on if it should be enabled
	  itEnabled->second = (wib->ReadFEMB(iFEMB,itEnabled->second) == 0x0) ? "0" : "1";
	}
      }
      
      //Check for an table name
      boost::unordered_map<std::string,std::string>::iterator itTable = parameters.find("Table");
       
      
      std::string tableName = itTable->second;
      //Add this Address to our Tables if it matches our singleTable option, or we are looking at all tables
      if( singleTable.empty() || TableNameCompare(tableName,singleTable)){
	tables[tableName].Add(FEMBName + *itName,
			      wib->ReadFEMB(iFEMB,*itName),
			      uint32_t(item->mask >> item->offset),
			      parameters);
      }
    }
  }
  */
}



void WIBTool::WIBStatus::Process(std::string const & singleTable){  
  // Main control function for the 'status' command in wibtools.
  // Need to output:
  //  x FEMB power status (read from Reg 0x08)
  //  - FEMB V and C readout for each board (PWR_MES_SEL, read out by 0x06 PWR_MES_OUT 31:0)
  //  - FEMB link status
  //  - ???

 
  //Build WIB tables
  std::vector<std::string> WIBNames = wib->GetNames("*"); 
  std::cout<<singleTable<<"\n";
 
  //Build FEMB tables
  std::cout<<"Building FEMB table\n";
  for(uint8_t i=1; i<=FEMB_COUNT;i++) {
    ProcessFEMB(i);
  }

  // =======================================================================================
  // Print the FEMB table
  char label[100];
  printf("\n\n%13s","FEMB Num:"); for(uint8_t i=0;i<FEMB_COUNT;i++) printf("%13d",i+1);             printf("\n");
  printf("   ==========");      for(uint8_t i=0;i<FEMB_COUNT;i++) printf("=============");      printf("\n");
  printf("%13s","ON/OFF");        for(uint8_t i=0;i<FEMB_COUNT;i++) printf("%13d",FEMB_PWR[i]);     printf("\n");
  printf("%13s","TEMP [C]");      for(uint8_t i=0;i<FEMB_COUNT;i++) printf("%13.2f",FEMB_TEMP[i]);  printf("\n");
  printf("%13s","Vcc [V]");       for(uint8_t i=0;i<FEMB_COUNT;i++) printf("%13.3f",FEMB_VCC[i]);   printf("\n");
  for(int iv=0; iv<6; iv++){
    sprintf(label,"V%d  [V]",iv+1);
    printf("%13s",label);        for(uint8_t i=0;i<FEMB_COUNT;i++) printf("%13.3f",FEMB_V[i][iv]); printf("\n");
    sprintf(label,"C%d [mA]",iv+1);
    printf("%13s",label);        for(uint8_t i=0;i<FEMB_COUNT;i++) printf("%13.3f",FEMB_C[i][iv]); printf("\n");
  }

  // =======================================================================================

  printf("\n");

  /*

  //process all the nodes and build table structure
  for(std::vector<std::string>::iterator itName = WIBNames.begin();
      itName != WIBNames.end();
      itName++){

    //Get the list of parameters for this node
    Item const * item = wib->GetItem(*itName);
    std::cout<<"item "<<item->name<<"\n";
    boost::unordered_map<std::string,std::string> parameters = item->user;
    
    //Look for a Status parameter
    if(parameters.find("Status") != parameters.end()){	
      std::cout<<"  Status parameter found\n";

      //Check for an enabled argument
      boost::unordered_map<std::string,std::string>::iterator itEnabled = parameters.find("Enabled");
      if(itEnabled != parameters.end()){
	if((itEnabled->second)[0] == '!'){
	  //We hvae the negative of our normal control
	  itEnabled->second = (itEnabled->second).substr(1); //Strip the '!'
	  itEnabled->second = (wib->Read(itEnabled->second) == 0x0) ? "1" : "0";	  
	}else{
	  //Change the value to 0 or 1 depending on if it should be enabled
	  itEnabled->second = (wib->Read(itEnabled->second) == 0x0) ? "0" : "1";
	}
      }
      
      //Check for an table name
      boost::unordered_map<std::string,std::string>::iterator itTable = parameters.find("Table");
      std::string tableName = itTable->second;
      
      //Add this Address to our Tables if it matches our singleTable option, or we are looking at all tables
      if( singleTable.empty() || TableNameCompare(tableName,singleTable)){
        std::cout<<"Adding to table\n";
	tables[tableName].Add(*itName,
			      wib->Read(*itName),
			      uint32_t(item->mask >> item->offset),
			      parameters);
      }
    }
  }  



  //Build FEMB tables
  for(uint8_t iFEMB = 1; iFEMB <= 4;iFEMB++){    
    std::string regName("PWR_EN_BRD"); 
    regName.append(1,'0' +iFEMB);
    std::cout<<regName<<"\n";
  
    // Read the item and get its bit mask 
    const Item *g = wib->GetItem(regName);
    
    //Only look at FEMBs that are powered up
    //TODO: need to use specific register masks from table
    //      that indicate board is on
    //if(0x1F == wib->Read(regName)){
    if(g->mask == wib->Read(regName)){
      std::cout<<" powered up\n";
      //Powered up, add to tables
      ProcessFEMB(iFEMB,singleTable);
    }
  }
  */
}

double WIBTool::WIBStatus::MeasureTemp(uint32_t in){
  return 0.0625*double(in);
}
