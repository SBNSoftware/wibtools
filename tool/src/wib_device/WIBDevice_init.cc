#include <wib_device/WIBDevice.hh>

CommandReturn::status WIBTool::WIBDevice::InitializeDTS(std::vector<std::string> /*strArg*/,std::vector<uint64_t> intArg){
  if(intArg.size() >= 3){
    wib->InitializeDTS(intArg[0],intArg[1],intArg[2]);
  }else if(intArg.size() >= 2){
    wib->InitializeDTS(intArg[0],intArg[1]);
  }else if(intArg.size() == 1){
    wib->InitializeDTS(intArg[0]);
  }else{
    wib->InitializeDTS();
  }
  return CommandReturn::OK;
}

CommandReturn::status WIBTool::WIBDevice::ResetWIB(std::vector<std::string> /*strArg*/, std::vector<uint64_t> intArg){
  
  if(intArg.size() > 1){
    return CommandReturn::BAD_ARGS;
  }
  else if(intArg.size() == 1){
    wib->ResetWIB(bool(intArg[0]));
  }
  else{
    wib->ResetWIB();
  }
  return CommandReturn::OK;
}

CommandReturn::status WIBTool::WIBDevice::ResetWIBAndCfgDTS(std::vector<std::string> /*strArg*/,std::vector<uint64_t> intArg){

  CommandReturn::status ret = CommandReturn::BAD_ARGS;
  if(intArg.size() != 0){
    if(1 == intArg[0]){
      //local clock
      wib->ResetWIBAndCfgDTS(1,0);
      ret = CommandReturn::OK;
    }else{
      if(2 == intArg.size()){
	//timing group and assumed back plane timing
	wib->ResetWIBAndCfgDTS(0,intArg[1]);
	ret = CommandReturn::OK;
      }else if (3 == intArg.size()){
	//timing group and timing
	wib->ResetWIBAndCfgDTS(0,intArg[1],intArg[2]);
	ret = CommandReturn::OK;
      }else if (intArg.size() >3){
	//timing group and timing
	wib->ResetWIBAndCfgDTS(0,intArg[1],intArg[2],intArg[3]);
	ret = CommandReturn::OK;
      }
    }
  }
  return ret;
}

CommandReturn::status WIBTool::WIBDevice::CheckedResetWIBAndCfgDTS(std::vector<std::string> /*strArg*/,std::vector<uint64_t> intArg){

  CommandReturn::status ret = CommandReturn::BAD_ARGS;
  if(intArg.size() != 0){
    if(1 == intArg[0]){
      //local clock
      wib->CheckedResetWIBAndCfgDTS(1,0);
      ret = CommandReturn::OK;
    }else{
      if(2 == intArg.size()){
	//timing group and assumed back plane timing
	wib->CheckedResetWIBAndCfgDTS(0,intArg[1]);
	ret = CommandReturn::OK;
      }else if (3 == intArg.size()){
	//timing group and timing
	wib->CheckedResetWIBAndCfgDTS(0,intArg[1],intArg[2]);
	ret = CommandReturn::OK;
      }else if (intArg.size() > 3){
	//timing group and timing
	wib->CheckedResetWIBAndCfgDTS(0,intArg[1],intArg[2],intArg[3]);
	ret = CommandReturn::OK;	
      }
    }
  }
  return ret;
}

CommandReturn::status WIBTool::WIBDevice::StartStreamToDAQ(std::vector<std::string> /*strArg*/,std::vector<uint64_t> /*intArg*/){
  
  wib->StartStreamToDAQ();
  return CommandReturn::OK;
}
