#include <MBBStatus.hh>
#include <MBB.hh>
#include <iostream>

void WIBTool::MBBStatus::ProcessPTC(){
  CRATE_PULSE_SRC = 0;
  PERIOD = 0;
  CRATE_PULSE_PERIOD = 0;
  CRATE_CLK_TYPE = 0;
  CRATE_CLK_STATUS = 0;

  //check if board is ON. PTC_CRATE_ADDRESS will be compared with PTC dipswitch that you put manually. Leave for now.
  /*for(int icrate=0; icrate<4; icrate++){
        CRATE_PWR[icrate]   = 0;
        const Item *g = mbb->GetItem("PTC_CRATE_ADDRESS");
        if( (g->mask & mbb->Read(g->address))==g->mask ){
             CRATE_PWR[icrate]=1;
             }
	     }*/

  // get type of pulse, pulse period, Clock type, Clock Status.
  CRATE_PULSE_SRC = mbb->Read("PULSE_SRC_SELECT");
  PERIOD = mbb->Read("PULSE_PERIOD");
  CRATE_PULSE_PERIOD = PERIOD*10;
  CRATE_CLK_TYPE = mbb->Read("PLL_ACTIVE_CLK");
  CRATE_CLK_STATUS = mbb->Read("PLL_CLK_STATUS");

  // get everything else
  FIRMWARE_VER   = mbb->Read("FIRMWARE_VERSION");
  FIRMWARE_TRK   = mbb->Read("FIRMWARE_TRACKER");
  COMPILATION_DT = mbb->Read("COMPILATION_DATE");
  COMPILATION_TM = mbb->Read("COMPILATION_TIME");
}

void WIBTool::MBBStatus::Process(std::string const & singleTable){
  // Main control function for the 'status' command in wibtools.              
  // Will output:                                                                                          
  // PTC PULSE TYPE AND PULSE PERIOD
  // Clock type
  // Firmware Version
  // WIB POWER STATUS

  //Build MBB tables                                                          
  std::cout<<singleTable<<"\n";
  std::cout<<"Building MBB table\n";
  
  //Storing and printing the values.
  ProcessPTC();

  // Printing the MBB table
  printf("   ======================================================================================================="); printf("\n");

  printf("%20s","PULSE TYPE:");  if(CRATE_PULSE_SRC == 0x0) {printf("%20s","LEMO");}
                                 if(CRATE_PULSE_SRC == 0x1) {printf("%20s","MBB");} printf("\n");

  printf("%20s","PULSE PERIOD ns:"); printf("%20d",CRATE_PULSE_PERIOD); printf("\n");

  printf("%20s","CLOCK TYPE");   if(CRATE_CLK_TYPE == 0x0) {printf("%20s","External");}
                                 if(CRATE_CLK_TYPE == 0x1) {printf("%20s","Internal");} printf("\n");

  printf("%20s","CLOCK STATUS"); if(CRATE_CLK_STATUS == 0x0) {printf("%20s","Good");}
                                 if(CRATE_CLK_STATUS == 0x1) {printf("%20s","Bad");}  printf("\n");
  printf("   ======================================================================================================="); printf("\n");

  printf("%20s","FW VERSION:");         printf("%20x",FIRMWARE_VER);   printf("\n");
  printf("%20s","FW TRACKER:");         printf("%20x",FIRMWARE_TRK);   printf("\n");
  printf("%20s","COMPILATION DATE:");   printf("%20x",COMPILATION_DT); printf("\n");
  printf("%20s","COMPILATION TIME:");   printf("%20x",COMPILATION_TM); printf("\n");
  printf("   ======================================================================================================="); printf("\n");

  for(uint32_t b = 0; b <= CRATE_COUNT; b++){ 
      power = mbb->wibpoweroncrate[b];
      std::cout<<" *******mask value = "<<power<<std::endl; 
      printf("\n%20s","PTC Num:"); printf("%14d",b);
      printf("\n%20s","WIB Num:"); for(uint8_t z=0;z<WIB_COUNT;z++) printf("%14d",z); printf("\n");
      printf("   ======================================================================================================="); printf("\n");
      printf("%20s","ON/OFF");
      int m=6;
      while(m>0){
	    if(power&1) printf("%14d", 0);
	    else printf("%14d" ,1);
	    power>>=1;
	    m--;
	    }
      printf("\n");
      printf("   ======================================================================================================="); printf("\n\n\n");
      power = 0x3F; //resetting the power value to all OFF.
  }

  // treating the D0 PTC separately.
  power = mbb->wibpoweroncrate[14];
  std::cout<<" *******mask value = "<<power<<std::endl;
  printf("\n%20s","PTC Num(D0 PTC):"); printf("%14d",14);
  printf("\n%20s","WIB Num:"); for(uint8_t z=0;z<WIB_COUNT;z++) printf("%14d",z); printf("\n");
  printf("   ======================================================================================================="); printf("\n");
  printf("%20s","ON/OFF");
  int m=6;
  while(m>0){
        if(power&1) printf("%14d", 0);
        else printf("%14d" ,1);
        power>>=1;
        m--;
        }
  printf("\n");
  printf("   ======================================================================================================="); printf("\n\n\n");
  
}


        
