#include <wib_device/WIBDevice.hh>
#define __STDC_FORMAT_MACROS
#include <inttypes.h> // for PRI macros


CommandReturn::status WIBTool::WIBDevice::Read_DTS_SI5344_I2C(std::vector<std::string> strArg,std::vector<uint64_t> intArg){
  (void) strArg; // to make compiler not complain about unused args
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
  uint32_t val = wib->Read_SI5344(address,byte_count);
  printf("I2C:%"PRIX16": 0x%0*X\n",address,2*byte_count,val);      
  return CommandReturn::OK;
}


CommandReturn::status WIBTool::WIBDevice::Write_DTS_SI5344_I2C(std::vector<std::string> strArg,std::vector<uint64_t> intArg){
  (void) strArg; // to make compiler not complain about unused args
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
  wib->Write_SI5344(address,val,byte_count);
  printf("I2C:%"PRIX16": 0x%0*X\n",address,2*byte_count,val);      
  return CommandReturn::OK;
}

CommandReturn::status WIBTool::WIBDevice::Write_DTS_SI5344_Config(std::vector<std::string> strArg,std::vector<uint64_t> intArg){
  (void) intArg; // to make compiler not complain about unused args
  if(strArg.size() == 0){
    wib->LoadConfig_SI5344("default");
  } else {
    wib->LoadConfig_SI5344(strArg[0]);
  }
  return CommandReturn::OK;
}


CommandReturn::status WIBTool::WIBDevice::Write_DTS_SI5344_Page(std::vector<std::string> strArg,std::vector<uint64_t> intArg){
  (void) strArg; // to make compiler not complain about unused args
  if(intArg.size() == 0){
    return CommandReturn::BAD_ARGS;    
  }
  wib->Set_SI5344Page(intArg[0]);
  return CommandReturn::OK;
}
CommandReturn::status WIBTool::WIBDevice::Read_DTS_SI5344_Page(std::vector<std::string> /*strArg*/,std::vector<uint64_t> /*intArg*/){
  printf("SI5344 page: %u\n",wib->Get_SI5344Page());
  return CommandReturn::OK;
}	   

CommandReturn::status WIBTool::WIBDevice::ResetSI5344(std::vector<std::string> /*strArg*/,std::vector<uint64_t> /*intArg*/){
  wib->ResetSi5344();
  return CommandReturn::OK;
}

CommandReturn::status WIBTool::WIBDevice::SelectSI5344(std::vector<std::string> /*strArg*/,std::vector<uint64_t> intArg){

  if(intArg.size() != 2) return CommandReturn::BAD_ARGS;
  if(intArg[0] > 3) return CommandReturn::BAD_ARGS;
  wib->SelectSI5344(intArg[0],bool(intArg[1]));
  
  return CommandReturn::OK;
}


