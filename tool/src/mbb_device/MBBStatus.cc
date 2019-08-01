#include <mbb_device/MBBStatus.hh>
//#include <MBBException.hh>

void WIBTool::MBBStatus::ProcessPTC(uint8_t crate)
  {
    int   icrate = crate-1; // crate index (0-3)
    CRATE_PWR[icrate]   = 0;
    CRATE_PULSE_SRC[icrate]  = 0;
    CRATE_PULSE_PERIOD[icrate]   = 0;
    CRATE_WR_REG[icrate] = 0;

    std::string regName("PTC_CRATE_ADDRESS");
    //regName.append(1,'0'+FEMB); there is no point in appending regname beacuse unlike WIBStatus the table doesn't contain individual data sets.
    const Item *g = mbb->GetItem(regName);
    

    // check if board is on
    if( (g->mask & mbb->Read(g->address))==g->mask )
      {
       CRATE_PWR[icrate]=1;
      }

    // get type of pulse, pulse period and whether data is being sent to PTC.
    CRATE_PULSE_SRC[icrate] = mbb->Read("PULSE_SRC_SELECT");
    CRATE_PULSE_PERIOD[icrate] = mbb->Read("PULSE_PERIOD");
    CRATE_WR_REG[icrate] = mbb->Read("PTC_WR_REG");

    // getting the power status of 6 WIBs
    for(int i=1; i<=6; i++)
      {
       WIB_PWR[i]=0;
       WIB_PWR[i] = mbb->Read("PTC_DATA");
      } 
    
  }

void WIBTool::MBBStatus::Process(std::string const & singleTable){
  // Main control function for the 'status' command in wibtools.              
  // Will output:                                                          
  // PTC power status (read from Reg 0x08)                                
  // PTC PULSE TYPE AND PULSE PERIODFEMB
  // WHETHER DATA IS BEING SENT TO PTC
  // WIB POWER STATUS

  //Build MBB tables                                                          
  std::vector<std::string> MBBNames = mbb->GetNames("*");
  std::cout<<singleTable<<"\n";

  std::cout<<"Building MBB table\n";
  for(uint8_t i=1; i<=CRATE_COUNT;i++)
   {
    ProcessPTC(i);
   }
 
  // Printing the MBB table
  // ========================================================================================================================================
  // char label[100];
  printf("\n\n%13s","PTC Num:"); for(uint8_t i=0;i<CRATE_COUNT;i++) printf("%13d",i+1);             printf("\n");
  printf("   ==========");      for(uint8_t i=0;i<CRATE_COUNT;i++) printf("=============");      printf("\n");
  printf("%13s","ON/OFF");        for(uint8_t i=0;i<CRATE_COUNT;i++) printf("%13d",CRATE_PWR[i]);     printf("\n");
  printf("%13s","PULSE TYPE");      for(uint8_t i=0;i<CRATE_COUNT;i++) printf("%13lf",CRATE_PULSE_SRC[i]);  printf("\n");
  printf("%13s","PULSE PERIOD");       for(uint8_t i=0;i<CRATE_COUNT;i++) printf("%13lf",CRATE_PULSE_PERIOD[i]);   printf("\n");
  printf("%13s","WHETHER DATA IS BEING SENT TO PTC");       for(uint8_t i=0;i<CRATE_COUNT;i++) printf("%13lf",CRATE_WR_REG[i]);   printf("\n");

  printf("\n\n%13s","PTC Num:"); 
  for(uint8_t i=0;i<CRATE_COUNT;i++)
    { 
	  printf("%13d",i+1);             
          printf("\n");
          for(int iv=0; iv<6; iv++)
	   {
             printf("\n\n%13s","WIB Num:");
             printf("%13d",iv+1);
	     printf("\n");
	     printf("%13s","ON/OFF");        printf("%13lf",WIB_PWR[iv]);     printf("\n"); 
           }
    }

  // ========================================================================================================================================

  printf("\n");

}
        
