#include <wib_device/WIBDevice.hh>
#define __STDC_FORMAT_MACROS
#include <inttypes.h> // for PRI macros


CommandReturn::status WIBTool::WIBDevice::Read_DTS_CDS_I2C(std::vector<std::string> strArg,std::vector<uint64_t> intArg){
  (void) strArg; // to make compiler not complain about unused arguments

  if((intArg.size() == 0) || (intArg.size() > 2)){
    return CommandReturn::BAD_ARGS;
  }

  uint8_t byte_count = 1;
  uint16_t address = 0;
  switch (intArg.size()){
  case 2:    
    if(intArg[1] > 4){
      printf("Byte count %"PRIu64" is too big, using %u\n",intArg[1],byte_count);
    }else{
      byte_count = intArg[1];
    }
  case 1:
    if (intArg[0] & (~0xFFFF) ){
      //Check for any bits outside of the 16 bits are set
      printf("Address out of range(16bit)\n");
      return CommandReturn::BAD_ARGS;      
    }
    address = intArg[0];
  }
  uint32_t val = wib->ReadDTS_CDS(address,byte_count);
  printf("I2C:%"PRIX16": 0x%0*X\n",address,2*byte_count,val);      
  return CommandReturn::OK;
}
CommandReturn::status WIBTool::WIBDevice::Write_DTS_CDS_I2C(std::vector<std::string> strArg,std::vector<uint64_t> intArg){
  (void) strArg; // to make compiler not complain about unused arguments

  if((intArg.size() < 2) || (intArg.size() > 3)){
    return CommandReturn::BAD_ARGS;
  }

  uint8_t byte_count = 1;
  uint16_t address = 0;
  uint32_t val = 0;
  switch (intArg.size()){
  case 3:
    //byte count
    if(intArg[2] > 4){
      printf("Byte count %"PRIu64" is too big, using %u\n",intArg[2],byte_count);
    }else{
      byte_count = intArg[2];
    }
  case 2:    

    //value to write
    if(intArg[1] & (~0xFFFFFFFF)){
      //Check for any bits outside of the 32 bits are set
      printf("Value out of range(32bit)\n");
      return CommandReturn::BAD_ARGS;      
    }
    val = intArg[1];

    //addresss
    if (intArg[0] & (~0xFFFF) ){
      //Check for any bits outside of the 16 bits are set
      printf("Address out of range(16bit)\n");
      return CommandReturn::BAD_ARGS;      
    }
    address = intArg[0];
  }
  wib->WriteDTS_CDS(address,val,byte_count);
  printf("I2C:%"PRIX16": 0x%0*X\n",address,2*byte_count,val);      
  return CommandReturn::OK;
}

CommandReturn::status WIBTool::WIBDevice::ConfigureDTSCDS(std::vector<std::string> strArg,std::vector<uint64_t> intArg) {   
  (void) strArg; // to make compiler not complain about unused arguments

  float dataRateHz = -1;
  if(intArg.size() == 0){
    dataRateHz = wib->ConfigureDTSCDS();
  }else{
    if(intArg[0] > 1){
      printf("Bad timing source\n");
      return CommandReturn::BAD_ARGS;
    }
    printf("  Set clock & data source to input %s\n",(intArg[0] == 1) ? "front-panel" : "back-panel");
    dataRateHz = wib->ConfigureDTSCDS(intArg[0]);
  }
  printf("  Resetting DTS CDS\n");
  printf("  Datarate: %fMhz\n",dataRateHz*1E-6);
  return CommandReturn::OK;
}
