#include <wib_device/WIBDevice.hh>

CommandReturn::status BUTool::WIBDevice::EnableDAQLink(std::vector<std::string> strArg,std::vector<uint64_t> intArg){
  (void) strArg; // to make compiler not complain about unused arguments

  uint8_t iDAQLink = 0; 
  switch (intArg.size()){
  case 1:
    iDAQLink = intArg[0];
    break;
  default:
    return CommandReturn::BAD_ARGS;    
  }    
  wib->EnableDAQLink(iDAQLink);  
  return CommandReturn::OK;    
}

CommandReturn::status BUTool::WIBDevice::EnableDAQLink_Lite(std::vector<std::string> strArg,std::vector<uint64_t> intArg){
  (void) strArg; // to make compiler not complain about unused arguments

  uint8_t iDAQLink = 0; 
  uint8_t enable = 0; 
  switch (intArg.size()){
  case 2:
    iDAQLink = intArg[0];
    enable = intArg[1];
    break;
  case 1:
    iDAQLink = intArg[0];
    enable = 1;//default on
    break;
  default:
    return CommandReturn::BAD_ARGS;    
  }    
  wib->EnableDAQLink_Lite(iDAQLink,enable);  
  return CommandReturn::OK;    
}
