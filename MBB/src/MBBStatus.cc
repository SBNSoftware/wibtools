#include <MBBStatus.hh>
#include <MBB.hh>
#include <iostream>

void WIBTool::MBBStatus::ProcessPTC(uint8_t crate){
    CRATE_PULSE_SRC  = 0;
    PERIOD = 0;
    CRATE_PULSE_PERIOD   = 0;
    CRATE_CLK_TYPE = 0;
    CRATE_CLK_STATUS = 0;

    // check if board is ON. PTC_CRATE_ADDRESS will be compared with PTC dipswitch that you put manually. Leave for now.
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
   
    //Calling WIB Power values stored in configMBB and storing them to print out later.
    p11 = mbb->p1;
    p22 = mbb->p2;
    p33 = mbb->p3;
    p44 = mbb->p4;

    // get everything else
    FIRMWARE_VER  = mbb->Read("FIRMWARE_VERSION");
    FIRMWARE_TRK  = mbb->Read("FIRMWARE_TRACKER");
    COMPILATION_DT= mbb->Read("COMPILATION_DATE");
    COMPILATION_TM= mbb->Read("COMPILATION_TIME");
 }

void WIBTool::MBBStatus::Process(std::string const & singleTable){
  // Main control function for the 'status' command in wibtools.              
  // Will output:                                                          
  // PTC power status (read from Reg 0x08)                                
  // PTC PULSE TYPE AND PULSE PERIOD
  // Clock type
  // WIB POWER STATUS

  //Build MBB tables                                                          
  std::cout<<singleTable<<"\n";
  std::cout<<"Building MBB table\n";
  
  //Storing and printing the values.
  for(uint8_t i=1; i<=CRATE_COUNT;i++){
      ProcessPTC(i);
      }
  // Printing the MBB table
  // ========================================================================================================================================
  printf("   ======================================================================================================="); printf("\n");


  printf("%20s","PULSE TYPE:");  if(CRATE_PULSE_SRC==0x0) {printf("%20s","LEMO");}
                                 if(CRATE_PULSE_SRC==0x1) {printf("%20s","MBB");} printf("\n");

  printf("%20s","PLS PERIOD ns:"); printf("%20d",CRATE_PULSE_PERIOD); printf("\n");

  printf("%20s","CLOCK TYPE");   if(CRATE_CLK_TYPE==0x0) {printf("%20s","External");}
                                 if(CRATE_CLK_TYPE==0x1) {printf("%20s","Internal");} printf("\n");

  printf("%20s","CLOCK STATUS"); if(CRATE_CLK_STATUS==0x0) {printf("%20s","Good");}
                                 if(CRATE_CLK_STATUS==0x1) {printf("%20s","Bad");}
                                 if(CRATE_CLK_STATUS==0x4) {printf("%20s","External");}
                                 if(CRATE_CLK_STATUS==0x5) {printf("%20s","Internal");} printf("\n");
  printf("   ======================================================================================================="); printf("\n");


  printf("%20s","FW VERSION:");         printf("%20x",FIRMWARE_VER);   printf("\n");
  printf("%20s","FW TRACKER:");         printf("%20x",FIRMWARE_TRK);   printf("\n");
  printf("%20s","COMPILATION DATE:");   printf("%20x",COMPILATION_DT); printf("\n");
  printf("%20s","COMPILATION TIME:");   printf("%20x",COMPILATION_TM); printf("\n");
  printf("   ======================================================================================================="); printf("\n");
  
  printf("\n%20s","PTC Num:"); printf("%14d",1);
  printf("\n%20s","WIB Num:"); for(uint8_t z=0;z<WIB_COUNT;z++) printf("%14d",z+1); printf("\n");
  printf("   ======================================================================================================="); printf("\n");
  printf("%20s","ON/OFF");
  int m=6;
  while(m>0){
	if(p11&1) printf("%14d", 0);
	else printf("%14d" ,1);
	p11>>=1;
	m--;
	}
  printf("\n");
  printf("   ======================================================================================================="); printf("\n\n\n");
      
  printf("\n%20s","PTC Num:"); printf("%14d",2);
  printf("\n%20s","WIB Num:"); for(uint8_t z=0;z<WIB_COUNT;z++) printf("%14d",z+1); printf("\n");
  printf("   ======================================================================================================="); printf("\n");
  printf("%20s","ON/OFF");
  int n=6;
  while(n>0){
	if(p22&1) printf("%14d", 0);
	else printf("%14d" ,1);
	p22>>=1;
	n--;
	}
  printf("\n");
  printf("   ======================================================================================================="); printf("\n\n\n");
       
  printf("\n%20s","PTC Num:"); printf("%14d",3);
  printf("\n%20s","WIB Num:"); for(uint8_t z=0;z<WIB_COUNT;z++) printf("%14d",z+1); printf("\n");
  printf("   ======================================================================================================="); printf("\n");
  printf("%20s","ON/OFF");
  int o=6;
  while(o>0){
	if(p33&1) printf("%14d", 0);
	else printf("%14d" ,1);
	p33>>=1;
	o--;
	}
  printf("\n");
  printf("   ======================================================================================================="); printf("\n\n\n");
       
  printf("\n%20s","PTC Num:"); printf("%14d",4);
  printf("\n%20s","WIB Num:"); for(uint8_t z=0;z<WIB_COUNT;z++) printf("%14d",z+1); printf("\n");
  printf("   ======================================================================================================="); printf("\n");
  printf("%20s","ON/OFF");
  int p=6;
  while(p>0){
	if(p44&1) printf("%14d", 0);
	else printf("%14d" ,1);
	p44>>=1;
	p--;
	}
  printf("\n");
  printf("   ======================================================================================================="); printf("\n\n\n");
}

        
