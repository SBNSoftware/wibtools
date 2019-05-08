#include <wib_device/WIBDevice.hh>
#define __STDC_FORMAT_MACROS
#include <inttypes.h> // for PRI macros

CommandReturn::status BUTool::WIBDevice::Read_DAQ_SI5342_I2C(std::vector<std::string> strArg,std::vector<uint64_t> intArg){
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
  uint32_t val = wib->ReadDAQ_SI5342(address,byte_count);
  printf("I2C:%"PRIX16": 0x%0*X\n",address,2*byte_count,val);      
  return CommandReturn::OK;
}


CommandReturn::status BUTool::WIBDevice::Write_DAQ_SI5342_I2C(std::vector<std::string> strArg,std::vector<uint64_t> intArg){
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
  wib->WriteDAQ_SI5342(address,val,byte_count);
  printf("I2C:%"PRIX16": 0x%0*X\n",address,2*byte_count,val);      
  return CommandReturn::OK;
}


CommandReturn::status BUTool::WIBDevice::Write_DAQ_SI5342_Config(std::vector<std::string> strArg,std::vector<uint64_t> intArg){
  (void) intArg; // to make compiler not complain about unused arguments
  if(strArg.size() == 0){
    wib->LoadConfigDAQ_SI5342("default");
  } else {
    wib->LoadConfigDAQ_SI5342(strArg[0]);
  }
  return CommandReturn::OK;
}


CommandReturn::status BUTool::WIBDevice::Write_DAQ_SI5342_Page(std::vector<std::string> strArg,std::vector<uint64_t> intArg){
  (void) strArg; // to make compiler not complain about unused arguments
  if(intArg.size() == 0){
    return CommandReturn::BAD_ARGS;    
  }
  wib->SetDAQ_SI5342Page(intArg[0]);
  return CommandReturn::OK;
}
CommandReturn::status BUTool::WIBDevice::Read_DAQ_SI5342_Page(std::vector<std::string> strArg,std::vector<uint64_t> intArg){
  (void) intArg; // to make compiler not complain about unused arguments
  (void) strArg; // to make compiler not complain about unused arguments
  printf("SI5342 page: %u\n",wib->GetDAQ_SI5342Page());
  return CommandReturn::OK;
}	   

CommandReturn::status BUTool::WIBDevice::ResetSI5342(std::vector<std::string> strArg,std::vector<uint64_t> intArg){
  (void) intArg; // to make compiler not complain about unused arguments
  (void) strArg; // to make compiler not complain about unused arguments
  wib->ResetSi5342();
  return CommandReturn::OK;
}

CommandReturn::status BUTool::WIBDevice::SelectSI5342(std::vector<std::string> /*strArg*/,std::vector<uint64_t> intArg){

  if(intArg.size() != 2) return CommandReturn::BAD_ARGS;
  if(intArg[0] > 3) return CommandReturn::BAD_ARGS;
  wib->SelectSI5342(intArg[0],bool(intArg[1]));
  
  return CommandReturn::OK;
}
