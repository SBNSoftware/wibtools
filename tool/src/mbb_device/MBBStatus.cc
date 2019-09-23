#include <mbb_device/MBBStatus.hh>

void WIBTool::MBBStatus::ProcessPTC(uint8_t crate){

    int   icrate = crate-1; // crate index (0-3)
    CRATE_PWR[icrate]   = 0;
    CRATE_PULSE_SRC  = 0;
    PERIOD = 0;
    CRATE_PULSE_PERIOD   = 0;
    CRATE_CLK_TYPE = 0;
    CRATE_CLK_STATUS = 0;

    const Item *g = mbb->GetItem("PTC_CRATE_ADDRESS");
    
    // check if board is on
    if( (g->mask & mbb->Read(g->address))==g->mask ){
         CRATE_PWR[icrate]=1;
         }

    // get type of pulse, pulse period, Clock type and Clock Status.
    CRATE_PULSE_SRC = mbb->Read("PULSE_SRC_SELECT");
    PERIOD = mbb->Read("PULSE_PERIOD");
    CRATE_PULSE_PERIOD = PERIOD*10;
    CRATE_CLK_TYPE = mbb->Read("PLL_ACTIVE_CLK");
    CRATE_CLK_STATUS = mbb->Read("PLL_CLK_STATUS");

    // getting the power status of 6 WIBs
    for(int icrate=0; icrate<=3; icrate++){
        for(int i=1; i<=6; i++){
            WIB_PWR[i]=0;
            WIB_PWR[i] = mbb->Read("PTC_DATA");
            }
    }
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
  
  //get the PTC power measurements
  for(uint8_t i=1; i<=CRATE_COUNT;i++){
      ProcessPTC(i);
      }
 
  // Printing the MBB table
  // ========================================================================================================================================
  
  printf("\n\n%13s","PTC Num:"); for(uint8_t i=0;i<CRATE_COUNT;i++) printf("%13d",i+1); printf("\n");
  printf("   =========="); for(uint8_t i=0;i<CRATE_COUNT;i++) printf("============="); printf("\n");
  printf("%13s","ON/OFF"); for(uint8_t i=0;i<CRATE_COUNT;i++) printf("%13d",CRATE_PWR[i]); printf("\n");
  printf("%13s","PULSE TYPE"); for(uint8_t i=0;i<CRATE_COUNT;i++) printf("%13d",CRATE_PULSE_SRC); printf("\n");
  printf("%13s","PLS PERIOD ns"); for(uint8_t i=0;i<CRATE_COUNT;i++) printf("%13d",CRATE_PULSE_PERIOD); printf("\n");
  printf("%13s","CLOCK TYPE"); for(uint8_t i=0;i<CRATE_COUNT;i++) printf("%13d",CRATE_CLK_TYPE); printf("\n");
  printf("%13s","CLOCK STATUS"); for(uint8_t i=0;i<CRATE_COUNT;i++) printf("%13d",CRATE_CLK_STATUS);  printf("\n");
  printf("============================================================================================");


  //=========================================================================================================================================    
      
  for(uint8_t i=0;i<CRATE_COUNT;i++){ 
      printf("\n\n%13s","PTC Num:");
      printf("%13d",i+1);             
      printf("\n");
      printf("\n%13s","WIB Num:"); for(int iv=0; iv<6; iv++) printf("%13d",iv+1); printf("\n");
      printf("%13s","ON/OFF");      for(int iv=0; iv<6; iv++)  printf("%13d",WIB_PWR[iv]);  printf("\n\n");
      printf("============================================================================================");
      }

  //=========================================================================================================================================

  printf("\n\n");
    
}
        
